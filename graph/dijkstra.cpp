// Dijkstra's Algorithm — single-source shortest paths, non-negative weights.
// Use case: GPS routing, network packet routing, any "cheapest path" problem
//           where edge weights cannot be negative.
// Complexity: O((V + E) log V) with a binary heap.
#include <iostream>
#include <vector>
#include <queue>
#include <limits>

using namespace std;

struct Edge {
    int to;
    long long weight;
};

vector<long long> dijkstra(int n, int source, const vector<vector<Edge>>& adj) {
    const long long INF = numeric_limits<long long>::max();
    vector<long long> dist(n, INF);
    dist[source] = 0;

    // min-heap of (distance, node)
    priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<>> pq;
    pq.push({0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue; // stale entry, skip

        for (const Edge& e : adj[u]) {
            long long nd = d + e.weight;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                pq.push({nd, e.to});
            }
        }
    }
    return dist;
}

int main() {
    // Small road network demo: 0=A, 1=B, 2=C, 3=D, 4=E
    int n = 5;
    vector<vector<Edge>> adj(n);
    auto addEdge = [&](int u, int v, long long w) {
        adj[u].push_back({v, w});
        adj[v].push_back({u, w}); // undirected for this demo
    };
    addEdge(0, 1, 4);
    addEdge(0, 2, 1);
    addEdge(2, 1, 2);
    addEdge(1, 3, 1);
    addEdge(2, 3, 5);
    addEdge(3, 4, 3);

    vector<long long> dist = dijkstra(n, 0, adj);

    cout << "Shortest distances from node A (0):\n";
    vector<string> names = {"A", "B", "C", "D", "E"};
    for (int i = 0; i < n; ++i) {
        cout << "  " << names[0] << " -> " << names[i] << " = " << dist[i] << "\n";
    }
    return 0;
}
