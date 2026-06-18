# Sorting & Searching

| Algorithm | File | Use Case |
|---|---|---|
| Merge Sort | `merge_sort.cpp` | Stable O(n log n) sort; multi-key sorting, external sorting |
| Quicksort | `quick_sort.cpp` | Fast average-case sort; default in many standard libraries |
| Heapsort | `heap_sort.cpp` | In-place O(n log n) sort, no extra memory |
| Binary Search | `binary_search.cpp` | O(log n) lookup in sorted data; database index lookups |

Build and run individually with `g++ -std=c++17 -O2 <file>.cpp -o out && ./out`, or `make all` from the repo root.
