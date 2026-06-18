# Probabilistic Data Structures

| Algorithm | File | Use Case |
|---|---|---|
| Bloom Filter | `bloom_filter.cpp` | Fast "definitely not present" checks (browser malicious-URL lists, DB skip-reads) |
| Count-Min Sketch | `count_min_sketch.cpp` | Frequency estimation in huge streams (network monitoring, query planners) |
| Cuckoo Filter | `cuckoo_filter.cpp` | Like a Bloom filter but supports deletion (dynamic caches, packet filtering) |
| HyperLogLog | `hyperloglog.cpp` | Distinct-count estimation at internet scale (unique visitors, Redis/BigQuery) |

All trade a small, bounded error rate for massive memory savings over exact data structures. Build and run individually with `g++ -std=c++17 -O2 <file>.cpp -o out && ./out`, or `make all` from the repo root.
