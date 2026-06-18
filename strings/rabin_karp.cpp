// Rabin-Karp — hashing-based substring search using a rolling hash.
// Use case: plagiarism detection (compare rolling hashes of text windows),
//           searching for multiple patterns simultaneously.
// Complexity: O(n + m) average case.
#include <iostream>
#include <string>
#include <vector>

using namespace std;

vector<int> rabinKarpSearch(const string& text, const string& pattern) {
    vector<int> matches;
    int n = text.size(), m = pattern.size();
    if (m > n) return matches;

    const long long base = 256, mod = 1000000007;
    long long patternHash = 0, windowHash = 0, power = 1;

    for (int i = 0; i < m; ++i) {
        patternHash = (patternHash * base + pattern[i]) % mod;
        windowHash  = (windowHash * base + text[i]) % mod;
        if (i < m - 1) power = (power * base) % mod;
    }

    for (int i = 0; i <= n - m; ++i) {
        if (windowHash == patternHash) {
            if (text.substr(i, m) == pattern) matches.push_back(i); // confirm (avoid hash collision false-positive)
        }
        if (i < n - m) {
            windowHash = ((windowHash - text[i] * power % mod + mod) % mod * base + text[i + m]) % mod;
        }
    }
    return matches;
}

int main() {
    string text = "abracadabra abracadabra";
    string pattern = "abra";
    auto matches = rabinKarpSearch(text, pattern);
    cout << "Text: " << text << "\nPattern: " << pattern << "\n";
    cout << "Matches found at indices: ";
    for (int idx : matches) cout << idx << " ";
    cout << "\n";
    return 0;
}
