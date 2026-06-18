// Union-Find (Disjoint Set Union) — track connected components efficiently.
// Use case: Kruskal's MST, cycle detection in undirected graphs,
//           image segmentation, network connectivity queries.
// Complexity: O(1) amortized per operation with path compression + union by rank.
#include <iostream>
#include <vector>

using namespace std;

class UnionFind {
    vector<int> parent, rank_;
public:
    UnionFind(int n) : parent(n), rank_(n, 0) {
        for (int i = 0; i < n; ++i) parent[i] = i;
    }
    int find(int x) {
        if (parent[x] != x) parent[x] = find(parent[x]); // path compression
        return parent[x];
    }
    bool unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false; // already connected -> would form a cycle
        if (rank_[ra] < rank_[rb]) swap(ra, rb);
        parent[rb] = ra;
        if (rank_[ra] == rank_[rb]) rank_[ra]++;
        return true;
    }
};

int main() {
    UnionFind uf(6);
    vector<pair<int,int>> edges = {{0,1},{1,2},{3,4},{0,2},{4,5}};

    cout << "Adding edges and detecting cycles:\n";
    for (auto [a, b] : edges) {
        bool merged = uf.unite(a, b);
        cout << "  edge (" << a << "," << b << "): "
             << (merged ? "merged" : "CYCLE detected, skipped") << "\n";
    }

    cout << "Are 0 and 5 connected? " << (uf.find(0) == uf.find(5) ? "yes" : "no") << "\n";
    return 0;
}
