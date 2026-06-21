// ε-Near-Optimal Subgraph Analysis
//
// For a source-goal pair with optimal cost C*, an edge (u,v) with weight w
// lies on a near-optimal path iff:
//
//   dist_forward[u] + w + dist_backward[v] <= C* * (1 + ε)
//
// where dist_forward is Dijkstra from source and dist_backward is Dijkstra
// from goal on the reversed graph.
//
// Subgraph density = (edges satisfying the above) / (total edges)
//
// High density means many parallel routes exist — A* has more room to deviate
// and still find a path close to optimal. Low density means the optimal path
// is structurally unique — deviations are expensive.
//
// Hypothesis: Manhattan (grid) has higher subgraph density than SF (irregular),
// which explains why Manhattan A* detours are larger.
//
// Usage: ./near_optimal_subgraph <graph.txt> [n_queries] [epsilon]
//   Output: CSV with per-query density + summary comparison across ε values

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>
#include <random>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <string>
#include <algorithm>

using namespace std;
using PD = pair<double,int>;

struct Node { double lat, lon; };
struct Edge { int to; double weight; };
struct Graph {
    vector<Node>         nodes;
    vector<vector<Edge>> adj;
    vector<vector<Edge>> radj; // reversed edges for backward Dijkstra
    long long            m;
};

Graph load(const string& path) {
    ifstream f(path);
    if (!f) { cerr << "Cannot open " << path << "\n"; exit(1); }
    int n; long long m; f >> n >> m;
    Graph g;
    g.nodes.resize(n); g.adj.resize(n); g.radj.resize(n); g.m = m;
    for (int i = 0; i < n; ++i) {
        int idx; double la, lo;
        f >> idx >> la >> lo;
        g.nodes[idx] = {la, lo};
    }
    for (long long i = 0; i < m; ++i) {
        int a, b; double d;
        f >> a >> b >> d;
        g.adj[a].push_back({b, d});
        g.adj[b].push_back({a, d});
        g.radj[b].push_back({a, d}); // reverse: b->a
        g.radj[a].push_back({b, d}); // reverse: a->b
    }
    return g;
}

vector<double> dijkstra(int src, const vector<vector<Edge>>& adj) {
    int n = adj.size();
    const double INF = numeric_limits<double>::infinity();
    vector<double> dist(n, INF);
    priority_queue<PD, vector<PD>, greater<PD>> pq;
    dist[src] = 0;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (const Edge& e : adj[u]) {
            double nd = dist[u] + e.weight;
            if (nd < dist[e.to]) { dist[e.to] = nd; pq.push({nd, e.to}); }
        }
    }
    return dist;
}

struct SubgraphResult {
    double optimal_cost;
    // density at each epsilon
    vector<double> densities; // indexed by epsilon tier
    int total_edges;
    int reachable_edges; // edges where both endpoints reachable
};

SubgraphResult analyze(int src, int goal, const Graph& g,
                       const vector<double>& epsilons) {
    const double INF = numeric_limits<double>::infinity();

    auto df = dijkstra(src,  g.adj);   // forward from source
    auto db = dijkstra(goal, g.radj);  // backward from goal (on reversed graph)

    double C = df[goal];
    if (C >= INF) return {-1, vector<double>(epsilons.size(), 0), 0, 0};

    // Count edges in ε-near-optimal subgraph for each ε
    // An edge (u,v) qualifies if df[u] + w(u,v) + db[v] <= C*(1+ε)
    // We count each undirected edge once (check both directions, take min)
    vector<int> counts(epsilons.size(), 0);
    int total = 0, reachable = 0;

    for (int u = 0; u < (int)g.nodes.size(); ++u) {
        for (const Edge& e : g.adj[u]) {
            if (e.to <= u) continue; // count each undirected edge once
            ++total;
            if (df[u] >= INF || db[e.to] >= INF) continue;
            ++reachable;
            // path cost through this edge: source->u->v->goal
            double through_uv = df[u] + e.weight + db[e.to];
            // also check the reverse direction: source->v->u->goal
            double through_vu = (df[e.to] < INF && db[u] < INF)
                                ? df[e.to] + e.weight + db[u]
                                : INF;
            double best_through = min(through_uv, through_vu);

            for (int ei = 0; ei < (int)epsilons.size(); ++ei) {
                if (best_through <= C * (1.0 + epsilons[ei]))
                    ++counts[ei];
            }
        }
    }

    vector<double> densities(epsilons.size());
    for (int ei = 0; ei < (int)epsilons.size(); ++ei)
        densities[ei] = (total > 0) ? 100.0 * counts[ei] / total : 0;

    return {C, densities, total, reachable};
}

int main(int argc, char* argv[]) {
    string path = "data/sf_roads.txt";
    if (argc > 1) path = argv[1];
    int nQueries = 100;
    if (argc > 2) nQueries = stoi(argv[2]);

    cerr << "Loading " << path << "...\n";
    Graph g = load(path);
    int n = g.nodes.size();
    cerr << "  " << n << " nodes, " << g.m << " edges\n";

    // ε values: how far above optimal we allow
    vector<double> epsilons = {0.01, 0.05, 0.10, 0.20, 0.50};

    // Sample random pairs (require both reachable)
    mt19937 rng(42);
    uniform_int_distribution<int> nd(0, n-1);

    cerr << "Running " << nQueries << " queries...\n\n";

    // CSV header
    cout << "query,src,goal,optimal_cost_m,total_edges,reachable_edges";
    for (double eps : epsilons)
        cout << ",density_eps" << (int)(eps*100) << "pct";
    cout << "\n";
    cout << fixed << setprecision(4);

    vector<vector<double>> all_densities(epsilons.size());
    vector<double> all_costs;
    int valid = 0;

    for (int q = 0; q < nQueries; ++q) {
        int src = nd(rng), goal = nd(rng);
        while (src == goal) goal = nd(rng);

        auto r = analyze(src, goal, g, epsilons);
        if (r.optimal_cost < 0) continue;

        ++valid;
        all_costs.push_back(r.optimal_cost);
        cout << q << "," << src << "," << goal << ","
             << r.optimal_cost << "," << r.total_edges << ","
             << r.reachable_edges;
        for (int ei = 0; ei < (int)epsilons.size(); ++ei) {
            cout << "," << r.densities[ei];
            all_densities[ei].push_back(r.densities[ei]);
        }
        cout << "\n";
        cout.flush();

        if ((q+1) % 10 == 0)
            cerr << "  " << (q+1) << "/" << nQueries << " done\n";
    }

    cerr << "\nSummary (" << valid << " valid queries):\n";
    cerr << fixed << setprecision(3);
    cerr << "  Mean optimal path: "
         << accumulate(all_costs.begin(), all_costs.end(), 0.0)/all_costs.size()/1000
         << " km\n";
    cerr << "\n  ε        mean density    std density\n";
    cerr << "  -------  ------------    -----------\n";
    for (int ei = 0; ei < (int)epsilons.size(); ++ei) {
        auto& v = all_densities[ei];
        double mean = accumulate(v.begin(), v.end(), 0.0) / v.size();
        double var  = 0;
        for (double x : v) var += (x-mean)*(x-mean);
        double sd = sqrt(var/v.size());
        cerr << "  " << setw(6) << (epsilons[ei]*100) << "%  "
             << setw(12) << mean << "%  "
             << setw(11) << sd << "%\n";
    }

    return 0;
}
