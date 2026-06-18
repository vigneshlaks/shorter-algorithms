# Distributed Systems

| Algorithm | File | Use Case |
|---|---|---|
| Vector Clocks | `vector_clocks.cpp` | Causality tracking across nodes without a shared clock (conflict detection) |
| Gossip Protocol | `gossip_protocol.cpp` | Epidemic-style state propagation (Cassandra, DynamoDB failure detection) |
| Consistent Hashing | `consistent_hashing.cpp` | Minimal-reshuffle data distribution across servers (caches, CDNs, sharding) |
| Raft (Leader Election) | `raft_leader_election.cpp` | Cluster consensus on a single leader (etcd, Consul, CockroachDB) |
| CRDT (G-Counter) | `crdt_counter.cpp` | Conflict-free merging of concurrent edits (Google Docs, Figma, distributed DBs) |

Note: `raft_leader_election.cpp` models only the election sub-protocol (terms, randomized timeouts, majority voting) as a deterministic single-process simulation, not real networking or log replication.

Build and run individually with `g++ -std=c++17 -O2 <file>.cpp -o out && ./out`, or `make all` from the repo root.
