// Test the internal edge selector in its own translation unit.
#include "../../dep/recastnavigation/Detour/Source/DetourNavMesh.cpp"
#include <cmath>
#include <iostream>

int main()
{
    float verts[] = {0,0,0, 4,0,0, 0,0,4};
    dtMeshHeader header = {};
    header.vertCount = 3;
    dtPoly poly = {};
    poly.vertCount = 3;
    poly.verts[0] = 0; poly.verts[1] = 1; poly.verts[2] = 2;
    dtPolyDetail detail = {};
    detail.triCount = 1;
    unsigned char tris[] = {0,1,2,0x15};
    dtMeshTile& tile = *static_cast<dtMeshTile*>(dtAlloc(sizeof(dtMeshTile), DT_ALLOC_TEMP));
    memset(&tile, 0, sizeof(tile));
    tile.header = &header; tile.polys = &poly; tile.verts = verts;
    tile.detailMeshes = &detail; tile.detailTris = tris;
    float pos[] = {2,0,-1}, closest[3];
    for (int mode = 0; mode < 3; ++mode)
    {
        if (mode == 1) tris[3] = 0;
        if (mode == 2) detail.triCount = 0;
        closestPointOnDetailEdges<true>(&tile, &poly, pos, closest);
        if (std::fabs(closest[0]-2) > 0.001f || std::fabs(closest[2]) > 0.001f)
            return 1;
    }
    poly.vertCount = 0;
    closestPointOnDetailEdges<true>(&tile, &poly, pos, closest);
    if (closest[0] != pos[0] || closest[2] != pos[2]) return 2;
    dtFree(&tile);
    std::cout << "PASS: marked edges, unmarked edges, zero triangles, zero vertices\n";
}
