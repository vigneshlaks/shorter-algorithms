// Ford-Fulkerson / Edmonds-Karp — Maximum Flow in a flow network.
// Use case: bipartite matching (assigning workers to jobs), bandwidth
//           allocation, modeling capacity-constrained logistics networks.
// Complexity: O(V * E^2) using BFS to find augmenting paths (Edmonds-Karp).
#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <cstring>

using namespace std;

const int MAXN = 10;
int capacity_[MAXN][MAXN];
int n;

int bfsFindPath(int s, int t, vector<int>& parent) {
    fill(parent.begin(), parent.end(), -1);
    parent[s] = s;
    queue<pair<int,int>> q;
    q.push({s, INT_MAX});

    while (!q.empty()) {
        auto [u, flow] = q.front(); q.pop();
        for (int v = 0; v < n; ++v) {
            if (parent[v] == -1 && capacity_[u][v] > 0) {
                parent[v] = u;
                int newFlow = min(flow, capacity_[u][v]);
                if (v == t) return newFlow;
                q.push({v, newFlow});
            }
        }
    }
    return 0;
}

int maxFlow(int s, int t) {
    int flow = 0;
    vector<int> parent(n);
    int newFlow;
    while ((newFlow = bfsFindPath(s, t, parent))) {
        flow += newFlow;
        int cur = t;
        while (cur != s) {
            int prev = parent[cur];
            capacity_[prev][cur] -= newFlow;
            capacity_[cur][prev] += newFlow; // residual edge
            cur = prev;
        }
    }
    return flow;
}

int main() {
    n = 6;
    memset(capacity_, 0, sizeof(capacity_));
    // 0 = source, 5 = sink
    capacity_[0][1] = 16; capacity_[0][2] = 13;
    capacity_[1][2] = 10; capacity_[1][3] = 12;
    capacity_[2][1] = 4;  capacity_[2][4] = 14;
    capacity_[3][2] = 9;  capacity_[3][5] = 20;
    capacity_[4][3] = 7;  capacity_[4][5] = 4;

    cout << "Maximum flow from source(0) to sink(5): " << maxFlow(0, 5) << "\n";
    return 0;
}
