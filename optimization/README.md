# Optimization Heuristics

| Algorithm | File | Use Case |
|---|---|---|
| Branch and Bound | `branch_and_bound_tsp.cpp` | Exact combinatorial optimization (used in commercial solvers like CPLEX/Gurobi) |
| Simulated Annealing | `simulated_annealing_tsp.cpp` | Escapes local optima via probabilistic "cooling" (circuit layout, scheduling) |
| Ant Colony Optimization | `ant_colony_tsp.cpp` | Swarm-based pathfinding (vehicle routing, network routing) |

All three demos solve the same small Traveling Salesman Problem instance so you can compare exact vs. approximate approaches directly. Build and run individually with `g++ -std=c++17 -O2 <file>.cpp -o out && ./out`, or `make all` from the repo root.
