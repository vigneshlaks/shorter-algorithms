# Database Internals

| Algorithm | File | Use Case |
|---|---|---|
| Skip List | `skip_list.cpp` | Probabilistic ordered structure; backs Redis's sorted sets |
| LSM Tree | `lsm_tree.cpp` | Write-optimized storage via sequential flush + compaction (Cassandra, RocksDB, LevelDB) |

Build and run individually with `g++ -std=c++17 -O2 <file>.cpp -o out && ./out`, or `make all` from the repo root.
