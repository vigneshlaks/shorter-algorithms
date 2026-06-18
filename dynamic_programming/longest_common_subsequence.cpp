// Longest Common Subsequence (LCS) — dynamic programming.
// Use case: diff tools (git diff shows LCS-based changes), DNA sequence
//           comparison, version control merge algorithms.
// Complexity: O(n * m)
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

string lcs(const string& a, const string& b) {
    int n = a.size(), m = b.size();
    vector<vector<int>> dp(n + 1, vector<int>(m + 1, 0));

    for (int i = 1; i <= n; ++i)
        for (int j = 1; j <= m; ++j)
            dp[i][j] = (a[i-1] == b[j-1]) ? dp[i-1][j-1] + 1 : max(dp[i-1][j], dp[i][j-1]);

    // reconstruct
    string result;
    int i = n, j = m;
    while (i > 0 && j > 0) {
        if (a[i-1] == b[j-1]) { result += a[i-1]; --i; --j; }
        else if (dp[i-1][j] > dp[i][j-1]) --i;
        else --j;
    }
    reverse(result.begin(), result.end());
    return result;
}

int main() {
    string a = "ABCBDAB", b = "BDCABA";
    string result = lcs(a, b);
    cout << "String A: " << a << "\nString B: " << b << "\n";
    cout << "Longest Common Subsequence: " << result << " (length " << result.size() << ")\n";
    return 0;
}
