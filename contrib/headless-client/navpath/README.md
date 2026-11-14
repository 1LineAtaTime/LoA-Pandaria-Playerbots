# navpath: navmesh paths for the headless client

`navpath` is a small native tool that answers path queries on the world server's own movement
maps (`mmaps/*.mmap`, `mmaps/*.mmtile`). With it a scripted client can walk from A to B along
the ground the way a real player would, sending ordinary movement packets, instead of using GM
teleports. `nav.py` is the Python front end that the headless client imports.

It is built only from the Detour runtime in `dep/recastnavigation`, the same library the server
uses. No server code or libraries are linked.

## Build

Windows, with Visual Studio 2022 (C++ workload), CMake and Ninja on `PATH`:

```
contrib\headless-client\navpath\build.cmd [jobs]      (default 4 parallel jobs)
```

This finds Visual Studio with `vswhere`, loads `vcvars64.bat`, configures with Ninja and builds
`build\navpath.exe`. The exe links the CRT statically, so it needs no redistributable. Building
by hand works the same way on any platform:

```
cmake -S contrib/headless-client/navpath -B contrib/headless-client/navpath/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build contrib/headless-client/navpath/build -j 4
```

`NAVPATH_RECAST_DIR` overrides where the recastnavigation sources are found. The default is
`../../../dep/recastnavigation`, relative to this folder.

## Data

Point it at the worldserver's `DataDir`, the folder that holds `mmaps/`, with `--data <dir>` or
the `MOP_NAV_DATA` environment variable. The mmaps must come from the same extractor run as the
server's, or the paths will not match what the server allows.

## Command line

```
navpath --data <dir> --map 0 --from -8914,-133,81 --to -8775,-185,84    one path
navpath --data <dir> --map 0 --nearest -8914,-133,90                      snap a point to the mesh
navpath --data <dir> --map 0 --height -8914,-133[,81]                     mesh height(s) at x,y
navpath --data <dir> --serve                                              JSON lines on stdin/stdout
```

Options: `--step <yd>` (spacing of the output points, default 5), `--water-cost <c>` (default 20),
`--no-water`, `--max-nodes <n>` (Detour node pool, default and maximum 65534) and
`--cache-tiles <n>` (tiles kept per map before that map's cache is dropped, default 600, about
1 MB each). Both `--opt value` and `--opt=value` work.

The one-shot forms print a single JSON object. The exit code is 0 when `ok` is true.

## Serve protocol

Send one JSON object per line. Each request gets exactly one line back, in order. Any `id` you
send is echoed in the answer. Tiles stay cached between requests.

| request | answer |
|---|---|
| `{"op":"path","map":0,"from":[x,y,z],"to":[x,y,z]}` | `ok`, `partial`, `path` `[[x,y,z],...]`, `points`, `length`, `end`, `polys`, `rounds`, `start_dist`, `end_dist`, `far_start`, `far_end`, `tiles_loaded`, `expansions`, `ms` |
| `{"op":"nearest","map":0,"pos":[x,y,z]}` | `ok`, `pos`, `dist`, `area` (1 ground, 8 water), `far` |
| `{"op":"height","map":0,"pos":[x,y]}` or `[x,y,z]` | `ok`, `z` (the level nearest the given z, or the highest without one), `heights` (every mesh level under x,y) |
| `{"op":"stats"}` | tiles cached per map |
| `{"op":"unload","map":0}` | drops that map's cache |
| `{"op":"ping"}`, `{"op":"quit"}` | |

Optional fields on a `path` request:

| field | default | meaning |
|---|---|---|
| `step` | 5 | yards between output points (1 to 50) |
| `water_cost` | 20 | cost multiplier for water polys |
| `allow_water` | true | false: never path through water |
| `margin` | 1 | tiles loaded on each side of the straight start-to-end line |
| `max_expand` | 12 | extra tile loads allowed when a search runs out of loaded mesh |
| `max_rounds` | 64 | chained searches allowed for one path |
| `max_tiles` | 400 | tiles one query may load |
| `h_ext`, `v_ext` | | `nearest` only: a custom search box instead of the server's two boxes |

