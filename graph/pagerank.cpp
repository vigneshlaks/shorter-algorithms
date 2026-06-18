// PageRank — ranks nodes by importance based on incoming link structure.
// Use case: originally Google's web ranking algorithm; now used for any
//           "influence" ranking over a graph (citation networks, social graphs).
// Complexity: O(k * E) for k power-iteration steps.
#include <iostream>
#include <vector>

using namespace std;

int main() {
    int n = 5;
    // adjacency: outLinks[u] = list of pages u links to
    vector<vector<int>> outLinks = {
        {1, 2},     // page 0 links to 1, 2
        {2},        // page 1 links to 2
        {0},        // page 2 links to 0
        {0, 2, 4},  // page 3 links to 0, 2, 4
        {2}         // page 4 links to 2
    };

    double damping = 0.85;
    vector<double> rank(n, 1.0 / n);
    int iterations = 50;

    for (int it = 0; it < iterations; ++it) {
        vector<double> newRank(n, (1.0 - damping) / n);
        for (int u = 0; u < n; ++u) {
            if (outLinks[u].empty()) continue;
            double share = rank[u] / outLinks[u].size();
            for (int v : outLinks[u]) newRank[v] += damping * share;
        }
        rank = newRank;
    }

    cout << "PageRank scores after convergence:\n";
    for (int i = 0; i < n; ++i) cout << "  page " << i << ": " << rank[i] << "\n";
    return 0;
}
