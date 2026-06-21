#!/usr/bin/env python3
"""
Compare divergence clustering between SF and Manhattan.

The Twin Peaks hypothesis: divergence points in SF cluster geographically
because a topographic obstacle (Twin Peaks hill) forces suboptimal routing.
Manhattan (flat grid) should show no such clustering.

We measure clustering via:
  1. Mean distance from each divergence point to the centroid
  2. Fraction of points within 500m of the centroid
  3. Standard deviation of lat/lon
"""
import csv, math, sys, json

def haversine(la1, lo1, la2, lo2):
    R = 6371000
    p1, p2 = math.radians(la1), math.radians(la2)
    dp = math.radians(la2 - la1)
    dl = math.radians(lo2 - lo1)
    a = math.sin(dp/2)**2 + math.cos(p1)*math.cos(p2)*math.sin(dl/2)**2
    return R * 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))

def load_csv(path):
    points = []
    with open(path) as f:
        for row in csv.DictReader(f):
            points.append((float(row["diverge_lat"]), float(row["diverge_lon"]),
                           float(row["cost_ratio"]), float(row["detour_m"])))
    return points

def cluster_stats(points):
    lats = [p[0] for p in points]
    lons = [p[1] for p in points]
    centroid_lat = sum(lats) / len(lats)
    centroid_lon = sum(lons) / len(lons)

    dists = [haversine(la, lo, centroid_lat, centroid_lon) for la, lo, *_ in points]
    mean_dist  = sum(dists) / len(dists)
    max_dist   = max(dists)
    pct_500m   = 100 * sum(1 for d in dists if d < 500) / len(dists)

    # Spread in degrees — smaller = more clustered
    lat_std = math.sqrt(sum((la - centroid_lat)**2 for la in lats) / len(lats))
    lon_std = math.sqrt(sum((lo - centroid_lon)**2 for lo in lons) / len(lons))

    # Find the hotspot: 0.01-degree grid cell with most points
    grid = {}
    for la, lo, *_ in points:
        cell = (round(la, 2), round(lo, 2))
        grid[cell] = grid.get(cell, 0) + 1
    hotspot_cell, hotspot_count = max(grid.items(), key=lambda x: x[1])
    hotspot_pct = 100 * hotspot_count / len(points)

    return {
        "n": len(points),
        "centroid": (centroid_lat, centroid_lon),
        "mean_dist_to_centroid_m": mean_dist,
        "max_dist_to_centroid_m": max_dist,
        "pct_within_500m_of_centroid": pct_500m,
        "lat_std_deg": lat_std,
        "lon_std_deg": lon_std,
        "hotspot_cell": hotspot_cell,
        "hotspot_pct": hotspot_pct,
        "mean_cost_ratio": sum(p[2] for p in points) / len(points),
        "mean_detour_m": sum(p[3] for p in points) / len(points),
    }

datasets = [
    ("SF",        "α=1.05", "results/divergence_1.05.csv"),
    ("SF",        "α=1.2",  "results/divergence_1.2.csv"),
    ("Manhattan", "α=1.05", "results/divergence_manhattan_1.05.csv"),
    ("Manhattan", "α=1.2",  "results/divergence_manhattan_1.2.csv"),
]

results = []
for city, alpha, path in datasets:
    pts = load_csv(path)
    s   = cluster_stats(pts)
    s["city"]  = city
    s["alpha"] = alpha
    results.append(s)

print(f"\n{'='*72}")
print(f"  Divergence Clustering: SF (topographic) vs Manhattan (flat grid)")
print(f"{'='*72}\n")

header = f"{'City':<12} {'α':<8} {'N':>5} {'Mean dist':>12} {'% <500m':>9} {'Hotspot%':>10} {'Lat σ':>8}"
print(header)
print("-" * 72)
for r in results:
    print(f"{r['city']:<12} {r['alpha']:<8} {r['n']:>5} "
          f"{r['mean_dist_to_centroid_m']:>10.0f}m "
          f"{r['pct_within_500m_of_centroid']:>8.1f}% "
          f"{r['hotspot_pct']:>9.1f}% "
          f"{r['lat_std_deg']:>7.4f}°")

print(f"\nHotspot locations (0.01° grid cell with most divergences):")
for r in results:
    la, lo = r["hotspot_cell"]
    print(f"  {r['city']} {r['alpha']}: ({la:.3f}, {lo:.3f})  "
          f"{r['hotspot_pct']:.1f}% of divergences in one cell")

print(f"\nDetour severity:")
for r in results:
    print(f"  {r['city']} {r['alpha']}: mean detour {r['mean_detour_m']:.1f}m  "
          f"cost ratio {r['mean_cost_ratio']:.5f}")

print(f"\nInterpretation:")
sf105   = next(r for r in results if r["city"]=="SF"        and r["alpha"]=="α=1.05")
mn105   = next(r for r in results if r["city"]=="Manhattan" and r["alpha"]=="α=1.05")
sf12    = next(r for r in results if r["city"]=="SF"        and r["alpha"]=="α=1.2")
mn12    = next(r for r in results if r["city"]=="Manhattan" and r["alpha"]=="α=1.2")

ratio_105 = mn105["mean_dist_to_centroid_m"] / max(1, sf105["mean_dist_to_centroid_m"])
ratio_12  = mn12["mean_dist_to_centroid_m"]  / max(1, sf12["mean_dist_to_centroid_m"])
print(f"  At α=1.05: Manhattan divergences spread {ratio_105:.1f}x wider than SF")
print(f"  At α=1.2:  Manhattan divergences spread {ratio_12:.1f}x wider than SF")

if ratio_105 > 1.5:
    print(f"\n  CONFIRMED: SF divergences cluster more tightly than Manhattan.")
    print(f"  SF clustering is not a graph-centrality artifact — it is terrain-driven.")
    print(f"  The Twin Peaks hill forces suboptimal routing in A* with α>1.")
else:
    print(f"\n  INCONCLUSIVE or NULL: Both cities show similar clustering.")
    print(f"  The SF hotspot may reflect graph centrality, not terrain.")

with open("results/divergence_comparison.json", "w") as f:
    # Make centroid JSON-serializable
    for r in results:
        r["centroid"] = list(r["centroid"])
        r["hotspot_cell"] = list(r["hotspot_cell"])
    json.dump(results, f, indent=2)
print(f"\n  Full data: results/divergence_comparison.json")