When `ok` is false, `reason` is one of `no_mmap`, `no_tiles`, `start_off_mesh`, `end_off_mesh`,
`findpath_failed`, `straightpath_failed`, `missing_map`, `bad_from_to`, `bad_pos`,
`unknown_op` or `bad_request: ...`.

`partial` is true when the destination cannot be reached, or when it lies more than 7 yd from
the mesh, which is the server's own "far from poly" limit. The path then ends at the reachable
point closest to the destination, given in `end`.

## Python API (`nav.py`)

Standard library only. It works under `python -I`.

```python
import nav                                   # put this folder on sys.path first
path = nav.find_path(654, (-1446.6, 1407.4, 35.7), (-1734.6, 1657.2, 20.6))
# -> [(x, y, z), ...]; the first point is the start snapped to the mesh, the last is the destination.
# -> None if there is no complete path. Pass allow_partial=True to get the part that is walkable.
r = nav.query_path(654, start, end, step=4)  # the full answer dict, including partial/end/length
p = nav.nearest(0, (-8914.6, -133.9, 90.0))  # (x, y, z) on the mesh, or None
z = nav.height(0, -8914.6, -133.9, z=81)     # float, or None; all_levels=True returns every level
nav.close()                                  # optional; stops the helper process
```

`nav.NavClient(data_dir=None, exe=None, extra_args=())` is the same thing as an object. It is
thread-safe, starts one `navpath --serve` process the first time it is used, and restarts it
once if the process dies. The executable is taken from `MOP_NAVPATH_EXE`, or else from
`build/navpath.exe` next to `nav.py`. The data folder is taken from `MOP_NAV_DATA`, or else from
the first of the repo-relative folders listed in `_DATA_CANDIDATES` that contains `mmaps/`.

You can also run it from the command line: `python -I nav.py --map 0 --from=-8914,-133,81 --to=-8775,-185,84`.
Use the `=` form there, because argparse would read a coordinate like `-8914,...` as an option.

## How it matches the server

These are taken from `src/common/Collision/Management/MMapManager.cpp`,
`src/common/Collision/Maps/MapDefines.h` and `src/server/game/Movement/PathGenerator.cpp`:

