// Suboptimality Cliff Investigation
//
// For each query where A* at α=1.05 produces a suboptimal path, this tool:
//   1. Finds the exact node where the A* path diverges from the optimal path
//   2. Records the divergence node's lat/lon
//   3. Measures how far into the path the divergence occurs
//   4. Computes the cost of the detour at the divergence point
//
// If divergence points cluster geographically, that reveals structural
// bottlenecks in the road network that the heuristic overshoot causes A*
// to route around incorrectly.
//
// Output: CSV of divergence points for plotting on a map
// Usage: ./divergence_analysis data/sf_roads.txt [n_queries] [alpha]
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>
#include <chrono>
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <string>
#include <set>

using namespace std;

static double haversine(double la1, double lo1, double la2, double lo2) {
    const double R = 6371000.0;
    double p1=la1*M_PI/180, p2=la2*M_PI/180;
    double dp=(la2-la1)*M_PI/180, dl=(lo2-lo1)*M_PI/180;
    double a=sin(dp/2)*sin(dp/2)+cos(p1)*cos(p2)*sin(dl/2)*sin(dl/2);
    return R*2*atan2(sqrt(a),sqrt(1-a));
}

struct Node { double lat, lon; };
struct Edge { int to; double weight; };
struct Graph { vector<Node> nodes; vector<vector<Edge>> adj; };

Graph load(const string& path) {
    ifstream f(path);
    int n; long long m; f >> n >> m;
    Graph g; g.nodes.resize(n); g.adj.resize(n);
    for (int i=0;i<n;++i){int idx;double la,lo;f>>idx>>la>>lo;g.nodes[idx]={la,lo};}
    for (long long i=0;i<m;++i){int a,b;double d;f>>a>>b>>d;
        g.adj[a].push_back({b,d});g.adj[b].push_back({a,d});}
    return g;
}

struct PathResult { double cost; vector<int> path; };

PathResult search(int s, int g, const Graph& gr, double alpha) {
    int n=gr.nodes.size();
    const double INF=numeric_limits<double>::infinity();
    vector<double> dist(n,INF); vector<int> par(n,-1); vector<bool> vis(n,false);
    double glat=gr.nodes[g].lat, glon=gr.nodes[g].lon;
    auto h=[&](int u){return alpha*haversine(gr.nodes[u].lat,gr.nodes[u].lon,glat,glon);};
    priority_queue<pair<double,int>,vector<pair<double,int>>,greater<pair<double,int>>> pq;
    dist[s]=0; pq.push({h(s),s});
    while(!pq.empty()){
        auto [f,u]=pq.top();pq.pop();
        if(vis[u])continue;vis[u]=true;if(u==g)break;
        for(const Edge& e:gr.adj[u]){double nd=dist[u]+e.weight;
            if(nd<dist[e.to]){dist[e.to]=nd;par[e.to]=u;pq.push({nd+h(e.to),e.to});}}
    }
    vector<int> path;
    if(dist[g]<INF){for(int v=g;v!=-1;v=par[v])path.push_back(v);reverse(path.begin(),path.end());}
    return {dist[g]<INF?dist[g]:-1.0,path};
}

struct Divergence {
    int    queryId;
    double costRatio;          // astar_cost / optimal_cost
    double detourMetres;       // extra distance
    int    divergeNode;        // first node where paths differ
    double divergeLat, divergeLon;
    double progressFraction;   // how far into path before divergence (0=start, 1=end)
    double straightLineKm;     // start-to-goal straight line
};

