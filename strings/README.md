# String Algorithms

| Algorithm | File | Use Case |
|---|---|---|
| KMP (Knuth-Morris-Pratt) | `kmp_search.cpp` | Linear-time substring search (text editor "find") |
| Rabin-Karp | `rabin_karp.cpp` | Rolling-hash substring search; plagiarism detection |
| Trie | `trie.cpp` | Autocomplete, spell-check, IP routing tables |

Build and run individually with `g++ -std=c++17 -O2 <file>.cpp -o out && ./out`, or `make all` from the repo root.
