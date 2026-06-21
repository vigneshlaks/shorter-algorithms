// A* Search — optimal heuristic pathfinding.
// Extends Dijkstra by adding a heuristic h(n): an estimate of the remaining
// distance from node n to the goal. The priority queue orders by f(n) = g(n) + h(n)
// where g(n) is the known cost from start to n.
//
// Use case: GPS routing, game AI pathfinding, robotics motion planning,
//           puzzle solving (15-puzzle, Sokoban). Used everywhere Dijkstra
//           is too slow because it explores in all directions equally.
//
// Complexity: O(E log V) same as Dijkstra, but explores far fewer nodes in
//             practice when the heuristic is good.
//
// Correctness guarantee (admissibility): A* finds the optimal path if and
// only if h(n) NEVER overestimates the true remaining distance.
// A heuristic satisfying this is called "admissible".
// Common admissible heuristics:
//   - Grid graphs: Manhattan distance or Euclidean distance
//   - General graphs: 0 (degenerates to Dijkstra — always admissible, never helpful)
//
// vs Dijkstra (dijkstra.cpp):
//   Dijkstra explores outward in all directions (h=0, cost-only).
//   A* focuses exploration toward the goal using the heuristic.
//   On a 1000x1000 grid, Dijkstra may explore 1M nodes; A* with
//   Euclidean heuristic typically explores only a thin corridor.
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <limits>
#include <functional>
#include <string>

using namespace std;

struct Edge { int to; double weight; };
using Graph = vector<vector<Edge>>;

// 2D position for the heuristic (optional — graph nodes can store coordinates)
struct Pos { double x, y; };

double euclidean(const Pos& a, const Pos& b) {
    double dx = a.x - b.x, dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

struct AStarResult {
    double cost;
    vector<int> path;
    int nodesExplored;
};

AStarResult astar(int start, int goal, const Graph& adj,
                  const vector<Pos>& coords) {
    int n = adj.size();
    const double INF = numeric_limits<double>::infinity();

    vector<double> g(n, INF); // known cost from start
    vector<int>    parent(n, -1);
    vector<bool>   closed(n, false);
    int explored = 0;

    // Min-heap of (f = g + h, node)
    priority_queue<pair<double,int>,
                   vector<pair<double,int>>,
                   greater<pair<double,int>>> pq;

    g[start] = 0;
    double h_start = euclidean(coords[start], coords[goal]);
    pq.push({h_start, start});

    while (!pq.empty()) {
        auto [f, u] = pq.top(); pq.pop();

        if (closed[u]) continue;
        closed[u] = true;
        ++explored;

        if (u == goal) break;

        for (const Edge& e : adj[u]) {
            double ng = g[u] + e.weight;
            if (ng < g[e.to]) {
                g[e.to]      = ng;
                parent[e.to] = u;
                double h     = euclidean(coords[e.to], coords[goal]);
                pq.push({ng + h, e.to});
            }
        }
    }

    // Reconstruct path
    vector<int> path;
    if (g[goal] < INF) {
        for (int v = goal; v != -1; v = parent[v]) path.push_back(v);
        reverse(path.begin(), path.end());
    }

    return {g[goal], path, explored};
}

void addEdge(Graph& g, int u, int v, double w) {
    g[u].push_back({v, w});
    g[v].push_back({u, w});
}

int main() {
    // Grid-like graph (7 nodes) with 2D coordinates.
    // Visualized layout:
    //
    //   0(0,4) ---5--- 1(2,4) ---3--- 2(4,4)
    //     |                            |
    //     4                            2
    //     |                            |
    //   3(0,2) ---6--- 4(2,2) ---4--- 5(4,2)
    //                   |
    //                   5
    //                   |
    //                 6(2,0)  <- GOAL
    //
    // Start: node 0, Goal: node 6

    int n = 7;
    Graph g(n);
    vector<Pos> coords = {
        {0,4}, {2,4}, {4,4},
        {0,2}, {2,2}, {4,2},
        {2,0}
    };

    addEdge(g, 0, 1, 2); addEdge(g, 1, 2, 2);
    addEdge(g, 0, 3, 2); addEdge(g, 2, 5, 2);
    addEdge(g, 3, 4, 2); addEdge(g, 4, 5, 2);
    addEdge(g, 4, 6, 2);

    int start = 0, goal = 6;
    auto result = astar(start, goal, g, coords);

    cout << "A* Search: node " << start << " -> node " << goal << "\n";
    cout << "Path: ";
    for (int i = 0; i < (int)result.path.size(); ++i) {
        cout << result.path[i];
        if (i + 1 < (int)result.path.size()) cout << " -> ";
    }
    cout << "\nCost: " << result.cost << "\n";
    cout << "Nodes explored: " << result.nodesExplored << " / " << n << "\n";

    // Compare: run Dijkstra (A* with h=0) to show the difference in nodes explored
    auto dijkstra_result = astar(start, goal, g, [&]() -> vector<Pos> {
        // Zero heuristic: all positions at same point as goal
        return vector<Pos>(n, coords[goal]);
    }());

    cout << "\nDijkstra (h=0) nodes explored: "
         << dijkstra_result.nodesExplored << " / " << n
         << "\n(A* explores fewer nodes thanks to the heuristic)\n";

    return 0;
}
