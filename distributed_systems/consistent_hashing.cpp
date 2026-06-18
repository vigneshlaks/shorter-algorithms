// Consistent Hashing — distributes data across servers with MINIMAL reshuffling
// when servers are added/removed (unlike naive hash % N).
// Use case: distributed caches (Memcached), CDN request routing, sharding
//           in distributed databases.
// Complexity: O(log n) lookup with a sorted ring, n = number of virtual nodes.
#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <vector>

using namespace std;

class ConsistentHashRing {
    map<size_t, string> ring; // hash -> server name
    int virtualNodesPerServer;

    size_t hashKey(const string& key) const { return hash<string>{}(key); }

public:
    ConsistentHashRing(int virtualNodesPerServer = 3) : virtualNodesPerServer(virtualNodesPerServer) {}

    void addServer(const string& server) {
        for (int i = 0; i < virtualNodesPerServer; ++i) {
            ring[hashKey(server + "#" + to_string(i))] = server;
        }
    }

    void removeServer(const string& server) {
        for (int i = 0; i < virtualNodesPerServer; ++i) {
            ring.erase(hashKey(server + "#" + to_string(i)));
        }
    }

    string getServer(const string& key) const {
        if (ring.empty()) return "";
        size_t h = hashKey(key);
        auto it = ring.lower_bound(h);
        if (it == ring.end()) it = ring.begin(); // wrap around the ring
        return it->second;
    }
};

int main() {
    ConsistentHashRing ring;
    for (string s : {"server-A", "server-B", "server-C"}) ring.addServer(s);

    vector<string> keys = {"user:1001", "user:1002", "user:1003", "user:1004", "user:1005"};
    cout << "Key placement with 3 servers:\n";
    for (auto& k : keys) cout << "  " << k << " -> " << ring.getServer(k) << "\n";

    cout << "\nAdding server-D...\n";
    ring.addServer("server-D");
    cout << "Key placement after adding server-D (only some keys move):\n";
    for (auto& k : keys) cout << "  " << k << " -> " << ring.getServer(k) << "\n";
    return 0;
}
