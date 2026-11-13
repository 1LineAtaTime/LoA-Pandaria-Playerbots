/*
 * This file is part of the Legends of Azeroth Pandaria Project.
 * Licensed under the GNU General Public License, version 2 or later.
 */

#ifndef DATABASE_SERVER_VERSION_H
#define DATABASE_SERVER_VERSION_H

#include <cstdint>
#include <optional>
#include <string_view>

namespace DatabaseServer
{
enum class Family { MySQL, MariaDB };

struct Version
{
    Family Product;
    std::uint32_t Number;

    constexpr bool IsSupported() const
    {
        // utf8mb4 was introduced in MySQL 5.5.3. Client-library requirements
        // remain independent of the database server's version.
        return Number >= (Product == Family::MariaDB ? 100209u : 50503u);
    }
};

inline bool ContainsMariaDB(std::string_view text)
{
    constexpr std::string_view name = "mariadb";
    for (std::size_t i = 0; i + name.size() <= text.size(); ++i)
    {
        std::size_t j = 0;
        for (; j < name.size(); ++j)
        {
            char value = text[i + j];
            if (value >= 'A' && value <= 'Z')
                value += 'a' - 'A';
            if (value != name[j])
                break;
        }
        if (j == name.size())
            return true;
    }
    return false;
}

// Parse the SQL VERSION() result, not mysql_get_server_version(), whose
// interpretation of MariaDB's compatibility handshake depends on the connector.
inline std::optional<Version> ParseVersion(std::string_view version, std::string_view comment)
{
    Family product = ContainsMariaDB(version) || ContainsMariaDB(comment) ? Family::MariaDB : Family::MySQL;
    constexpr std::string_view compatibilityPrefix = "5.5.5-";
    if (product == Family::MariaDB && version.substr(0, compatibilityPrefix.size()) == compatibilityPrefix)
        version.remove_prefix(compatibilityPrefix.size());

    std::uint32_t parts[3] = {};
    std::size_t offset = 0;
    for (std::size_t i = 0; i < 3; ++i)
    {
        std::size_t start = offset;
        std::uint32_t const limit = i == 0 ? 9999u : 99u;
        while (offset < version.size() && version[offset] >= '0' && version[offset] <= '9')
        {
            parts[i] = parts[i] * 10 + (version[offset++] - '0');
            if (parts[i] > limit)
                return std::nullopt;
        }
        if (offset == start || (i != 2 && (offset == version.size() || version[offset++] != '.')))
            return std::nullopt;
    }
    if (!parts[0] || (offset != version.size() && version[offset] != '-' && version[offset] != '+'))
        return std::nullopt;

    return Version{ product, parts[0] * 10000u + parts[1] * 100u + parts[2] };
}
}

#endif
