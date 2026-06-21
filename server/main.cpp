// Route Planning Server
//
// Reads the SF road graph at startup, then serves route requests from stdin.
// Each request is a line: "<start> <goal>"
// Each response is a line of JSON with cost, path length, latency, α used,
// cache status, and nodes explored.
//
// Components:
//   Router       — A* with configurable α (server/router.hpp)
//   RouteBloom   — Bloom filter for fast "not in cache" check (server/cache.hpp)
//   RouteCache   — Exact LRU-style store for computed routes (server/cache.hpp)
//   AlphaSelector— Per-region adaptive α based on observed quality (server/alpha_selector.hpp)
//
// Usage:
//   ./route_server data/sf_roads.txt
//   echo "0 54697" | ./route_server data/sf_roads.txt
//   python3 scripts/gen_queries.py 1000 | ./route_server data/sf_roads.txt
//
// The server periodically prints stats to stderr so you can watch the
// α selector adapt and see cache hit rates evolve over time.
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <random>
#include <iomanip>
#include <atomic>
#include <thread>

#include "graph.hpp"
#include "router.hpp"
#include "cache.hpp"
#include "alpha_selector.hpp"

using namespace std;
using Clock = chrono::high_resolution_clock;

// ── Stats tracker ─────────────────────────────────────────────────────────────
struct Stats {
    atomic<long long> totalQueries{0};
    atomic<long long> cacheHits{0};
    atomic<long long> bloomFP{0};      // bloom said "maybe" but cache missed
    atomic<long long> totalNodes{0};
    atomic<long long> totalTimeUs{0};  // microseconds
    atomic<long long> verifyCount{0};  // queries verified against Dijkstra
    atomic<long long> suboptCount{0};  // queries where A* was suboptimal

    void print(int graphNodes) const {
        long long q = totalQueries.load();
        if (q == 0) return;
        double hitRate  = 100.0 * cacheHits / q;
        double meanNodes = (double)totalNodes / (q - cacheHits);
        double meanMs    = (double)totalTimeUs / (q - cacheHits) / 1000.0;
        long long v = verifyCount.load();
        double suboptRate = v > 0 ? 100.0 * suboptCount / v : 0;

        cerr << "\n── Stats (" << q << " queries) ──\n"
             << "  Cache hit rate:    " << fixed << setprecision(1) << hitRate << "%\n"
             << "  Bloom false pos:   " << bloomFP.load() << "\n"
             << "  Mean nodes/query:  " << (int)meanNodes
             << " / " << graphNodes
             << " (" << 100.0*meanNodes/graphNodes << "%)\n"
             << "  Mean latency:      " << setprecision(3) << meanMs << " ms\n"
             << "  Suboptimal (sampl):" << setprecision(1) << suboptRate
             << "% of " << v << " verified\n";
    }
};

// ── Verification: occasionally check A* result against Dijkstra ───────────────
// Returns cost ratio (1.0 = optimal). Only called on ~5% of queries.
double verify(int s, int g, double astarCost, const Graph& graph) {
    auto opt = route(s, g, graph, 0.0); // Dijkstra
    if (opt.cost <= 0) return 1.0;
    return astarCost / opt.cost;
}

