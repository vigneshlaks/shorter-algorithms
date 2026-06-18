// Vector Clocks — track causality ("what happened before what") across
// distributed nodes without a shared global clock.
// Use case: distributed databases (e.g., Riak, early DynamoDB) detect
//           conflicting concurrent writes by comparing vector clocks.
// Complexity: O(n) per comparison, n = number of nodes.
#include <iostream>
#include <vector>
#include <string>

using namespace std;

enum class Relation { BEFORE, AFTER, CONCURRENT, EQUAL };

class VectorClock {
public:
    vector<int> clock;
    int nodeId;

    VectorClock(int numNodes, int nodeId) : clock(numNodes, 0), nodeId(nodeId) {}

    void tick() { clock[nodeId]++; }

    void merge(const VectorClock& other) {
        for (size_t i = 0; i < clock.size(); ++i) clock[i] = max(clock[i], other.clock[i]);
        tick();
    }

    static Relation compare(const VectorClock& a, const VectorClock& b) {
        bool aLessEq = true, bLessEq = true;
        for (size_t i = 0; i < a.clock.size(); ++i) {
            if (a.clock[i] > b.clock[i]) aLessEq = false; // violates a <= b
            if (b.clock[i] > a.clock[i]) bLessEq = false; // violates b <= a
        }
        if (aLessEq && bLessEq) return Relation::EQUAL;
        if (aLessEq) return Relation::BEFORE;
        if (bLessEq) return Relation::AFTER;
        return Relation::CONCURRENT;
    }

    string toString() const {
        string s = "[";
        for (size_t i = 0; i < clock.size(); ++i) s += to_string(clock[i]) + (i + 1 < clock.size() ? "," : "");
        return s + "]";
    }
};

string relationStr(Relation r) {
    switch (r) {
        case Relation::BEFORE: return "happened-before";
        case Relation::AFTER: return "happened-after";
        case Relation::CONCURRENT: return "CONCURRENT (conflict!)";
        default: return "equal";
    }
}

int main() {
    // 3 nodes: A, B, C — simulate a small distributed write scenario
    VectorClock A(3, 0), B(3, 1), C(3, 2);

    A.tick(); // A writes locally: [1,0,0]
    B.tick(); // B writes locally, independently: [0,1,0]
    C.merge(A); // C learns about A's write
    C.tick();

    cout << "Node A clock: " << A.toString() << "\n";
    cout << "Node B clock: " << B.toString() << "\n";
    cout << "Node C clock: " << C.toString() << "\n\n";

    cout << "A vs B: " << relationStr(VectorClock::compare(A, B)) << "\n";
    cout << "A vs C: " << relationStr(VectorClock::compare(A, C)) << "\n";
    cout << "B vs C: " << relationStr(VectorClock::compare(B, C)) << "\n";
    return 0;
}
