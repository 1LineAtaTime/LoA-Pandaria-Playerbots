/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/ListSpellsAction.cpp (#611).
 * 5.4.8 / fork adaptations vs AC (each also marked at the site): SpellName is char const*; PlayerSpell has lowercase
 * state/active and no specMask; skill-line trivial ranks are max_value; ParseSkill / the reagent count are the
 * ChatHelper::parseSkill / InventoryAction::GetItemCount bodies (neither exists on this fork); LOG_ERROR -> TC_LOG_ERROR (printf-style on this core);
 * PlayerbotSpellRepository is Mgr/Spell/PlayerbotSpellRepository.h. AC declares ListSpellsAction::InitSpellCaches but
 * never defines it, so it is declared only.
 */

#include "ListSpellsAction.h"
#include "Event.h"
#include "ChatHelper.h"
#include "Helper.h"
#include "ItemVisitors.h"
#include "PlayerbotSpellRepository.h"
#include "Playerbots.h"

#include <algorithm>
#include <cstring>
#include <map>

using SpellListEntry = std::pair<uint32, std::string>;

// CHANGE: Simplified and cheap comparator used in MapUpdater worker thread.
// It now avoids scanning the entire SkillLineAbilityStore for each comparison
// and only relies on spell school and spell name to keep sorting fast and bounded.
// lhs = the left element, rhs = the right element.
// 5.4.8: this fork's ChatHelper has no parseSkill() (AC Bot/Cmd/ChatHelper.cpp:105-117,663-669), so the skills map and
// parseSkill are carried here verbatim (ChatHelper.h is outside this lane).
static uint32 ParseSkill(std::string const& text)
{
    static std::map<std::string, uint32> const skills = {
        {"first aid", SKILL_FIRST_AID},
        {"fishing", SKILL_FISHING},
        {"cooking", SKILL_COOKING},
        {"alchemy", SKILL_ALCHEMY},
        {"enchanting", SKILL_ENCHANTING},
        {"engineering", SKILL_ENGINEERING},
        {"leatherworking", SKILL_LEATHERWORKING},
        {"blacksmithing", SKILL_BLACKSMITHING},
        {"tailoring", SKILL_TAILORING},
        {"herbalism", SKILL_HERBALISM},
        {"mining", SKILL_MINING},
        {"skinning", SKILL_SKINNING},
        {"jewelcrafting", SKILL_JEWELCRAFTING},
    };

    auto const itr = skills.find(text);
    if (itr != skills.end())
        return itr->second;

    return SKILL_NONE;
}

static bool CompareSpells(SpellListEntry const& lhSpell, SpellListEntry const& rhSpell)
{
    SpellInfo const* lhSpellInfo = sSpellMgr->GetSpellInfo(lhSpell.first);
    SpellInfo const* rhSpellInfo = sSpellMgr->GetSpellInfo(rhSpell.first);

    if (!lhSpellInfo || !rhSpellInfo)
    {
        TC_LOG_ERROR("playerbots", "SpellInfo missing for spell %u or %u", lhSpell.first, rhSpell.first);
        // Fallback: order by spell id to keep comparator strict and deterministic.
        return lhSpell.first < rhSpell.first;
    }

    uint32 lhsKey = lhSpellInfo->SchoolMask;
    uint32 rhsKey = rhSpellInfo->SchoolMask;

    if (lhsKey == rhsKey)
    {
        // Defensive check: if DBC data is broken and spell names are nullptr,
        // fall back to id ordering instead of risking a crash in std::strcmp.
        // 5.4.8: SpellInfo::SpellName is a plain `char const*` (SpellInfo.h:353), not AC's per-locale array.
        if (!lhSpellInfo->SpellName || !rhSpellInfo->SpellName)
            return lhSpell.first < rhSpell.first;

        return std::strcmp(lhSpellInfo->SpellName, rhSpellInfo->SpellName) > 0;
    }
    return lhsKey > rhsKey;
}

std::vector<std::pair<uint32, std::string>> ListSpellsAction::GetSpellList(std::string filter)
{
    uint32 skill = 0;

    std::vector<std::string> ss = split(filter, ' ');
    if (!ss.empty())
    {
        skill = ParseSkill(ss[0]);
        if (skill != SKILL_NONE)
        {
            filter = ss.size() > 1 ? ss[1] : "";
        }

        // Guard access to ss[1]/ss[2] to avoid out-of-bounds
        // when the player only types "first" without "aid".
        if (ss[0] == "first" && ss.size() > 1 && ss[1] == "aid")
        {
            skill = SKILL_FIRST_AID;
            filter = ss.size() > 2 ? ss[2] : "";
        }
    }

    std::string const ignoreList =
        ",Opening,Closing,Stuck,Remove Insignia,Opening - No Text,Grovel,Duel,Honorless Target,";
    std::string alreadySeenList = ",";

    uint32 minLevel = 0;
    uint32 maxLevel = 0;
    if (filter.find('-') != std::string::npos)
    {
        std::vector<std::string> ff = split(filter, '-');
        if (ff.size() >= 2)
        {
            minLevel = std::atoi(ff[0].c_str());
            maxLevel = std::atoi(ff[1].c_str());
            if (minLevel > maxLevel)
                std::swap(minLevel, maxLevel);
        }
        filter.clear();
    }

    bool canCraftNow = false;
    if (filter.find('+') != std::string::npos)
    {
        canCraftNow = true;

        // Support "+<skill>" syntax (e.g. "spells +tailoring" or "spells tailoring+").
        // If no explicit skill was detected yet, try to parse the filter (without '+')
        // as a profession/skill name so that craftable-only filters still work with skills.
        if (skill == SKILL_NONE)
        {
            std::string skillFilter = filter;

            // Remove '+' before trying to interpret the first token as a skill name.
            skillFilter.erase(remove(skillFilter.begin(), skillFilter.end(), '+'), skillFilter.end());

            std::vector<std::string> skillTokens = split(skillFilter, ' ');
            if (!skillTokens.empty())
            {
                uint32 parsedSkill = ParseSkill(skillTokens[0]);
                if (parsedSkill != SKILL_NONE)
                {
                    skill = parsedSkill;

                    // Any remaining text after the skill token becomes the "name" filter
                    // (e.g. "spells +tailoring cloth" -> skill = tailoring, filter = "cloth").
                    filter = skillTokens.size() > 1 ? skillTokens[1] : "";
                }
            }
        }
        // Finally remove '+' from the filter that will be used for name/range parsing.
        filter.erase(remove(filter.begin(), filter.end(), '+'), filter.end());
    }

    uint32 slot = chat->parseSlot(filter);
    if (slot != EQUIPMENT_SLOT_END)
        filter.clear();

    std::vector<SpellListEntry> spells;
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        if (itr->second->state == PLAYERSPELL_REMOVED || !itr->second->active)
            continue;

        // AC: `if (!(itr->second->specMask & bot->GetActiveSpecMask())) continue;` - 5.4.8's PlayerSpell has no specMask
        // (Player.h:140-146); every learned spell is in the single active set.

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
        if (!spellInfo)
            continue;

        if (spellInfo->IsPassive())
            continue;

        SkillLineAbilityEntry const* skillLine = PlayerbotSpellRepository::Instance().GetSkillLine(itr->first);
        if (skill != SKILL_NONE && (!skillLine || skillLine->SkillLine != skill))
            continue;

        if (!spellInfo->SpellName)
            continue;

        std::string const comp = spellInfo->SpellName;
        if (!(ignoreList.find(comp) == std::string::npos && alreadySeenList.find(comp) == std::string::npos))
            continue;

        if (!filter.empty() && !strstri(spellInfo->SpellName, filter.c_str()))
            continue;

        bool first = true;
        int32 craftsPossible = -1;
        std::ostringstream materials;
        for (uint32 x = 0; x < MAX_SPELL_REAGENTS; ++x)
        {
            if (spellInfo->Reagent[x] <= 0)
            {
                continue;
            }

            uint32 itemid = spellInfo->Reagent[x];
            uint32 reagentsRequired = spellInfo->ReagentCount[x];
            if (itemid)
            {
                if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemid))
                {
                    if (first)
                    {
                        materials << ": ";
                        first = false;
                    }
                    else
                        materials << ", ";

                    materials << chat->FormatItem(proto, reagentsRequired);

                    FindItemByIdVisitor visitor(itemid);
                    // AC InventoryAction::GetItemCount (InventoryAction.cpp:400-410), inlined: this fork's InventoryAction has
                    // no GetItemCount; same sum over the bag items (AC's default mask).
                    IterateItems(&visitor);
                    uint32 reagentsInInventory = 0;
                    for (Item* reagentItem : visitor.GetResult())
                        reagentsInInventory += reagentItem->GetCount();
                    bool buyable = PlayerbotSpellRepository::Instance().IsItemBuyable(itemid);
                    if (!buyable)
                    {
                        uint32 craftable = reagentsInInventory / reagentsRequired;
                        if (craftsPossible < 0 || craftsPossible > static_cast<int32>(craftable))
                            craftsPossible = static_cast<int32>(craftable);
                    }

                    if (reagentsInInventory)
                        materials << "|cffffff00(x" << reagentsInInventory << ")|r ";
                    else if (buyable)
                        materials << "|cffffff00(buy)|r ";
                }
            }
        }

        if (craftsPossible < 0)
            craftsPossible = 0;

        std::ostringstream out;
        bool filtered = false;
        if (skillLine)
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                if (spellInfo->Effects[i].Effect == SPELL_EFFECT_CREATE_ITEM)
                {
                    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(spellInfo->Effects[i].ItemType))
                    {
                        if (craftsPossible)
                            out << "|cffffff00(x" << craftsPossible << ")|r ";

                        out << chat->FormatItem(proto);

                        if ((minLevel || maxLevel) && (!proto->RequiredLevel || proto->RequiredLevel < minLevel ||
                                                       proto->RequiredLevel > maxLevel))
                        {
                            filtered = true;
                            break;
                        }

                        if (slot != EQUIPMENT_SLOT_END && bot->FindEquipSlot(proto, slot, true) != slot)
                        {
                            filtered = true;
                            break;
                        }
                    }
                }
            }
        }

        if (out.str().empty())
            out << chat->FormatSpell(spellInfo);

        if (filtered)
            continue;

        if (canCraftNow && !craftsPossible)
            continue;

        out << materials.str();

        if (skillLine && skillLine->SkillLine)
        {
            // 5.4.8 SkillLineAbilityEntry: max_value = trivial skill high (DBCStructure.h:2139).
            uint32 GrayLevel = skillLine->max_value;
            uint32 GreenLevel = (skillLine->max_value + skillLine->MinSkillLineRank) / 2;
            uint32 YellowLevel = skillLine->MinSkillLineRank;
            uint32 SkillValue = bot->GetSkillValue(skillLine->SkillLine);

            out << " - ";
            if (SkillValue >= GrayLevel)
                out << " |cff808080gray";
            else if (SkillValue >= GreenLevel)
                out << " |cff80be80green";
            else if (SkillValue >= YellowLevel)
                out << " |cffffff00yellow";
            else
                out << " |cffff8040orange";

            out << "|r";
        }

        if (out.str().empty())
            continue;

        if (itr->first == 0)
            TC_LOG_ERROR("playerbots", "?! %u", itr->first);

        spells.emplace_back(itr->first, out.str());
        alreadySeenList += spellInfo->SpellName;
        alreadySeenList += ",";
    }

    return spells;
}

bool ListSpellsAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    std::string const filter = event.getParam();

    std::vector<SpellListEntry> spells = GetSpellList(filter);

    if (spells.empty())
    {
        // CHANGE: Give early feedback when no spells match the filter.
        botAI->TellMaster("No spells found.");
        return true;
    }

    botAI->TellMaster("=== Spells ===");

    std::sort(spells.begin(), spells.end(), CompareSpells);

    // CHANGE: Send the full spell list again so client-side addons
    // (e.g. Multibot / Unbot) can reconstruct the
    // complete spellbook for configuration. The heavy part that caused
    // freezes before was the old CompareSpells implementation scanning
    // the entire SkillLineAbility DBC on every comparison. With the new
    // cheap comparator above, sending all lines here is safe and keeps
    // behaviour compatible with existing addons.
    for (std::vector<SpellListEntry>::const_iterator i = spells.begin(); i != spells.end(); ++i)
        botAI->TellMasterNoFacing(i->second);

    return true;
}
