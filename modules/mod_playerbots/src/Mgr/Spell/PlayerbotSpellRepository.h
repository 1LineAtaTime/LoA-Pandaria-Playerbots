/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Db/PlayerbotSpellRepository.h/.cpp (#611), the cache ListSpellsAction reads.
 * 5.4.8 / fork adaptations:
 *  - AC fills it from Playerbots.cpp:460 at startup. Playerbots.cpp is outside this lane, so Initialize() is also run
 *    lazily (std::call_once) by the first GetSkillLine()/IsItemBuyable() call; the result is identical, it only runs
 *    on first use. Only the dormant "cast random spell"/"craft random item"/"spells" actions call it.
 *  - LOG_INFO/LOG_DEBUG -> TC_LOG_INFO/TC_LOG_DEBUG, Field::Get<T>() -> Field::GetInt32().
 */

#ifndef PLAYERBOTS_PLAYERBOTSPELLREPOSITORY_H
#define PLAYERBOTS_PLAYERBOTSPELLREPOSITORY_H

#include "Common.h"

#include <mutex>
#include <unordered_map>
#include <unordered_set>

struct SkillLineAbilityEntry;

class PlayerbotSpellRepository
{
public:
    static PlayerbotSpellRepository& Instance()
    {
        static PlayerbotSpellRepository instance;
        return instance;
    }

    void Initialize();
    SkillLineAbilityEntry const* GetSkillLine(uint32 spellId);
    bool IsItemBuyable(uint32 itemId);

private:
    PlayerbotSpellRepository() = default;
    void EnsureInitialized();

    std::once_flag initFlag;
    std::unordered_map<uint32, SkillLineAbilityEntry const*> skillSpells;
    std::unordered_set<uint32> vendorItems;
};

#endif
