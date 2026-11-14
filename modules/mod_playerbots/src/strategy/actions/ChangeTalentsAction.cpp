/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/ChangeTalentsAction.cpp: the "talents" chat command
 * (talents / help / switch <1/2> / autopick / spec list / spec <name> / apply <link>), same branches, same order,
 * same replies.
 *
 * 5.4.8 adaptations vs AC (a 5.4.8 character has one specialization per talent group plus one talent per tier, not
 * three point-spent trees):
 *  - The premade specs are AiPlayerbot.PremadeSpecName/Link.<class>.<specno> (PlayerbotAIConfig.cpp), where <specno>
 *    is the ChrSpecialization TabPage and the link is one Talent.dbc column (0-2) per tier. "spec list" prints that
 *    column list instead of AC's per-tree point counts ("1. arms pve (2-1-0-x-1-2)", x = no preference).
 *  - "spec <name>" sets the specialization (CMSG_SET_PRIMARY_TALENT_TREE, as BotFactory::InitTalentsTree does) and
 *    learns the tiers through BotFactory::InitTalentsTree/InitGlyphs, replacing AC's
 *    PlayerbotFactory::InitTalentsBySpecNo + InitGlyphs. Specializations unlock at level 10, so it is refused below that.
 *  - "apply <link>" takes the same per-tier column list ("012012", '-' leaves a tier empty) instead of a WotLK talent
 *    calculator link, and learns it through Player::LearnTalent (AC: PlayerbotFactory::InitTalentsByParsedSpecLink).
 *  - "autopick" keeps the current specialization and fills the empty tiers (BotFactory::InitTalentsTree(false)), the
 *    5.4.8 counterpart of AC's InitTalentsTree(true) (increment: keep the spec tab, spend the free points).
 *  - "switch 2" only activates the second talent group when the bot has one (Player::ActivateSpec does not check that
 *    the group was learned); otherwise it learns dual specialization first, as AC does, when the level allows it.
 *  - AC's "custom_glyphs" flag reset has no counterpart: that value (AC EquipGlyphsAction) is not ported.
 */

#include "ChangeTalentsAction.h"

#include "AiFactory.h"
#include "BotFactory.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "Event.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <algorithm>
#include <sstream>
#include <vector>

namespace
{
constexpr uint32 TALENT_TIERS = 6;    // Talent.dbc TierID 0-5
constexpr uint32 TALENT_COLUMNS = 3;  // Talent.dbc ColumnIndex 0-2
constexpr uint8 SPECIALIZATION_MIN_LEVEL = 10;

bool ClassHasSpecTab(uint8 cls, uint32 tab)
{
    for (uint32 i = 0; i < sChrSpecializationStore.GetNumRows(); ++i)
        if (ChrSpecializationEntry const* spec = sChrSpecializationStore.LookupEntry(i))
            if (spec->classId == cls && spec->TabPage == tab)
                return true;
    return false;
}

// Learns one talent per unlocked tier from a column list (BotFactory::InitTalentTiers' loop, with the given link
// instead of the configured one). Talents already held are kept; the caller resets first.
void LearnTalentColumns(Player* bot, std::vector<uint16> const& columns)
{
    uint32 const tiers = std::min<uint32>(bot->CalculateTalentsPoints(), TALENT_TIERS);
    if (!tiers)
        return;

    TalentEntry const* grid[TALENT_TIERS][TALENT_COLUMNS] = {};
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talent = sTalentStore.LookupEntry(i);
        if (!talent || talent->PlayerClass != bot->GetClass() || talent->TierID >= TALENT_TIERS ||
            talent->ColumnIndex >= TALENT_COLUMNS)
            continue;
        grid[talent->TierID][talent->ColumnIndex] = talent;
    }

    if (bot->GetUInt32Value(PLAYER_FIELD_MAX_TALENT_TIERS) < tiers)
        bot->SetUInt32Value(PLAYER_FIELD_MAX_TALENT_TIERS, bot->CalculateTalentsPoints());

