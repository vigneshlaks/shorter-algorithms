// Breadth-First Search — shortest path in an UNWEIGHTED graph, connected components.
// Use case: "degrees of separation" in social networks, web crawlers,
//           level-order exploration, finding connected components.
// Complexity: O(V + E)
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

int main() {
    int n = 6;
    vector<vector<int>> adj(n);
    auto addEdge = [&](int u, int v) { adj[u].push_back(v); adj[v].push_back(u); };
    addEdge(0, 1); addEdge(0, 2); addEdge(1, 3); addEdge(2, 3); addEdge(3, 4); addEdge(4, 5);

    int source = 0;
    vector<int> dist(n, -1);
    vector<int> parent(n, -1);
    queue<int> q;
    dist[source] = 0;
    q.push(source);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : adj[u]) {
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                parent[v] = u;
                q.push(v);
            }
        }
    }

    cout << "BFS distances (hops) from node 0:\n";
    for (int i = 0; i < n; ++i) cout << "  0 -> " << i << " = " << dist[i] << " hops\n";

    // Reconstruct path to node 5
    cout << "Path from 0 to 5: ";
    vector<int> path;
    for (int at = 5; at != -1; at = parent[at]) path.push_back(at);
    for (auto it = path.rbegin(); it != path.rend(); ++it) cout << *it << (it + 1 != path.rend() ? " -> " : "\n");
    return 0;
}
