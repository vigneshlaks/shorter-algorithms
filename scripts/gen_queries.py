#!/usr/bin/env python3
"""
Generate realistic query distributions for the route server.

Modes:
  uniform   — random pairs across all nodes (baseline)
  skewed    — 80% of queries target 20% of nodes (simulates commute patterns)
  repeated  — mix of fresh queries and repeated ones (tests cache)

Usage:
  python3 scripts/gen_queries.py 1000 uniform <n_nodes>
  python3 scripts/gen_queries.py 1000 skewed  <n_nodes>
  python3 scripts/gen_queries.py 1000 repeated <n_nodes>
"""
import sys, random

def gen(n_queries, mode, n_nodes):
    rng = random.Random(42)

    if mode == "uniform":
        for _ in range(n_queries):
            s = rng.randint(0, n_nodes-1)
            g = rng.randint(0, n_nodes-1)
            while g == s:
                g = rng.randint(0, n_nodes-1)
            print(f"{s} {g}")

    elif mode == "skewed":
        # 20% of nodes get 80% of traffic — simulates popular destinations
        hot_nodes = rng.sample(range(n_nodes), max(1, n_nodes // 5))
        for _ in range(n_queries):
            if rng.random() < 0.8:
                g = rng.choice(hot_nodes)
            else:
                g = rng.randint(0, n_nodes-1)
            s = rng.randint(0, n_nodes-1)
            while s == g:
                s = rng.randint(0, n_nodes-1)
            print(f"{s} {g}")

    elif mode == "repeated":
        # Generate a pool of queries, then replay with repeats
        pool_size = max(10, n_queries // 10)
        pool = []
        for _ in range(pool_size):
            s = rng.randint(0, n_nodes-1)
            g = rng.randint(0, n_nodes-1)
            while g == s:
                g = rng.randint(0, n_nodes-1)
            pool.append((s, g))

        for _ in range(n_queries):
            if rng.random() < 0.4:  # 40% repeat from pool
                s, g = rng.choice(pool)
            else:
                s = rng.randint(0, n_nodes-1)
                g = rng.randint(0, n_nodes-1)
                while g == s:
                    g = rng.randint(0, n_nodes-1)
            print(f"{s} {g}")

if __name__ == "__main__":
    n_queries = int(sys.argv[1]) if len(sys.argv) > 1 else 100
    mode      = sys.argv[2]      if len(sys.argv) > 2 else "uniform"
    n_nodes   = int(sys.argv[3]) if len(sys.argv) > 3 else 72930
    gen(n_queries, mode, n_nodes)
