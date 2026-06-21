#!/usr/bin/env python3
"""
Convert Overpass API JSON to a simple edge list with coordinates.

Output format:
  Line 1: <num_nodes> <num_edges>
  Next num_nodes lines: <idx> <lat> <lon>
  Next num_edges lines: <from_idx> <to_idx> <distance_metres>
"""
import json, sys, math
from collections import defaultdict

def haversine(lat1, lon1, lat2, lon2):
    R = 6371000
    phi1, phi2 = math.radians(lat1), math.radians(lat2)
    dphi = math.radians(lat2 - lat1)
    dlam = math.radians(lon2 - lon1)
    a = math.sin(dphi/2)**2 + math.cos(phi1)*math.cos(phi2)*math.sin(dlam/2)**2
    return R * 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))

def convert(in_path, out_path):
    with open(in_path) as f:
        data = json.load(f)

    coords = {}   # osm_id -> (lat, lon)
    for el in data['elements']:
        if el['type'] == 'node':
            coords[el['id']] = (el['lat'], el['lon'])

    # Build edge set (undirected, deduplicated)
    edges = set()
    for el in data['elements']:
        if el['type'] != 'way': continue
        nodes = el['nodes']
        for i in range(len(nodes) - 1):
            a, b = nodes[i], nodes[i+1]
            if a in coords and b in coords:
                edges.add((min(a,b), max(a,b)))

    # Map osm ids to dense indices
    node_ids = sorted(set(n for e in edges for n in e))
    idx = {osm: i for i, osm in enumerate(node_ids)}

    edge_list = []
    for a, b in edges:
        la, loa = coords[a]
        lb, lob = coords[b]
        d = haversine(la, loa, lb, lob)
        edge_list.append((idx[a], idx[b], d))

    with open(out_path, 'w') as f:
        f.write(f"{len(node_ids)} {len(edge_list)}\n")
        for osm in node_ids:
            i = idx[osm]
            lat, lon = coords[osm]
            f.write(f"{i} {lat} {lon}\n")
        for a, b, d in edge_list:
            f.write(f"{a} {b} {d:.2f}\n")

    print(f"Wrote {len(node_ids)} nodes, {len(edge_list)} edges to {out_path}")

if __name__ == '__main__':
    in_path  = sys.argv[1] if len(sys.argv) > 1 else 'data/sf_roads.json'
    out_path = sys.argv[2] if len(sys.argv) > 2 else 'data/sf_roads.txt'
    convert(in_path, out_path)
