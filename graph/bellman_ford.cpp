// Bellman-Ford Algorithm — single-source shortest paths, handles negative weights.
// Use case: currency arbitrage detection (negative cycle = profitable loop),
//           distance-vector routing protocols like RIP.
// Complexity: O(V * E)
#include <iostream>
#include <vector>
#include <limits>

using namespace std;

struct Edge { int from, to; long long weight; };

int main() {
    int n = 5; // nodes 0..4
    vector<Edge> edges = {
        {0, 1, 6}, {0, 2, 7}, {1, 2, 8}, {1, 3, 5}, {1, 4, -4},
        {2, 3, -3}, {2, 4, 9}, {3, 1, -2}, {4, 0, 2}, {4, 3, 7}
    };

    const long long INF = numeric_limits<long long>::max() / 2;
    vector<long long> dist(n, INF);
    int source = 0;
    dist[source] = 0;

    bool negativeCycle = false;
    for (int i = 0; i < n - 1; ++i) {
        for (const Edge& e : edges) {
            if (dist[e.from] != INF && dist[e.from] + e.weight < dist[e.to]) {
                dist[e.to] = dist[e.from] + e.weight;
            }
        }
    }
    // One extra pass: if anything still improves, a negative cycle exists.
    for (const Edge& e : edges) {
        if (dist[e.from] != INF && dist[e.from] + e.weight < dist[e.to]) {
            negativeCycle = true;
        }
    }

    cout << "Shortest distances from node 0 (Bellman-Ford):\n";
    for (int i = 0; i < n; ++i) {
        cout << "  0 -> " << i << " = " << dist[i] << "\n";
    }
    cout << "Negative cycle detected: " << (negativeCycle ? "yes" : "no") << "\n";
    return 0;
}
