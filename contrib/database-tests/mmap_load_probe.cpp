// Read-only integration probe using a real, operator-supplied navigation tile.
#include "Config.h"
#include "MMapManager.h"
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <cstdio>

int main(int argc, char** argv)
{
    if (argc != 5)
    {
        std::cerr << "Usage: mmap_load_probe <DataDir-only config> <map> <x> <y>\n";
        return 2;
    }
    std::string error;
    if (!sConfigMgr->LoadInitial(argv[1], {}, error))
    {
        std::cerr << error << '\n';
        return 2;
    }
    uint32 map = uint32(std::stoul(argv[2]));
    int32 x = std::stoi(argv[3]);
    int32 y = std::stoi(argv[4]);
    MMAP::MMapManager manager;
    if (!manager.loadMap(map, x, y) || manager.getLoadedTilesCount() != 1)
        return 3;
    auto* mesh = manager.GetNavMesh(map);
    // A repeated load must succeed without replacing the mesh or adding a tile.
    if (!manager.loadMap(map, x, y) || manager.getLoadedTilesCount() != 1 || manager.GetNavMesh(map) != mesh)
        return 4;
    if (!manager.unloadMap(map, x, y) || manager.getLoadedTilesCount() != 0)
        return 5;
    if (!manager.loadMap(map, x, y) || manager.getLoadedTilesCount() != 1)
        return 6;
    // A genuinely missing map remains an error; the caller's warning stays useful.
    if (manager.loadMap(4294967295u, 0, 0) || manager.getLoadedTilesCount() != 1)
        return 7;
    // A tile loading by itself is insufficient: ensure its internal location
    // agrees with the parent origin and existing neighboring tiles can coexist.
    auto directory = std::filesystem::path(sConfigMgr->GetStringDefault("DataDir", ".")) / "mmaps";
    unsigned expectedTiles=1;
    for (int dx=-1; dx<=1; ++dx)
        for (int dy=-1; dy<=1; ++dy)
        {
            if ((!dx && !dy) || x+dx<0 || y+dy<0) continue;
            char tileFile[64];
            std::snprintf(tileFile,sizeof(tileFile),"%04u_%02i_%02i.mmtile",map,x+dx,y+dy);
            if (!std::filesystem::exists(directory / tileFile)) continue;
            if (!manager.loadMap(map,x+dx,y+dy)) return 9;
            ++expectedTiles;
        }
    if (manager.getLoadedTilesCount()!=expectedTiles) return 10;
    mesh=manager.GetNavMesh(map);
    for (int i=0; i<mesh->getMaxTiles(); ++i)
    {
        dtMeshTile const* tile=mesh->getTile(i);
        if (!tile || !tile->header) continue;
        float center[3];
        for (int axis=0;axis<3;++axis) center[axis]=(tile->header->bmin[axis]+tile->header->bmax[axis])*0.5f;
        int tx,ty;
        mesh->calcTileLoc(center,&tx,&ty);
        if (tx!=tile->header->x || ty!=tile->header->y)
        {
            std::cerr << "FAIL: tile coordinates disagree with existing parent origin\n";
            return 11;
        }
    }
    if (!manager.unloadMap(map) || manager.getLoadedTilesCount() != 0)
        return 8;
    std::cout << "PASS: real tile load, duplicate, unload/reload, missing-map rejection, parent origin and " << expectedTiles << " neighboring tiles\n";
}
