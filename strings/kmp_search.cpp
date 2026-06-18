// Knuth-Morris-Pratt (KMP) — linear-time substring search.
// Use case: text editor "find" function, bioinformatics motif search,
//           any repeated substring search where naive O(n*m) is too slow.
// Complexity: O(n + m)
#include <iostream>
#include <vector>
#include <string>

using namespace std;

vector<int> buildLPS(const string& pattern) {
    int m = pattern.size();
    vector<int> lps(m, 0);
    int len = 0, i = 1;
    while (i < m) {
        if (pattern[i] == pattern[len]) {
            lps[i++] = ++len;
        } else if (len > 0) {
            len = lps[len - 1];
        } else {
            lps[i++] = 0;
        }
    }
    return lps;
}

vector<int> kmpSearch(const string& text, const string& pattern) {
    vector<int> matches;
    vector<int> lps = buildLPS(pattern);
    int n = text.size(), m = pattern.size();
    int i = 0, j = 0;
    while (i < n) {
        if (text[i] == pattern[j]) {
            ++i; ++j;
            if (j == m) {
                matches.push_back(i - j);
                j = lps[j - 1];
            }
        } else if (j > 0) {
            j = lps[j - 1];
        } else {
            ++i;
        }
    }
    return matches;
}

int main() {
    string text = "ababcabcabababd";
    string pattern = "abab";
    auto matches = kmpSearch(text, pattern);
    cout << "Text: " << text << "\nPattern: " << pattern << "\n";
    cout << "Matches found at indices: ";
    for (int idx : matches) cout << idx << " ";
    cout << "\n";
    return 0;
}
