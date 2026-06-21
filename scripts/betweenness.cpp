// Approximate Betweenness Centrality
//
// Exact betweenness is O(VE) — infeasible on 70k-node graphs.
// This uses the Brandes sampling approximation: run Dijkstra from
// k random source nodes and accumulate dependency scores. The result
// is proportional to true betweenness with high probability when k
// is large enough relative to graph diameter.
//
// For each source s, Brandes computes:
//   sigma[s][v]  = number of shortest paths from s to v
//   delta[s][v]  = dependency of s on v (fraction of s-paths through v)
//
// Betweenness[v] += sum over s of delta[s][v]
//
// Output: CSV of top-N nodes by betweenness score + geographic distribution
// Usage: ./betweenness <graph.txt> [k_sources] [top_n]

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <stack>
#include <limits>
#include <random>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <string>
#include <cmath>

using namespace std;

struct Node { double lat, lon; };
struct Edge { int to; double weight; };
struct Graph {
    vector<Node>         nodes;
    vector<vector<Edge>> adj;
};

Graph load(const string& path) {
    ifstream f(path);
    if (!f) { cerr << "Cannot open " << path << "\n"; exit(1); }
    int n; long long m; f >> n >> m;
    Graph g; g.nodes.resize(n); g.adj.resize(n);
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
    }
    return g;
}

// Single-source Brandes: accumulate betweenness contribution from source s
void brandes_source(int s, const Graph& g, vector<double>& bc) {
    int n = g.nodes.size();
    const double INF = numeric_limits<double>::infinity();

    vector<double>  dist(n, INF);
    vector<double>  sigma(n, 0);   // #shortest paths from s
    vector<double>  delta(n, 0);   // dependency
    vector<vector<int>> pred(n);   // predecessors on shortest paths
    stack<int>      order;

    dist[s] = 0; sigma[s] = 1;
    priority_queue<pair<double,int>,
                   vector<pair<double,int>>,
                   greater<pair<double,int>>> pq;
    pq.push({0, s});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        order.push(u);
        for (const Edge& e : g.adj[u]) {
            double nd = dist[u] + e.weight;
            if (nd < dist[e.to] - 1e-9) {
                dist[e.to] = nd;
                sigma[e.to] = 0;
                pred[e.to].clear();
                pq.push({nd, e.to});
            }
            if (fabs(nd - dist[e.to]) < 1e-9) {
                sigma[e.to] += sigma[u];
                pred[e.to].push_back(u);
            }
        }
    }

    // Back-propagate dependencies
    while (!order.empty()) {
        int w = order.top(); order.pop();
        for (int v : pred[w]) {
            double c = (sigma[v] / sigma[w]) * (1.0 + delta[w]);
            delta[v] += c;
        }
        if (w != s) bc[w] += delta[w];
    }
}

int main(int argc, char* argv[]) {
    string path = "data/sf_roads.txt";
    if (argc > 1) path = argv[1];
    int k     = 500;   // source samples
    int top_n = 500;   // nodes to output
    if (argc > 2) k     = stoi(argv[2]);
    if (argc > 3) top_n = stoi(argv[3]);

    cerr << "Loading " << path << "...\n";
    Graph g = load(path);
    int n = g.nodes.size();
    cerr << "  " << n << " nodes\n";

    mt19937 rng(42);
    uniform_int_distribution<int> nd(0, n-1);

    vector<double> bc(n, 0.0);

    cerr << "Running Brandes from " << k << " random sources...\n";
    for (int i = 0; i < k; ++i) {
        brandes_source(nd(rng), g, bc);
        if ((i+1) % 50 == 0)
            cerr << "  " << (i+1) << "/" << k << "\n";
    }

    // Normalize by number of sources (approximation factor)
    for (double& v : bc) v /= k;

    // Rank nodes by betweenness
    vector<int> ranked(n);
    iota(ranked.begin(), ranked.end(), 0);
    sort(ranked.begin(), ranked.end(),
         [&](int a, int b){ return bc[a] > bc[b]; });

    // Summary stats
    double total = accumulate(bc.begin(), bc.end(), 0.0);
    double top1pct_sum = 0;
    for (int i = 0; i < n/100; ++i) top1pct_sum += bc[ranked[i]];

    cerr << "\nBetweenness distribution:\n";
    cerr << "  Total score:       " << fixed << setprecision(1) << total << "\n";
    cerr << "  Top 1% nodes hold: " << 100.0*top1pct_sum/total << "% of total\n";
    cerr << "  Top node score:    " << bc[ranked[0]] << "\n";
    cerr << "  Top node location: ("
         << g.nodes[ranked[0]].lat << ", " << g.nodes[ranked[0]].lon << ")\n";

    // Geographic spread of top nodes — lat/lon std dev
    int top_k = min(top_n, n);
    vector<double> top_lats, top_lons;
    for (int i = 0; i < top_k; ++i) {
        top_lats.push_back(g.nodes[ranked[i]].lat);
        top_lons.push_back(g.nodes[ranked[i]].lon);
    }
    auto mean_v = [](const vector<double>& v){
        return accumulate(v.begin(),v.end(),0.0)/v.size(); };
    auto std_v  = [&](const vector<double>& v, double m){
        double s=0; for(double x:v) s+=(x-m)*(x-m);
        return sqrt(s/v.size()); };
    double lat_m = mean_v(top_lats), lon_m = mean_v(top_lons);
    double lat_s = std_v(top_lats, lat_m), lon_s = std_v(top_lons, lon_m);

    cerr << "\nTop " << top_k << " nodes geographic spread:\n";
    cerr << "  Centroid: (" << lat_m << ", " << lon_m << ")\n";
    cerr << "  Lat std:  " << lat_s << "°  Lon std: " << lon_s << "°\n";

    // Output CSV
    cout << "rank,node_id,betweenness,lat,lon,pct_of_total\n";
    cout << fixed << setprecision(6);
    for (int i = 0; i < top_k; ++i) {
        int v = ranked[i];
        cout << i+1 << "," << v << "," << bc[v] << ","
             << g.nodes[v].lat << "," << g.nodes[v].lon << ","
             << 100.0*bc[v]/total << "\n";
    }

    return 0;
}