int main(int argc, char* argv[]) {
    string graphPath = "data/sf_roads.txt";
    if (argc > 1) graphPath = argv[1];

    cerr << "Loading graph from " << graphPath << "...\n";
    auto t0 = Clock::now();
    Graph g = loadGraph(graphPath);
    double loadMs = chrono::duration<double,milli>(Clock::now()-t0).count();
    int n = g.nodes.size();
    cerr << "Ready: " << n << " nodes loaded in " << loadMs << " ms\n\n";

    // Find graph bounding box for the α selector grid
    double latMin=90, latMax=-90, lonMin=180, lonMax=-180;
    for (const auto& node : g.nodes) {
        latMin = min(latMin, node.lat); latMax = max(latMax, node.lat);
        lonMin = min(lonMin, node.lon); lonMax = max(lonMax, node.lon);
    }

    RouteBloom   bloom;
    RouteCache   cache;
    AlphaSelector selector(latMin, latMax, lonMin, lonMax, 10, 10, 0.02);
    Stats        stats;

    mt19937 rng(42);
    uniform_real_distribution<double> verifyDist(0.0, 1.0);

    cerr << "Ready. Enter queries as: <start_node> <goal_node>\n";
    cerr << "Type 'stats' to print stats, 'quit' to exit.\n\n";

    auto printStats = [&]() {
        stats.print(n);
        selector.printStats();
        cerr << "  Cache size: " << cache.size() << " routes\n";
        cerr << "  Bloom FPR (est): "
             << bloom.expectedFPR(cache.size()) * 100 << "%\n\n";
    };

    string line;
    int queryNum = 0;

    while (getline(cin, line)) {
        if (line == "quit") break;
        if (line == "stats") { printStats(); continue; }
        if (line.empty()) continue;

        istringstream ss(line);
        int start, goal;
        if (!(ss >> start >> goal)) {
            cerr << "Bad input: " << line << "\n"; continue;
        }
        if (start < 0 || start >= n || goal < 0 || goal >= n) {
            cerr << "Node out of range [0, " << n-1 << "]\n"; continue;
        }

        ++queryNum;
        stats.totalQueries++;

        // ── Cache check ───────────────────────────────────────────────────────
        bool bloomHit = bloom.mightContain(start, goal);
        CachedRoute cached;
        if (bloomHit) {
            if (cache.get(start, goal, cached)) {
                stats.cacheHits++;
                cout << "{\"start\":" << start << ",\"goal\":" << goal
                     << ",\"cost_km\":" << fixed << setprecision(3)
                     << cached.cost/1000.0
                     << ",\"path_len\":" << cached.pathLen
                     << ",\"alpha\":" << cached.alphaUsed
                     << ",\"cache\":\"hit\""
                     << ",\"nodes\":0,\"ms\":0.0}\n";
                cout.flush();
                if (queryNum % 50 == 0) printStats();
                continue;
            }
            stats.bloomFP++; // bloom said yes, cache said no
        }

        // ── Route computation ─────────────────────────────────────────────────
        double alpha = selector.selectAlpha(g.nodes[start].lat,
                                            g.nodes[start].lon);

        auto t1 = Clock::now();
        RouteResult result = route(start, goal, g, alpha);
        long long us = chrono::duration_cast<chrono::microseconds>(
            Clock::now()-t1).count();

        stats.totalNodes += result.nodesExplored;
        stats.totalTimeUs += us;

        // ── Verification (5% sample) ──────────────────────────────────────────
        double costRatio = 1.0;
        bool verified = false;
        if (result.cost > 0 && verifyDist(rng) < 0.05) {
            costRatio = verify(start, goal, result.cost, g);
            stats.verifyCount++;
            if (costRatio > 1.001) stats.suboptCount++;
            selector.observe(g.nodes[start].lat, g.nodes[start].lon, costRatio);
            verified = true;
        }

        // ── Cache store ───────────────────────────────────────────────────────
        if (result.cost > 0) {
            bloom.add(start, goal);
            cache.put(start, goal, {result.cost, alpha, (int)result.path.size()});
        }

        // ── Response ──────────────────────────────────────────────────────────
        cout << "{\"start\":" << start
             << ",\"goal\":" << goal
             << ",\"cost_km\":" << setprecision(3) << result.cost/1000.0
             << ",\"path_len\":" << result.path.size()
             << ",\"alpha\":" << setprecision(2) << alpha
             << ",\"nodes\":" << result.nodesExplored
             << ",\"ms\":" << setprecision(3) << us/1000.0
             << ",\"cache\":\"miss\""
             << (verified ? string(",\"verified\":true,\"cost_ratio\":") +
                            to_string(costRatio) : "")
             << "}\n";
        cout.flush();

        if (queryNum % 50 == 0) printStats();
    }

    cerr << "\nFinal stats:\n";
    printStats();
    return 0;
}
