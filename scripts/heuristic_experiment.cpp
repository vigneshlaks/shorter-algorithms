// Heuristic Quality Experiment — systematic study of how A* performance
// and optimality degrade as the heuristic scaling factor α varies.
//
// Heuristic: h_α(n) = α × haversine(n, goal)
//   α = 0.0  → Dijkstra (no heuristic)
//   α = 1.0  → optimal A* (admissible Haversine)
//   α > 1.0  → inadmissible (faster but potentially suboptimal)
//
// For each α, runs N_QUERIES random start/goal pairs and records:
//   - mean nodes explored
//   - mean path cost ratio vs optimal (Dijkstra answer)
//   - mean time (ms)
//   - fraction of queries returning optimal path
//
// Output: CSV to stdout, pipe to scripts/plot_experiment.py for visualization.
// Usage: ./heuristic_experiment data/sf_roads.txt [n_queries] > results.csv
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>
#include <chrono>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <string>

using namespace std;
using Clock = chrono::high_resolution_clock;

static double haversine(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000.0;
    double phi1 = lat1*M_PI/180, phi2 = lat2*M_PI/180;
    double dphi = (lat2-lat1)*M_PI/180, dlam = (lon2-lon1)*M_PI/180;
    double a = sin(dphi/2)*sin(dphi/2) + cos(phi1)*cos(phi2)*sin(dlam/2)*sin(dlam/2);
    return R * 2 * atan2(sqrt(a), sqrt(1-a));
}

struct Node { double lat, lon; };
struct Edge { int to; double weight; };

struct Graph {
    vector<Node>         nodes;
    vector<vector<Edge>> adj;
};

Graph load(const string& path) {
    ifstream f(path);
    if (!f) { cerr << "Cannot open " << path << "\n"; exit(1); }
    int n; long long m;
    f >> n >> m;
    Graph g;
    g.nodes.resize(n);
    g.adj.resize(n);
    for (int i = 0; i < n; ++i) {
        int idx; double lat, lon;
        f >> idx >> lat >> lon;
        g.nodes[idx] = {lat, lon};
    }
    for (long long i = 0; i < m; ++i) {
        int a, b; double d;
        f >> a >> b >> d;
        g.adj[a].push_back({b, d});
        g.adj[b].push_back({a, d});
    }
    return g;
}

struct QueryResult {
    double cost;       // -1 if no path
    int    explored;
    double ms;
};

QueryResult runAstar(int start, int goal, const Graph& g, double alpha) {
    int n = g.nodes.size();
    const double INF = numeric_limits<double>::infinity();
    vector<double> dist(n, INF);
    vector<bool>   visited(n, false);

    double goalLat = g.nodes[goal].lat, goalLon = g.nodes[goal].lon;
    auto h = [&](int u) {
        return alpha * haversine(g.nodes[u].lat, g.nodes[u].lon, goalLat, goalLon);
    };

    priority_queue<pair<double,int>,
                   vector<pair<double,int>>,
                   greater<pair<double,int>>> pq;
    dist[start] = 0;
    pq.push({h(start), start});
    int explored = 0;

    auto t0 = Clock::now();
    while (!pq.empty()) {
        auto top = pq.top(); pq.pop();
        int u = top.second;
        if (visited[u]) continue;
        visited[u] = true;
        ++explored;
        if (u == goal) break;
        for (const Edge& e : g.adj[u]) {
            double nd = dist[u] + e.weight;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                pq.push({nd + h(e.to), e.to});
            }
        }
    }
    double ms = chrono::duration<double,milli>(Clock::now()-t0).count();
    return {dist[goal] < INF ? dist[goal] : -1.0, explored, ms};
}

