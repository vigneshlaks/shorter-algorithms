#!/usr/bin/env python3
"""
Plot heuristic quality experiment results.

Usage:
    ./heuristic_experiment data/sf_roads.txt | python3 scripts/plot_experiment.py
    python3 scripts/plot_experiment.py results.csv
"""
import sys
import csv
import json

def load(path):
    rows = []
    with open(path) as f:
        for row in csv.DictReader(f):
            rows.append({k: float(v) for k, v in row.items()})
    return rows

def render(rows):
    alphas       = [r['alpha'] for r in rows]
    nodes        = [r['mean_nodes'] for r in rows]
    ratios       = [r['mean_cost_ratio'] for r in rows]
    times        = [r['mean_time_ms'] for r in rows]
    pct_optimal  = [r['pct_optimal'] for r in rows]
    std_nodes    = [r['std_nodes'] for r in rows]
    std_ratios   = [r['std_cost_ratio'] for r in rows]

    # Normalize nodes to Dijkstra (α=0) baseline
    baseline_nodes = nodes[0] if nodes[0] > 0 else 1
    nodes_norm = [n / baseline_nodes for n in nodes]
    std_nodes_norm = [s / baseline_nodes for s in std_nodes]

    print("=== Heuristic Quality Experiment Results ===\n")
    print(f"{'α':>6}  {'Nodes (norm)':>13}  {'Cost ratio':>10}  "
          f"{'Time (ms)':>10}  {'% Optimal':>10}")
    print("-" * 58)
    for i, r in enumerate(rows):
        marker = " ← optimal A*" if abs(r['alpha'] - 1.0) < 0.01 else ""
        marker = " ← Dijkstra"   if abs(r['alpha'] - 0.0) < 0.01 else marker
        print(f"{r['alpha']:>6.2f}  {nodes_norm[i]:>13.3f}  "
              f"{ratios[i]:>10.4f}  {times[i]:>10.3f}  "
              f"{pct_optimal[i]:>10.1f}%{marker}")

    # Find the α where optimality first drops below 100%
    first_subopt = next(
        (r['alpha'] for r in rows if r['pct_optimal'] < 100 and r['alpha'] > 0),
        None
    )
    if first_subopt:
        print(f"\nOptimality first breaks at α ≈ {first_subopt:.2f}")

    # Find α where nodes explored drops to 10% of Dijkstra
    tenth = next(
        (r['alpha'] for r, n in zip(rows, nodes_norm) if n <= 0.10 and r['alpha'] > 0),
        None
    )
    if tenth:
        print(f"Nodes drop to ≤10% of Dijkstra at α ≈ {tenth:.2f}")

    # Speedup at α=1
    a1 = next((r for r in rows if abs(r['alpha'] - 1.0) < 0.01), None)
    a0 = rows[0]
    if a1 and a0:
        speedup_nodes = a0['mean_nodes'] / a1['mean_nodes']
        speedup_time  = a0['mean_time_ms'] / a1['mean_time_ms']
        print(f"\nAt α=1.0 (optimal A*):")
        print(f"  {speedup_nodes:.1f}x fewer nodes than Dijkstra")
        print(f"  {speedup_time:.1f}x faster than Dijkstra")
        print(f"  {a1['pct_optimal']:.0f}% of queries return optimal path")

    # Write JSON summary for further analysis
    summary = {
        "n_alphas": len(rows),
        "first_suboptimal_alpha": first_subopt,
        "nodes_10pct_alpha": tenth,
        "dijkstra_mean_nodes": a0['mean_nodes'],
        "optimal_astar_mean_nodes": a1['mean_nodes'] if a1 else None,
        "optimal_astar_speedup_nodes": speedup_nodes if a1 else None,
        "data": rows
    }
    out = "results/heuristic_experiment.json"
    import os; os.makedirs("results", exist_ok=True)
    with open(out, "w") as f:
        json.dump(summary, f, indent=2)
    print(f"\nFull results written to {out}")

def main():
    if len(sys.argv) > 1:
        rows = load(sys.argv[1])
    else:
        import io
        data = sys.stdin.read()
        rows = []
        reader = csv.DictReader(io.StringIO(data))
        for row in reader:
            rows.append({k: float(v) for k, v in row.items()})

    if not rows:
        print("No data. Run heuristic_experiment first.")
        sys.exit(1)

    render(rows)

if __name__ == "__main__":
    main()
