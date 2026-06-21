#pragma once
#include <vector>
#include <fstream>
#include <string>
#include <cmath>
#include <iostream>

struct Node { double lat, lon; };
struct Edge { int to; double weight; };
struct Graph {
    std::vector<Node>          nodes;
    std::vector<std::vector<Edge>> adj;
};

inline double haversine(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000.0;
    double phi1 = lat1*M_PI/180, phi2 = lat2*M_PI/180;
    double dphi = (lat2-lat1)*M_PI/180, dlam = (lon2-lon1)*M_PI/180;
    double a = sin(dphi/2)*sin(dphi/2)+cos(phi1)*cos(phi2)*sin(dlam/2)*sin(dlam/2);
    return R * 2 * atan2(sqrt(a), sqrt(1-a));
}

inline Graph loadGraph(const std::string& path) {
    std::ifstream f(path);
    if (!f) { std::cerr << "Cannot open " << path << "\n"; exit(1); }
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
