// Branch and Bound — systematically prunes a search space to solve
// combinatorial problems EXACTLY (no approximation).
// Use case: backbone of commercial solvers (CPLEX, Gurobi) for problems like
//           the Traveling Salesman Problem, integer programming, chip layout.
// Demo: solves a small Traveling Salesman Problem exactly.
#include <iostream>
#include <vector>
#include <limits>

using namespace std;

int n;
vector<vector<int>> dist;
int bestCost = numeric_limits<int>::max();
vector<int> bestPath;

void branchAndBound(vector<int>& path, vector<bool>& visited, int cost, int level) {
    if (level == n) {
        int totalCost = cost + dist[path.back()][path[0]]; // return to start
        if (totalCost < bestCost) { bestCost = totalCost; bestPath = path; }
        return;
    }

    for (int next = 0; next < n; ++next) {
        if (visited[next]) continue;
        int newCost = cost + dist[path.back()][next];
        if (newCost >= bestCost) continue; // BOUND: prune this branch, it can't beat the best

        visited[next] = true;
        path.push_back(next);
        branchAndBound(path, visited, newCost, level + 1);
        path.pop_back();
        visited[next] = false;
    }
}

int main() {
    n = 5;
    dist = {
        {0, 10, 15, 20, 25},
        {10, 0, 35, 25, 30},
        {15, 35, 0, 30, 20},
        {20, 25, 30, 0, 15},
        {25, 30, 20, 15, 0}
    };

    vector<int> path = {0};
    vector<bool> visited(n, false);
    visited[0] = true;

    branchAndBound(path, visited, 0, 1);

    cout << "Optimal TSP route: ";
    for (int city : bestPath) cout << city << " -> ";
    cout << bestPath[0] << "\n";
    cout << "Total cost: " << bestCost << "\n";
    return 0;
}
