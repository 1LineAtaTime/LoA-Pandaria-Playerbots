/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// AC Ai/Dungeon/DungeonStrategyUtils.h:1-27 (#545). Unchanged: 5.4.8 has the same enum names
// (src/server/shared/DataStores/DBCEnums.h:411-412, NORMAL=1 / HEROIC=2 instead of 0 / 1) and
// Map::GetDifficulty() (src/server/game/Maps/Map.h:424).
// 5.4.8 note for spell ids: a boss's heroic cast keeps the SAME spell id with a per-difficulty SpellInfo
// (SpellMgr.cpp:471-476), so a DUNGEON_MODE(<normal id>, <heroic id>) on a spell is only right where the
// fork's boss script itself casts the heroic id - each dungeon port checks its scripts.

#ifndef PLAYERBOTS_DUNGEONSTRATEGYUTILS_H
#define PLAYERBOTS_DUNGEONSTRATEGYUTILS_H

#include "Define.h"  // uint8 for DBCEnums.h (it has no includes of its own)
#include "DBCEnums.h"
#include "Player.h"

template<class T> inline
T const& DUNGEON_MODE(Player* bot, T const& normal5, T const& heroic10)
{
    switch (bot->GetMap()->GetDifficulty())
    {
        case DUNGEON_DIFFICULTY_NORMAL:
            return normal5;
        case DUNGEON_DIFFICULTY_HEROIC:
            return heroic10;
        default:
            break;
    }

    return heroic10;
}

#endif
