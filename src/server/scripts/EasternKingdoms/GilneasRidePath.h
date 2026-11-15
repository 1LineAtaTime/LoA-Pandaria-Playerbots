#ifndef GILNEAS_RIDE_PATH_H
#define GILNEAS_RIDE_PATH_H
#include <cmath>
#include <cstdint>
#include <map>

namespace GilneasRidePath
{
    struct Point { float x, y, z; };
    using Snapshot = std::map<std::uint32_t, Point>;

    template<class Path> Snapshot Copy(Path const& source)
    {
        Snapshot result;
        std::uint32_t expected = 1;
        for (auto const& node : source)
        {
            // A ride indexes points1..N. Reject gaps instead of recursively
            // searching a huge invalid point-ID range. Never retain DB pointers.
            if (node.first != expected++ || !node.second || !std::isfinite(node.second->x) ||
                !std::isfinite(node.second->y) || !std::isfinite(node.second->z))
                return {};
            result.emplace(node.first, Point{node.second->x,node.second->y,node.second->z});
        }
        return result;
    }
}
#endif
