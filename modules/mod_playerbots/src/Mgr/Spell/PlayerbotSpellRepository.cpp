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

#include "PlayerbotSpellRepository.h"
#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "Field.h"
#include "Log.h"
#include "QueryResult.h"

//  caches the result set (AC Db/PlayerbotSpellRepository.cpp:13-40)
void PlayerbotSpellRepository::Initialize()
{
    TC_LOG_INFO("playerbots", "Playerbots: ListSpellsAction caches initialized");

    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        if (SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j))
            skillSpells[skillLine->Spell] = skillLine;
    }

    // Fill the vendorItems cache once from the world database.
    QueryResult results = WorldDatabase.Query("SELECT item FROM npc_vendor WHERE maxcount = 0");
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            int32 entry = fields[0].GetInt32();
            if (entry <= 0)
                continue;

            vendorItems.insert(static_cast<uint32>(entry));
        } while (results->NextRow());
    }

    TC_LOG_DEBUG("playerbots", "ListSpellsAction: initialized caches (skillSpells=%u, vendorItems=%u).",
                 uint32(skillSpells.size()), uint32(vendorItems.size()));
}

void PlayerbotSpellRepository::EnsureInitialized()
{
    std::call_once(initFlag, [this]() { Initialize(); });
}

SkillLineAbilityEntry const* PlayerbotSpellRepository::GetSkillLine(uint32 spellId)
{
    EnsureInitialized();
    auto itr = skillSpells.find(spellId);
    if (itr != skillSpells.end())
        return itr->second;
    return nullptr;
}

bool PlayerbotSpellRepository::IsItemBuyable(uint32 itemId)
{
    EnsureInitialized();
    return vendorItems.find(itemId) != vendorItems.end();
}
