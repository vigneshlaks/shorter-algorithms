// Bloom Filter — probabilistic set membership test (no false negatives,
// possible false positives).
// Use case: browsers checking "have I seen this malicious URL before" without
//           storing the full list, database engines skipping disk reads for
//           keys that definitely don't exist.
// Complexity: O(k) per insert/query, k = number of hash functions.
#include <iostream>
#include <vector>
#include <string>
#include <functional>

using namespace std;

class BloomFilter {
    vector<bool> bits;
    int size;
    int numHashes;

    size_t hashItem(const string& item, int seed) const {
        size_t h = hash<string>{}(item);
        h ^= seed * 0x9e3779b9;
        return h % size;
    }

public:
    BloomFilter(int size, int numHashes) : bits(size, false), size(size), numHashes(numHashes) {}

    void add(const string& item) {
        for (int i = 0; i < numHashes; ++i) bits[hashItem(item, i)] = true;
    }

    bool mightContain(const string& item) const {
        for (int i = 0; i < numHashes; ++i)
            if (!bits[hashItem(item, i)]) return false;
        return true; // definitely not absent, but could be a false positive
    }
};

int main() {
    BloomFilter filter(100, 4);
    vector<string> urls = {"malware.com", "phishing.net", "spam.org"};
    for (auto& u : urls) filter.add(u);

    vector<string> tests = {"malware.com", "google.com", "phishing.net", "safe-site.com"};
    cout << "Bloom filter membership checks:\n";
    for (auto& t : tests) {
        cout << "  \"" << t << "\": " << (filter.mightContain(t) ? "might be in set" : "definitely NOT in set") << "\n";
    }
    return 0;
}
