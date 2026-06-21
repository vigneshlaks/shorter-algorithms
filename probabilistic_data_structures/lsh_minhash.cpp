// Locality-Sensitive Hashing via MinHash — groups similar SETS together
// quickly by estimating Jaccard similarity without comparing every pair.
// Use case: near-duplicate document detection at web scale, deduplication
//           pipelines, and the precursor technique behind modern vector search.
// Complexity: O(k) per item for k hash functions, vs O(n^2) for all-pairs comparison.
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <functional>
#include <limits>
#include <cstdint>

using namespace std;

set<string> shingle(const string& text, int k = 3) {
    // Break text into overlapping k-word "shingles" (a common LSH preprocessing step)
    istringstream iss(text);
    vector<string> words;
    string w;
    while (iss >> w) words.push_back(w);

    set<string> shingles;
    for (size_t i = 0; i + k <= words.size(); ++i) {
        string s;
        for (int j = 0; j < k; ++j) s += words[i + j] + " ";
        shingles.insert(s);
    }
    return shingles;
}

vector<uint64_t> minHashSignature(const set<string>& shingles, int numHashes) {
    vector<uint64_t> signature(numHashes, numeric_limits<uint64_t>::max());
    for (auto& s : shingles) {
        size_t baseHash = hash<string>{}(s);
        for (int h = 0; h < numHashes; ++h) {
            uint64_t hv = baseHash ^ (h * 0x9e3779b97f4a7c15ULL); // simulate independent hash functions
            signature[h] = min(signature[h], hv);
        }
    }
    return signature;
}

double estimateJaccard(const vector<uint64_t>& sigA, const vector<uint64_t>& sigB) {
    int matches = 0;
    for (size_t i = 0; i < sigA.size(); ++i) if (sigA[i] == sigB[i]) matches++;
    return (double)matches / sigA.size();
}

double realJaccard(const set<string>& a, const set<string>& b) {
    set<string> inter, uni;
    for (auto& x : a) if (b.count(x)) inter.insert(x);
    uni = a; for (auto& x : b) uni.insert(x);
    return uni.empty() ? 1.0 : (double)inter.size() / uni.size();
}

int main() {
    string docA = "the quick brown fox jumps over the lazy dog near the river";
    string docB = "the quick brown fox leaps over the lazy dog near the river";
    string docC = "completely unrelated text about cooking pasta and tomato sauce";

    auto shA = shingle(docA), shB = shingle(docB), shC = shingle(docC);
    int numHashes = 100;
    auto sigA = minHashSignature(shA, numHashes);
    auto sigB = minHashSignature(shB, numHashes);
    auto sigC = minHashSignature(shC, numHashes);

    cout << "Doc A vs Doc B (similar):\n";
    cout << "  Real Jaccard similarity:      " << realJaccard(shA, shB) << "\n";
    cout << "  MinHash estimated similarity: " << estimateJaccard(sigA, sigB) << "\n\n";

    cout << "Doc A vs Doc C (unrelated):\n";
    cout << "  Real Jaccard similarity:      " << realJaccard(shA, shC) << "\n";
    cout << "  MinHash estimated similarity: " << estimateJaccard(sigA, sigC) << "\n";
    cout << "\n-> MinHash estimates similarity in O(numHashes) instead of comparing full sets,\n";
    cout << "   which is what lets LSH scale to billions of documents.\n";
    return 0;
}
