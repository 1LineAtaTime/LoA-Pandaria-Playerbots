/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TellReputationAction.cpp. 5.4.8 differences: FactionEntry::name is a
 * plain string here (AC's is a localized array, entry->name[0]), and the targeted-unit branch checks the unit's
 * faction template before using it (AC dereferences it unchecked). The reputation list itself is read from the
 * bot's own faction state, so the Pandaren factions show up exactly like the older ones.
 */

#include "TellReputationAction.h"

#include "DBCStores.h"
#include "Event.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "ReputationMgr.h"
#include "SharedDefines.h"

#include <algorithm>

std::string TellReputationAction::BuildReputationLine(FactionEntry const* entry)
{
    ReputationMgr& repMgr = bot->GetReputationMgr();
    ReputationRank rank = repMgr.GetRank(entry);
    int32 reputation = repMgr.GetReputation(entry->ID);

    std::ostringstream out;
    out << entry->name << ": |cff";

    switch (rank)
    {
        case REP_HATED:
            out << "cc2222hated";
            break;
        case REP_HOSTILE:
            out << "ff0000hostile";
            break;
        case REP_UNFRIENDLY:
            out << "ee6622unfriendly";
            break;
        case REP_NEUTRAL:
            out << "ffff00neutral";
            break;
        case REP_FRIENDLY:
            out << "00ff00friendly";
            break;
        case REP_HONORED:
            out << "00ff88honored";
            break;
        case REP_REVERED:
            out << "00ffccrevered";
            break;
        case REP_EXALTED:
            out << "00ffffexalted";
            break;
        default:
            out << "808080unknown";
            break;
    }

    out << "|cffffffff";

    int32 base = ReputationMgr::Reputation_Cap + 1;
    for (int32 i = MAX_REPUTATION_RANK - 1; i >= rank; --i)
        base -= ReputationMgr::PointsInRank[i];

    out << " (" << (reputation - base) << "/" << ReputationMgr::PointsInRank[rank] << ")";
    return out.str();
}

bool TellReputationAction::Execute(Event event)
{
    std::string const param = event.getParam();
    if (param == "all")
    {
        ReputationMgr& repMgr = bot->GetReputationMgr();
        std::vector<std::string> lines;

        FactionStateList const& stateList = repMgr.GetStateList();
        lines.reserve(stateList.size());

        for (auto const& itr : stateList)
        {
            FactionState const& faction = itr.second;
            if (!(faction.Flags & FACTION_FLAG_VISIBLE))
                continue;

            if (faction.Flags & (FACTION_FLAG_HIDDEN | FACTION_FLAG_INVISIBLE_FORCED) &&
                !(faction.Flags & FACTION_FLAG_SPECIAL))
                continue;

            FactionEntry const* entry = sFactionStore.LookupEntry(faction.ID);
            if (!entry)
                continue;

            lines.push_back(BuildReputationLine(entry));
        }

        std::sort(lines.begin(), lines.end());

        botAI->TellMaster("=== Reputations ===");
        for (auto const& line : lines)
            botAI->TellMaster(line);

        return true;
    }

    Player* master = GetMaster();
    if (!master)
        return false;

    ObjectGuid selection = master->GetTarget();
    if (selection.IsEmpty())
        return false;

    Unit* unit = ObjectAccessor::GetUnit(*master, selection);
    if (!unit)
        return false;

    FactionTemplateEntry const* factionTemplate = unit->GetFactionTemplateEntry();
    if (!factionTemplate)
        return false;

    FactionEntry const* entry = sFactionStore.LookupEntry(factionTemplate->faction);
    if (!entry)
        return false;

    botAI->TellMaster(BuildReputationLine(entry));

    return true;
}
