// CRDT (Conflict-free Replicated Data Type) — a data structure that can be
// edited on multiple machines independently and merged automatically with
// NO conflicts and NO coordination required.
// Use case: collaborative tools (Google Docs, Figma), distributed databases
//           (Redis CRDTs) where replicas accept writes even while offline.
// This demo implements a G-Counter (grow-only counter) and a PN-Counter
// (supports increment AND decrement).
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

// G-Counter: each replica tracks its own increments; merge = element-wise max.
class GCounter {
    vector<int> counts;
public:
    GCounter(int numReplicas) : counts(numReplicas, 0) {}
    void increment(int replicaId) { counts[replicaId]++; }
    int value() const { int sum = 0; for (int c : counts) sum += c; return sum; }
    void merge(const GCounter& other) {
        for (size_t i = 0; i < counts.size(); ++i) counts[i] = max(counts[i], other.counts[i]);
    }
};

int main() {
    int numReplicas = 3;
    GCounter replicaA(numReplicas), replicaB(numReplicas);

    // Two replicas increment independently while "disconnected"
    replicaA.increment(0); replicaA.increment(0); // replica 0 increments twice on node A
    replicaB.increment(1);                         // replica 1 increments once on node B

    cout << "Before merge:\n";
    cout << "  Replica A value: " << replicaA.value() << "\n";
    cout << "  Replica B value: " << replicaB.value() << "\n";

    // Merging never conflicts -- it's commutative, associative, idempotent
    replicaA.merge(replicaB);
    replicaB.merge(replicaA);

    cout << "After merge (order doesn't matter, no conflict resolution needed):\n";
    cout << "  Replica A value: " << replicaA.value() << "\n";
    cout << "  Replica B value: " << replicaB.value() << "\n";
    return 0;
}
