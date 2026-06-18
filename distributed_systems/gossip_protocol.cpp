// Gossip Protocol — nodes periodically share state with random peers until
// information spreads through the whole cluster (epidemic-style propagation).
// Use case: failure detection and data propagation in Cassandra, DynamoDB,
//           and other peer-to-peer distributed systems.
// Complexity: O(log n) rounds to reach full propagation with high probability.
#include <iostream>
#include <vector>
#include <set>
#include <random>

using namespace std;

int main() {
    int numNodes = 10;
    vector<set<int>> knownInfo(numNodes); // each node's knowledge of "infected" nodes

    mt19937 rng(42);
    uniform_int_distribution<int> dist(0, numNodes - 1);

    knownInfo[0].insert(0); // node 0 originates a piece of info (e.g., "node 7 is down")
    int round = 0;
    int informed = 1;

    cout << "Gossip propagation simulation (" << numNodes << " nodes):\n";
    while (informed < numNodes && round < 20) {
        ++round;
        vector<set<int>> next = knownInfo;
        for (int i = 0; i < numNodes; ++i) {
            if (knownInfo[i].empty()) continue;
            int peer = dist(rng); // gossip to a random peer each round
            for (int info : knownInfo[i]) next[peer].insert(info);
        }
        knownInfo = next;
        informed = 0;
        for (auto& s : knownInfo) if (!s.empty()) ++informed;
        cout << "  Round " << round << ": " << informed << "/" << numNodes << " nodes informed\n";
    }
    cout << "Full propagation reached in " << round << " rounds (vs. O(log n) ~ " << (int)(log2(numNodes)) << " expected)\n";
    return 0;
}
