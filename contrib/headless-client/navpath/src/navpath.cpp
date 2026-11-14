// navpath - client-side navmesh path queries over the server's .mmap/.mmtile files.
//
// It mirrors the world server's own path code (PathGenerator / MMapManager):
//   * the same file layout: <data>/mmaps/<map:04>.mmap holds dtNavMeshParams, and
//     <data>/mmaps/<map:04>_<gx:02>_<gy:02>.mmtile holds an MmapTileHeader followed by Detour tile data;
//   * the same coordinate swap: WoW (x, y, z) is Detour (y, z, x);
//   * the same poly-flag convention: poly flags/area = NavTerrain (ground 1, magma 2, slime 4, water 8);
//   * the same nearest-poly search boxes (3/5/3, then 3/50/3) and the 7-yard "far from poly" limit.
// It differs on purpose in its limits: a much larger node pool and poly corridor, corridor chaining
// for very long walks, and tiles loaded on demand along the route, so multi-hundred-yard walks work.
//
// Usage:
//   navpath --data <dir> --map <id> --from x,y,z --to x,y,z      one path, JSON on stdout
//   navpath --data <dir> --map <id> --nearest x,y,z               snap a point to the mesh
//   navpath --data <dir> --map <id> --height x,y[,z]              mesh height(s) at x,y
//   navpath --data <dir> --serve                                  one JSON request per stdin line
// <dir> may also come from the MOP_NAV_DATA environment variable.

#include "DetourAlloc.h"
#include "DetourCommon.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "DetourNode.h"

