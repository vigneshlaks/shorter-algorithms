// Kruskal's Algorithm — Minimum Spanning Tree (uses Union-Find internally).
// Use case: designing a network with minimum total cable/wire cost,
//           clustering (cut the most expensive edges from the MST).
// Complexity: O(E log E)
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Edge { int u, v; long long w; };

class UnionFind {
    vector<int> parent;
public:
    UnionFind(int n) : parent(n) { for (int i = 0; i < n; ++i) parent[i] = i; }
    int find(int x) { return parent[x] == x ? x : parent[x] = find(parent[x]); }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        parent[a] = b;
        return true;
    }
};

int main() {
    int n = 6;
    vector<Edge> edges = {
        {0,1,4}, {0,2,4}, {1,2,2}, {1,0,4}, {2,3,3},
        {2,5,2}, {2,4,4}, {3,4,3}, {5,4,3}
    };

    sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b){ return a.w < b.w; });

    UnionFind uf(n);
    long long total = 0;
    cout << "Edges chosen for Minimum Spanning Tree:\n";
    for (const Edge& e : edges) {
        if (uf.unite(e.u, e.v)) {
            cout << "  " << e.u << " - " << e.v << " (weight " << e.w << ")\n";
            total += e.w;
        }
    }
    cout << "Total MST weight: " << total << "\n";
    return 0;
}
