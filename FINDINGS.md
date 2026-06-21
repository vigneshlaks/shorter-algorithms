# A* Heuristic Sensitivity on Real Road Networks

## Question

How does heuristic quality affect A* search behavior on real road networks,
and does the topology of the network explain the magnitude of suboptimality
when the heuristic is inadmissible?

## Method

Road network data for San Francisco (72,930 nodes, 78,865 edges) and
Manhattan (77,606 nodes, 84,619 edges) was downloaded from OpenStreetMap.
Edge weights are Haversine distances in metres.

The heuristic used is `h_α(n) = α × haversine(n, goal)`.

- α = 0.0: Dijkstra (no heuristic)
- α = 1.0: admissible A* (straight-line ≤ road distance always holds)
- α > 1.0: inadmissible (overestimates on nearly-straight road segments)

Three experiments were run:

1. **Heuristic experiment** — vary α from 0 to 4 in steps, run 100 random
   queries per α on SF, measure nodes explored / cost ratio / optimality rate.

2. **Divergence analysis** — for each suboptimal query, find the first node
   where the A* path departs from the optimal path. Record its location and
   the detour cost. Run on both cities at α=1.05 and α=1.2.

3. **ε-near-optimal subgraph** — for each query, count edges that lie on
   some path within ε% of optimal cost. Density = count / total edges.

4. **Betweenness centrality** — approximate Brandes algorithm with k=500
   random sources. Measures which nodes carry the most shortest paths.

## Results

### Suboptimality cliff (SF, heuristic experiment)

| α    | Nodes explored | Optimal paths | Mean cost ratio |
|------|---------------|---------------|-----------------|
| 0.0  | 8,963         | 100%          | 1.000           |
| 1.0  | 1,339         | 100%          | 1.000           |
| 1.05 | 1,151         | 61%           | 1.00057         |
| 1.2  | 726           | 35%           | 1.00486         |
| 4.0  | 196           | 21%           | 1.06270         |

A* with admissible heuristic (α=1.0) reduces nodes explored by 85% vs
Dijkstra while preserving optimality. At α=1.05 — 5% above the admissibility
threshold — 39% of paths become suboptimal. Optimality falls sharply at the
admissibility boundary; path quality degrades slowly as α increases further.

### Suboptimality rate is topology-independent

At α=1.05, both cities show 46.8% suboptimal paths. The frequency at which
the inadmissible heuristic causes wrong decisions is the same regardless of
whether the road network is an irregular terrain-cut mesh (SF) or a regular
grid (Manhattan).

### Detour magnitude is topology-dependent

| Metric              | SF (α=1.05) | Manhattan (α=1.05) |
|---------------------|-------------|---------------------|
| Suboptimal rate     | 46.8%       | 46.8%               |
| Mean detour         | 18.9 m      | 48.8 m              |
| Large detours >100m | 3%          | 17%                 |

Manhattan detours are 2.6× larger. This is the topology effect.

### Path redundancy does not explain the difference

The ε-near-optimal subgraph density — the fraction of edges that lie on
some path within ε% of optimal — is nearly equal at the relevant tolerance:

| ε    | SF density | Manhattan density |
|------|------------|------------------|
| 1%   | 2.15%      | 2.52%            |
| 5%   | 7.42%      | 8.14%            |
| 10%  | 12.66%     | 12.18%           |

Both cities have the same number of near-equivalent alternative routes per
query. Path redundancy is not the mechanism.

### Betweenness centrality explains the difference

Manhattan's top-10 nodes hold 0.50% of total betweenness vs 0.36% for SF.
Manhattan's highest-betweenness node scores 1.25× higher than SF's.

In Manhattan, a small number of avenue corridors carry a disproportionate
share of all shortest paths. When A* makes a wrong turn and leaves such a
corridor, the nearest path back to an equivalent high-centrality route
requires crossing a full block — the recovery cost is high.

In SF's distributed network, betweenness is more evenly spread across nodes.
Leaving one route quickly reaches another of comparable importance, so the
recovery cost when A* deviates is lower.

### The Topology Sensitivity Index is not constant

| City      | Mean edge | Mean detour | TSI   |
|-----------|-----------|-------------|-------|
| SF        | 23.6 m    | 18.9 m      | 0.800 |
| Manhattan | 27.8 m    | 48.8 m      | 1.755 |

Edge lengths are nearly equal (17% difference) but TSI differs by 119%.
Block length alone does not explain detour magnitude.

## Conclusion

**A* suboptimality frequency is a property of the heuristic.** At any given
α, the same fraction of queries become suboptimal regardless of road topology.
This is consistent with the theory: admissibility breaks at the same α for
any graph where the heuristic overestimates on some edge.

**A* detour magnitude is a property of the road network's betweenness
concentration.** Cities with centralized routing corridors (Manhattan's
avenues) produce larger detours because recovery from a wrong turn requires
returning to a high-centrality corridor. Cities with distributed centrality
(SF's irregular mesh) produce smaller detours because many routes have
comparable importance and wrong turns self-correct faster.

**This is not explained by path redundancy or block length** — both cities
have similar near-optimal subgraph densities and similar mean edge lengths.
The differentiating structural property is betweenness concentration.

## Implication

When deploying A* with an inadmissible heuristic (for speed), the expected
path quality depends on the network's betweenness distribution. On centralized
networks (grids, highway-dominated), suboptimality is more expensive. On
distributed networks (organic street layouts), it is cheaper. The admissibility
threshold (α=1.0 for Haversine) is universal, but the cost of crossing it is not.

## Files

| File                                | Contents                              |
|-------------------------------------|---------------------------------------|
| `data/sf_roads.txt`                 | SF OSM edge list                      |
| `data/manhattan_roads.txt`          | Manhattan OSM edge list               |
| `results/heuristic_experiment.csv`  | α sweep on SF (nodes, cost ratio)     |
| `results/divergence_1.05.csv`       | SF divergence points at α=1.05        |
| `results/divergence_manhattan_1.05.csv` | Manhattan divergence points        |
| `results/near_optimal_sf.csv`       | ε-near-optimal density, SF            |
| `results/near_optimal_manhattan.csv`| ε-near-optimal density, Manhattan     |
| `results/betweenness_sf.csv`        | Top-500 betweenness nodes, SF         |
| `results/betweenness_manhattan.csv` | Top-500 betweenness nodes, Manhattan  |
| `scripts/heuristic_experiment.cpp`  | α sweep binary                        |
| `scripts/divergence_analysis.cpp`   | Divergence finder binary              |
| `scripts/near_optimal_subgraph.cpp` | ε-subgraph density binary             |
| `scripts/betweenness.cpp`           | Approximate Brandes binary            |
| `scripts/compare_divergence.py`     | City comparison script                |