#include "json.h"

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    // ---- file format (must match the server's MapDefines.h) ----
    constexpr uint32_t MMAP_MAGIC = 0x4d4d4150; // 'MMAP'
    constexpr uint32_t MMAP_VERSION = 7;

    struct MmapTileHeader
    {
        uint32_t mmapMagic;
        uint32_t dtVersion;
        uint32_t mmapVersion;
        uint32_t size;
        char usesLiquids;
        char padding[3];
    };
    static_assert(sizeof(MmapTileHeader) == 20, "MmapTileHeader must be 20 bytes");

    enum NavTerrain : unsigned short
    {
        NAV_GROUND = 0x01,
        NAV_MAGMA = 0x02,
        NAV_SLIME = 0x04,
        NAV_WATER = 0x08,
    };

    // One map grid (= one mmtile) is 533.33 yards; grid 32 is the world centre.
    constexpr double GRID_SIZE = 1600.0 / 3.0;
    constexpr int MAX_GRIDS = 64;

    // Server limits worth keeping.
    constexpr float FAR_FROM_POLY = 7.0f;          // PathGenerator: startFarFromPoly / endFarFromPoly
    constexpr float SNAP_EXT_SMALL[3] = {3.0f, 5.0f, 3.0f};
    constexpr float SNAP_EXT_LARGE[3] = {3.0f, 50.0f, 3.0f};

    struct V3
    {
        float x, y, z;
    };

    // WoW <-> Detour
    inline void toDt(V3 const& w, float* d) { d[0] = w.y; d[1] = w.z; d[2] = w.x; }
    inline V3 fromDt(float const* d) { return V3{d[2], d[0], d[1]}; }

    // Grid coordinate of a world position, as the server's GridCoord/mmtile naming uses it.
    inline double gridF(float v) { return 32.0 - double(v) / GRID_SIZE; }
    inline int gridI(float v)
    {
        int g = int(std::floor(gridF(v)));
        return std::min(std::max(g, 0), MAX_GRIDS - 1);
    }
    inline uint32_t packGrid(int gx, int gy) { return uint32_t(gx) << 8 | uint32_t(gy); }

    double nowMs()
    {
        using namespace std::chrono;
        return duration<double, std::milli>(steady_clock::now().time_since_epoch()).count();
    }

    struct Options
    {
        float step = 5.0f;         // spacing of the output points, yards
        float waterCost = 20.0f;   // area cost of water polys (server bot filter uses 20)
        bool allowWater = true;    // include NAV_WATER polys
        int margin = 1;            // tiles loaded either side of the straight start->end line
        int maxExpand = 12;        // frontier tile loads allowed when the search runs out of loaded mesh
        int maxRounds = 64;        // findPath calls chained for one long path
        int maxQueryTiles = 400;   // tiles a single query may load
    };

    struct MapData
    {
        uint32_t id = 0;
        dtNavMesh* mesh = nullptr;
        dtNavMeshQuery* query = nullptr;
        std::unordered_map<uint32_t, dtTileRef> loaded;
        std::unordered_set<uint32_t> missing;

        ~MapData()
        {
            if (query)
                dtFreeNavMeshQuery(query);
            if (mesh)
                dtFreeNavMesh(mesh); // frees tile data too (DT_TILE_FREE_DATA)
        }
    };

    struct PathResult
    {
        bool ok = false;
        std::string reason;
        std::vector<V3> path;
        bool partial = false;
        bool farStart = false;
        bool farEnd = false;
        float startDist = 0.0f;
        float endDist = 0.0f;
        int polys = 0;
        int rounds = 0;
        int expansions = 0;
        int tilesLoaded = 0;
        float length = 0.0f;
    };

    class Navigator
    {
    public:
        Navigator(std::string dataDir, int maxNodes, int cacheTiles)
            : _dataDir(std::move(dataDir)), _maxNodes(maxNodes), _cacheTiles(cacheTiles) { }

        std::string const& dataDir() const { return _dataDir; }

        MapData* getMap(uint32_t mapId, std::string& err)
        {
            auto it = _maps.find(mapId);
            if (it != _maps.end())
                return it->second.get();

            char name[64];
            std::snprintf(name, sizeof(name), "/mmaps/%04u.mmap", mapId);
            std::string fileName = _dataDir + name;
            FILE* f = std::fopen(fileName.c_str(), "rb");
            if (!f)
            {
                err = "no_mmap";
                return nullptr;
            }
            dtNavMeshParams params;
            size_t count = std::fread(&params, sizeof(params), 1, f);
            std::fclose(f);
            if (count != 1)
            {
                err = "bad_mmap";
                return nullptr;
            }

            auto md = std::make_unique<MapData>();
            md->id = mapId;
            md->mesh = dtAllocNavMesh();
            if (!md->mesh || dtStatusFailed(md->mesh->init(&params)))
            {
                err = "navmesh_init_failed";
                return nullptr;
            }
            md->query = dtAllocNavMeshQuery();
            if (!md->query || dtStatusFailed(md->query->init(md->mesh, _maxNodes)))
            {
                err = "query_init_failed";
                return nullptr;
            }
            MapData* raw = md.get();
            _maps[mapId] = std::move(md);
            return raw;
        }

        void unloadMap(uint32_t mapId) { _maps.erase(mapId); }

        // Drops a map's cached tiles when the cache grows past its limit.
        MapData* trimmed(uint32_t mapId, std::string& err)
        {
            auto it = _maps.find(mapId);
            if (it != _maps.end() && int(it->second->loaded.size()) > _cacheTiles)
                _maps.erase(it);
            return getMap(mapId, err);
        }

        // Returns true when the tile is (now) loaded. 'added' counts fresh loads.
        bool loadTile(MapData& md, int gx, int gy, int* added)
        {
            if (gx < 0 || gy < 0 || gx >= MAX_GRIDS || gy >= MAX_GRIDS)
                return false;
            uint32_t key = packGrid(gx, gy);
            if (md.loaded.count(key))
                return true;
            if (md.missing.count(key))
                return false;

            char name[64];
            std::snprintf(name, sizeof(name), "/mmaps/%04u_%02i_%02i.mmtile", md.id, gx, gy);
            std::string fileName = _dataDir + name;
            FILE* f = std::fopen(fileName.c_str(), "rb");
            if (!f)
            {
                md.missing.insert(key);
                return false;
            }

            MmapTileHeader header;
            if (std::fread(&header, sizeof(header), 1, f) != 1 || header.mmapMagic != MMAP_MAGIC ||
                header.mmapVersion != MMAP_VERSION || header.dtVersion != uint32_t(DT_NAVMESH_VERSION))
            {
                std::fclose(f);
                std::fprintf(stderr, "navpath: bad header in %s\n", fileName.c_str());
                md.missing.insert(key);
                return false;
            }

            long pos = std::ftell(f);
            std::fseek(f, 0, SEEK_END);
            long end = std::ftell(f);
            std::fseek(f, pos, SEEK_SET);
            if (pos < 0 || long(header.size) > end - pos || header.size == 0)
            {
                std::fclose(f);
                std::fprintf(stderr, "navpath: corrupted data size in %s\n", fileName.c_str());
                md.missing.insert(key);
                return false;
            }

            unsigned char* data = static_cast<unsigned char*>(dtAlloc(header.size, DT_ALLOC_PERM));
            if (!data || std::fread(data, header.size, 1, f) != 1)
            {
                std::fclose(f);
                if (data)
                    dtFree(data);
                md.missing.insert(key);
                return false;
            }
            std::fclose(f);

            dtTileRef ref = 0;
            if (dtStatusFailed(md.mesh->addTile(data, int(header.size), DT_TILE_FREE_DATA, 0, &ref)))
            {
                dtFree(data);
                std::fprintf(stderr, "navpath: could not add %s to the navmesh\n", fileName.c_str());
                md.missing.insert(key);
                return false;
            }
            md.loaded[key] = ref;
            if (added)
                ++*added;
            return true;
        }

        // Loads the tiles within 'margin' grids of the straight line a -> b. Returns tiles newly loaded.
        int loadCorridor(MapData& md, V3 const& a, V3 const& b, int margin, int budget)
        {
            std::set<std::pair<int, int>> cells;
            double ax = gridF(a.x), ay = gridF(a.y), bx = gridF(b.x), by = gridF(b.y);
            double len = std::sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));
            int samples = std::max(1, int(std::ceil(len / 0.25)));
            for (int i = 0; i <= samples; ++i)
            {
                double t = double(i) / samples;
                int cx = int(std::floor(ax + (bx - ax) * t));
                int cy = int(std::floor(ay + (by - ay) * t));
                for (int dx = -margin; dx <= margin; ++dx)
                    for (int dy = -margin; dy <= margin; ++dy)
                        cells.insert({cx + dx, cy + dy});
            }
            int added = 0;
            for (auto const& c : cells)
            {
                if (added >= budget)
                    break;
                loadTile(md, c.first, c.second, &added);
            }
            return added;
        }

        // Loads the not-yet-loaded neighbours of every tile that holds a node of the last search.
        int loadFrontier(MapData& md, int budget)
        {
            dtNodePool const* pool = md.query->getNodePool();
            if (!pool)
                return 0;
            dtNavMeshParams const* params = md.mesh->getParams();
            std::set<std::pair<int, int>> touched;
            for (int i = 0; i < pool->getNodeCount(); ++i)
            {
                dtNode const* node = pool->getNodeAtIdx(unsigned(i + 1));
                if (!node || !node->id)
                    continue;
                dtMeshTile const* tile = nullptr;
                dtPoly const* poly = nullptr;
                if (dtStatusFailed(md.mesh->getTileAndPolyByRef(node->id, &tile, &poly)) || !tile->header)
                    continue;
                // Detour tile x runs along world y, tile y along world x (see toDt).
                float wy = params->orig[0] + (tile->header->x + 0.5f) * params->tileWidth;
                float wx = params->orig[2] + (tile->header->y + 0.5f) * params->tileHeight;
                touched.insert({gridI(wx), gridI(wy)});
            }
            int added = 0;
            for (auto const& t : touched)
                for (int dx = -1; dx <= 1; ++dx)
                    for (int dy = -1; dy <= 1; ++dy)
                        if (added < budget)
                            loadTile(md, t.first + dx, t.second + dy, &added);
            return added;
        }

        int loadAround(MapData& md, V3 const& p, int margin)
        {
            int added = 0;
            int gx = gridI(p.x), gy = gridI(p.y);
            for (int dx = -margin; dx <= margin; ++dx)
                for (int dy = -margin; dy <= margin; ++dy)
                    loadTile(md, gx + dx, gy + dy, &added);
            return added;
        }

        static dtQueryFilter makeFilter(Options const& o)
        {
            // Same choice as the server's bot filter: walk ground, swim water, never path through lava/slime.
            dtQueryFilter filter;
            filter.setIncludeFlags(NAV_GROUND | (o.allowWater ? NAV_WATER : 0));
            filter.setExcludeFlags(NAV_MAGMA | NAV_SLIME);
            filter.setAreaCost(NAV_WATER, o.waterCost);
            return filter;
        }

        // PathGenerator::GetPolyByLocation without the previous-path shortcut.
        static dtPolyRef snap(dtNavMeshQuery const* q, dtQueryFilter const& filter, float const* pt, float* closest, float* dist)
        {
            dtPolyRef ref = 0;
            if (dtStatusSucceed(q->findNearestPoly(pt, SNAP_EXT_SMALL, &filter, &ref, closest)) && ref)
            {
                *dist = dtVdist(closest, pt);
                return ref;
            }
            if (dtStatusSucceed(q->findNearestPoly(pt, SNAP_EXT_LARGE, &filter, &ref, closest)) && ref)
            {
                *dist = dtVdist(closest, pt);
                return ref;
            }
            *dist = FLT_MAX;
            return 0;
        }

        PathResult findPath(uint32_t mapId, V3 const& start, V3 const& end, Options const& o)
        {
            PathResult r;
            std::string err;
            MapData* md = trimmed(mapId, err);
            if (!md)
            {
                r.reason = err;
                return r;
            }
            dtNavMeshQuery* q = md->query;
            dtQueryFilter filter = makeFilter(o);

            int budget = o.maxQueryTiles;
            r.tilesLoaded += loadAround(*md, start, 1);
            r.tilesLoaded += loadAround(*md, end, 1);
            int margin = std::max(0, o.margin);
            r.tilesLoaded += loadCorridor(*md, start, end, margin, budget - r.tilesLoaded);

            float sPt[3], ePt[3], sPos[3], ePos[3];
            toDt(start, sPt);
            toDt(end, ePt);
            dtPolyRef sRef = snap(q, filter, sPt, sPos, &r.startDist);
            dtPolyRef eRef = snap(q, filter, ePt, ePos, &r.endDist);
            if (!sRef)
            {
                r.reason = md->loaded.empty() ? "no_tiles" : "start_off_mesh";
                return r;
            }
            if (!eRef)
            {
                r.reason = "end_off_mesh";
                return r;
            }
            r.farStart = r.startDist > FAR_FROM_POLY;
            r.farEnd = r.endDist > FAR_FROM_POLY;

            std::vector<dtPolyRef> corridor;
            std::vector<dtPolyRef> buf(static_cast<size_t>(_maxNodes));
            dtPolyRef curRef = sRef;
            float cur[3];
            dtVcopy(cur, sPos);
            bool reached = (sRef == eRef);
            if (reached)
                corridor.push_back(sRef);

            while (!reached && r.rounds < o.maxRounds)
            {
                ++r.rounds;
                int n = 0;
                dtStatus st = q->findPath(curRef, eRef, cur, ePos, &filter, buf.data(), &n, _maxNodes);
                if (dtStatusFailed(st) || n == 0)
                {
                    if (corridor.empty())
                    {
                        r.reason = "findpath_failed";
                        return r;
                    }
                    break;
                }

                bool complete = buf[n - 1] == eRef;
                bool outOfNodes = (st & DT_OUT_OF_NODES) != 0;
                if (!complete && !outOfNodes)
                {
                    // The search used up every reachable poly in the loaded tiles. Load the missing
                    // neighbours of every tile the search reached, then retry this round.
                    if (r.expansions < o.maxExpand && r.tilesLoaded < budget)
                    {
                        int added = loadFrontier(*md, budget - r.tilesLoaded);
                        r.tilesLoaded += added;
                        ++r.expansions;
                        if (added > 0)
                        {
                            --r.rounds;
                            continue;
                        }
                    }
                }

                size_t first = (!corridor.empty() && corridor.back() == buf[0]) ? 1 : 0;
                corridor.insert(corridor.end(), buf.begin() + first, buf.begin() + n);
                if (complete)
                {
                    reached = true;
                    break;
                }
                if (!outOfNodes || buf[n - 1] == curRef)
                    break; // dead end, or no progress this round

                // Chain: continue from the poly that got closest to the goal.
                curRef = buf[n - 1];
                if (dtStatusFailed(q->closestPointOnPoly(curRef, ePos, cur, nullptr)))
                    break;
                r.tilesLoaded += loadCorridor(*md, fromDt(cur), end, margin, budget - r.tilesLoaded);
            }

            r.polys = int(corridor.size());
            float target[3];
            if (reached)
                dtVcopy(target, ePos);
            else if (dtStatusFailed(q->closestPointOnPoly(corridor.back(), ePos, target, nullptr)))
                dtVcopy(target, cur);

            if (!buildPoints(q, sPos, target, corridor, o.step, r.path))
            {
                r.reason = "straightpath_failed";
                return r;
            }
            for (size_t i = 1; i < r.path.size(); ++i)
            {
                float dx = r.path[i].x - r.path[i - 1].x, dy = r.path[i].y - r.path[i - 1].y, dz = r.path[i].z - r.path[i - 1].z;
                r.length += std::sqrt(dx * dx + dy * dy + dz * dz);
            }
            r.partial = !reached || r.farEnd;
            r.ok = true;
            return r;
        }

        // Straight path through the corridor (findStraightPath with a vertex at every poly crossing),
        // then resampled to one point every 'step' yards plus every real corner. Heights come from the
        // poly each sample lies in, so the points follow the ground between corners.
        static bool buildPoints(dtNavMeshQuery const* q, float const* sPos, float const* ePos,
            std::vector<dtPolyRef> const& corridor, float step, std::vector<V3>& out)
        {
            int maxPts = int(corridor.size()) * 2 + 16;
            std::vector<float> pts(static_cast<size_t>(maxPts) * 3);
            std::vector<unsigned char> flags(static_cast<size_t>(maxPts));
            std::vector<dtPolyRef> refs(static_cast<size_t>(maxPts));
            int n = 0;
            dtStatus st = q->findStraightPath(sPos, ePos, corridor.data(), int(corridor.size()), pts.data(), flags.data(),
                refs.data(), &n, maxPts, DT_STRAIGHTPATH_ALL_CROSSINGS);
            if (dtStatusFailed(st) || n < 1)
                return false;

            auto P = [&](int i) { return &pts[size_t(i) * 3]; };
            auto heightAt = [&](dtPolyRef ref, float* p)
            {
                float h;
                if (ref && dtStatusSucceed(q->getPolyHeight(ref, p, &h)))
                    p[1] = h;
            };
            auto isCorner = [&](int i) -> bool
            {
                if (i <= 0 || i >= n - 1)
                    return true;
                if (flags[size_t(i)] & DT_STRAIGHTPATH_OFFMESH_CONNECTION)
                    return true;
                // look back/forward past near-duplicate points
                int a = i - 1, b = i + 1;
                float d1x = P(i)[0] - P(a)[0], d1z = P(i)[2] - P(a)[2];
                float d2x = P(b)[0] - P(i)[0], d2z = P(b)[2] - P(i)[2];
                float l1 = std::sqrt(d1x * d1x + d1z * d1z), l2 = std::sqrt(d2x * d2x + d2z * d2z);
                if (l1 < 1e-3f || l2 < 1e-3f)
                    return false;
                float cosA = (d1x * d2x + d1z * d2z) / (l1 * l2);
                return cosA < 0.9962f; // > ~5 degrees
            };

            out.clear();
            out.push_back(fromDt(P(0)));
            if (n == 1)
            {
                out.push_back(fromDt(ePos));
                return true;
            }

            float last[3];
            dtVcopy(last, P(0));
            float sinceLast = 0.0f;
            for (int i = 0; i + 1 < n; ++i)
            {
                float const* a = P(i);
                float const* b = P(i + 1);
                float seg = dtVdist(a, b);
                dtPolyRef ref = refs[size_t(i)];
                // emit evenly spaced samples inside this segment
                float pos = 0.0f;
                while (seg > 1e-4f && sinceLast + (seg - pos) >= step)
                {
                    pos += step - sinceLast;
                    float t = pos / seg;
                    float p[3];
                    dtVlerp(p, a, b, t);
                    heightAt(ref, p);
                    // do not drop a sample right on top of the coming corner
                    if (seg - pos < 0.5f && isCorner(i + 1))
                        break;
                    out.push_back(fromDt(p));
                    dtVcopy(last, p);
                    sinceLast = 0.0f;
                }
                sinceLast += seg - pos;
                if (isCorner(i + 1))
                {
                    if (dtVdist(last, b) > 0.05f)
                        out.push_back(fromDt(b));
                    dtVcopy(last, b);
                    sinceLast = 0.0f;
                }
            }
            return true;
        }

        struct NearestResult
        {
            bool ok = false;
            std::string reason;
            V3 pos{};
            float dist = 0.0f;
            dtPolyRef ref = 0;
            int area = 0;
        };

        NearestResult nearest(uint32_t mapId, V3 const& p, float hExt, float vExt, Options const& o)
        {
            NearestResult r;
            std::string err;
            MapData* md = trimmed(mapId, err);
            if (!md)
            {
                r.reason = err;
                return r;
            }
            loadAround(*md, p, 1);
            dtQueryFilter filter = makeFilter(o);
            float pt[3], closest[3];
            toDt(p, pt);
            dtPolyRef ref = 0;
            if (hExt > 0.0f || vExt > 0.0f)
            {
                float ext[3] = {std::max(hExt, 0.1f), std::max(vExt, 0.1f), std::max(hExt, 0.1f)};
                if (dtStatusSucceed(md->query->findNearestPoly(pt, ext, &filter, &ref, closest)) && ref)
                    r.dist = dtVdist(closest, pt);
                else
                    ref = 0;
            }
            else
                ref = snap(md->query, filter, pt, closest, &r.dist);
            if (!ref)
            {
                r.reason = md->loaded.empty() ? "no_tiles" : "off_mesh";
                return r;
            }
            dtMeshTile const* tile = nullptr;
            dtPoly const* poly = nullptr;
            if (dtStatusSucceed(md->mesh->getTileAndPolyByRef(ref, &tile, &poly)))
                r.area = poly->getArea();
            r.ok = true;
            r.ref = ref;
            r.pos = fromDt(closest);
            return r;
        }

        struct HeightResult
        {
            bool ok = false;
            std::string reason;
            float z = 0.0f;
            std::vector<float> heights;
        };

        // Every mesh height under (x, y); 'z' is the one nearest the hint (or the highest without one).
        HeightResult height(uint32_t mapId, float x, float y, bool haveHint, float hint, float range, Options const& o)
        {
            HeightResult r;
            std::string err;
            MapData* md = trimmed(mapId, err);
            if (!md)
            {
                r.reason = err;
                return r;
            }
            V3 p{x, y, haveHint ? hint : 0.0f};
            loadAround(*md, p, 0);
            dtQueryFilter filter = makeFilter(o);
            float center[3];
            toDt(p, center);
            float ext[3] = {0.25f, haveHint ? range : 5000.0f, 0.25f};
            dtPolyRef polys[256];
            int count = 0;
            md->query->queryPolygons(center, ext, &filter, polys, &count, 256);
            for (int i = 0; i < count; ++i)
            {
                float h;
                if (dtStatusSucceed(md->query->getPolyHeight(polys[i], center, &h)))
                    r.heights.push_back(h);
            }
            if (r.heights.empty())
            {
                r.reason = md->loaded.empty() ? "no_tiles" : "no_mesh_here";
                return r;
            }
            std::sort(r.heights.begin(), r.heights.end());
            r.heights.erase(std::unique(r.heights.begin(), r.heights.end(),
                [](float a, float b) { return std::fabs(a - b) < 0.01f; }), r.heights.end());
            if (haveHint)
            {
                r.z = r.heights[0];
                for (float h : r.heights)
                    if (std::fabs(h - hint) < std::fabs(r.z - hint))
                        r.z = h;
            }
            else
                r.z = r.heights.back();
            r.ok = true;
            return r;
        }

        void stats(json::Writer& w) const
        {
            w.key("maps");
            w.beginArray();
            for (auto const& kv : _maps)
            {
                w.beginObject();
                w.key("map");
                w.integer(kv.first);
                w.key("tiles");
                w.integer((long long)kv.second->loaded.size());
                w.endObject();
            }
            w.endArray();
            w.key("max_nodes");
            w.integer(_maxNodes);
            w.key("cache_tiles");
            w.integer(_cacheTiles);
        }

    private:
        std::string _dataDir;
        int _maxNodes;
        int _cacheTiles;
        std::map<uint32_t, std::unique_ptr<MapData>> _maps;
    };

    // ---- protocol helpers ----

    bool parseVec(json::Value const* v, float* out, int minCount, int maxCount, int* got = nullptr)
    {
        if (!v || v->type != json::Value::Array || int(v->a.size()) < minCount || int(v->a.size()) > maxCount)
            return false;
        for (size_t i = 0; i < v->a.size(); ++i)
        {
            if (v->a[i].type != json::Value::Number)
                return false;
            out[i] = float(v->a[i].n);
        }
        if (got)
            *got = int(v->a.size());
        return true;
    }

    bool parseCsv(char const* s, float* out, int minCount, int maxCount, int* got = nullptr)
    {
        int count = 0;
        char const* p = s;
        while (*p && count < maxCount)
        {
            char* end = nullptr;
            out[count] = std::strtof(p, &end);
            if (end == p)
                return false;
            ++count;
            p = end;
            while (*p == ',' || *p == ' ')
                ++p;
        }
        if (*p || count < minCount)
            return false;
        if (got)
            *got = count;
        return true;
    }

    void writeVec(json::Writer& w, V3 const& v)
    {
        w.beginArray();
        w.number(v.x);
        w.number(v.y);
        w.number(v.z);
        w.endArray();
    }

    Options readOptions(json::Value const& req, Options o)
    {
        o.step = std::min(50.0f, std::max(1.0f, float(req.num("step", o.step))));
        o.waterCost = std::max(1.0f, float(req.num("water_cost", o.waterCost)));
        o.allowWater = req.boolean("allow_water", o.allowWater);
        o.margin = std::min(8, std::max(0, int(req.num("margin", o.margin))));
        o.maxExpand = std::min(16, std::max(0, int(req.num("max_expand", o.maxExpand))));
        o.maxRounds = std::min(1024, std::max(1, int(req.num("max_rounds", o.maxRounds))));
        o.maxQueryTiles = std::min(4096, std::max(1, int(req.num("max_tiles", o.maxQueryTiles))));
        return o;
    }

    void writePath(json::Writer& w, PathResult const& r, double ms)
    {
        w.key("ok");
        w.boolean(r.ok);
        if (!r.ok)
        {
            w.key("reason");
            w.string(r.reason);
        }
        else
        {
            w.key("partial");
            w.boolean(r.partial);
            w.key("path");
            w.beginArray();
            for (V3 const& p : r.path)
                writeVec(w, p);
            w.endArray();
            w.key("points");
            w.integer((long long)r.path.size());
            w.key("length");
            w.number(r.length, 1);
            w.key("end");
            writeVec(w, r.path.back());
            w.key("polys");
            w.integer(r.polys);
            w.key("rounds");
            w.integer(r.rounds);
        }
        w.key("start_dist");
        w.number(r.startDist < 1e30f ? r.startDist : -1.0, 2);
        w.key("end_dist");
        w.number(r.endDist < 1e30f ? r.endDist : -1.0, 2);
        w.key("far_start");
        w.boolean(r.farStart);
        w.key("far_end");
        w.boolean(r.farEnd);
        w.key("tiles_loaded");
        w.integer(r.tilesLoaded);
        w.key("expansions");
        w.integer(r.expansions);
        w.key("ms");
        w.number(ms, 2);
    }

    // Handles one request object; returns false when the caller asked to quit.
    bool handle(Navigator& nav, json::Value const& req, Options const& defaults, json::Writer& w)
    {
        double t0 = nowMs();
        w.beginObject();
        if (json::Value const* id = req.get("id"))
        {
            w.key("id");
            w.value(*id);
        }
        std::string op = req.str("op", req.get("from") ? "path" : "");
        Options o = readOptions(req, defaults);
        uint32_t mapId = uint32_t(req.num("map", -1.0));
        bool haveMap = req.get("map") && req.num("map", -1.0) >= 0.0;

        auto fail = [&](char const* reason)
        {
            w.key("ok");
            w.boolean(false);
            w.key("reason");
            w.string(reason);
        };

        bool keepGoing = true;
        if (op == "path")
        {
            float a[3], b[3];
            if (!haveMap)
                fail("missing_map");
            else if (!parseVec(req.get("from"), a, 3, 3) || !parseVec(req.get("to"), b, 3, 3))
                fail("bad_from_to");
            else
            {
                PathResult r = nav.findPath(mapId, V3{a[0], a[1], a[2]}, V3{b[0], b[1], b[2]}, o);
                writePath(w, r, nowMs() - t0);
            }
        }
        else if (op == "nearest")
        {
            float p[3];
            if (!haveMap)
                fail("missing_map");
            else if (!parseVec(req.get("pos"), p, 3, 3))
                fail("bad_pos");
            else
            {
                auto r = nav.nearest(mapId, V3{p[0], p[1], p[2]}, float(req.num("h_ext", 0.0)), float(req.num("v_ext", 0.0)), o);
                w.key("ok");
                w.boolean(r.ok);
                if (!r.ok)
                {
                    w.key("reason");
                    w.string(r.reason);
                }
                else
                {
                    w.key("pos");
                    writeVec(w, r.pos);
                    w.key("dist");
                    w.number(r.dist, 2);
                    w.key("area");
                    w.integer(r.area);
                    w.key("far");
                    w.boolean(r.dist > FAR_FROM_POLY);
                }
            }
        }
        else if (op == "height")
        {
            float p[3];
            int got = 0;
            if (!haveMap)
                fail("missing_map");
            else if (!parseVec(req.get("pos"), p, 2, 3, &got))
                fail("bad_pos");
            else
            {
                auto r = nav.height(mapId, p[0], p[1], got == 3, got == 3 ? p[2] : 0.0f, float(req.num("range", 50.0)), o);
                w.key("ok");
                w.boolean(r.ok);
                if (!r.ok)
                {
                    w.key("reason");
                    w.string(r.reason);
                }
                else
                {
                    w.key("z");
                    w.number(r.z);
                    w.key("heights");
                    w.beginArray();
                    for (float h : r.heights)
                        w.number(h);
                    w.endArray();
                }
            }
        }
        else if (op == "ping")
        {
            w.key("ok");
            w.boolean(true);
            w.key("pong");
            w.boolean(true);
        }
        else if (op == "stats")
        {
            w.key("ok");
            w.boolean(true);
            nav.stats(w);
        }
        else if (op == "unload")
        {
            if (haveMap)
                nav.unloadMap(mapId);
            w.key("ok");
            w.boolean(haveMap);
        }
        else if (op == "quit")
        {
            w.key("ok");
            w.boolean(true);
            keepGoing = false;
        }
        else
            fail("unknown_op");

        if (op != "path")
        {
            w.key("ms");
            w.number(nowMs() - t0, 2);
        }
        w.endObject();
        return keepGoing;
    }

    void usage()
    {
        std::fprintf(stderr,
            "usage: navpath --data <dir> [options] (--serve | --map <id> (--from x,y,z --to x,y,z | --nearest x,y,z | --height x,y[,z]))\n"
            "  --data <dir>       server data dir holding mmaps/ (or env MOP_NAV_DATA)\n"
            "  --serve            read one JSON request per line from stdin, answer one line each\n"
            "  --step <yd>        spacing of output points (default 5)\n"
            "  --water-cost <c>   cost multiplier for water polys (default 20)\n"
            "  --no-water         never path through water\n"
            "  --max-nodes <n>    Detour node pool size (default 65534, max 65534)\n"
            "  --cache-tiles <n>  tiles kept per map before the cache is reset (default 600, ~1 MB each)\n"
            "requests: {\"op\":\"path\",\"map\":0,\"from\":[x,y,z],\"to\":[x,y,z],\"step\":5,\"id\":1}\n"
            "          {\"op\":\"nearest\",\"map\":0,\"pos\":[x,y,z]}  {\"op\":\"height\",\"map\":0,\"pos\":[x,y,z?]}\n"
            "          {\"op\":\"ping\"} {\"op\":\"stats\"} {\"op\":\"unload\",\"map\":0} {\"op\":\"quit\"}\n");
    }
}

