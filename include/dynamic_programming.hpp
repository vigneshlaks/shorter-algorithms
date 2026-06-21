#pragma once
#include <vector>
#include <string>
#include <algorithm>

namespace algo {

// ── Edit Distance (Levenshtein) ───────────────────────────────────────────────
inline int editDistance(const std::string& a, const std::string& b) {
    int m = a.size(), n = b.size();
    std::vector<std::vector<int>> dp(m+1, std::vector<int>(n+1));
    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;
    for (int i = 1; i <= m; ++i)
        for (int j = 1; j <= n; ++j)
            dp[i][j] = (a[i-1] == b[j-1])
                ? dp[i-1][j-1]
                : 1 + std::min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]});
    return dp[m][n];
}

// ── Longest Common Subsequence ────────────────────────────────────────────────
inline int lcs(const std::string& a, const std::string& b) {
    int m = a.size(), n = b.size();
    std::vector<std::vector<int>> dp(m+1, std::vector<int>(n+1, 0));
    for (int i = 1; i <= m; ++i)
        for (int j = 1; j <= n; ++j)
            dp[i][j] = (a[i-1] == b[j-1])
                ? dp[i-1][j-1] + 1
                : std::max(dp[i-1][j], dp[i][j-1]);
    return dp[m][n];
}

// ── 0/1 Knapsack ──────────────────────────────────────────────────────────────
inline int knapsack(int capacity, const std::vector<int>& weights,
                    const std::vector<int>& values) {
    int n = weights.size();
    std::vector<std::vector<int>> dp(n+1, std::vector<int>(capacity+1, 0));
    for (int i = 1; i <= n; ++i)
        for (int w = 0; w <= capacity; ++w) {
            dp[i][w] = dp[i-1][w];
            if (weights[i-1] <= w)
                dp[i][w] = std::max(dp[i][w], dp[i-1][w-weights[i-1]] + values[i-1]);
        }
    return dp[n][capacity];
}

} // namespace algo
