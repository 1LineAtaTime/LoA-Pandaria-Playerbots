/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TrainerAction.cpp for issue #258 slice "Trainer +
 * GossipHello" (the NPC interaction layer).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *
 *  - AzerothCore's Trainer class does not exist on this tree. AC walks `sObjectMgr->GetTrainer(entry)`
 *    returning a `Trainer::Trainer*` with GetSpells() / GetSpell(id) / GetTrainerType() /
 *    IsTrainerValidForPlayer() / CanTeachSpell() (AC TrainerAction.cpp:26-28, 63-80, 107-129). This core
 *    keeps the same world.npc_trainer rows in ObjectMgr's cache store - `sObjectMgr->GetNpcTrainerSpells`
 *    (Globals/ObjectMgr.h:1640, loaded by ObjectMgr.cpp AddSpellToTrainer/LoadTrainerSpell) returning a
 *    `TrainerSpellData const*` with a `TrainerSpellMap spellList` and a `trainerType`
 *    (Entities/Creature/Creature.h:204-233). That is exactly the API the trainer-taught-spell sweep added
 *    in Factory/BotFactory.cpp:466-541 (InitAvailableSpells, issue #230) already walks, so this action
 *    reuses the same primitive rather than re-porting the learning path.
 *
 *  - CanTeachSpell + PlayerbotFactory::IsTrainerSpellAllowedForBot collapse into one call:
 *    `bot->GetTrainerSpellState(trainerSpell)` (Entities/Player/Player.h:2019). It already enforces level,
 *    reqSkill, class, race, previous rank, required spells and the primary profession limit - the union of
 *    AC's CanTeachSpell plus IsTrainerSpellAllowedForBot (see BotFactory.cpp:454-456). AC's
 *    IsTrainerValidForPlayer (which gates on the trainer being for the player's faction/class) is subsumed:
 *    the same NPC-flag interaction gate that reaches the trainer is Player::GetTrainerSpellState's caller in
 *    the core (WorldSession::HandleTrainerBuySpellOpcode, Handlers/NPCHandler.cpp:336-420, first calls
 *    GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER)), so a trainer the bot may not talk
 *    to never reaches here either.
 *
 *  - AC Learner branch. AC casts on SPELL_EFFECT_LEARN_SPELL and LearnSpell()s otherwise
 *    (AC TrainerAction.cpp:163-166). This core's own trainer handler branches on
 *    `TrainerSpell::IsCastable()` - `if (trainer_spell->IsCastable()) CastSpell(...) else LearnSpell(...)`
 *    (Handlers/NPCHandler.cpp:413-417) - and so does the sweep (BotFactory.cpp:517-527). Same branch taken.
 *
 *  - Cost. AC `trainerSpell->MoneyCost` -> this core's `trainerSpell->spellCost`; reputation discount via
 *    `bot->GetReputationPriceDiscount(creature)` (Entities/Player/Player.h:1650), same call as AC.
 *
 *  - Trainer type. AC `trainer->GetTrainerType() != Trainer::Type::Tradeskill` -> this core's
 *    `data->trainerType != 2`: AC's enum (src/server/game/Entities/Creature/Trainer.h:31-37) is
 *    Class=0/Mount=1/Tradeskill=2/Pet=3 and TrainerSpellData::trainerType uses the same numbering
 *    (Creature.h:227-229). So AC's Tradeskill test is exactly `trainerType != 2`.
 *
 *  - `chat->parseSpell(param)` -> `botAI->GetChatHelper()->parseSpell(param)` (#331): the ChatHelper
 *    member landed in AI/ChatHelper.h, replacing the file-local ParseSpell() this file carried. Like
 *    AC's (AC Bot/Cmd/ChatHelper.cpp:499-503) it is a non-static one-line wrapper over
 *    PlayerbotChatHandler::extractSpellId (AI/PlayerbotAI.h:64-75) because it needs botAI->GetBot();
 *    behaviour is unchanged.
 *
 *  - `chat->FormatSpell(spellInfo)` / `chat->formatMoney(cost)` -> ChatHelper::FormatSpell() /
 *    ChatHelper::formatMoney() (#331, replacing the file-local FormatSpell()/FormatMoney() copies this
 *    file carried; AC Bot/Cmd/ChatHelper.cpp:423-430 and :190-229).
 *    BEHAVIOUR CHANGE, visible in the "can learn from" list: the file-local FormatMoney() here was NOT
 *    AC's formatter - it always printed all three units and a trailing space ("0g 0s 0c", "1g 0s 0c"),
 *    where AC's formatMoney omits zero units and truncates silver/copper above 50g/10g. The two copies
 *    in RepairAllAction.cpp / TradeStatusAction.cpp still carry the non-AC variant and are untouched by
 *    this slice; aligning those is a separate decision, so this file's output changes and theirs does
 *    not. FormatSpell() was already byte-identical to what ChatHelper::FormatSpell now produces.
 *
 *  - sPlayerbotAIConfig.allowLearnTrainerSpells does not exist in this fork's PlayerbotAIConfig
 *    (Utils/PlayerbotAIConfig.h has no such member). The key is read locally below with AC's documented
 *    default (1 = enabled; AC conf/playerbots.conf.dist:208-211). Reading it here rather than adding a
 *    header member avoids the PlayerbotAIConfig.h -> ~48 minute rebuild.
 *
 *  - `IsRealPlayer(botAI->GetMaster())` is the established fork mapping for AC's
 *    `!IsRealPlayer(botAI->GetMaster())` guard (AC TrainerAction.cpp:38); see TalkToQuestGiverAction.cpp:68-72.
 *
 *  - `botAI->HasCheat(BotCheatMask::gold)` (AC TrainerAction.cpp:152): the whole bot-cheat-mask feature is
 *    dormant on this fork (AI/PlayerbotAI.h:319 has "cheatMask" commented out; there is no HasCheat() or
 *    BotCheatMask), exactly as recorded in RepairAllAction.cpp:10-15 and BuyAction.cpp:25-30. The temporary
 *    "give the bot 10000000 copper" step is omitted, so the bot pays its real money - the same ceiling a
 *    real player is under.
 *
 *  - NeedMoneyFor and the budget values were file-local workarounds here until #332 ported AC's own
 *    Ai/Base/Value/BudgetValues.{h,cpp} to strategy/value/BudgetValues.{h,cpp}. Both the file-local
 *    enum copy and the file-local FreeMoneyFor() are gone; this file now includes "BudgetValues.h" and
 *    uses AC's enum and AC's AI_VALUE2(uint32, "free money for", ...) comparison verbatim. A bot now
 *    trains inside its money minus the reserve TotalMoneyNeededForValue holds for repair/ammo/guild/
 *    travel, which is what the affordability gate has always been meant to test.
 *
 *  - sRandomPlayerbotMgr.IsRandomBot(bot) -> sRandomPlayerbotMgr->IsRandomBot(bot) (pointer singleton).
 *
 *  - BroadcastHelper: not ported on this fork (issue #322). AC's TrainerAction makes no BroadcastHelper
 *    call; nothing omitted here.
 *
 *  - PlayerbotTextMgr: not ported on this fork (issue #322). AC's TrainerAction reports no failure through
 *    it either - its user-visible strings are literals (" - too expensive", " - learned", "--- Can learn from
 *    X ---", "Total cost: ..."), reproduced verbatim below.
 *
 *  - AC's `for (Trainer::Spell const& spell : trainer->GetSpells())` + `trainer->GetSpell(spell.SpellId)`
 *    double lookup has no equivalent here: TrainerSpellMap is an unordered_map keyed by spell id and already
 *    yields the TrainerSpell, so one iteration replaces both. Iteration order is therefore unordered, as in
 *    the sweep (BotFactory.cpp:511).
 */

#include "TrainerAction.h"

// #332: AC's own NeedMoneyFor enum and budget values (was a file-local copy of the enum).
#include "BisListMgr.h"
#include "BotFactory.h"
#include "BudgetValues.h"
#include "ChatHelper.h"
#include "Config.h"
#include "Creature.h"
#include "DBCStores.h"
#include "Event.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotSpec.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "ReputationMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "World.h"
#include "AiObjectContext.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <map>
#include <set>
#include <sstream>

namespace
{
// sPlayerbotAIConfig.allowLearnTrainerSpells, which this fork's PlayerbotAIConfig does not carry
// (Utils/PlayerbotAIConfig.h has no such member). AC reads it in its config loader; the key and AC's
// documented default are in AC conf/playerbots.conf.dist:208-211 ("AllowLearnTrainerSpells", default 1).
// Read once, locally, rather than adding a PlayerbotAIConfig.h member (a ~48 minute rebuild).
bool AllowLearnTrainerSpells()
{
    static bool const allow = sConfigMgr->GetBoolDefault("AiPlayerbot.AllowLearnTrainerSpells", true);
    return allow;
}

// AC PlayerbotAI::IsSelfBot(bot) maps to HasRealPlayerMaster() on this fork (see TalkToQuestGiverAction.cpp:65-72);
// the guard is a free function over a master pointer, matching AC's IsRealPlayer(Player*) helper. AC's
// IsSelfBot(nullptr) is false, so a masterless bot (random bot, account bot) is a real player here too.
bool IsRealPlayer(Player* master)
{
    if (!master)
        return true;

    PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(master);
    return !masterBotAI || masterBotAI->IsRealPlayer();
}

// AC Trainer::Type::Tradeskill == 2 (AC src/server/game/Entities/Creature/Trainer.h:35), and
// TrainerSpellData::trainerType uses the same numbering (Creature.h:227-229).
constexpr uint32 TRAINER_TYPE_TRADESKILL = 2;
}  // namespace

bool TrainerAction::Execute(Event event)
{
    std::string const param = event.getParam();

    Creature* target = GetCreatureTarget();
    if (!target)
        return false;

    TrainerSpellData const* trainer = sObjectMgr->GetNpcTrainerSpells(target->GetEntry());
    if (!trainer)
        return false;

    // NOTE: Original version uses SpellIds here, but occasionally only inserts
    // a single spell ID value from parameters. If someone wants to impl multiple
    // spells as parameters, check SkipSpellsListAction::parseIds as an example.
    uint32 spellId = botAI->GetChatHelper()->parseSpell(param);

    bool learnSpells = param.find("learn") != std::string::npos || sRandomPlayerbotMgr->IsRandomBot(bot) ||
                       (AllowLearnTrainerSpells() &&
                        // TODO: Rewrite to only exclude start primary profession skills and make config dependent.
                        (trainer->trainerType != TRAINER_TYPE_TRADESKILL || !IsRealPlayer(botAI->GetMaster())));

    Iterate(target, learnSpells, spellId);

    return true;
}

bool TrainerAction::isUseful()
{
    Creature* target = GetCreatureTarget();
    if (!target)
        return false;

    if (!target->IsInWorld() || target->IsDuringRemoveFromWorld() || !target->IsAlive())
        return false;

    return target->IsTrainer();
}

bool TrainerAction::isPossible()
{
    Creature* target = GetCreatureTarget();
    if (!target)
        return false;

    TrainerSpellData const* trainer = sObjectMgr->GetNpcTrainerSpells(target->GetEntry());
    if (!trainer)
        return false;

    // AC TrainerAction.cpp:67-80 walks the spell list and returns true on the first spell it is both allowed
    // to learn and can learn. GetTrainerSpellState == TRAINER_SPELL_GREEN is the same predicate on this core
    // (see the file header), and is exactly the gate the sweep uses (BotFactory.cpp:514). This core's enum
    // has no YELLOW (Entities/Player/Player.h:343-348: GRAY=0 GREEN=1 RED=2 GREEN_DISABLED=10).
    for (TrainerSpellMap::const_iterator ref = trainer->spellList.begin(); ref != trainer->spellList.end(); ++ref)
    {
        if (bot->GetTrainerSpellState(&ref->second) == TRAINER_SPELL_GREEN)
            return true;
    }

    return false;
}

Unit* TrainerAction::GetTarget()
{
    // There are just two scenarios: the bot has a master or it doesn't. If the
    // bot has a master, the master should target a unit; otherwise, the bot
    // should target the unit itself.
    if (Player* master = GetMaster())
        return master->GetSelectedUnit();

    return bot->GetSelectedUnit();
}

Creature* TrainerAction::GetCreatureTarget()
{
    Unit* target = GetTarget();
    return target ? target->ToCreature() : nullptr;
}

void TrainerAction::Iterate(Creature* creature, bool learnSpells, uint32 spellId)
{
    TellHeader(creature);

    TrainerSpellData const* trainer = sObjectMgr->GetNpcTrainerSpells(creature->GetEntry());
    if (!trainer)
        return;

    float reputationDiscount = bot->GetReputationPriceDiscount(creature);
    uint32 totalCost = 0;

    for (TrainerSpellMap::const_iterator ref = trainer->spellList.begin(); ref != trainer->spellList.end(); ++ref)
    {
        // simplified version of Trainer::TeachSpell method

        TrainerSpell const* trainerSpell = &ref->second;
        if (spellId && trainerSpell->spell != spellId)
            continue;

        // AC's CanTeachSpell + PlayerbotFactory::IsTrainerSpellAllowedForBot; see the file header.
        if (bot->GetTrainerSpellState(trainerSpell) != TRAINER_SPELL_GREEN)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(trainerSpell->spell);
        if (!spellInfo)
            continue;

        uint32 cost = static_cast<uint32>(floor(trainerSpell->spellCost * reputationDiscount));
        totalCost += cost;

        std::ostringstream out;
        out << ChatHelper::FormatSpell(spellInfo) << ChatHelper::formatMoney(cost);

        if (learnSpells)
            Learn(spellInfo, trainerSpell, cost, out);

        botAI->TellMaster(out);
    }

    TellFooter(totalCost);
}

void TrainerAction::Learn(SpellInfo const* /*spellInfo*/, TrainerSpell const* trainerSpell, uint32 cost,
                          std::ostringstream& out)
{
    // AC TrainerAction.cpp:152 `if (!botAI->HasCheat(BotCheatMask::gold)) {` - not portable here, see the
    // file header (no bot-cheat-mask system on this fork). The bot pays its real money below.
    // AC TrainerAction.cpp:154, verbatim. The budget is real as of #332 - see the file header.
    if (AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::spells)) < cost)
    {
        out << " - too expensive";
        return;
    }

    bot->ModifyMoney(-static_cast<int64>(cost));

    // Same branch the core's own trainer handler takes (Handlers/NPCHandler.cpp:413-417).
    if (trainerSpell->IsCastable())
        bot->CastSpell(bot, trainerSpell->spell, true);
    else
        bot->LearnSpell(trainerSpell->spell, false);

    out << " - learned";
}

void TrainerAction::TellHeader(Creature* creature)
{
    std::ostringstream out;
    out << "--- Can learn from " << creature->GetName() << " ---";
    botAI->TellMaster(out);
}

void TrainerAction::TellFooter(uint32 totalCost)
{
    if (totalCost)
    {
        std::ostringstream out;
        out << "Total cost: " << ChatHelper::formatMoney(totalCost);
        botAI->TellMaster(out);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// AC TrainerAction.cpp MaintenanceAction (:188-287), BisGearAction (:289-538), RemoveGlyphAction (:540-548),
// AutoGearAction (:569-732): the "maintenance", "autogear", "autogear bis" and "remove glyph" chat commands.
//
// Fork / 5.4.8 differences, each also marked at the site:
//  - The config members these actions read on AC (maintenanceCommand, altMaintenance*, autoGearBisCommand) are not in this
//    fork's PlayerbotAIConfig; the keys are read here with AC's names and defaults (AC PlayerbotAIConfig.cpp:650-668 and
//    conf.dist). autoGearCommand / autoGearCommandAltBots / autoGearQualityLimit / autoGearScoreLimit / twoRoundsGearInit
//    exist in PlayerbotAIConfig and are used as on AC. MinEnchantingBotLevel comes from BotFactory.
//  - PlayerbotAI::IsAltBot does not exist here; IsAltBotOf() below is AC's definition (AC PlayerbotAI.cpp:4699: a bot whose
//    master is a client-driven player, that is not a random bot and not its own master).
//  - Maintenance steps with no 5.4.8 counterpart are left out, as the level-up maintenance already does: attunement
//    quests, ammo, pet talents (no pet talent trees), keyring, and the TBC heroic-key reputation grant (InitReputation).
//    "Available spells" are part of InitClassSpells on this fork (the trainer sweep ends it), so one call covers both.
//    InitSpecialSpells (AiPlayerbot.RandomBotSpellIds plus the death knight starter spell) is not carried by BotFactory.
//  - AC's InitTalentsTree(true) (increment) is BotFactory::InitTalentsTree(false) on 5.4.8 (see
//    AutoMaintenanceOnLevelupAction::AutoPickTalents).
//  - Player::SetGlyph takes no "save" flag here, and SendTalentsInfoData takes no argument.
//  - autogear bis: the best-in-slot table (BisListMgr) is read on first use and ships empty, the spec tab is
//    PlayerBotSpec::GetSpectab (5.4.8 has a Guardian druid spec, so AC's bear sentinel tab 10 is not needed), the
//    PvP-spec refusal (AC sRandomPlayerbotMgr.IsSpecPvp) is not carried because this fork has no per-bot PvP spec flag,
//    and ReputationMgr::ReputationRankToStanding (not on this core) is computed from the rank thresholds.
// ---------------------------------------------------------------------------------------------------------------------

namespace
{
bool MaintenanceCommandEnabled() { return sConfigMgr->GetBoolDefault("AiPlayerbot.MaintenanceCommand", true); }
bool AutoGearBisCommandEnabled() { return sConfigMgr->GetBoolDefault("AiPlayerbot.AutoGearBisCommand", false); }
bool AltMaintenance(char const* key) { return sConfigMgr->GetBoolDefault(key, true); }

// AC PlayerbotAI::IsAltBot (Bot/PlayerbotAI.cpp:4699). A selfbot master (AC IsSelfBot) is not modelled here.
bool IsAltBotOf(Player* bot, PlayerbotAI* botAI)
{
    Player* master = botAI->GetMaster();
    bool const hasGameClientMaster = master && !GET_PLAYERBOT_AI(master);
    return hasGameClientMaster && !sRandomPlayerbotMgr->IsRandomBot(bot) && master != bot;
}

// AC ReputationMgr::ReputationRankToStanding: the standing at which a rank starts.
int32 RankStartStanding(ReputationRank rank)
{
    int32 standing = ReputationMgr::Reputation_Bottom;
    for (int32 i = 0; i < int32(rank); ++i)
        standing += ReputationMgr::PointsInRank[i];

    return standing;
}

// AC TrainerAction.cpp:550-567 (GetWornItemLevelAverage).
uint32 GetWornItemLevelAverage(Player* player)
{
    uint32 sumLevel = 0;
    uint32 count = 0;
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
            continue;

        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            sumLevel += item->GetTemplate()->ItemLevel;
            ++count;
        }
    }

    return count ? sumLevel / count : 0;
}
}  // namespace

bool MaintenanceAction::Execute(Event /*event*/)
{
    if (!MaintenanceCommandEnabled())
    {
        botAI->TellError("maintenance command is not allowed, please check the configuration.");
        return false;
    }

    botAI->TellMaster("I'm maintaining");
    BotFactory factory(bot, bot->GetLevel());

    if (!IsAltBotOf(bot, botAI))
    {
        factory.InitBags(false);
        factory.InitFood();
        factory.InitReagents();
        factory.InitConsumables();
        factory.InitPotions();
        factory.InitTalentsTree(false);
        factory.InitPet();
        factory.InitSkills();
        factory.InitClassSpells();
        factory.InitMountSpells();
        factory.InitGlyphs(false);
        if (bot->GetLevel() >= BotFactory::GetMinEnchantingBotLevel())
            factory.ApplyEnchantAndGemsNew();
    }
    else
    {
        if (AltMaintenance("AiPlayerbot.AltMaintenanceBags"))
            factory.InitBags(false);

        if (AltMaintenance("AiPlayerbot.AltMaintenanceFood"))
            factory.InitFood();

        if (AltMaintenance("AiPlayerbot.AltMaintenanceReagents"))
            factory.InitReagents();

        if (AltMaintenance("AiPlayerbot.AltMaintenanceConsumables"))
            factory.InitConsumables();

        if (AltMaintenance("AiPlayerbot.AltMaintenancePotions"))
            factory.InitPotions();

        if (AltMaintenance("AiPlayerbot.AltMaintenanceTalentTree"))
            factory.InitTalentsTree(false);

        if (AltMaintenance("AiPlayerbot.AltMaintenancePet"))
            factory.InitPet();

        if (AltMaintenance("AiPlayerbot.AltMaintenanceSkills"))
            factory.InitSkills();

        // InitClassSpells ends in the trainer sweep (AC InitAvailableSpells), so either switch runs it.
        if (AltMaintenance("AiPlayerbot.AltMaintenanceClassSpells") ||
            AltMaintenance("AiPlayerbot.AltMaintenanceAvailableSpells"))
            factory.InitClassSpells();

        if (AltMaintenance("AiPlayerbot.AltMaintenanceMounts"))
            factory.InitMountSpells();

        if (AltMaintenance("AiPlayerbot.AltMaintenanceGlyphs"))
            factory.InitGlyphs(false);

        if (AltMaintenance("AiPlayerbot.AltMaintenanceGemsEnchants") &&
            bot->GetLevel() >= BotFactory::GetMinEnchantingBotLevel())
            factory.ApplyEnchantAndGemsNew();
    }

    bot->DurabilityRepairAll(false, 1.0f, false);
    bot->SendTalentsInfoData();

    return true;
}

bool BisGearAction::RunAutogearFallback(uint16 effectiveIlvl)
{
    if (!sPlayerbotAIConfig->autoGearCommand)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_autogear_unavailable_error", "autogear command is not allowed, please check the configuration.",
            std::map<std::string, std::string>()));
        return false;
    }

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "bis_no_rows_fallback", "No BiS for your tier/spec/level, check cfg, running autogear instead",
        std::map<std::string, std::string>()));

    // Wipe all equipped slots so autogear gears from scratch at the requested ilvl
    // (avoids old high-tier items surviving the incremental 1.2x threshold).
    BotFactory::DestroyEquippedGear(bot);

    BotFactory::AutoGear(bot, sPlayerbotAIConfig->autoGearQualityLimit, effectiveIlvl, /*incremental*/ false,
                         sPlayerbotAIConfig->twoRoundsGearInit);
    return true;
}

