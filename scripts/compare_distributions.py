#!/usr/bin/env python3
"""
Run the route server under all three query distributions and compare results.

Usage: python3 scripts/compare_distributions.py [n_queries] [graph_path]
"""
import subprocess, sys, json, statistics, os

N        = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
GRAPH    = sys.argv[2]      if len(sys.argv) > 2 else "data/sf_roads.txt"
N_NODES  = 72930
SERVER   = "build/route_server"
GEN      = "scripts/gen_queries.py"

def run_distribution(mode, n, graph):
    gen = subprocess.Popen(
        ["python3", GEN, str(n), mode, str(N_NODES)],
        stdout=subprocess.PIPE)
    srv = subprocess.Popen(
        [SERVER, graph],
        stdin=gen.stdout,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)
    gen.stdout.close()
    out, err = srv.communicate()

    responses = []
    for line in out.decode().splitlines():
        line = line.strip()
        if line.startswith("{"):
            try: responses.append(json.loads(line))
            except: pass

    # Parse final stats from stderr
    stderr = err.decode()
    stats = {}
    for line in stderr.splitlines():
        if "Cache hit rate:" in line:
            stats["cache_hit_rate"] = float(line.split(":")[1].strip().rstrip("%"))
        if "Mean nodes/query:" in line:
            parts = line.split()
            stats["mean_nodes"] = int(parts[2])
        if "Mean latency:" in line:
            stats["mean_latency_ms"] = float(line.split(":")[1].strip().rstrip(" ms"))
        if "Suboptimal" in line:
            stats["suboptimal_pct"] = float(line.split(":")[1].strip().rstrip("%").split("%")[0])
        if "mean α=" in line:
            stats["mean_alpha"] = float(line.split("mean α=")[1])
        if "Cache size:" in line:
            stats["cache_size"] = int(line.split(":")[1].strip().split()[0])

    stats["responses"] = len(responses)
    stats["mode"] = mode

    # Compute latency distribution from responses
    latencies = [r["ms"] for r in responses if r.get("cache") == "miss" and r["ms"] > 0]
    if latencies:
        stats["p50_ms"]  = statistics.median(latencies)
        stats["p95_ms"]  = sorted(latencies)[int(len(latencies)*0.95)]
        stats["p99_ms"]  = sorted(latencies)[int(len(latencies)*0.99)]

    cache_hits = [r for r in responses if r.get("cache") == "hit"]
    stats["actual_cache_hits"] = len(cache_hits)
    stats["actual_cache_rate"] = 100.0 * len(cache_hits) / max(1, len(responses))

    return stats

def print_comparison(results):
    print(f"\n{'='*65}")
    print(f"  Distribution Comparison — {N} queries each on SF road network")
    print(f"{'='*65}\n")

    metrics = [
        ("Cache hit rate",   "actual_cache_rate",  "{:.1f}%"),
        ("Mean nodes/query", "mean_nodes",          "{:,}"),
        ("Mean latency",     "mean_latency_ms",     "{:.3f} ms"),
        ("p50 latency",      "p50_ms",              "{:.3f} ms"),
        ("p95 latency",      "p95_ms",              "{:.3f} ms"),
        ("p99 latency",      "p99_ms",              "{:.3f} ms"),
        ("Suboptimal %",     "suboptimal_pct",      "{:.1f}%"),
        ("Settled α",        "mean_alpha",          "{:.3f}"),
        ("Cache size",       "cache_size",          "{:,} routes"),
    ]

    modes = [r["mode"] for r in results]
    print(f"{'Metric':<22}" + "".join(f"{m:>14}" for m in modes))
    print("-" * (22 + 14*len(modes)))

    for label, key, fmt in metrics:
        row = f"{label:<22}"
        for r in results:
            val = r.get(key)
            row += f"{(fmt.format(val) if val is not None else '—'):>14}"
        print(row)

    print(f"\nKey findings:")

    # Cache comparison
    uniform = next(r for r in results if r["mode"] == "uniform")
    repeated = next(r for r in results if r["mode"] == "repeated")
    skewed = next(r for r in results if r["mode"] == "skewed")

    if uniform and repeated:
        speedup = uniform["mean_latency_ms"] / max(0.001, repeated["mean_latency_ms"])
        print(f"  Repeated queries {speedup:.1f}x faster than uniform due to {repeated['actual_cache_rate']:.0f}% cache hit rate")

    if uniform and skewed:
        diff = skewed["actual_cache_rate"] - uniform["actual_cache_rate"]
        print(f"  Skewed distribution: {diff:+.1f}% cache hit rate vs uniform")

    # Save JSON
    os.makedirs("results", exist_ok=True)
    out = "results/distribution_comparison.json"
    with open(out, "w") as f:
        json.dump({"n_queries": N, "graph": GRAPH, "results": results}, f, indent=2)
    print(f"\n  Full results: {out}")

if __name__ == "__main__":
    results = []
    for mode in ["uniform", "skewed", "repeated"]:
        print(f"Running {mode} ({N} queries)...", flush=True)
        r = run_distribution(mode, N, GRAPH)
        results.append(r)
        print(f"  Done — cache={r.get('actual_cache_rate',0):.1f}% "
              f"latency={r.get('mean_latency_ms',0):.3f}ms "
              f"α={r.get('mean_alpha',1.0):.3f}")

    print_comparison(results)