int main(int argc, char* argv[]) {
    string path = "data/sf_roads.txt";
    if (argc > 1) path = argv[1];
    int nQueries = 500;
    if (argc > 2) nQueries = stoi(argv[2]);
    double alpha = 1.05;
    if (argc > 3) alpha = stod(argv[3]);

    cerr << "Loading graph...\n";
    Graph g = load(path);
    int n = g.nodes.size();
    cerr << "Loaded " << n << " nodes\n";

    // Sample random pairs with minimum path length
    mt19937 rng(42);
    uniform_int_distribution<int> nd(0, n-1);

    vector<Divergence> divergences;
    int totalSubopt = 0, totalNoPath = 0, totalChecked = 0;

    cerr << "Analyzing " << nQueries << " queries at α=" << alpha << "...\n";

    for (int q = 0; q < nQueries; ++q) {
        int s = nd(rng), gl = nd(rng);
        if (s == gl) continue;

        auto opt  = search(s, gl, g, 0.0);   // Dijkstra
        auto fast = search(s, gl, g, alpha);  // A* with test α

        if (opt.cost < 0 || fast.cost < 0) { ++totalNoPath; continue; }
        ++totalChecked;

        double ratio = fast.cost / opt.cost;
        if (ratio <= 1.0001) continue; // optimal — skip
        ++totalSubopt;

        // Find first divergence point between the two paths
        set<int> optSet(opt.path.begin(), opt.path.end());
        int divergeNode = -1;
        int divergeIdx  = 0;

        for (int i = 0; i < (int)fast.path.size(); ++i) {
            if (optSet.find(fast.path[i]) == optSet.end()) {
                divergeNode = fast.path[i];
                divergeIdx  = i;
                break;
            }
        }

        if (divergeNode == -1) continue; // paths share all nodes somehow

        double sl = haversine(g.nodes[s].lat, g.nodes[s].lon,
                              g.nodes[gl].lat, g.nodes[gl].lon);

        divergences.push_back({
            q,
            ratio,
            fast.cost - opt.cost,
            divergeNode,
            g.nodes[divergeNode].lat,
            g.nodes[divergeNode].lon,
            (double)divergeIdx / fast.path.size(),
            sl / 1000.0
        });
    }

    cerr << "\nResults:\n";
    cerr << "  Queries checked:  " << totalChecked << "\n";
    cerr << "  Suboptimal:       " << totalSubopt
         << " (" << 100.0*totalSubopt/totalChecked << "%)\n";
    cerr << "  No path:          " << totalNoPath << "\n";
    cerr << "  Divergences found:" << divergences.size() << "\n";

    if (!divergences.empty()) {
        vector<double> ratios, detours, progress;
        for (const auto& d : divergences) {
            ratios.push_back(d.costRatio);
            detours.push_back(d.detourMetres);
            progress.push_back(d.progressFraction);
        }
        auto mean = [](const vector<double>& v) {
            return accumulate(v.begin(),v.end(),0.0)/v.size();
        };
        cerr << "  Mean cost ratio:  " << mean(ratios) << "\n";
        cerr << "  Mean detour:      " << mean(detours) << " m\n";
        cerr << "  Mean divergence:  " << mean(progress)*100 << "% into path\n";
    }

    // Output CSV for map visualization
    cout << "query_id,cost_ratio,detour_m,diverge_lat,diverge_lon,"
         << "progress_fraction,straight_line_km\n";
    cout << fixed << setprecision(6);
    for (const auto& d : divergences) {
        cout << d.queryId << ","
             << d.costRatio << ","
             << d.detourMetres << ","
             << d.divergeLat << ","
             << d.divergeLon << ","
             << d.progressFraction << ","
             << d.straightLineKm << "\n";
    }

    // Write summary JSON
    if (!divergences.empty()) {
        ofstream jf("results/divergence_analysis.json");
        jf << fixed << setprecision(6);
        jf << "{\n"
           << "  \"alpha\": " << alpha << ",\n"
           << "  \"n_queries\": " << totalChecked << ",\n"
           << "  \"n_suboptimal\": " << totalSubopt << ",\n"
           << "  \"suboptimal_rate\": " << 100.0*totalSubopt/totalChecked << ",\n"
           << "  \"n_divergences\": " << divergences.size() << ",\n"
           << "  \"divergence_points\": [\n";
        for (size_t i = 0; i < divergences.size(); ++i) {
            const auto& d = divergences[i];
            jf << "    {\"lat\":" << d.divergeLat
               << ",\"lon\":" << d.divergeLon
               << ",\"cost_ratio\":" << d.costRatio
               << ",\"detour_m\":" << d.detourMetres
               << ",\"progress\":" << d.progressFraction << "}";
            if (i+1 < divergences.size()) jf << ",";
            jf << "\n";
        }
        jf << "  ]\n}\n";
        cerr << "\nDivergence map data: results/divergence_analysis.json\n";
    }

    return 0;
}
