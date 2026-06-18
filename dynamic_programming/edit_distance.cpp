// Edit Distance (Levenshtein Distance) — minimum insert/delete/substitute ops
// to turn one string into another.
// Use case: spell checkers, fuzzy string matching, DNA mutation distance,
//           plagiarism / similarity detection.
// Complexity: O(n * m)
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

int editDistance(const string& a, const string& b) {
    int n = a.size(), m = b.size();
    vector<vector<int>> dp(n + 1, vector<int>(m + 1));

    for (int i = 0; i <= n; ++i) dp[i][0] = i;
    for (int j = 0; j <= m; ++j) dp[0][j] = j;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (a[i-1] == b[j-1]) dp[i][j] = dp[i-1][j-1];
            else dp[i][j] = 1 + min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]});
        }
    }
    return dp[n][m];
}

int main() {
    string a = "kitten", b = "sitting";
    cout << "\"" << a << "\" -> \"" << b << "\"\n";
    cout << "Edit distance: " << editDistance(a, b) << "\n";
    return 0;
}
