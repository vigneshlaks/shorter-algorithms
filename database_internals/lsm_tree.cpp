// LSM Tree (Log-Structured Merge Tree, simplified) — optimized for
// WRITE-HEAVY workloads by buffering writes in memory and flushing them
// sequentially to sorted, immutable segments on disk.
// Use case: Cassandra, RocksDB, LevelDB all use LSM trees internally because
//           sequential disk writes are far faster than random in-place updates.
// This demo simulates memtable -> sorted-segment flush -> merged reads.
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

class LSMTree {
    map<string, string> memtable;             // in-memory buffer for recent writes
    vector<map<string, string>> segments;      // immutable "disk" segments, oldest first
    size_t memtableLimit;

public:
    LSMTree(size_t memtableLimit = 3) : memtableLimit(memtableLimit) {}

    void put(const string& key, const string& value) {
        memtable[key] = value;
        if (memtable.size() >= memtableLimit) flush();
    }

    void flush() {
        if (memtable.empty()) return;
        cout << "  [flush] writing memtable (" << memtable.size() << " keys) to a new sorted segment on disk\n";
        segments.push_back(memtable);
        memtable.clear();
    }

    string get(const string& key) {
        // Check memtable first (most recent data)
        if (memtable.count(key)) return memtable[key];
        // Then check segments newest-to-oldest (most recent write wins)
        for (auto it = segments.rbegin(); it != segments.rend(); ++it) {
            if (it->count(key)) return (*it)[key];
        }
        return "(not found)";
    }

    void compact() {
        cout << "  [compaction] merging " << segments.size() << " segments into one sorted segment\n";
        map<string, string> merged;
        for (auto& seg : segments) for (auto& [k, v] : seg) merged[k] = v; // later segments overwrite earlier
        segments.clear();
        segments.push_back(merged);
    }
};

int main() {
    LSMTree db(3);

    cout << "Writing keys (memtable flushes every 3 writes):\n";
    db.put("user:1", "alice");
    db.put("user:2", "bob");
    db.put("user:1", "alice-updated"); // update, doesn't grow memtable size
    db.put("user:3", "carol");          // memtable now has 3 distinct keys -> flush
    db.put("user:2", "bob-updated");
    db.put("user:4", "dave");
    db.put("user:5", "eve");            // memtable hits 3 distinct keys again -> flush

    cout << "\nReads (checks memtable, then newest-to-oldest segments):\n";
    cout << "  user:1 = " << db.get("user:1") << "\n";
    cout << "  user:2 = " << db.get("user:2") << "\n";
    cout << "  user:3 = " << db.get("user:3") << "\n";

    cout << "\nRunning compaction:\n";
    db.compact();
    cout << "  user:1 = " << db.get("user:1") << " (still correct after merge)\n";
    cout << "  user:2 = " << db.get("user:2") << " (correct version kept after merge)\n";
    return 0;
}
