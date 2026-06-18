// Cuckoo Filter — like a Bloom filter but supports DELETION.
// Use case: caching systems that need to evict stale entries, network packet
//           filtering where rules change dynamically.
// Complexity: O(1) amortized insert/lookup/delete.
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <optional>
#include <cstdint>

using namespace std;

class CuckooFilter {
    vector<optional<uint32_t>> buckets;
    int numBuckets;
    int maxKicks = 50;

    uint32_t fingerprint(const string& item) const {
        return (uint32_t)(hash<string>{}(item) & 0xFFFF) | 1; // ensure non-zero
    }
    size_t hash1(const string& item) const { return hash<string>{}(item) % numBuckets; }
    size_t hash2(uint32_t fp) const { return (hash<uint32_t>{}(fp)) % numBuckets; }

public:
    CuckooFilter(int numBuckets) : buckets(numBuckets), numBuckets(numBuckets) {}

    bool insert(const string& item) {
        uint32_t fp = fingerprint(item);
        size_t i1 = hash1(item);
        size_t i2 = (i1 ^ hash2(fp)) % numBuckets;

        if (!buckets[i1]) { buckets[i1] = fp; return true; }
        if (!buckets[i2]) { buckets[i2] = fp; return true; }

        // Both occupied: kick out a random existing entry (cuckoo eviction)
        size_t i = i1;
        for (int n = 0; n < maxKicks; ++n) {
            uint32_t evicted = *buckets[i];
            buckets[i] = fp;
            fp = evicted;
            i = (i ^ hash2(fp)) % numBuckets;
            if (!buckets[i]) { buckets[i] = fp; return true; }
        }
        return false; // filter too full
    }

    bool mightContain(const string& item) const {
        uint32_t fp = fingerprint(item);
        size_t i1 = hash1(item);
        size_t i2 = (i1 ^ hash2(fp)) % numBuckets;
        return (buckets[i1] && *buckets[i1] == fp) || (buckets[i2] && *buckets[i2] == fp);
    }

    bool remove(const string& item) {
        uint32_t fp = fingerprint(item);
        size_t i1 = hash1(item);
        size_t i2 = (i1 ^ hash2(fp)) % numBuckets;
        if (buckets[i1] && *buckets[i1] == fp) { buckets[i1].reset(); return true; }
        if (buckets[i2] && *buckets[i2] == fp) { buckets[i2].reset(); return true; }
        return false;
    }
};

int main() {
    CuckooFilter filter(32);
    for (string item : {"alice", "bob", "carol"}) filter.insert(item);

    cout << "Contains \"bob\"? " << (filter.mightContain("bob") ? "yes" : "no") << "\n";
    filter.remove("bob");
    cout << "After removing \"bob\" — contains \"bob\"? " << (filter.mightContain("bob") ? "yes" : "no") << "\n";
    cout << "Contains \"alice\"? " << (filter.mightContain("alice") ? "yes" : "no") << "\n";
    return 0;
}
