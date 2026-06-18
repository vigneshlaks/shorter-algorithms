// HyperLogLog — estimates the number of DISTINCT elements in a massive
// dataset using very little memory (a few KB for billions of items).
// Use case: "unique visitors" counters in analytics systems (Redis, BigQuery),
//           counting distinct IPs/queries at internet scale.
// Complexity: O(1) per insert, O(m) space for m registers.
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <cstdint>

using namespace std;

class HyperLogLog {
    vector<int> registers;
    int numRegisters;
    int b; // number of bits used for register index

    int leadingZeros(uint64_t x, int maxBits) const {
        int count = 0;
        for (int i = maxBits - 1; i >= 0; --i) {
            if ((x >> i) & 1) break;
            ++count;
        }
        return count + 1;
    }

public:
    HyperLogLog(int b) : b(b), numRegisters(1 << b), registers(1 << b, 0) {}

    void add(const string& item) {
        uint64_t h = hash<string>{}(item);
        int idx = h >> (64 - b);                  // top b bits select the register
        uint64_t rest = h << b;                    // remaining bits
        int rank = leadingZeros(rest, 64 - b);
        registers[idx] = max(registers[idx], rank);
    }

    double estimate() const {
        double sum = 0;
        for (int r : registers) sum += pow(2.0, -r);
        double alpha = 0.7213 / (1 + 1.079 / numRegisters);
        double raw = alpha * numRegisters * numRegisters / sum;
        return raw;
    }
};

int main() {
    HyperLogLog hll(10); // 1024 registers ~ a few KB

    // Simulate 50,000 events with only 5,000 truly distinct visitor IDs
    int trueDistinct = 5000;
    for (int i = 0; i < 50000; ++i) {
        hll.add("visitor_" + to_string(i % trueDistinct));
    }

    double est = hll.estimate();
    cout << "True distinct count: " << trueDistinct << "\n";
    cout << "HyperLogLog estimate: " << (long long)est << "\n";
    cout << "Error: " << (100.0 * abs(est - trueDistinct) / trueDistinct) << "%\n";
    cout << "Memory used: " << (1 << 10) << " registers (~a few KB) regardless of stream size\n";
    return 0;
}
