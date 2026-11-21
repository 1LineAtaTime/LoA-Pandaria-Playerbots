"""Runs a fixed set of path queries on several maps and prints a Markdown table.

    python -I validate.py [--data <server DataDir>] [--step 5]

Columns: straight-line distance, path length, point count, largest gap between consecutive points,
largest height step between consecutive points, polys in the corridor, chained findPath rounds,
tiles loaded by this query, partial flag, distance of the destination from the mesh, and the time of
the first (cold, tiles read from disk) and second (warm, tiles cached) run of the same query.
"""

import argparse
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nav  # noqa: E402

CASES = [
    # (label, map, start, end)
    ("Northshire abbey steps -> kobold mine", 0, (-8914.57, -133.91, 80.54), (-8775.0, -185.0, 84.0)),
    ("Northshire abbey -> Goldshire inn (indoors)", 0, (-8914.57, -133.91, 80.54), (-9462.7, 16.2, 57.0)),
    ("Goldshire -> Lakeshire town hall", 0, (-9465.5, 74.0, 56.8), (-9218.6, -2221.8, 67.6)),
    ("Gilneas start -> -1734.6,1657.2,20.6", 654, (-1446.6, 1407.4, 35.7), (-1734.6, 1657.2, 20.6)),
    ("Gilneas Walden -> Krennan Aranas (across city)", 654, (-1405.5, 1447.4, 35.6), (-1727.6, 1871.8, 17.9)),
    ("Echo Isles start -> Tora'jin", 1, (-1171.45, -5263.65, 0.85), (-1282.2, -5585.0, 21.1)),
    ("Valley of Trials -> Razor Hill inn", 1, (-618.52, -4251.67, 38.72), (340.4, -4686.3, 16.6)),
    ("Wandering Isle start -> Ji Firepaw", 860, (1470.97, 3466.06, 181.64), (1407.1, 3637.3, 83.5)),
    ("Wandering Isle start -> Jojo Ironbrow", 860, (1470.97, 3466.06, 181.64), (1039.5, 3283.1, 129.5)),
    ("Kezan start -> Coach Crosscheck", 648, (-8423.81, 1361.3, 104.67), (-8250.0, 1470.0, 42.8)),
    ("Acherus start -> Razuvious (same floor)", 609, (2355.84, -5664.77, 426.03), (2486.9, -5566.6, 420.5)),
    ("Acherus start -> Siouxsie (floor below)", 609, (2355.84, -5664.77, 426.03), (2379.4, -5648.5, 382.6)),
    ("Acherus start -> Salanar (ground)", 609, (2355.84, -5664.77, 426.03), (2351.8, -5701.3, 154.0)),
]


def dist(a, b):
    return math.sqrt(sum((x - y) ** 2 for x, y in zip(a, b)))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--data")
    ap.add_argument("--step", type=float, default=5.0)
    a = ap.parse_args()

    client = nav.NavClient(data_dir=a.data)
    print("| case | map | straight yd | path yd | points | max gap | max dz | polys | rounds | tiles | partial | end off-mesh yd | cold ms | warm ms |")
    print("|---|---|---|---|---|---|---|---|---|---|---|---|---|---|")
    for label, map_id, start, end in CASES:
        r = client.query_path(map_id, start, end, step=a.step)
        warm = client.query_path(map_id, start, end, step=a.step)
        if not r.get("ok"):
            print("| %s | %d | %.0f | - | - | - | - | - | - | %d | **no path: %s** | - | %.1f | %.1f |" % (
                label, map_id, dist(start, end), r.get("tiles_loaded", 0), r.get("reason"), r.get("ms", 0), warm.get("ms", 0)))
            continue
        pts = r["path"]
        gaps = [dist(pts[i], pts[i + 1]) for i in range(len(pts) - 1)] or [0.0]
        dzs = [abs(pts[i][2] - pts[i + 1][2]) for i in range(len(pts) - 1)] or [0.0]
        partial = "yes" if r["partial"] else "no"
        if r["partial"]:
            partial += " (stops %.0f yd short)" % dist(r["end"], end)
        print("| %s | %d | %.0f | %.0f | %d | %.1f | %.1f | %d | %d | %d | %s | %.1f | %.1f | %.1f |" % (
            label, map_id, dist(start, end), r["length"], r["points"], max(gaps), max(dzs), r["polys"],
            r["rounds"], r["tiles_loaded"], partial, r["end_dist"], r["ms"], warm["ms"]))
    st = client.stats()
    print()
    print("tiles cached: " + ", ".join("map %d: %d" % (m["map"], m["tiles"]) for m in st.get("maps", [])))
    client.close()


if __name__ == "__main__":
    main()