    for (uint32 tier = 0; tier < tiers && tier < columns.size(); ++tier)
    {
        if (columns[tier] >= TALENT_COLUMNS)
            continue;
        if (TalentEntry const* pick = grid[tier][columns[tier]])
            bot->LearnTalent(uint16(pick->ID));
    }

    bot->SendTalentsInfoData();
}

std::string FormatColumns(std::vector<uint16> const& columns)
{
    std::ostringstream out;
    for (size_t i = 0; i < columns.size(); ++i)
    {
        if (i)
            out << "-";
        if (columns[i] < TALENT_COLUMNS)
            out << columns[i];
        else
            out << "x";
    }
    return out.str();
}
}  // namespace

bool ChangeTalentsAction::Execute(Event event)
{
    std::string param = event.getParam();

    std::ostringstream out;

    if (!param.empty())
    {
        if (param.find("help") != std::string::npos)
        {
            out << TalentsHelp();
        }
        else if (param.find("switch") != std::string::npos)
        {
            if (param.find("switch 1") != std::string::npos)
            {
                bot->ActivateSpec(0);
                out << "Active first talent";
                botAI->ResetStrategies();
            }
            else if (param.find("switch 2") != std::string::npos)
            {
                if (bot->GetSpecsCount() == 1 && bot->GetLevel() >= sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL))
                {
                    bot->CastSpell(bot, 63680, true, nullptr, nullptr, bot->GetGUID());
                    bot->CastSpell(bot, 63624, true, nullptr, nullptr, bot->GetGUID());
                }
                // 5.4.8: see the "switch 2" note in the file header.
                if (bot->GetSpecsCount() < 2)
                {
                    out << "I do not have a second talent specialization";
                }
                else
                {
                    bot->ActivateSpec(1);
                    out << "Active second talent";
                    botAI->ResetStrategies();
                }
            }
        }
        else if (param.find("autopick") != std::string::npos)
        {
            BotFactory factory(bot, bot->GetLevel());
            factory.InitTalentsTree(false);
            out << "Auto pick talents";
            botAI->ResetStrategies();
        }
        else if (param.find("spec list") != std::string::npos)
        {
            out << SpecList();
        }
        else if (param.find("spec ") != std::string::npos)
        {
            param = param.substr(5);
            out << SpecPick(param);
            botAI->ResetStrategies();
        }
        else if (param.find("apply ") != std::string::npos)
        {
            param = param.substr(6);
            out << SpecApply(param);
            botAI->ResetStrategies();
        }
        else
        {
            out << "Unknown command.";
        }
    }
    else
    {
        // AC: chat->FormatClass(bot, tab), "<spec> (<tree points>)|r <class>"; 5.4.8 has no trees (see WhoAction.cpp).
        out << "My current talent spec is: "
            << "|h|cffffffff";
        out << AiFactory::GetPlayerSpecName(bot) << "|r ";
        if (ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(bot->GetClass()))
            out << classEntry->name;
        out << "\n";
        out << TalentsHelp();
    }

    botAI->TellMaster(out);

    return true;
}

std::string ChangeTalentsAction::TalentsHelp()
{
    std::ostringstream out;
    out << "Talents usage: talents switch <1/2>, talents autopick, talents spec list, "
           "talents spec <specName>, talents apply <link>.";
    return out.str();
}

std::string ChangeTalentsAction::SpecList()
{
    int cls = bot->GetClass();
    int specFound = 0;
    std::ostringstream out;
    for (uint32 specNo = 0; specNo < PlayerbotAIConfig::MAX_SPECNO; ++specNo)
    {
        if (sPlayerbotAIConfig->premadeSpecName[cls][specNo].size() == 0)
        {
            break;
        }
        specFound++;
        std::ostringstream out;
        out << specFound << ". " << sPlayerbotAIConfig->premadeSpecName[cls][specNo] << " (";
        out << FormatColumns(sPlayerbotAIConfig->premadeSpecLink[cls][specNo]) << ")";
        botAI->TellMasterNoFacing(out.str());
    }
    out << "Total " << specFound << " specs found";
    return out.str();
}

