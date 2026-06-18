// Trie (Prefix Tree) — efficient prefix-based string storage and lookup.
// Use case: autocomplete systems, spell-checkers, IP routing tables (longest
//           prefix match), dictionary word lookup.
// Complexity: O(L) per insert/search, where L = word length.
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

using namespace std;

struct TrieNode {
    unordered_map<char, TrieNode*> children;
    bool isWord = false;
};

class Trie {
    TrieNode* root;

    void collect(TrieNode* node, string prefix, vector<string>& results) {
        if (node->isWord) results.push_back(prefix);
        for (auto& [ch, child] : node->children) collect(child, prefix + ch, results);
    }

public:
    Trie() { root = new TrieNode(); }

    void insert(const string& word) {
        TrieNode* node = root;
        for (char c : word) {
            if (!node->children.count(c)) node->children[c] = new TrieNode();
            node = node->children[c];
        }
        node->isWord = true;
    }

    vector<string> autocomplete(const string& prefix) {
        TrieNode* node = root;
        for (char c : prefix) {
            if (!node->children.count(c)) return {};
            node = node->children[c];
        }
        vector<string> results;
        collect(node, prefix, results);
        return results;
    }
};

int main() {
    Trie trie;
    for (string w : {"cat", "car", "card", "care", "dog", "do"}) trie.insert(w);

    string prefix = "car";
    cout << "Autocomplete suggestions for \"" << prefix << "\":\n";
    for (auto& w : trie.autocomplete(prefix)) cout << "  " << w << "\n";
    return 0;
}