// Sample random connected pairs by doing a small BFS from start
// and picking a goal that is reachable and at least minHops away.
vector<pair<int,int>> samplePairs(const Graph& g, int nPairs, int minHops,
                                   int seed = 42) {
    mt19937 rng(seed);
    int n = g.nodes.size();
    uniform_int_distribution<int> dist(0, n-1);
    vector<pair<int,int>> pairs;

    while ((int)pairs.size() < nPairs) {
        int start = dist(rng);

        // BFS to find reachable nodes at depth >= minHops
        vector<int> depth(n, -1);
        queue<int> q;
        depth[start] = 0;
        q.push(start);
        vector<int> candidates;

        while (!q.empty()) {
            int u = q.front(); q.pop();
            if (depth[u] >= minHops) candidates.push_back(u);
            if (depth[u] >= minHops * 3) continue; // don't explore forever
            for (const Edge& e : g.adj[u])
                if (depth[e.to] == -1) {
                    depth[e.to] = depth[u] + 1;
                    q.push(e.to);
                }
        }

        if (candidates.empty()) continue;
        uniform_int_distribution<int> cd(0, candidates.size()-1);
        int goal = candidates[cd(rng)];
        pairs.push_back({start, goal});
    }
    return pairs;
}

int main(int argc, char* argv[]) {
    string path = "data/sf_roads.txt";
    if (argc > 1) path = argv[1];
    int nQueries = 100;
    if (argc > 2) nQueries = stoi(argv[2]);

    cerr << "Loading graph...\n";
    Graph g = load(path);
    int n = g.nodes.size();
    cerr << "Loaded " << n << " nodes\n";

    cerr << "Sampling " << nQueries << " query pairs...\n";
    auto pairs = samplePairs(g, nQueries, 50);
    cerr << "Sampled " << pairs.size() << " pairs\n\n";

    // α values: dense near 0 and 1, sparser at higher values
    vector<double> alphas;
    for (double a = 0.0; a <= 0.9; a += 0.1)  alphas.push_back(a);
    for (double a = 1.0; a <= 2.0; a += 0.05) alphas.push_back(a);
    for (double a = 2.0; a <= 4.0; a += 0.25) alphas.push_back(a);

    // Get optimal costs from Dijkstra (α=0) for all pairs
    cerr << "Computing optimal costs (Dijkstra)...\n";
    vector<double> optimalCosts(nQueries);
    for (int i = 0; i < nQueries; ++i) {
        auto [s, gl] = pairs[i];
        optimalCosts[i] = runAstar(s, gl, g, 0.0).cost;
    }

    // Print CSV header
    cout << "alpha,mean_nodes,std_nodes,mean_cost_ratio,std_cost_ratio,"
         << "mean_time_ms,std_time_ms,pct_optimal,pct_no_path\n";
    cout << fixed << setprecision(6);

    for (double alpha : alphas) {
        vector<double> nodes, ratios, times;
        int noPath = 0, isOptimal = 0;

        for (int i = 0; i < nQueries; ++i) {
            auto [s, gl] = pairs[i];
            auto r = runAstar(s, gl, g, alpha);

            if (r.cost < 0) { ++noPath; continue; }

            nodes.push_back(r.explored);
            times.push_back(r.ms);

            double ratio = (optimalCosts[i] > 0)
                           ? r.cost / optimalCosts[i]
                           : 1.0;
            ratios.push_back(ratio);
            if (fabs(ratio - 1.0) < 1e-6) ++isOptimal;
        }

        int valid = nodes.size();
        if (valid == 0) continue;

        auto mean = [](const vector<double>& v) {
            return accumulate(v.begin(), v.end(), 0.0) / v.size();
        };
        auto stddev = [&](const vector<double>& v, double m) {
            double s = 0;
            for (double x : v) s += (x-m)*(x-m);
            return sqrt(s / v.size());
        };

        double mn = mean(nodes), mr = mean(ratios), mt = mean(times);
        double sn = stddev(nodes, mn), sr = stddev(ratios, mr), st = stddev(times, mt);

        cout << alpha << ","
             << mn << "," << sn << ","
             << mr << "," << sr << ","
             << mt << "," << st << ","
             << 100.0*isOptimal/valid << ","
             << 100.0*noPath/nQueries << "\n";
        cout.flush();

        cerr << "α=" << setw(5) << alpha
             << "  nodes=" << setw(8) << (int)mn
             << "  ratio=" << setprecision(4) << mr
             << "  optimal=" << 100*isOptimal/valid << "%"
             << "  time=" << setprecision(3) << mt << "ms\n";
    }

    return 0;
}
