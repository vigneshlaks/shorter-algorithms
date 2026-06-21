#pragma once
#include "graph.hpp"
#include <vector>
#include <queue>
#include <limits>
#include <chrono>
#include <algorithm>

struct RouteResult {
    double      cost;          // metres, -1 if no path
    int         nodesExplored;
    double      ms;
    double      alphaUsed;
    std::vector<int> path;
};

inline RouteResult route(int start, int goal, const Graph& g, double alpha) {
    int n = g.nodes.size();
    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> dist(n, INF);
    std::vector<int>    parent(n, -1);
    std::vector<bool>   visited(n, false);

    double goalLat = g.nodes[goal].lat, goalLon = g.nodes[goal].lon;
    auto h = [&](int u) {
        return alpha * haversine(g.nodes[u].lat, g.nodes[u].lon, goalLat, goalLon);
    };

    std::priority_queue<std::pair<double,int>,
                        std::vector<std::pair<double,int>>,
                        std::greater<std::pair<double,int>>> pq;
    dist[start] = 0;
    pq.push({h(start), start});
    int explored = 0;

    auto t0 = std::chrono::high_resolution_clock::now();
    while (!pq.empty()) {
        auto top = pq.top(); pq.pop();
        int u = top.second;
        if (visited[u]) continue;
        visited[u] = true; ++explored;
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
    double ms = std::chrono::duration<double,std::milli>(
        std::chrono::high_resolution_clock::now()-t0).count();

    std::vector<int> path;
    if (dist[goal] < INF) {
        for (int v = goal; v != -1; v = parent[v]) path.push_back(v);
        std::reverse(path.begin(), path.end());
    }
    return {dist[goal] < INF ? dist[goal] : -1.0, explored, ms, alpha, path};
}
