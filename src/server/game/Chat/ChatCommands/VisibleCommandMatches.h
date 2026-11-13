#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace Trinity::Impl::ChatCommands
{
// An inaccessible exact command must not be interpreted as another command's
// abbreviation. Permission checks still apply before any handler is dispatched.
template<class Map, class Visible, class StartsWith>
auto VisibleCommandMatches(Map const& map, std::string_view token, Visible visible, StartsWith startsWith)
    -> std::vector<typename Map::mapped_type const*>
{
    std::vector<typename Map::mapped_type const*> matches;
    if (token.empty()) return matches;
    auto exact = map.find(std::string(token));
    if (exact != map.end())
    {
        if (visible(exact->second)) matches.push_back(&exact->second);
        return matches;
    }
    for (auto const& pair : map)
        if (startsWith(pair.first, token) && visible(pair.second))
            matches.push_back(&pair.second);
    return matches;
}
}
