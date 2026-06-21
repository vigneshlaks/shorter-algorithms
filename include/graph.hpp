#pragma once
#include <vector>
#include <queue>
#include <limits>
#include <functional>

namespace algo {

struct Edge { int to; long long weight; };
using Graph = std::vector<std::vector<Edge>>;

// ── Dijkstra ──────────────────────────────────────────────────────────────────
inline std::vector<long long> dijkstra(int n, int src, const Graph& adj) {
    const long long INF = std::numeric_limits<long long>::max();
    std::vector<long long> dist(n, INF);
    dist[src] = 0;
    std::priority_queue<std::pair<long long,int>,
                        std::vector<std::pair<long long,int>>,
                        std::greater<>> pq;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (const Edge& e : adj[u]) {
            long long nd = d + e.weight;
            if (nd < dist[e.to]) { dist[e.to] = nd; pq.push({nd, e.to}); }
        }
    }
    return dist;
}

// ── Bellman-Ford ──────────────────────────────────────────────────────────────
struct FlatEdge { int u, v; long long w; };

inline std::vector<long long> bellmanFord(int n, int src,
                                           const std::vector<FlatEdge>& edges) {
    const long long INF = std::numeric_limits<long long>::max();
    std::vector<long long> dist(n, INF);
    dist[src] = 0;
    for (int i = 0; i < n - 1; ++i)
        for (const auto& e : edges)
            if (dist[e.u] != INF && dist[e.u] + e.w < dist[e.v])
                dist[e.v] = dist[e.u] + e.w;
    return dist;
}

// ── BFS Shortest Path (unweighted) ────────────────────────────────────────────
inline std::vector<int> bfs(int n, int src,
                              const std::vector<std::vector<int>>& adj) {
    std::vector<int> dist(n, -1);
    std::queue<int> q;
    dist[src] = 0; q.push(src);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : adj[u])
            if (dist[v] == -1) { dist[v] = dist[u] + 1; q.push(v); }
    }
    return dist;
}

} // namespace algo
