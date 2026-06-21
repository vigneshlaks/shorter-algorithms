// Bipartite Matching — Hopcroft-Karp Algorithm.
// Finds the maximum matching in a bipartite graph: the largest set of edges
// such that no two edges share a vertex.
//
// Use case: job assignment (workers to tasks), student dormitory allocation,
//           scheduling, network flow, computer vision (feature matching).
//
// Complexity: O(E * sqrt(V)) — significantly faster than the naive O(VE)
//             augmenting path approach for large graphs.
//
// Key idea: instead of finding one augmenting path at a time (like Ford-Fulkerson
//           in ford_fulkerson_maxflow.cpp), find ALL shortest augmenting paths
//           simultaneously using BFS, then augment along all of them with DFS.
//           Each BFS+DFS phase increases the length of the shortest augmenting
//           path, and there are at most sqrt(V) distinct lengths.
//
// Terminology:
//   Left set L (e.g. workers), Right set R (e.g. jobs).
//   A matching M is a set of edges with no shared endpoints.
//   An augmenting path alternates between unmatched and matched edges,
//   starting and ending at unmatched vertices — flipping it increases |M| by 1.
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

const int INF = 1e9;

class HopcroftKarp {
    int nl_, nr_;              // size of left and right sets
    vector<vector<int>> adj_; // adj_[u] = list of right vertices u connects to
    vector<int> matchL_;      // matchL_[u] = right vertex matched to left u (-1 if none)
    vector<int> matchR_;      // matchR_[v] = left vertex matched to right v (-1 if none)
    vector<int> dist_;        // BFS distance layers for left vertices

    // BFS: build layered graph of shortest augmenting paths
    bool bfs() {
        queue<int> q;
        dist_.assign(nl_, INF);
        for (int u = 0; u < nl_; ++u) {
            if (matchL_[u] == -1) { dist_[u] = 0; q.push(u); }
        }
        bool found = false;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : adj_[u]) {
                int w = matchR_[v]; // left vertex matched to v (or -1)
                if (w == -1) {
                    found = true; // found an unmatched right vertex
                } else if (dist_[w] == INF) {
                    dist_[w] = dist_[u] + 1;
                    q.push(w);
                }
            }
        }
        return found;
    }

    // DFS: augment along shortest paths found by BFS
    bool dfs(int u) {
        for (int v : adj_[u]) {
            int w = matchR_[v];
            if (w == -1 || (dist_[w] == dist_[u] + 1 && dfs(w))) {
                matchL_[u] = v;
                matchR_[v] = u;
                return true;
            }
        }
        dist_[u] = INF; // remove u from layered graph to avoid re-visiting
        return false;
    }

public:
    HopcroftKarp(int nl, int nr)
        : nl_(nl), nr_(nr), adj_(nl), matchL_(nl, -1), matchR_(nr, -1) {}

    void addEdge(int u, int v) { adj_[u].push_back(v); }

    int maxMatching() {
        int matching = 0;
        while (bfs())
            for (int u = 0; u < nl_; ++u)
                if (matchL_[u] == -1 && dfs(u)) ++matching;
        return matching;
    }

    void printMatching(const vector<string>& leftNames,
                       const vector<string>& rightNames) const {
        cout << "Matching:\n";
        for (int u = 0; u < nl_; ++u)
            if (matchL_[u] != -1)
                cout << "  " << leftNames[u] << " -> " << rightNames[matchL_[u]] << "\n";
    }
};

int main() {
    // Example 1: job assignment
    // Workers: Alice, Bob, Carol
    // Jobs:    Python, C++, Design
    // Edges represent who can do what.
    cout << "=== Job Assignment ===\n";
    vector<string> workers = {"Alice", "Bob", "Carol"};
    vector<string> jobs    = {"Python", "C++", "Design"};

    HopcroftKarp hk1(3, 3);
    hk1.addEdge(0, 0); // Alice  -> Python
    hk1.addEdge(0, 1); // Alice  -> C++
    hk1.addEdge(1, 1); // Bob    -> C++
    hk1.addEdge(1, 2); // Bob    -> Design
    hk1.addEdge(2, 0); // Carol  -> Python
    hk1.addEdge(2, 2); // Carol  -> Design

    int m1 = hk1.maxMatching();
    cout << "Maximum matching size: " << m1 << "/" << workers.size() << "\n";
    hk1.printMatching(workers, jobs);

    // Example 2: intentionally impossible full matching
    // Two workers both only able to do the same one job
    cout << "\n=== Constrained Assignment ===\n";
    vector<string> w2 = {"Dave", "Eve", "Frank"};
    vector<string> j2 = {"Only Job", "Another Job"};

    HopcroftKarp hk2(3, 2);
    hk2.addEdge(0, 0); // Dave  -> Only Job
    hk2.addEdge(1, 0); // Eve   -> Only Job
    hk2.addEdge(2, 1); // Frank -> Another Job

    int m2 = hk2.maxMatching();
    cout << "Maximum matching size: " << m2 << "/" << w2.size()
         << " (full matching impossible)\n";
    hk2.printMatching(w2, j2);

    return 0;
}
