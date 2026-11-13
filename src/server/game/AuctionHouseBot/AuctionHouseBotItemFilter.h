#ifndef AUCTION_HOUSE_BOT_ITEM_FILTER_H
#define AUCTION_HOUSE_BOT_ITEM_FILTER_H

#include <algorithm>
#include <cctype>
#include <string>

namespace AuctionHouseBotItemFilter
{
    inline bool HasRestrictedMarker(std::string const& name)
    {
        std::string upper = name;
        std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) { return char(std::toupper(c)); });

        static char const* markers[] = {
            "TEST", "DEPRECATED", "UNUSED", "DEBUG", "PLACEHOLDER", "DNT", "DNU", "QA", "NYI",
            "DO NOT USE", "NOT USED"
        };
        for (char const* marker : markers)
        {
            std::size_t length = std::char_traits<char>::length(marker);
            for (std::size_t pos = upper.find(marker); pos != std::string::npos; pos = upper.find(marker, pos + 1))
            {
                bool leftBoundary = pos == 0 || !std::isalnum(static_cast<unsigned char>(upper[pos - 1]));
                bool rightBoundary = pos + length == upper.size() || !std::isalnum(static_cast<unsigned char>(upper[pos + length]));
                if (leftBoundary && rightBoundary)
                    return true;
            }
        }
        return false;
    }
}

#endif
