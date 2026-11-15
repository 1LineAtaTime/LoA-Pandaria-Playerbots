/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Mgr/Item/BisListMgr.{h,cpp}: the best-in-slot table behind the "autogear bis" chat
 * command. Differences: the table is read the first time it is needed (query text instead of a prepared statement, the
 * same way CheckMountStateAction reads its table), so no startup hook is required, and the table ships empty - there is
 * no 5.4.8 best-in-slot data, so "autogear bis" falls back to a normal autogear until rows are added.
 */

#ifndef _PLAYERBOT_BISLISTMGR_H
#define _PLAYERBOT_BISLISTMGR_H

#include "Define.h"

#include <map>
#include <mutex>

class BisListMgr
{
public:
    static BisListMgr* instance()
    {
        static BisListMgr inst;
        return &inst;
    }

    void LoadAll();

    // faction: 1=Alliance, 2=Horde. Faction-specific rows override faction=0 (Both).
    // Returns slot -> itemId for the matching auto_gear_score_limit tier. Empty map = no data.
    std::map<uint8, uint32> GetBisFor(uint16 autoGearScoreLimit, uint8 cls, uint8 tab, uint8 faction) const;

    // Closest-lower fallback: scan ilvls down from requested to (requested - maxDrop), return first non-empty set.
    // outResolved receives the matched ilvl (0 if nothing matched within the window).
    std::map<uint8, uint32> GetBisForNearest(uint16 requestedIlvl, uint16 maxDrop, uint8 cls, uint8 tab,
                                             uint8 faction, uint16* outResolved = nullptr) const;

private:
    BisListMgr() = default;

    static uint16 MakeKey(uint8 cls, uint8 tab) { return (uint16(cls) << 8) | tab; }

    // autoGearScoreLimit -> (cls<<8|tab) -> faction (0/1/2) -> slot -> itemId
    std::map<uint16, std::map<uint16, std::map<uint8, std::map<uint8, uint32>>>> _bis;
    std::once_flag _loaded;
};

#define sBisListMgr BisListMgr::instance()

#endif
