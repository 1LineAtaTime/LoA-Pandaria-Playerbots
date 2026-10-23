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
#include "BudgetValues.h"
#include "ChatHelper.h"
#include "Config.h"
#include "Creature.h"
#include "Event.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "World.h"
#include "AiObjectContext.h"

#include <cmath>
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