// Raft — Leader Election (simplified, single-process simulation).
// Use case: getting a cluster of machines to agree on a single leader despite
//           failures; the consensus core behind etcd, Consul, CockroachDB.
// Note: this models only the election sub-protocol of Raft (terms, votes,
//       randomized timeouts) as a deterministic simulation, not real networking.
#include <iostream>
#include <vector>
#include <random>

using namespace std;

enum class Role { FOLLOWER, CANDIDATE, LEADER };

struct Node {
    int id;
    int term = 0;
    Role role = Role::FOLLOWER;
    int votedFor = -1;
    int votesReceived = 0;
};

int main() {
    int n = 5;
    vector<Node> nodes(n);
    for (int i = 0; i < n; ++i) nodes[i].id = i;

    mt19937 rng(7);
    uniform_int_distribution<int> timeoutDist(150, 300); // randomized election timeout (ms), Raft's split-vote mitigation

    cout << "Simulating Raft leader election among " << n << " nodes.\n";

    // Node 2 times out first and starts an election (smallest randomized timeout)
    vector<int> timeouts(n);
    for (int i = 0; i < n; ++i) timeouts[i] = timeoutDist(rng);
    int candidateId = 0;
    for (int i = 1; i < n; ++i) if (timeouts[i] < timeouts[candidateId]) candidateId = i;

    cout << "Election timeouts (ms): ";
    for (int t : timeouts) cout << t << " ";
    cout << "\nNode " << candidateId << " times out first and becomes a CANDIDATE.\n";

    Node& candidate = nodes[candidateId];
    candidate.role = Role::CANDIDATE;
    candidate.term++;
    candidate.votedFor = candidate.id;
    candidate.votesReceived = 1; // votes for itself

    cout << "Node " << candidateId << " starts term " << candidate.term << " and requests votes.\n";

    // Other nodes vote yes if they haven't voted this term and the candidate's term is >= theirs
    for (int i = 0; i < n; ++i) {
        if (i == candidateId) continue;
        Node& voter = nodes[i];
        bool grantVote = (voter.votedFor == -1 || candidate.term > voter.term);
        if (grantVote) {
            voter.term = candidate.term;
            voter.votedFor = candidateId;
            candidate.votesReceived++;
            cout << "  Node " << i << " votes YES\n";
        } else {
            cout << "  Node " << i << " votes NO\n";
        }
    }

    int majority = n / 2 + 1;
    if (candidate.votesReceived >= majority) {
        candidate.role = Role::LEADER;
        cout << "\nNode " << candidateId << " received " << candidate.votesReceived
             << "/" << n << " votes (majority = " << majority << ") -> becomes LEADER for term " << candidate.term << ".\n";
    } else {
        cout << "\nElection failed to reach majority; a new term would start.\n";
    }
    return 0;
}
