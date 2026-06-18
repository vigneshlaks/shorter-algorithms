// 0/1 Knapsack — dynamic programming for resource allocation under a hard constraint.
// Use case: budget-limited investment selection, cargo loading,
//           any "pick a subset to maximize value without exceeding capacity" problem.
// Complexity: O(n * capacity)
#include <iostream>
#include <vector>

using namespace std;

int knapsack(int capacity, const vector<int>& weights, const vector<int>& values) {
    int n = weights.size();
    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int w = 0; w <= capacity; ++w) {
            dp[i][w] = dp[i-1][w]; // don't take item i-1
            if (weights[i-1] <= w) {
                dp[i][w] = max(dp[i][w], dp[i-1][w - weights[i-1]] + values[i-1]);
            }
        }
    }
    return dp[n][capacity];
}

int main() {
    vector<int> weights = {2, 3, 4, 5};
    vector<int> values  = {3, 4, 5, 6};
    int capacity = 5;

    cout << "Items (weight, value): ";
    for (size_t i = 0; i < weights.size(); ++i) cout << "(" << weights[i] << "," << values[i] << ") ";
    cout << "\nCapacity: " << capacity << "\n";
    cout << "Maximum achievable value: " << knapsack(capacity, weights, values) << "\n";
    return 0;
}