bool BisGearAction::Execute(Event event)
{
    if (!AutoGearBisCommandEnabled())
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_command_unavailable_error", "bis command is not allowed, please check the configuration.",
            std::map<std::string, std::string>()));
        return false;
    }

    if (!sPlayerbotAIConfig->autoGearCommandAltBots &&
        !sPlayerbotAIConfig->IsInRandomAccountList(bot->GetSession()->GetAccountId()))
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_altbot_refused_error", "You cannot use bis on alt bots.", std::map<std::string, std::string>()));
        return false;
    }

    if (sPlayerbotAIConfig->autoGearQualityLimit < 4)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_quality_floor_error", "AutoGearQualityLimit must be 4 for BiS.",
            std::map<std::string, std::string>()));
        return false;
    }

    uint16 ilvl = static_cast<uint16>(sPlayerbotAIConfig->autoGearScoreLimit);

    // Optional explicit ilvl override: `/p autogear bis 55`.
    // Garbage or out-of-range args are hard-rejected: no autogear fallback, no gear change.
    std::string const param = event.getParam();
    if (!param.empty())
    {
        unsigned long parsed = 0;
        size_t pos = 0;
        bool valid = false;
        try
        {
            parsed = std::stoul(param, &pos);
            valid = (parsed > 0 && pos == param.size() && parsed <= 0xFFFFu);
        }
        catch (...)
        {
            valid = false;
        }

        if (!valid)
        {
            std::map<std::string, std::string> phs;
            phs["%param"] = param;
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "bis_invalid_arg_error", "Invalid BiS ilvl argument '%param'. Use a positive integer.", phs));
            return false;
        }
        if (parsed > static_cast<unsigned long>(sPlayerbotAIConfig->autoGearScoreLimit))
        {
            std::map<std::string, std::string> phs;
            phs["%requested"] = std::to_string(parsed);
            phs["%limit"] = std::to_string(sPlayerbotAIConfig->autoGearScoreLimit);
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "bis_arg_above_limit_error", "BiS ilvl %requested exceeds AutoGearScoreLimit %limit, refusing", phs));
            return false;
        }
        ilvl = static_cast<uint16>(parsed);
    }
    uint8 cls = bot->GetClass();
    uint8 tab = static_cast<uint8>(PlayerBotSpec::GetSpectab(bot));
    uint8 faction = bot->GetTeamId() == TEAM_ALLIANCE ? 1 : 2;

    constexpr uint16 BIS_ILVL_FALLBACK_WINDOW = 20;
    uint16 resolvedIlvl = 0;
    std::map<uint8, uint32> bisMap =
        sBisListMgr->GetBisForNearest(ilvl, BIS_ILVL_FALLBACK_WINDOW, cls, tab, faction, &resolvedIlvl);

    // No rows within fallback window -> full autogear fallback at the effective ilvl.
    if (bisMap.empty())
    {
        std::map<std::string, std::string> phs;
        phs["%ilvl"] = std::to_string(ilvl);
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_no_rows_autogear_msg", "No BiS at ilvl %ilvl, using Autogear %ilvl instead", phs));
        return RunAutogearFallback(ilvl);
    }

    if (resolvedIlvl != ilvl)
    {
        std::map<std::string, std::string> phs;
        phs["%requested"] = std::to_string(ilvl);
        phs["%resolved"] = std::to_string(resolvedIlvl);
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "bis_closest_match_msg", "No BiS at ilvl %requested, using closest match at ilvl %resolved", phs));
    }

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault("bis_applying_msg", "Applying BiS gear",
                                                                       std::map<std::string, std::string>()));

    // 1. Wipe everything currently equipped so autogear starts from a clean slate.
    //    Old items linger in inventory otherwise and autogear leaves slots empty on bag conflicts.
    BotFactory::DestroyEquippedGear(bot);

    // Wipe equippable items from bags too. Autogear can shove old equipped items into bags
    // (HandleAutoStoreBagItemOpcode), and a unique-equipped duplicate stuck in a bag blocks
    // CanEquipNewItem on subsequent BiS runs. Spare consumables/reagents.
    auto destroyIfEquippable = [&](uint8 bag, uint8 slot)
    {
        Item* item = bot->GetItemByPos(bag, slot);
        if (!item)
            return;
        ItemTemplate const* tmpl = item->GetTemplate();
        if (!tmpl)
            return;
        if (tmpl->Class == ITEM_CLASS_WEAPON || tmpl->Class == ITEM_CLASS_ARMOR)
            bot->DestroyItem(bag, slot, true);
    };
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        destroyIfEquippable(INVENTORY_SLOT_BAG_0, slot);
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag* container = bot->GetBagByPos(bag))
            for (uint32 slot = 0; slot < container->GetBagSize(); ++slot)
                destroyIfEquippable(bag, slot);
    }

    // 2. Run full autogear on the empty bot so every slot gets a best-available pick.
    //    Uncovered slots will keep the autogear pick; BiS overwrites the rest below.
    if (sPlayerbotAIConfig->autoGearCommand)
        BotFactory::AutoGear(bot, sPlayerbotAIConfig->autoGearQualityLimit, ilvl, /*incremental*/ false,
                             sPlayerbotAIConfig->twoRoundsGearInit, /*applyFinishers*/ false);

    // Autogear may have placed the exact item BiS wants into trinket2/finger2 (or vice versa);
    // unique-equipped enforcement would then make BiS's equip silently drop one copy.
    std::set<uint32> bisEntries;
    for (auto const& kv : bisMap)
        bisEntries.insert(kv.second);
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            if (bisEntries.count(item->GetEntry()))
                bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
    }

    // 3. Apply BiS: only touch slots where the bot can actually equip the BiS item.
    //    If item requires reputation, grant the required rank first. If CanUseItem still
    //    fails (class/race/skill/level), keep autogear's pick for that slot.
    for (auto const& kv : bisMap)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(kv.second);
        if (!proto)
            continue;

        // Grant required reputation rank if the item gates on it.
        if (proto->RequiredReputationFaction && proto->RequiredReputationRank > 0)
        {
            if (FactionEntry const* fac = sFactionStore.LookupEntry(proto->RequiredReputationFaction))
            {
                ReputationRank requiredRank = static_cast<ReputationRank>(proto->RequiredReputationRank);
                if (bot->GetReputationRank(proto->RequiredReputationFaction) < requiredRank)
                {
                    int32 standing = RankStartStanding(requiredRank);
                    bot->GetReputationMgr().SetReputation(fac, standing);
                }
            }
        }

        if (bot->CanUseItem(proto) != EQUIP_ERR_OK)
            continue;

        uint8 slot = kv.first;
        if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);

        uint16 dest = 0;
        InventoryResult eqResult = bot->CanEquipNewItem(slot, dest, kv.second, false);

        // Paired slots (finger 10<->11, trinket 12<->13): destroy paired slot and retry once
        // when unique-equipped or autogear residue blocks the first attempt.
        if (eqResult != EQUIP_ERR_OK)
        {
            uint8 pairedSlot = 0xFF;
            if (slot == EQUIPMENT_SLOT_FINGER1)
                pairedSlot = EQUIPMENT_SLOT_FINGER2;
            else if (slot == EQUIPMENT_SLOT_FINGER2)
                pairedSlot = EQUIPMENT_SLOT_FINGER1;
            else if (slot == EQUIPMENT_SLOT_TRINKET1)
                pairedSlot = EQUIPMENT_SLOT_TRINKET2;
            else if (slot == EQUIPMENT_SLOT_TRINKET2)
                pairedSlot = EQUIPMENT_SLOT_TRINKET1;

            if (pairedSlot != 0xFF)
            {
                if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, pairedSlot))
                    bot->DestroyItem(INVENTORY_SLOT_BAG_0, pairedSlot, true);
                eqResult = bot->CanEquipNewItem(slot, dest, kv.second, false);
            }
        }

        if (eqResult == EQUIP_ERR_OK)
        {
            bot->EquipNewItem(dest, kv.second, true);
            bot->AutoUnequipOffhandIfNeed();
        }
    }

    // AC also runs InitAmmo here; there is no ammo on 5.4.8.
    BotFactory factory(bot, bot->GetLevel(), ITEM_QUALITY_EPIC, 0);
    if (bot->GetLevel() >= BotFactory::GetMinEnchantingBotLevel())
        factory.ApplyEnchantAndGemsNew();

    bot->DurabilityRepairAll(false, 1.0f, false);

    botAI->TellMaster(
        PlayerbotTextMgr::instance().GetBotTextOrDefault("bis_applied_msg", "BiS applied",
                                                         std::map<std::string, std::string>()));
    return true;
}

