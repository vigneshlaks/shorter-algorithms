# Graph Algorithms

| Algorithm | File | Use Case |
|---|---|---|
| Dijkstra's Algorithm | `dijkstra.cpp` | Shortest path, non-negative weights (GPS routing, network routing) |
| Bellman-Ford | `bellman_ford.cpp` | Shortest path with negative weights; negative-cycle detection (arbitrage) |
| Floyd-Warshall | `floyd_warshall.cpp` | All-pairs shortest paths (precomputed transit network distances) |
| BFS Shortest Path | `bfs_shortest_path.cpp` | Unweighted shortest path, connected components, "degrees of separation" |
| Union-Find | `union_find.cpp` | Connectivity queries, cycle detection |
| Kruskal's Algorithm | `kruskal_mst.cpp` | Minimum spanning tree (network design, clustering) |
| Topological Sort | `topological_sort.cpp` | Dependency ordering (build systems, course prerequisites) |
| Tarjan's Algorithm | `tarjan_scc.cpp` | Strongly connected components (compiler analysis, feedback loops) |
| Ford-Fulkerson / Edmonds-Karp | `ford_fulkerson_maxflow.cpp` | Maximum flow (bipartite matching, bandwidth allocation) |
| PageRank | `pagerank.cpp` | Importance ranking over a graph (web search, citation networks) |

Each file is self-contained — build and run individually with:
```
g++ -std=c++17 -O2 dijkstra.cpp -o dijkstra && ./dijkstra
```
Or build everything from the repo root with `make all`.
