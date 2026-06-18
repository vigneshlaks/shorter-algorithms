// Tarjan's Algorithm — find Strongly Connected Components (SCCs) in a directed graph.
// Use case: compiler dependency-cycle analysis, finding clusters of mutually
//           reachable web pages, analyzing feedback loops in directed networks.
// Complexity: O(V + E)
#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>

using namespace std;

class Tarjan {
    int n, idxCounter = 0;
    vector<vector<int>> adj;
    vector<int> disc, low;
    vector<bool> onStack;
    stack<int> st;
    vector<vector<int>> sccs;

    void dfs(int u) {
        disc[u] = low[u] = idxCounter++;
        st.push(u);
        onStack[u] = true;

        for (int v : adj[u]) {
            if (disc[v] == -1) {
                dfs(v);
                low[u] = min(low[u], low[v]);
            } else if (onStack[v]) {
                low[u] = min(low[u], disc[v]);
            }
        }

        if (low[u] == disc[u]) { // u is a root of an SCC
            vector<int> component;
            while (true) {
                int v = st.top(); st.pop();
                onStack[v] = false;
                component.push_back(v);
                if (v == u) break;
            }
            sccs.push_back(component);
        }
    }

public:
    Tarjan(int n) : n(n), adj(n), disc(n, -1), low(n, -1), onStack(n, false) {}
    void addEdge(int u, int v) { adj[u].push_back(v); }
    vector<vector<int>> run() {
        for (int i = 0; i < n; ++i) if (disc[i] == -1) dfs(i);
        return sccs;
    }
};

int main() {
    Tarjan g(8);
    g.addEdge(0,1); g.addEdge(1,2); g.addEdge(2,0); // cycle: SCC {0,1,2}
    g.addEdge(1,3); g.addEdge(3,4); g.addEdge(4,5); g.addEdge(5,3); // cycle: SCC {3,4,5}
    g.addEdge(5,6); g.addEdge(6,7);

    auto sccs = g.run();
    cout << "Strongly Connected Components found: " << sccs.size() << "\n";
    for (auto& comp : sccs) {
        cout << "  { ";
        for (int v : comp) cout << v << " ";
        cout << "}\n";
    }
    return 0;
}