std::string ChangeTalentsAction::SpecPick(std::string param)
{
    uint8 const cls = bot->GetClass();
    for (uint32 specNo = 0; specNo < PlayerbotAIConfig::MAX_SPECNO; ++specNo)
    {
        if (sPlayerbotAIConfig->premadeSpecName[cls][specNo].size() == 0)
        {
            break;
        }
        if (sPlayerbotAIConfig->premadeSpecName[cls][specNo] == param)
        {
            if (bot->GetLevel() < SPECIALIZATION_MIN_LEVEL)
            {
                std::ostringstream out;
                out << "I can not pick a specialization before level " << uint32(SPECIALIZATION_MIN_LEVEL);
                return out.str();
            }

            // <specno> is the TabPage; a premade entry past the class's tabs is reached through
            // AiPlayerbot.RandomClassSpecIndex (BotFactory.cpp GetPremadeSpecIndex), as AC's randomClassSpecIndex.
            uint32 tab = specNo;
            if (!ClassHasSpecTab(cls, tab))
            {
                tab = PlayerbotAIConfig::MAX_SPECNO;
                for (uint32 t = 0; t < PlayerbotAIConfig::MAX_SPECNO; ++t)
                    if (sPlayerbotAIConfig->randomClassSpecIndex[cls][t] == specNo && ClassHasSpecTab(cls, t))
                        tab = t;
                if (tab == PlayerbotAIConfig::MAX_SPECNO)
                    break;
            }

            // The PlayerbotMgr "setspec" sequence: reset talents and specialization, set the new one, learn it.
            bot->ResetTalents(true, true, true);
            WorldPacket packet(CMSG_SET_PRIMARY_TALENT_TREE);
            packet << uint32(tab);
            bot->GetSession()->HandeSetTalentSpecialization(packet);

            BotFactory factory(bot, bot->GetLevel());
            factory.InitTalentsTree(false);
            factory.InitGlyphs(false);

            std::ostringstream out;
            out << "Picking " << sPlayerbotAIConfig->premadeSpecName[cls][specNo];
            return out.str();
        }
    }
    std::ostringstream out;
    out << "Spec " << param << " not found";
    return out.str();
}

std::string ChangeTalentsAction::SpecApply(std::string param)
{
    std::ostringstream out;
    std::vector<uint16> columns;
    for (char c : param)
    {
        if (c >= '0' && c <= '2')
            columns.push_back(uint16(c - '0'));
        else if (c == '-')
            columns.push_back(PlayerbotAIConfig::PREMADE_SPEC_NO_CHOICE);
        else
        {
            columns.clear();
            break;
        }
    }
    if (columns.size() == 0 || columns.size() > TALENT_TIERS)
    {
        out << "Invalid link " << param;
        return out.str();
    }
    bot->ResetTalents(true, true, false);
    LearnTalentColumns(bot, columns);
    out << "Applying " << param;
    return out.str();
}

bool AutoSetTalentsAction::Execute(Event /*event*/)
{
    std::ostringstream out;

    if (!sPlayerbotAIConfig->autoPickTalents || !sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    if ((bot->CalculateTalentsPoints() - bot->GetUsedTalentCount()) <= 0)
        return false;

    // 5.4.8: AC's InitTalentsTree(true, true, true) + InitPetTalents (no 5.4.8 pet talent trees), as in
    // AutoMaintenanceOnLevelupAction::AutoPickTalents.
    BotFactory factory(bot, bot->GetLevel());
    factory.InitTalentsTree(false);
    botAI->TellMaster(out);

    return true;
}