- **Files.** `mmaps/%04u.mmap` holds a raw `dtNavMeshParams` (28 bytes: orig, tile width and
  height, max tiles, max polys). `mmaps/%04u_%02i_%02i.mmtile` is a 20-byte `MmapTileHeader`
  (magic `'MMAP'` 0x4d4d4150, Detour version 7, mmap version 7, data size, usesLiquids, 3 padding
  bytes) followed by the Detour tile data, which is passed to `dtNavMesh::addTile` with
  `DT_TILE_FREE_DATA`. Poly refs are 64-bit (`DT_POLYREF64` in the tree's Detour).
- **Tile naming.** The two numbers in a tile name are the map grid coordinates
  `gx = floor(32 - x / 533.333)` and `gy = floor(32 - y / 533.333)`, from world x and y. Inside
  Detour, tile x runs along world y and tile y along world x.
- **Coordinates.** World (x, y, z) is Detour (y, z, x).
- **Area flags.** Poly flags and area are both the `NavTerrain` value: ground 0x01, magma 0x02,
  slime 0x04, water 0x08. The filter is the server's bot filter: include ground and water,
  exclude magma and slime, and give water an area cost of 20, so the path only swims when it has
  to.
- **Start and end snapping.** The same two `findNearestPoly` boxes as `GetPolyByLocation` are
  used, half extents (3, 5, 3) and then (3, 50, 3). A point more than 7 yd from its poly is "far
  from poly".

### Where it differs on purpose

The server's path code is built for short chases. It caps a path at 200 polys
(`MAX_PATH_LENGTH`), 200 smooth points of 4 yd (`MAX_POINT_PATH_LENGTH`) and 1024 search nodes.
navpath raises these limits so that it can plan walks of several kilometres:

- **Node pool and corridor.** The node pool has 65534 nodes, Detour's maximum, and the poly
  buffer is the same size.
- **Chaining.** When a search runs out of nodes, the next search starts from the poly that got
  closest to the goal, and the corridors are joined. A round that makes no progress stops the
  chain. Each round's closest node is never farther from the goal than its start, so the chain
  always ends.
- **Lazy tiles.** A query loads the 3x3 tiles around the start and the end, plus a corridor of
  tiles along the straight line between them (±`margin`). If a search uses up all the loaded
  mesh without reaching the goal, navpath loads the missing neighbours of every tile that the
  search reached and tries again. Tiles stay cached per map.
- **Output points.** The server's `FindSmoothPath` steps 4 yd at a time with
  `moveAlongSurface`. navpath instead runs `findStraightPath` with
  `DT_STRAIGHTPATH_ALL_CROSSINGS` and resamples the result:
  - one point every `step` yards, plus every real corner (a turn of more than about 5 degrees);
  - each sample's z comes from `getPolyHeight` on the poly it lies in, so points between corners
    follow the ground;
  - no +0.5 z lift is added. The z values are raw mesh heights.

## Validation

All results below use the same mmaps that the test world server loads, with the
default options (5 yd step). Coordinates are player start positions and quest-giver spawns from
the test world DB. "Cold" is the first query, including reading its tiles from disk. "Warm" is
the same query again with the tiles cached. To reproduce, run `python -I validate.py`.

| case | map | straight yd | path yd | points | max gap | max dz | polys | rounds | tiles | partial | end off-mesh yd | cold ms | warm ms |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Northshire abbey steps -> kobold mine | 0 | 149 | 152 | 33 | 5.0 | 0.8 | 30 | 1 | 9 | no | 0.5 | 138.2 | 0.4 |
| Northshire abbey -> Goldshire inn (indoors) | 0 | 569 | 646 | 136 | 6.6 | 4.7 | 89 | 1 | 6 | no | 0.8 | 178.7 | 1.5 |
| Goldshire -> Lakeshire town hall | 0 | 2309 | 2632 | 544 | 7.4 | 5.7 | 360 | 1 | 12 | no | 0.6 | 209.3 | 19.1 |
| Gilneas start -> -1734.6,1657.2,20.6 | 654 | 382 | 581 | 123 | 5.5 | 2.8 | 91 | 1 | 15 | no | 0.1 | 259.2 | 0.8 |
| Gilneas Walden -> Krennan Aranas (across city) | 654 | 533 | 789 | 171 | 5.7 | 4.3 | 123 | 1 | 1 | no | 0.3 | 8.8 | 1.1 |
| Echo Isles start -> Tora'jin | 1 | 341 | 351 | 75 | 5.5 | 2.1 | 59 | 1 | 12 | no | 0.1 | 60.5 | 0.3 |
| Valley of Trials -> Razor Hill inn | 1 | 1053 | 1396 | 290 | 5.5 | 2.5 | 164 | 1 | 14 | no | 0.0 | 202.3 | 2.8 |
| Wandering Isle start -> Ji Firepaw | 860 | 207 | 612 | 132 | 6.4 | 4.3 | 102 | 1 | 9 | no | 0.2 | 125.3 | 0.5 |
| Wandering Isle start -> Jojo Ironbrow | 860 | 472 | 1255 | 269 | 6.4 | 4.9 | 213 | 1 | 3 | no | 0.3 | 49.8 | 1.9 |
| Kezan start -> Coach Crosscheck | 648 | 214 | 348 | 76 | 6.8 | 5.4 | 67 | 1 | 9 | no | 1.2 | 143.5 | 0.5 |
| Acherus start -> Razuvious (same floor) | 609 | 164 | 191 | 41 | 5.3 | 2.1 | 29 | 1 | 9 | no | 0.1 | 122.8 | 0.2 |
| Acherus start -> Siouxsie (floor below) | 609 | 52 | 50 | 14 | 5.3 | 2.1 | 12 | 1 | 0 | yes (stops 39 yd short) | 0.2 | 0.3 | 0.3 |
| Acherus start -> Salanar (ground) | 609 | 275 | 38 | 9 | 5.0 | 0.0 | 4 | 1 | 0 | yes (stops 272 yd short) | 0.1 | 0.2 | 0.2 |

How to read the table:

- "max gap" is measured in 3D. On a steep slope a 5 yd horizontal step can be about 7 yd long.
- The Acherus floors are linked only by the teleport pads, and the Heap floats above the ground.
  So the last two rows are correctly partial: the path ends where the floor ends.
- The Wandering Isle start sits on top of the temple, so the walk down is about three times the
  straight-line distance.

### Long walks

These test the chaining, using the serve mode on the same data.

| case | straight yd | path yd | points | polys | rounds | tiles | ms (cold) | result |
|---|---|---|---|---|---|---|---|---|
| Northshire -> Sentinel Hill (Westfall) | 2076 | 2295 | 479 | 335 | 1 | 23 | 514 | complete |
| Goldshire -> Darkshire | 1642 | 1705 | 346 | 214 | 1 | 11 | 341 | complete |
| Razor Hill -> Orgrimmar gate | 1065 | 1156 | 241 | 127 | 1 | 15 | 362 | complete |
| Razor Hill -> Crossroads | 2186 | 2505 | 519 | 288 | 1 | 14 | 177 | complete |
| Razor Hill -> Echo Isles start | 1618 | 1814 | 374 | 214 | 1 | 20 | 296 | complete |
| Orgrimmar gate -> Thunder Bluff top | 5226 | 6104 | 1243 | 582 | 3 | 53 | 906 | partial: ends at the foot of the mesa, because the lifts are transports |
| same four walks with `--max-nodes 256` | | +2 to +4 % | | | 4-12 | | about the same | complete (forces the chaining) |
| Orgrimmar -> Thunder Bluff with `margin:0` | | 6104 | | | 3 | 70 | 1158 | the same path; one frontier expansion |

### Mesh compared with the server's spawns

These use 300 random non-flying, non-swimming, stationary creature spawns per map from the test
world DB. "dist" is the `nearest` distance and "dz" is the `height` z (nearest level) minus the
spawn z.

| map | spawns | on mesh (<= 7 yd) | median dist | p90 dist | median abs dz | p90 abs dz |
|---|---|---|---|---|---|---|
| 0 | 300 | 287 (96%) | 0.21 | 0.71 | 0.20 | 0.70 |
| 1 | 300 | 292 (97%) | 0.23 | 0.72 | 0.23 | 0.72 |
| 654 | 300 | 286 (95%) | 0.17 | 0.58 | 0.17 | 0.59 |
| 860 | 300 | 298 (99%) | 0.40 | 0.95 | 0.39 | 0.95 |
| 648 | 300 | 286 (95%) | 0.30 | 0.87 | 0.29 | 0.78 |
| 609 | 300 | 298 (99%) | 0.27 | 0.55 | 0.27 | 0.56 |

On these validation paths, every output point is within 0.35 yd of the mesh. A warm `nearest`
or `height` query takes about 0.1 ms. Memory is about 1 MB per cached tile: 76 tiles took 79 MB.

## Limits

- **Ground only.** There is no flying, mounting, transports, elevators, teleport pads or
  portals. A destination reached only by one of these comes back `partial`.
- **No dynamic objects.** The mesh has no doors, gates, destructible buildings or phased
  geometry (for example Gilneas phases and Kezan's later phases). It shows what the extractor
  saw.
- **Raw mesh z.** z is the mesh height, which matches the server's spawn z to about 0.2 to
  0.4 yd. The server's own `FindSmoothPath` lifts points by 0.5 yd and then snaps them to map
  height. A client that wants to look exactly like a real client should send its own
  ground-snapped z, or add a small lift.
- **Long chains are greedy.** A chained path (only when one search runs out of 65534 nodes) can
  be a few percent longer than the optimum. One query loads at most 400 tiles (`max_tiles`), and
  a map's cache is dropped when it holds more than `--cache-tiles`.
- **Map-agnostic.** It does not know which maps the server has pathfinding disabled for
  (`DisableMgr`). On those maps the server moves creatures in straight lines. Players are not
  affected.
- **Different from server paths.** Paths are not identical to what the server would produce for
  a creature. Corner placement differs (straight-path resampling instead of
  `moveAlongSurface` steps), and the server's 200-poly cap cuts long paths short.