int main(int argc, char** argv)
{
    std::string dataDir;
    if (char const* env = std::getenv("MOP_NAV_DATA"))
        dataDir = env;
    bool serve = false;
    long mapId = -1;
    float from[3], to[3], nearestPt[3], heightPt[3];
    bool haveFrom = false, haveTo = false, haveNearest = false, haveHeight = false;
    int heightCount = 0;
    int maxNodes = 65534;
    int cacheTiles = 600;
    Options defaults;

    for (int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];
        // accept --opt=value as well as --opt value
        std::string inlineValue;
        bool haveInline = false;
        size_t eq = a.find('=');
        if (a.compare(0, 2, "--") == 0 && eq != std::string::npos)
        {
            inlineValue = a.substr(eq + 1);
            a.resize(eq);
            haveInline = true;
        }
        auto next = [&]() -> char const*
        {
            if (haveInline)
                return inlineValue.c_str();
            if (i + 1 >= argc)
            {
                std::fprintf(stderr, "navpath: %s needs a value\n", a.c_str());
                std::exit(2);
            }
            return argv[++i];
        };
        if (a == "--data")
            dataDir = next();
        else if (a == "--serve")
            serve = true;
        else if (a == "--map")
            mapId = std::strtol(next(), nullptr, 10);
        else if (a == "--from")
            haveFrom = parseCsv(next(), from, 3, 3);
        else if (a == "--to")
            haveTo = parseCsv(next(), to, 3, 3);
        else if (a == "--nearest")
            haveNearest = parseCsv(next(), nearestPt, 3, 3);
        else if (a == "--height")
            haveHeight = parseCsv(next(), heightPt, 2, 3, &heightCount);
        else if (a == "--step")
            defaults.step = std::min(50.0f, std::max(1.0f, std::strtof(next(), nullptr)));
        else if (a == "--water-cost")
            defaults.waterCost = std::max(1.0f, std::strtof(next(), nullptr));
        else if (a == "--no-water")
            defaults.allowWater = false;
        else if (a == "--max-nodes")
            maxNodes = std::min(65534, std::max(256, int(std::strtol(next(), nullptr, 10))));
        else if (a == "--cache-tiles")
            cacheTiles = std::max(16, int(std::strtol(next(), nullptr, 10)));
        else if (a == "-h" || a == "--help")
        {
            usage();
            return 0;
        }
        else
        {
            std::fprintf(stderr, "navpath: unknown argument %s\n", a.c_str());
            usage();
            return 2;
        }
    }

    while (!dataDir.empty() && (dataDir.back() == '/' || dataDir.back() == '\\'))
        dataDir.pop_back();
    if (dataDir.empty())
    {
        std::fprintf(stderr, "navpath: no data dir (use --data or MOP_NAV_DATA)\n");
        usage();
        return 2;
    }

    Navigator nav(dataDir, maxNodes, cacheTiles);

    if (serve)
    {
        std::string line;
        while (std::getline(std::cin, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (line.find_first_not_of(" \t") == std::string::npos)
                continue;
            json::Writer w;
            json::Value req;
            std::string err;
            bool keepGoing = true;
            if (!json::Parser(line).parse(req, err) || req.type != json::Value::Object)
            {
                w.beginObject();
                w.key("ok");
                w.boolean(false);
                w.key("reason");
                w.string("bad_request: " + (err.empty() ? std::string("not an object") : err));
                w.endObject();
            }
            else
                keepGoing = handle(nav, req, defaults, w);
            std::fputs(w.out.c_str(), stdout);
            std::fputc('\n', stdout);
            std::fflush(stdout);
            if (!keepGoing)
                break;
        }
        return 0;
    }

    if (mapId < 0 || (!(haveFrom && haveTo) && !haveNearest && !haveHeight))
    {
        usage();
        return 2;
    }

    // One-shot: build the same request the server mode would get.
    json::Value req;
    req.type = json::Value::Object;
    auto num = [](double v) { json::Value x; x.type = json::Value::Number; x.n = v; return x; };
    auto vec = [&](float const* p, int count) { json::Value x; x.type = json::Value::Array; for (int k = 0; k < count; ++k) x.a.push_back(num(p[k])); return x; };
    auto str = [](char const* s) { json::Value x; x.type = json::Value::String; x.s = s; return x; };
    req.o.emplace_back("map", num(double(mapId)));
    if (haveFrom && haveTo)
    {
        req.o.emplace_back("op", str("path"));
        req.o.emplace_back("from", vec(from, 3));
        req.o.emplace_back("to", vec(to, 3));
    }
    else if (haveNearest)
    {
        req.o.emplace_back("op", str("nearest"));
        req.o.emplace_back("pos", vec(nearestPt, 3));
    }
    else
    {
        req.o.emplace_back("op", str("height"));
        req.o.emplace_back("pos", vec(heightPt, heightCount));
    }
    json::Writer w;
    handle(nav, req, defaults, w);
    std::fputs(w.out.c_str(), stdout);
    std::fputc('\n', stdout);
    json::Value const* okv = nullptr;
    json::Value res;
    std::string err;
    if (json::Parser(w.out).parse(res, err))
        okv = res.get("ok");
    return (okv && okv->type == json::Value::Bool && okv->b) ? 0 : 1;
}