bool RemoveGlyphAction::Execute(Event /*event*/)
{
    // AC: for slots 0..MAX_GLYPH_SLOT_INDEX, SetGlyph(slot, 0, true). 5.4.8 has MAX_GLYPH_SLOT_INDEX (6) glyph slots with
    // the same slot indices on every spec, so the loop carries over as is; SetGlyph takes no save flag here.
    for (uint8 slotIndex = 0; slotIndex < MAX_GLYPH_SLOT_INDEX; ++slotIndex)
    {
        bot->SetGlyph(slotIndex, 0);
    }
    bot->SendTalentsInfoData();
    return true;
}

bool AutoGearAction::Execute(Event event)
{
    if (!sPlayerbotAIConfig->autoGearCommand)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "autogear_command_disabled_error", "autogear command is not allowed, please check the configuration.",
            std::map<std::string, std::string>()));
        return false;
    }

    if (!sPlayerbotAIConfig->autoGearCommandAltBots &&
        !sPlayerbotAIConfig->IsInRandomAccountList(bot->GetSession()->GetAccountId()))
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "autogear_altbot_refused_error", "You cannot use autogear on alt bots.",
            std::map<std::string, std::string>()));
        return false;
    }

    uint32 const qualityCap = static_cast<uint32>(sPlayerbotAIConfig->autoGearQualityLimit);
    uint32 const ilvlCap = static_cast<uint32>(sPlayerbotAIConfig->autoGearScoreLimit);  // 0 == no limit

    uint32 quality = qualityCap;
    uint32 ilvl = ilvlCap;

    // get optional chat quality
    std::string param = event.getParam();
    std::transform(param.begin(), param.end(), param.begin(), [](unsigned char c) { return std::tolower(c); });
    size_t const first = param.find_first_not_of(" \t");
    if (first == std::string::npos)
        param.clear();
    else
        param = param.substr(first, param.find_last_not_of(" \t") - first + 1);

    // 'reset' erases all equipped gear and regears from scratch instead of upgrading incrementally.
    // It can be combined with a quality/ilvl argument: 'autogear reset green', 'autogear reset 200'.
    bool reset = false;
    if (param == "reset" || param.rfind("reset ", 0) == 0)
    {
        reset = true;
        param = param.size() > 5 ? param.substr(6) : "";
        size_t const start = param.find_first_not_of(" \t");
        param = start == std::string::npos ? "" : param.substr(start);
    }

    if (!param.empty())
    {
        if (param == "match")
        {
            Player* master = GetMaster();
            if (!master)
            {
                botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_no_master_error", "I have no master to match gear with.",
                    std::map<std::string, std::string>()));
                return false;
            }

            uint32 const requestedIlvl = GetWornItemLevelAverage(master);
            if (requestedIlvl == 0)
            {
                botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_master_no_gear_error", "You have no gear equipped for me to match.",
                    std::map<std::string, std::string>()));
                return false;
            }

            ilvl = (ilvlCap == 0) ? requestedIlvl : std::min(requestedIlvl, ilvlCap);
            if (ilvlCap != 0 && requestedIlvl > ilvlCap)
            {
                std::map<std::string, std::string> phs;
                phs["%limit"] = std::to_string(ilvlCap);
                phs["%requested"] = std::to_string(requestedIlvl);
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_ilvl_capped_msg",
                    "Config caps item level at %limit, gearing to that instead of %requested.", phs));
            }
        }
        else if (ChatHelper::parseItemQuality(param) != MAX_ITEM_QUALITY)
        {
            uint32 const requestedQuality = std::max<uint32>(ChatHelper::parseItemQuality(param), ITEM_QUALITY_NORMAL);

            quality = std::min(requestedQuality, qualityCap);
            if (requestedQuality > qualityCap)
            {
                std::map<std::string, std::string> phs;
                phs["%quality"] = ChatHelper::FormatItemQuality(qualityCap);
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_quality_capped_msg", "Config limits me to %quality quality, gearing to that.", phs));
            }
        }
        else
        {
            // Handle negative numbers, garbage, and numbers that are too large to fit in a uint32.
            constexpr size_t MAX_ILVL_DIGITS = 5;
            bool const digitsOnly = !param.empty() && param.size() <= MAX_ILVL_DIGITS &&
                                    param.find_first_not_of("0123456789") == std::string::npos;
            uint32 const requestedIlvl = digitsOnly ? static_cast<uint32>(std::stoul(param)) : 0;

            if (!requestedIlvl)
            {
                std::map<std::string, std::string> phs;
                phs["%param"] = param;
                botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_invalid_arg_error",
                    "Unknown autogear option '%param'. Use bis, reset, match, a quality "
                    "(white, green, blue, purple, orange) or a positive item level number.",
                    phs));
                return false;
            }

            if (requestedIlvl <= ITEM_QUALITY_LEGENDARY)
            {
                std::map<std::string, std::string> phs;
                phs["%param"] = param;
                botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_quality_as_number_error",
                    "'%param' looks like a quality. Numbers are item levels (e.g. 200); use a colour word "
                    "for quality: white, green, blue, purple, orange.",
                    phs));
                return false;
            }

            ilvl = (ilvlCap == 0) ? requestedIlvl : std::min(requestedIlvl, ilvlCap);
            if (ilvlCap != 0 && requestedIlvl > ilvlCap)
            {
                std::map<std::string, std::string> phs;
                phs["%limit"] = std::to_string(ilvlCap);
                phs["%requested"] = std::to_string(requestedIlvl);
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "autogear_ilvl_capped_msg",
                    "Config caps item level at %limit, gearing to that instead of %requested.", phs));
            }
        }
    }

    std::map<std::string, std::string> announcePhs;
    announcePhs["%quality"] = ChatHelper::FormatItemQuality(quality);
    announcePhs["%ilvl"] = std::to_string(ilvl);
    if (reset)
        botAI->TellMaster(
            ilvl != 0
                ? PlayerbotTextMgr::instance().GetBotTextOrDefault(
                      "autogear_reset_start_msg",
                      "I'm erasing my gear and regearing from scratch (%quality, up to item level %ilvl).", announcePhs)
                : PlayerbotTextMgr::instance().GetBotTextOrDefault(
                      "autogear_reset_start_any_ilvl_msg", "I'm erasing my gear and regearing from scratch (%quality).",
                      announcePhs));
    else
        botAI->TellMaster(
            ilvl != 0 ? PlayerbotTextMgr::instance().GetBotTextOrDefault(
                            "autogear_start_msg", "I'm auto gearing (%quality, up to item level %ilvl).", announcePhs)
                      : PlayerbotTextMgr::instance().GetBotTextOrDefault("autogear_start_any_ilvl_msg",
                                                                         "I'm auto gearing (%quality).", announcePhs));

    if (reset)
    {
        // Wipe everything equipped first so old high-tier items can't survive; then gear every
        // slot from scratch instead of only upgrading past the incremental threshold.
        BotFactory::DestroyEquippedGear(bot);
        BotFactory::AutoGear(bot, quality, ilvl, /*incremental*/ false, sPlayerbotAIConfig->twoRoundsGearInit);
    }
    else
        BotFactory::AutoGear(bot, quality, ilvl, /*incremental*/ true);

    return true;
}
