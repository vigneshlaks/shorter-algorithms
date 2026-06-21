// A* vs Dijkstra on real San Francisco road network (OpenStreetMap).
// Input: sf_roads.txt — edge list with geographic coordinates.
//        Convert from OSM JSON with: python3 scripts/osm_to_edgelist.py
//
// Heuristic: Haversine distance to goal — admissible because no road path
// can be shorter than the straight-line distance between two points on Earth.
//
// Usage: ./osm_astar data/sf_roads.txt [start_idx] [goal_idx]
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>
#include <chrono>
#include <string>
#include <cmath>
#include <algorithm>

using namespace std;
using Clock = chrono::high_resolution_clock;

static double haversine(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000.0;
    double phi1 = lat1 * M_PI / 180.0, phi2 = lat2 * M_PI / 180.0;
    double dphi = (lat2-lat1) * M_PI / 180.0;
    double dlam = (lon2-lon1) * M_PI / 180.0;
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

struct Result {
    double cost;
    int    explored;
    double ms;
    vector<int> path;
};

Result runSearch(int start, int goal, const Graph& g, bool astar) {
    int n = g.nodes.size();
    const double INF = numeric_limits<double>::infinity();
    vector<double> dist(n, INF);
    vector<int>    parent(n, -1);
    vector<bool>   visited(n, false);

    auto h = [&](int u) -> double {
        if (!astar) return 0.0;
        return haversine(g.nodes[u].lat, g.nodes[u].lon,
                         g.nodes[goal].lat, g.nodes[goal].lon);
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
                dist[e.to]   = nd;
                parent[e.to] = u;
                pq.push({nd + h(e.to), e.to});
            }
        }
    }
    double ms = chrono::duration<double,milli>(Clock::now()-t0).count();

    vector<int> path;
    if (dist[goal] < INF) {
        for (int v = goal; v != -1; v = parent[v]) path.push_back(v);
        reverse(path.begin(), path.end());
    }
    return {dist[goal] < INF ? dist[goal] : -1.0, explored, ms, path};
}

int main(int argc, char* argv[]) {
    string path = "data/sf_roads.txt";
    if (argc > 1) path = argv[1];

    cout << "Loading " << path << "...\n";
    auto t0 = Clock::now();
    Graph g = load(path);
    double loadMs = chrono::duration<double,milli>(Clock::now()-t0).count();
    int n = g.nodes.size();
    cout << "Loaded " << n << " nodes in " << loadMs << " ms\n\n";

    int start = 0, goal = n * 3 / 4;
    if (argc > 2) start = min(stoi(argv[2]), n-1);
    if (argc > 3) goal  = min(stoi(argv[3]), n-1);

    double sl = haversine(g.nodes[start].lat, g.nodes[start].lon,
                          g.nodes[goal].lat,  g.nodes[goal].lon);

    cout << "Start: " << start << " (" << g.nodes[start].lat
         << ", " << g.nodes[start].lon << ")\n";
    cout << "Goal:  " << goal  << " (" << g.nodes[goal].lat
         << ", " << g.nodes[goal].lon  << ")\n";
    cout << "Straight-line: " << sl/1000.0 << " km\n\n";

    Result dr = runSearch(start, goal, g, false);
    Result ar = runSearch(start, goal, g, true);

    auto print = [&](const string& name, const Result& r) {
        cout << "── " << name << " ──\n";
        if (r.cost < 0) { cout << "  No path found\n\n"; return; }
        cout << "  Road distance:  " << r.cost/1000.0 << " km\n";
        cout << "  Path nodes:     " << r.path.size() << "\n";
        cout << "  Nodes explored: " << r.explored << " / " << n
             << " (" << 100.0*r.explored/n << "%)\n";
        cout << "  Time:           " << r.ms << " ms\n\n";
    };

    print("Dijkstra (h=0, explores everything)", dr);
    print("A* (Haversine heuristic)", ar);

    if (dr.cost > 0 && ar.cost > 0) {
        bool optimal = fabs(dr.cost - ar.cost) < 0.1;
        cout << "── Results ──\n";
        cout << "  Optimal path match: " << (optimal ? "YES" : "NO") << "\n";
        cout << "  Detour ratio: " << dr.cost/sl << "x straight-line\n";
        if (optimal) {
            cout << "  Nodes saved: " << dr.explored - ar.explored
                 << " (" << (double)dr.explored/ar.explored << "x fewer)\n";
            cout << "  Time speedup: " << dr.ms/ar.ms << "x\n";
        }
    }
    return 0;
}
