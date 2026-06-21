// Articulation Points & Bridges — find nodes/edges whose removal disconnects
// the graph. Critical for network reliability: an articulation point is a
// single point of failure; a bridge is a single link of failure.
//
// Use case: telecom network planning, circuit board analysis, road network
//           resilience, social network bottleneck detection.
//
// Algorithm: Tarjan's DFS-based method (same author as tarjan_scc.cpp).
// Complexity: O(V + E) — single DFS pass over the graph.
//
// Key idea: track the DFS discovery time of each node (disc[]) and the
// lowest discovery time reachable from its subtree (low[]). A node u is an
// articulation point if a child v in the DFS tree has low[v] >= disc[u],
// meaning v cannot reach any ancestor of u without going through u.
// An edge (u,v) is a bridge if low[v] > disc[u].
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class Graph {
    int n_;
    vector<vector<int>> adj_;
    vector<int> disc_, low_, parent_;
    vector<bool> visited_, isAP_;
    vector<pair<int,int>> bridges_;
    int timer_ = 0;

    void dfs(int u) {
        visited_[u] = true;
        disc_[u] = low_[u] = timer_++;
        int children = 0;

        for (int v : adj_[u]) {
            if (!visited_[v]) {
                ++children;
                parent_[v] = u;
                dfs(v);

                low_[u] = min(low_[u], low_[v]);

                // u is an articulation point if:
                // 1. u is root of DFS tree and has 2+ children, OR
                // 2. u is not root and low[v] >= disc[u] (v can't bypass u)
                if (parent_[u] == -1 && children > 1) isAP_[u] = true;
                if (parent_[u] != -1 && low_[v] >= disc_[u]) isAP_[u] = true;

                // (u,v) is a bridge if v cannot reach u or any ancestor of u
                if (low_[v] > disc_[u]) bridges_.push_back({u, v});

            } else if (v != parent_[u]) {
                // Back edge — update low to reflect reachable ancestor
                low_[u] = min(low_[u], disc_[v]);
            }
        }
    }

public:
    explicit Graph(int n) : n_(n), adj_(n), disc_(n), low_(n),
                             parent_(n, -1), visited_(n, false), isAP_(n, false) {}

    void addEdge(int u, int v) {
        adj_[u].push_back(v);
        adj_[v].push_back(u);
    }

    void analyze() {
        for (int i = 0; i < n_; ++i)
            if (!visited_[i]) dfs(i);
    }

    void printResults() const {
        cout << "Articulation Points: ";
        bool any = false;
        for (int i = 0; i < n_; ++i)
            if (isAP_[i]) { cout << i << " "; any = true; }
        if (!any) cout << "(none)";
        cout << "\n";

        cout << "Bridges: ";
        if (bridges_.empty()) { cout << "(none)"; }
        for (auto [u, v] : bridges_) cout << u << "-" << v << " ";
        cout << "\n";
    }
};

int main() {
    // Graph 1: classic example with clear articulation points
    //   0 -- 1 -- 3 -- 4
    //   |  /
    //   2
    // Node 1 and 3 are articulation points; edge 3-4 is a bridge.
    cout << "=== Graph 1 ===\n";
    Graph g1(5);
    g1.addEdge(0, 1);
    g1.addEdge(0, 2);
    g1.addEdge(1, 2);
    g1.addEdge(1, 3);
    g1.addEdge(3, 4);
    g1.analyze();
    g1.printResults();

    // Graph 2: fully connected — no articulation points or bridges
    cout << "\n=== Graph 2 (fully connected triangle) ===\n";
    Graph g2(3);
    g2.addEdge(0, 1);
    g2.addEdge(1, 2);
    g2.addEdge(2, 0);
    g2.analyze();
    g2.printResults();

    // Graph 3: chain — every node (except endpoints) is an articulation point,
    // every edge is a bridge
    cout << "\n=== Graph 3 (chain: 0-1-2-3-4) ===\n";
    Graph g3(5);
    g3.addEdge(0, 1); g3.addEdge(1, 2); g3.addEdge(2, 3); g3.addEdge(3, 4);
    g3.analyze();
    g3.printResults();

    return 0;
}
