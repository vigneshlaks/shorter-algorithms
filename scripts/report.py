#!/usr/bin/env python3
"""
report.py — Render benchmark results from bench_results.json as a table.

Usage:
    python scripts/report.py [bench_results.json]
    python scripts/report.py --compare run1.json run2.json
"""

import json
import sys
import os
from collections import defaultdict


def load(path: str) -> list[dict]:
    with open(path) as f:
        return json.load(f)


def render_table(results: list[dict], label: str = "") -> None:
    if label:
        print(f"\n{'='*60}")
        print(f"  {label}")
        print(f"{'='*60}")

    grouped: dict[str, dict[int, float]] = defaultdict(dict)
    ns: set[int] = set()
    for r in results:
        grouped[r["algo"]][r["n"]] = r["ms"]
        ns.add(r["n"])

    ns_sorted = sorted(ns)
    col_w = 14
    header = f"{'Algorithm':<28}" + "".join(f"{'n='+str(n):>{col_w}}" for n in ns_sorted)
    print(header)
    print("-" * len(header))

    for algo, times in sorted(grouped.items()):
        row = f"{algo:<28}"
        for n in ns_sorted:
            ms = times.get(n)
            row += f"{ms:>{col_w}.3f}" if ms is not None else f"{'—':>{col_w}}"
        print(row)

    print(f"\nAll times in milliseconds (ms). Lower is better.\n")


def render_compare(path1: str, path2: str) -> None:
    r1 = {(r["algo"], r["n"]): r["ms"] for r in load(path1)}
    r2 = {(r["algo"], r["n"]): r["ms"] for r in load(path2)}

    keys = sorted(set(r1) | set(r2))
    print(f"\n{'='*70}")
    print(f"  Comparison: {os.path.basename(path1)} vs {os.path.basename(path2)}")
    print(f"{'='*70}")
    print(f"{'Algorithm':<28} {'N':>10} {'Run 1 (ms)':>12} {'Run 2 (ms)':>12} {'Delta':>10}")
    print("-" * 74)

    for (algo, n) in keys:
        ms1 = r1.get((algo, n))
        ms2 = r2.get((algo, n))
        s1 = f"{ms1:.3f}" if ms1 is not None else "—"
        s2 = f"{ms2:.3f}" if ms2 is not None else "—"
        if ms1 and ms2:
            pct = (ms2 - ms1) / ms1 * 100
            delta = f"{pct:+.1f}%"
        else:
            delta = "—"
        print(f"{algo:<28} {n:>10} {s1:>12} {s2:>12} {delta:>10}")


def main() -> None:
    args = sys.argv[1:]

    if "--compare" in args:
        idx = args.index("--compare")
        paths = args[idx+1:]
        if len(paths) < 2:
            print("Usage: report.py --compare <file1.json> <file2.json>")
            sys.exit(1)
        render_compare(paths[0], paths[1])
        return

    path = args[0] if args else "bench_results.json"
    if not os.path.exists(path):
        print(f"File not found: {path}")
        print("Run `./bench` first to generate results.")
        sys.exit(1)

    render_table(load(path), label=os.path.basename(path))


if __name__ == "__main__":
    main()
