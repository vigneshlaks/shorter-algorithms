// Prim's Algorithm — Minimum Spanning Tree.
// Grows the MST one vertex at a time by always picking the cheapest edge
// that connects a new vertex to the already-built tree.
//
// Use case: network design (cheapest way to connect all nodes), cluster
//           analysis, image segmentation, approximation algorithms for TSP.
//
// Complexity: O((V + E) log V) with a binary heap.
//
// vs Kruskal (kruskal_mst.cpp):
//   Kruskal: sort ALL edges globally, add cheapest that doesn't form a cycle.
//            Better for sparse graphs (few edges). Uses Union-Find.
//   Prim's:  grow from one vertex, always extend to the nearest unvisited node.
//            Better for dense graphs (many edges). Uses a priority queue.
//   Both always produce a valid MST — different paths to the same answer.
#include <iostream>
#include <vector>
#include <queue>
#include <limits>

using namespace std;

struct Edge { int to; long long weight; };
using Graph = vector<vector<Edge>>;

struct MSTResult {
    long long totalWeight;
    vector<pair<int,int>> edges; // (from, to) pairs in the MST
};

MSTResult primsMST(int n, const Graph& adj) {
    const long long INF = numeric_limits<long long>::max();
    vector<long long> key(n, INF);    // cheapest edge connecting v to tree
    vector<int>       parent(n, -1); // which tree node v connects through
    vector<bool>      inMST(n, false);

    // Min-heap of (edge weight, vertex)
    priority_queue<pair<long long,int>,
                   vector<pair<long long,int>>,
                   greater<pair<long long,int>>> pq;

    key[0] = 0;
    pq.push({0, 0});

    long long totalWeight = 0;
    vector<pair<int,int>> mstEdges;

    while (!pq.empty()) {
        auto [w, u] = pq.top(); pq.pop();

        if (inMST[u]) continue; // already included via a cheaper edge
        inMST[u] = true;
        totalWeight += w;

        if (parent[u] != -1)
            mstEdges.push_back({parent[u], u});

        for (const Edge& e : adj[u]) {
            if (!inMST[e.to] && e.weight < key[e.to]) {
                key[e.to]    = e.weight;
                parent[e.to] = u;
                pq.push({e.weight, e.to});
            }
        }
    }

    return {totalWeight, mstEdges};
}

void addEdge(Graph& g, int u, int v, long long w) {
    g[u].push_back({v, w});
    g[v].push_back({u, w});
}

int main() {
    // Same graph used in kruskal_mst.cpp for direct comparison:
    //   0 --4-- 1 --8-- 2
    //   |       |       |
    //   8       11      7
    //   |       |       |
    //   7 --1-- 6 --2-- 5
    //       \       /
    //        4     14
    //         \   /
    //          \ /
    //           4 (node 8 not in this simplified version)
    //
    // Simple 5-node weighted graph:
    //   0 -2- 1 -3- 2
    //   |         / |
    //   6       8   5
    //   |     /     |
    //   3 -7- 4 -9- 2 (not added — let's use a cleaner example)

    int n = 6;
    Graph g(n);
    addEdge(g, 0, 1, 4);
    addEdge(g, 0, 2, 3);
    addEdge(g, 1, 2, 1);
    addEdge(g, 1, 3, 2);
    addEdge(g, 2, 3, 4);
    addEdge(g, 3, 4, 2);
    addEdge(g, 4, 5, 6);
    addEdge(g, 3, 5, 5);

    auto [totalWeight, edges] = primsMST(n, g);

    cout << "Prim's MST:\n";
    for (auto [u, v] : edges)
        cout << "  " << u << " -- " << v << "\n";
    cout << "Total MST weight: " << totalWeight << "\n";

    // Verify: an MST of n nodes always has exactly n-1 edges
    cout << "Edges in MST: " << edges.size() << " (expected " << n-1 << ")\n";

    return 0;
}
