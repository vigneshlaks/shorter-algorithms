// Floyd-Warshall Algorithm — all-pairs shortest paths.
// Use case: precomputing travel times between every pair of stations in a
//           transit network, dense small graphs where you need every distance.
// Complexity: O(V^3)
#include <iostream>
#include <vector>
#include <limits>

using namespace std;

int main() {
    const long long INF = numeric_limits<long long>::max() / 4;
    int n = 4;
    vector<vector<long long>> dist(n, vector<long long>(n, INF));
    for (int i = 0; i < n; ++i) dist[i][i] = 0;

    // Directed weighted edges
    dist[0][1] = 5;
    dist[0][3] = 10;
    dist[1][2] = 3;
    dist[2][3] = 1;
    dist[3][0] = 2;

    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];

    cout << "All-pairs shortest distances:\n";
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (dist[i][j] >= INF) cout << "INF\t";
            else cout << dist[i][j] << "\t";
        }
        cout << "\n";
    }
    return 0;
}
