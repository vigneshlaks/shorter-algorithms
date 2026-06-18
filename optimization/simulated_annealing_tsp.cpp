// Simulated Annealing — mimics the physical cooling of metal to escape local
// optima in a search space (probabilistically accepts worse moves early on).
// Use case: circuit layout design, job-shop scheduling, any large combinatorial
//           problem where exact solving (branch & bound) is too slow.
// Demo: approximately solves the Traveling Salesman Problem.
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

using namespace std;

int n;
vector<vector<double>> dist;

double routeCost(const vector<int>& route) {
    double cost = 0;
    for (int i = 0; i < n; ++i) cost += dist[route[i]][route[(i + 1) % n]];
    return cost;
}

int main() {
    n = 6;
    // Random small set of city coordinates
    vector<pair<double,double>> coords = {{0,0},{2,4},{5,2},{6,6},{8,3},{3,7}};
    dist.assign(n, vector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            dist[i][j] = hypot(coords[i].first - coords[j].first, coords[i].second - coords[j].second);

    vector<int> route(n);
    for (int i = 0; i < n; ++i) route[i] = i;

    mt19937 rng(42);
    uniform_real_distribution<double> uniform01(0.0, 1.0);
    uniform_int_distribution<int> cityDist(0, n - 1);

    double temperature = 1000.0;
    double coolingRate = 0.995;
    double currentCost = routeCost(route);
    vector<int> bestRoute = route;
    double bestCost = currentCost;

    while (temperature > 1.0) {
        int i = cityDist(rng), j = cityDist(rng);
        swap(route[i], route[j]); // propose a neighboring solution
        double newCost = routeCost(route);
        double delta = newCost - currentCost;

        // Accept better solutions always; accept worse ones with probability
        // that shrinks as temperature cools -- this is what lets it escape local optima.
        if (delta < 0 || exp(-delta / temperature) > uniform01(rng)) {
            currentCost = newCost;
            if (currentCost < bestCost) { bestCost = currentCost; bestRoute = route; }
        } else {
            swap(route[i], route[j]); // reject: undo the swap
        }
        temperature *= coolingRate;
    }

    cout << "Simulated Annealing route: ";
    for (int city : bestRoute) cout << city << " -> ";
    cout << bestRoute[0] << "\n";
    cout << "Total distance: " << bestCost << "\n";
    return 0;
}
