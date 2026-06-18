// Count-Min Sketch — estimates item frequencies in a massive stream using
// fixed, tiny memory (never undercounts, may overcount due to collisions).
// Use case: network traffic monitoring (top talkers), database query
//           planners estimating selectivity, trending-topic detection.
// Complexity: O(k) per update/query, independent of stream size.
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <climits>

using namespace std;

class CountMinSketch {
    vector<vector<int>> table;
    int width, depth;

    size_t hashItem(const string& item, int row) const {
        size_t h = hash<string>{}(item);
        h ^= row * 0x9e3779b9;
        return h % width;
    }

public:
    CountMinSketch(int width, int depth) : table(depth, vector<int>(width, 0)), width(width), depth(depth) {}

    void add(const string& item, int count = 1) {
        for (int r = 0; r < depth; ++r) table[r][hashItem(item, r)] += count;
    }

    int estimate(const string& item) const {
        int minCount = INT_MAX;
        for (int r = 0; r < depth; ++r) minCount = min(minCount, table[r][hashItem(item, r)]);
        return minCount;
    }
};

int main() {
    CountMinSketch cms(50, 4);

    // Simulate a stream of network requests
    vector<pair<string,int>> stream = {{"192.168.1.1", 150}, {"10.0.0.5", 30}, {"192.168.1.1", 50}, {"8.8.8.8", 5}};
    for (auto& [ip, count] : stream) cms.add(ip, count);

    cout << "Estimated request counts (Count-Min Sketch):\n";
    for (string ip : {"192.168.1.1", "10.0.0.5", "8.8.8.8", "1.1.1.1"}) {
        cout << "  " << ip << ": ~" << cms.estimate(ip) << " requests\n";
    }
    cout << "(Real total for 192.168.1.1 was 200 — estimate never undercounts)\n";
    return 0;
}
