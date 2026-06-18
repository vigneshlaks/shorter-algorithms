// Topological Sort (Kahn's Algorithm) — order nodes so every edge points forward.
// Use case: build systems resolving dependencies (Makefiles), course
//           prerequisite scheduling, spreadsheet formula evaluation order.
// Complexity: O(V + E)
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

int main() {
    int n = 6;
    // Example: tasks 0..5 representing a build pipeline
    vector<vector<int>> adj(n);
    vector<int> indegree(n, 0);
    auto addEdge = [&](int u, int v) { adj[u].push_back(v); indegree[v]++; };

    addEdge(0, 1); // 0 must finish before 1
    addEdge(0, 2);
    addEdge(1, 3);
    addEdge(2, 3);
    addEdge(3, 4);
    addEdge(4, 5);

    queue<int> q;
    for (int i = 0; i < n; ++i) if (indegree[i] == 0) q.push(i);

    vector<int> order;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (int v : adj[u]) {
            if (--indegree[v] == 0) q.push(v);
        }
    }

    if ((int)order.size() != n) {
        cout << "Cycle detected — no valid topological order.\n";
    } else {
        cout << "Valid build order: ";
        for (size_t i = 0; i < order.size(); ++i) cout << order[i] << (i + 1 < order.size() ? " -> " : "\n");
    }
    return 0;
}
