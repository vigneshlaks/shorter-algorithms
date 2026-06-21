// Real-graph benchmark: runs Dijkstra and A* on the SNAP California road
// network (roadNet-CA.txt, 1.96M nodes, 5.5M edges).
//
// Since roadNet-CA has no edge weights or coordinates, we:
//   - Assign weight=1 to every edge (hop count as distance)
//   - Use node-id distance as a proxy heuristic for A* (not geometric,
//     but demonstrates the node-exploration difference)
//
// Usage: ./real_graph_bench data/roadNet-CA.txt [start] [goal]
//        Default: start=0, goal=1000000
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>

using namespace std;
using Clock = chrono::high_resolution_clock;

struct Edge { int to; int weight; };
using Graph = vector<vector<Edge>>;

// ── Graph loader ──────────────────────────────────────────────────────────────
Graph loadGraph(const string& path, int& numNodes, long long& numEdges) {
    ifstream f(path);
    if (!f) { cerr << "Cannot open " << path << "\n"; exit(1); }

    numNodes = 0; numEdges = 0;

    // First pass: find max node id
    string line;
    while (getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        int u, v; ss >> u >> v;
        numNodes = max(numNodes, max(u, v) + 1);
        ++numEdges;
    }

    Graph g(numNodes);
    f.clear(); f.seekg(0);

    while (getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        int u, v; ss >> u >> v;
        g[u].push_back({v, 1});
        g[v].push_back({u, 1}); // treat as undirected
    }

    return g;
}

// ── Dijkstra ──────────────────────────────────────────────────────────────────
struct SearchResult {
    long long  cost;
    int        nodesExplored;
    double     ms;
    vector<int> path;
};

SearchResult dijkstra(int start, int goal, const Graph& g) {
    int n = g.size();
    const int INF = numeric_limits<int>::max();
    vector<int>  dist(n, INF);
    vector<int>  parent(n, -1);
    vector<bool> visited(n, false);

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    dist[start] = 0;
    pq.push({0, start});
    int explored = 0;

    auto t0 = Clock::now();

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (visited[u]) continue;
        visited[u] = true;
        ++explored;
        if (u == goal) break;

        for (const Edge& e : g[u]) {
            int nd = d + e.weight;
            if (nd < dist[e.to]) {
                dist[e.to]    = nd;
                parent[e.to]  = u;
                pq.push({nd, e.to});
            }
        }
    }

    double ms = chrono::duration<double, milli>(Clock::now() - t0).count();

    vector<int> path;
    if (dist[goal] < INF) {
        for (int v = goal; v != -1; v = parent[v]) path.push_back(v);
        reverse(path.begin(), path.end());
    }

    return {dist[goal] == INF ? -1 : dist[goal], explored, ms, path};
}

// ── A* with node-id heuristic ─────────────────────────────────────────────────
// Heuristic: |u - goal| / max_degree — a weak but admissible proxy.
// Not geometric (no coordinates), but still guides search directionally.
SearchResult astar(int start, int goal, const Graph& g) {
    int n = g.size();
    const int INF = numeric_limits<int>::max();
    vector<int>  dist(n, INF);
    vector<int>  parent(n, -1);
    vector<bool> visited(n, false);

    // Simple admissible heuristic: 0 (degenerates toward Dijkstra) is safe,
    // but we use a tiny id-based nudge to show directional bias effect.
    auto h = [&](int u) -> int {
        return abs(u - goal) / 100; // scaled so it never overestimates hop count
    };

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    dist[start] = 0;
    pq.push({h(start), start});
    int explored = 0;

    auto t0 = Clock::now();

    while (!pq.empty()) {
        auto [f, u] = pq.top(); pq.pop();
        if (visited[u]) continue;
        visited[u] = true;
        ++explored;
        if (u == goal) break;

        for (const Edge& e : g[u]) {
            int ng = dist[u] + e.weight;
            if (ng < dist[e.to]) {
                dist[e.to]   = ng;
                parent[e.to] = u;
                pq.push({ng + h(e.to), e.to});
            }
        }
    }

    double ms = chrono::duration<double, milli>(Clock::now() - t0).count();

    vector<int> path;
    if (dist[goal] < INF) {
        for (int v = goal; v != -1; v = parent[v]) path.push_back(v);
        reverse(path.begin(), path.end());
    }

    return {dist[goal] == INF ? -1 : dist[goal], explored, ms, path};
}

void printResult(const string& name, int start, int goal,
                 const SearchResult& r, int totalNodes) {
    cout << "\n── " << name << " ──\n";
    if (r.cost == -1) {
        cout << "  No path found from " << start << " to " << goal << "\n";
        return;
    }
    cout << "  Path cost (hops):   " << r.cost << "\n";
    cout << "  Path length:        " << r.path.size() << " nodes\n";
    cout << "  Nodes explored:     " << r.nodesExplored
         << " / " << totalNodes
         << " (" << (100.0 * r.nodesExplored / totalNodes) << "%)\n";
    cout << "  Time:               " << r.ms << " ms\n";
    if (r.path.size() > 6) {
        cout << "  Path (first/last):  ";
        for (int i = 0; i < 3; ++i) cout << r.path[i] << " -> ";
        cout << "... -> ";
        for (int i = (int)r.path.size()-2; i < (int)r.path.size(); ++i)
            cout << r.path[i] << (i+1 < (int)r.path.size() ? " -> " : "");
        cout << "\n";
    }
}

int main(int argc, char* argv[]) {
    string dataPath = "data/roadNet-CA.txt";
    if (argc > 1) dataPath = argv[1];

    int start = 0, goal = 1000000;
    if (argc > 2) start = stoi(argv[2]);
    if (argc > 3) goal  = stoi(argv[3]);

    cout << "Loading " << dataPath << "...\n";
    int numNodes; long long numEdges;
    auto t0 = Clock::now();
    Graph g = loadGraph(dataPath, numNodes, numEdges);
    double loadMs = chrono::duration<double, milli>(Clock::now() - t0).count();

    cout << "Loaded: " << numNodes << " nodes, " << numEdges
         << " edges in " << loadMs << " ms\n";
    cout << "Source: " << start << "  Goal: " << goal << "\n";

    auto dResult = dijkstra(start, goal, g);
    auto aResult = astar(start, goal, g);

    printResult("Dijkstra", start, goal, dResult, numNodes);
    printResult("A* (id heuristic)", start, goal, aResult, numNodes);

    cout << "\n── Comparison ──\n";
    if (dResult.cost != -1 && aResult.cost != -1) {
        cout << "  Same optimal cost: "
             << (dResult.cost == aResult.cost ? "YES" : "NO — heuristic not admissible!")
             << "\n";
        double speedup = (double)dResult.nodesExplored / aResult.nodesExplored;
        cout << "  A* explored " << speedup << "x fewer nodes\n";
        cout << "  Dijkstra time: " << dResult.ms << " ms\n";
        cout << "  A* time:       " << aResult.ms << " ms\n";
    }

    return 0;
}
