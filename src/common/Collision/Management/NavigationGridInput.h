// GPL v2 or later. Navigation input availability for map format10.
#ifndef NAVIGATION_GRID_INPUT_H
#define NAVIGATION_GRID_INPUT_H
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

namespace NavigationGridInput
{
    using File = std::unique_ptr<FILE, decltype(&std::fclose)>;
    inline File Open(std::string const& path) { return File(std::fopen(path.c_str(), "rb"), &std::fclose); }

    // Only a confirmed absence is empty; permission and other I/O failures
    // must still go through the ordinary loader and its diagnostics.
    inline bool Missing(std::string const& path)
    {
        errno = 0;
        File file = Open(path);
        return !file && errno == ENOENT;
    }

    struct MapHeader { std::uint32_t fields[11]; };
    struct HeightHeader { std::uint32_t magic, flags; float minimum, maximum; };
    struct LiquidHeader
    {
        std::uint32_t magic;
        std::uint16_t flags, type;
        std::uint8_t x, y, width, height;
        float level;
    };
    static_assert(sizeof(MapHeader) == 44 && sizeof(HeightHeader) == 16 && sizeof(LiquidHeader) == 16, "Map format10 layout");

    inline bool EmptyTerrain(std::string const& path)
    {
        errno = 0;
        File file = Open(path);
        if (!file)
            return errno == ENOENT;
        MapHeader map{};
        if (std::fread(&map, sizeof(map), 1, file.get()) != 1 ||
            std::memcmp(&map.fields[0], "MAPS", 4) || map.fields[1] != 10)
            return false;
        if (std::fseek(file.get(), 0, SEEK_END) != 0)
            return false;
        long length = std::ftell(file.get());
        if (length < long(sizeof(map)))
            return false;
        for (int field = 3; field < 11; field += 2)
            if (map.fields[field] && (map.fields[field] < sizeof(map) ||
                std::uint64_t(map.fields[field]) + map.fields[field + 1] > std::uint64_t(length)))
                return false;
        HeightHeader height{};
        if (map.fields[5] < sizeof(map) || map.fields[6] < sizeof(height) ||
            std::fseek(file.get(), map.fields[5], SEEK_SET) ||
            std::fread(&height, sizeof(height), 1, file.get()) != 1 || std::memcmp(&height.magic, "MHGT", 4) ||
            !(height.flags & 1) || height.minimum != -500.0f || height.maximum != -500.0f)
            return false;
        if (!map.fields[7])
            return true;
        LiquidHeader liquid{};
        if (map.fields[8] < sizeof(liquid) || std::fseek(file.get(), map.fields[7], SEEK_SET) ||
            std::fread(&liquid, sizeof(liquid), 1, file.get()) != 1 || std::memcmp(&liquid.magic, "MLIQ", 4))
            return false;
        // The generator excludes dark ocean. Mixed/per-cell liquids remain
        // uncertain and must use the normal loader, as must ordinary water.
        return (liquid.flags & 1) && (!liquid.type || (liquid.type & 0x10));
    }

    inline bool EmptyGrid(std::string const& terrain, std::string const& collisionTree,
        std::string const& collisionTile, std::string const& navigationTile)
    {
        if (!Missing(navigationTile) || !Missing(collisionTile))
            return false; // Existing files, including corrupt ones, must be validated.
        File tree = Open(collisionTree);
        char header[9]{};
        if (!tree || std::fread(header, sizeof(header), 1, tree.get()) != 1 ||
            std::memcmp(header, "VMAP_4.3", 8) || header[8] != 1)
            return false; // Missing/bad roots and global-WMO maps are not empty.
        return EmptyTerrain(terrain);
    }
}
#endif
