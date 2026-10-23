/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AiFactory.h"

#include <mutex>
#include <set>

#include "BotAIObjectContext.h"

#include "Engine.h"
#include "Group.h"

#include "Item.h"

#include "PlayerbotAI.h"
#include "Config.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

// After Playerbots.h, which brings ObjectGuid into scope, and with DatabaseEnv.h first:
// Battleground.h reaches BattlegroundQueue.h -> RatedPvp.h, which uses both
// CharacterDatabaseTransaction and ObjectGuid without declaring either.
#include "DatabaseEnv.h"

#include "Battleground.h"
#include "BgTacticsSA.h"  // #717: SotaBeyondAc::Enabled() gates the SotA strategy
#include "PlayerbotSpec.h"

#include "SharedDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

#include "DKAiObjectContext.h"
#include "DruidAiObjectContext.h"
#include "HunterAiObjectContext.h"
#include "PaladinAiObjectContext.h"
#include "PriestAiObjectContext.h"
#include "RogueAiObjectContext.h"
#include "ShamanAiObjectContext.h"
#include "MageAiObjectContext.h"
#include "MonkAiObjectContext.h"
#include "WarriorAIObjectContext.h"
#include "WarlockAiObjectContext.h"
#include "BgTacticsTB.h"

namespace
{
// #408 (AC parity, AC AiFactory.cpp:307-320). Frostfire Bolt (44614) is NOT a trainable 5.4.8 mage
// spell - only leftover DBC rows remain (src/server/game/Entities/Player/Player.cpp:23630,
// src/server/game/Spells/SpellMgr.cpp:7797) - so the frostfire branch below is inert on this core.
// It is still ported 1:1 so the branch matches AC structurally.
// AC's Ice Shards constant is 15047 (the WotLK rank); 5.4.8's Ice Shards talent is 93527
// (src/server/game/Spells/SpellMgr.cpp:4423).
constexpr uint32 SPELL_FROSTFIRE_BOLT = 44614;
constexpr uint32 SPELL_ICE_SHARDS = 93527;

// #817 (AC Bot/Factory/AiFactory.cpp:409-412 + PlayerbotAIConfig.cpp:276-283,1121-1124 IsRestrictedHealerDPSMap).
// AC reads AiPlayerbot.HealerDPSMapRestriction (code default false, conf.dist ships 1) and the
// AiPlayerbot.RestrictedHealerDPSMaps list into PlayerbotAIConfig; read here as static locals instead of adding to
// PlayerbotAIConfig.h. Code defaults: restriction ON (AC's shipped conf value) and AC's list followed by the
// beyond-AC Cata/MoP 5-man map ids (same ids as config/playerbots.conf.dist).
bool IsRestrictedHealerDPSMap(uint32 mapId)
{
    static bool const restrictOn = sConfigMgr->GetBoolDefault("AiPlayerbot.HealerDPSMapRestriction", true);
    static std::set<uint32> const maps = []
    {
        std::string const list = sConfigMgr->GetStringDefault(
            "AiPlayerbot.RestrictedHealerDPSMaps",
            "33,34,36,43,47,48,70,90,109,129,209,229,230,329,349,389,429,1001,1004,1007,269,540,542,543,545,546,547,552,"
            "553,554,555,556,557,558,560,585,574,575,576,578,595,599,600,601,602,604,608,619,632,650,658,668,409,469,"
            "509,531,532,534,544,548,550,564,565,580,249,533,603,615,616,624,631,649,724,"
            "568,643,644,645,657,670,725,755,859,938,939,940,959,960,961,962,994,1011");
        std::set<uint32> out;
        size_t pos = 0;
        while (pos < list.size())
        {
            size_t end = list.find(',', pos);
            if (end == std::string::npos)
                end = list.size();
            std::string const token = list.substr(pos, end - pos);
            char* tail = nullptr;
            unsigned long const id = strtoul(token.c_str(), &tail, 10);
            if (tail != token.c_str())
                out.insert(uint32(id));
            pos = end + 1;
        }
        return out;
    }();
    return restrictOn && maps.count(mapId) != 0;
}
}  // namespace

// #817 (AC Bot/PlayerbotAI.cpp:7046-7055 PlayerbotAI::EvaluateHealerDpsStrategy, called at :954-955 after a far
// teleport when restrictHealerDPS). AC's factory guard only runs when the engine is built; this is what flips
// "healer dps" when a grouped bot changes map (into / out of a dungeon). Not a PlayerbotAI member here so that
// PlayerbotAI.h stays untouched: PlayerbotAI.cpp forward-declares it at file scope and calls it after
// ApplyInstanceStrategies in the far-teleport branch.
void AiFactoryEvaluateHealerDps(Player* bot, PlayerbotAI* botAI)
{
    if (!PlayerBotSpec::IsHeal(bot, true))
        return;

    if (IsRestrictedHealerDPSMap(bot->GetMapId()))
        botAI->ChangeStrategy("-healer dps", BOT_STATE_COMBAT);
    else
        botAI->ChangeStrategy("+healer dps", BOT_STATE_COMBAT);
}

AiObjectContext* AiFactory::createAiObjectContext(Player* player, PlayerbotAI* botAI)
{
    switch (player->GetClass())
    {
        case CLASS_PRIEST:
            return new PriestAiObjectContext(botAI);
        case CLASS_MAGE:
            return new MageAiObjectContext(botAI);
        case CLASS_WARLOCK:
            return new WarlockAiObjectContext(botAI);
        case CLASS_WARRIOR:
            return new WarriorAiObjectContext(botAI);
        case CLASS_SHAMAN:
            return new ShamanAiObjectContext(botAI);
        case CLASS_PALADIN:
            return new PaladinAiObjectContext(botAI);
        case CLASS_DRUID:
            return new DruidAiObjectContext(botAI);
        case CLASS_HUNTER:
            return new HunterAiObjectContext(botAI);
        case CLASS_ROGUE:
            return new RogueAiObjectContext(botAI);
        case CLASS_DEATH_KNIGHT:
            return new DKAiObjectContext(botAI);
        case CLASS_MONK:
            return new MonkAiObjectContext(botAI);
    }

    return new BotAiObjectContext(botAI);
}

Specializations AiFactory::GetPlayerSpecTab(Player* bot)
{
    return bot->GetSpecialization();
}

BotRoles AiFactory::GetPlayerRoles(Player* player)
{
    BotRoles role = BOT_ROLE_NONE;
    const Specializations spec = player->GetSpecialization();

    switch (player->GetClass())
    {
        case CLASS_PRIEST:
            if (spec == Specializations::SPEC_PRIEST_SHADOW)
                role = BOT_ROLE_DPS;
            else
                role = BOT_ROLE_HEALER;
            break;
        case CLASS_SHAMAN:
            if (spec == Specializations::SPEC_DRUID_RESTORATION)
                role = BOT_ROLE_HEALER;
            else
                role = BOT_ROLE_DPS;
            break;
        case CLASS_WARRIOR:
            if (spec == Specializations::SPEC_WARRIOR_PROTECTION)
                role = BOT_ROLE_TANK;
            else
                role = BOT_ROLE_DPS;
            break;
        case CLASS_PALADIN:
            if (spec == Specializations::SPEC_PALADIN_HOLY)
                role = BOT_ROLE_HEALER;
            else if (spec == Specializations::SPEC_PALADIN_PROTECTION)
                role = BOT_ROLE_TANK;
            else if (spec == 2)
                role = BOT_ROLE_DPS;
            break;
        case CLASS_DRUID:
            if (spec == Specializations::SPEC_DRUID_FERAL || spec == Specializations::SPEC_DRUID_BALANCE)
                role = BOT_ROLE_DPS;
            else if (spec == Specializations::SPEC_DRUID_GUARDIAN)
                role = BOT_ROLE_TANK;
            else if (spec == Specializations::SPEC_DRUID_RESTORATION)
                role = BOT_ROLE_HEALER;
            break;
        case CLASS_MONK:
            if (spec == Specializations::SPEC_MONK_BREWMASTER)
                role = BOT_ROLE_TANK;
            else if (spec == Specializations::SPEC_MONK_MISTWEAVER)
                role = BOT_ROLE_HEALER;
            else if (spec == Specializations::SPEC_MONK_WINDWALKER)
                role = BOT_ROLE_DPS;
            break;
        default:
            role = BOT_ROLE_DPS;
            break;
    }

    return role;
}

std::string AiFactory::GetPlayerSpecName(Player* player)
{
    std::string specName;
    const Specializations spec = GetPlayerSpecTab(player);
    switch (player->GetClass())
    {
        case CLASS_PRIEST:
            if (spec == Specializations::SPEC_PRIEST_SHADOW)
                specName = "shadow";
            else if (spec == Specializations::SPEC_PRIEST_HOLY)
                specName = "holy";
            else
                specName = "disc";
            break;
        case CLASS_SHAMAN:
            if (spec == Specializations::SPEC_SHAMAN_RESTORATION)
                specName = "resto";
            else if (spec == Specializations::SPEC_SHAMAN_ENHANCEMENT)
                specName = "enhance";
            else
                specName = "elem";
            break;
        case CLASS_WARRIOR:
            if (spec == Specializations::SPEC_WARRIOR_PROTECTION)
                specName = "prot";
            else if (spec == Specializations::SPEC_WARRIOR_FURY)
                specName = "fury";
            else
                specName = "arms";
            break;
        case CLASS_PALADIN:
            if (spec == Specializations::SPEC_PALADIN_HOLY)
                specName = "holy";
            else if (spec == Specializations::SPEC_PALADIN_PROTECTION)
                specName = "prot";
            else
                specName = "retrib";
            break;
        case CLASS_DRUID:
            if (spec == Specializations::SPEC_DRUID_FERAL)
                specName = "feraldps";
            else if (spec == Specializations::SPEC_DRUID_GUARDIAN)
                specName = "guardiantank";
            else if (spec == Specializations::SPEC_DRUID_RESTORATION)
                specName = "resto";
            else
                specName = "balance";
            break;
        case CLASS_ROGUE:
            if (spec == Specializations::SPEC_ROGUE_ASSASSINATION)
                specName = "assas";
            else if (spec == Specializations::SPEC_ROGUE_COMBAT)
                specName = "combat";
            else
                specName = "subtle";
            break;
        case CLASS_HUNTER:
            if (spec == Specializations::SPEC_HUNTER_BEAST_MASTERY)
                specName = "beast";
            else if (spec == Specializations::SPEC_HUNTER_MARKSMANSHIP)
                specName = "marks";
            else
                specName = "surv";
            break;
        case CLASS_DEATH_KNIGHT:
            if (spec == Specializations::SPEC_DEATH_KNIGHT_FROST)
                specName = "frostdps";
            else if (spec == Specializations::SPEC_DEATH_KNIGHT_UNHOLY)
                specName = "unholydps";
            else
                specName = "blooddps";
            break;
        case CLASS_MAGE:
            if (spec == Specializations::SPEC_MAGE_ARCANE)
                specName = "arcane";
            else if (spec == Specializations::SPEC_MAGE_FIRE)
                specName = "fire";
            else
                specName = "frost";
            break;
        case CLASS_WARLOCK:
            if (spec == Specializations::SPEC_WARLOCK_DESTRUCTION)
                specName = "destru";
            else if (spec == Specializations::SPEC_WARLOCK_DEMONOLOGY)
                specName = "demono";
            else
                specName = "affli";
            break;
        case CLASS_MONK:
            if (spec == Specializations::SPEC_MONK_BREWMASTER)
                specName = "brewmaster";
            else if (spec == Specializations::SPEC_MONK_MISTWEAVER)
                specName = "mistweaver";
            else
                specName = "windwalker";
            break;
        default:
            break;
    }

    return specName;
}

void AiFactory::AddDefaultCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* engine)
{
    if (!player->InBattleground())
    {
        // #163 S8 (AC parity, AiFactory.cpp:288): "potions" re-added alongside the rest.
        engine->addStrategiesNoInit("racials", "chat", "default", "cast time", "potions", "duel", "boost", nullptr);
    }
    // AC AiFactory.cpp:290-291 gates on HasGameClientMaster() (AC PlayerbotAI.cpp:4661: IsRealPlayer(master) ||
    // IsSelfBot(master)); this fork's HasRealPlayerMaster() is the same test (master has no bot AI, or is a selfbot).
    // #628: the strategy itself must be registered in StrategyContext ("avoid aoe" -> AvoidAoeStrategy), else
    // addStrategy() silently drops the name.
    if (sPlayerbotAIConfig->autoAvoidAoe && facade->HasRealPlayerMaster())
    {
        engine->addStrategy("avoid aoe", false);
    }
    engine->addStrategy("formation", false);

    const Specializations& spec = player->GetSpecialization();
    switch (player->GetClass())
    {
        case CLASS_PRIEST:
        {
            if (spec == Specializations::SPEC_PRIEST_SHADOW)
            {
                engine->addStrategiesNoInit("dps", "shadow debuff", "shadow aoe", nullptr);
            }
            else if (spec == Specializations::SPEC_PRIEST_DISCIPLINE)
            {
                engine->addStrategiesNoInit("heal", nullptr);
            }
            else
            {
                engine->addStrategiesNoInit("holy heal", nullptr);
            }

            engine->addStrategiesNoInit("dps assist", "cure", nullptr);
            break;
        }
        case CLASS_MAGE:
        {
            // #408 (AC parity, AC AiFactory.cpp:307-320): AC's "cc" + "aoe" and the Frostfire branch.
            // MoP's per-spec aoe strategies ("arcane/fire/frost aoe") are kept and paired with their
            // spec strategy, since AC has no per-spec aoe equivalent.
            if (spec == Specializations::SPEC_MAGE_ARCANE)
                engine->addStrategiesNoInit("arcane", "arcane aoe", nullptr);
            else if (spec == Specializations::SPEC_MAGE_FIRE)
            {
                if (player->HasSpell(SPELL_FROSTFIRE_BOLT) && player->HasAura(SPELL_ICE_SHARDS))
                    engine->addStrategiesNoInit("frostfire", "fire aoe", nullptr);
                else
                    engine->addStrategiesNoInit("fire", "fire aoe", nullptr);
            }
            else
                engine->addStrategiesNoInit("frost", "frost aoe", nullptr);

            engine->addStrategiesNoInit("dps", "dps assist", "cure", "cc", "aoe", nullptr);
            break;
        }
        case CLASS_WARRIOR:
        {
            if (spec == Specializations::SPEC_WARRIOR_PROTECTION)
                // #408 (AC AiFactory.cpp:324): pull + pull back for warrior protection tanks.
                engine->addStrategiesNoInit("tank", "tank assist", "pull", "pull back", "aoe", nullptr);
            else if (player->GetLevel() < 10 || spec == Specializations::SPEC_WARRIOR_ARMS)
                engine->addStrategiesNoInit("arms", "aoe", "dps assist", /*"behind",*/ nullptr);
            else
                engine->addStrategiesNoInit("fury", "aoe", "dps assist", /*"behind",*/ nullptr);
            break;
        }
        case CLASS_SHAMAN:
        {
            // #408 (AC parity, AC AiFactory.cpp:330-339): AC's shaman strategy set.
            // MoP's unified "totems" strategy replaces AC's per-spec totem strategies.
            // #382 (AC Bot/Factory/AiFactory.cpp:332-336): AC's spec keys ele/resto/enh, registered as aliases
            // of this module's caster/heal/melee (the engine stores them under the class's getName()).
            if (spec == Specializations::SPEC_SHAMAN_ELEMENTAL)
                engine->addStrategiesNoInit("ele", "totems", nullptr);
            else if (spec == Specializations::SPEC_SHAMAN_RESTORATION)
                engine->addStrategiesNoInit("resto", "totems", nullptr);
            else
                engine->addStrategiesNoInit("enh", "totems", nullptr);

            engine->addStrategiesNoInit("dps assist", "cure", "aoe", nullptr);
            break;
        }
        case CLASS_PALADIN:
        {
            if (spec == Specializations::SPEC_PALADIN_PROTECTION)
                // #408 (AC AiFactory.cpp:342): pull + pull back for paladin protection tanks.
        engine->addStrategiesNoInit("tank", "tank assist", "pull", "pull back", "bthreat", "barmor", "cure", nullptr);
            else if (spec == Specializations::SPEC_PALADIN_HOLY)
                engine->addStrategiesNoInit("heal", "dps assist", "cure", "bcast", nullptr);
            else
                engine->addStrategiesNoInit("dps", "dps assist", "cure", "baoe", nullptr);
            break;
        }
        case CLASS_DRUID:
        {
            if (spec == Specializations::SPEC_DRUID_BALANCE)
            {
                // #163 S8 gap fix (b) (AC parity, AiFactory.cpp:351): "cc" was never added to any
                // druid engine, so Cyclone/Hibernate/Entangling Roots's CC actions had no strategy to
                // run from regardless of their trigger registration.
                // #408 (AC parity, Bot/Factory/AiFactory.cpp:351): AC's "aoe" (DruidAoeStrategy) is
                // the spec-gated AoE rotation. On MoP it sits alongside the fork's own "caster aoe"
                // instead of replacing it.
                // #384 (AC :351): AC's "balance" key (alias of this module's "caster").
                // #593 (AC Bot/Factory/AiFactory.cpp:351): AC's exact list. The fork-only "caster aoe" and
                // "caster debuff" are dropped: DruidAoeStrategy ("aoe") already carries AC's balance AoE edges
                // (hurricane, starfall, DoTs on attackers) and "caster" (= balance) carries Faerie Fire at 29.5.
                engine->addStrategiesNoInit("balance", "cure", "aoe", "cc", "dps assist", nullptr);
            }
            else if (spec == Specializations::SPEC_DRUID_RESTORATION)
                // #163 S8 gap fix (b) (AC parity, AiFactory.cpp:352): Resto also gets "cc" (research
                // s8-pet-control-and-class-spells.md B.6).
                // #408 (AC parity, Bot/Factory/AiFactory.cpp:354): resto bots get "tranquility"
                // (DruidTranquilityStrategy) last, after the core rotation.
                // #384 (AC :354): AC's "resto" key (alias of this module's "heal").
                // #593 (AC :354): AC's list has no "cc" for resto (the S8 "cc" add above is dropped).
                engine->addStrategiesNoInit("resto", "cure", "dps assist", "tranquility", nullptr);
            else if (spec == Specializations::SPEC_DRUID_FERAL)
            {
                if (player->GetLevel() >= 20 && !player->HasAura(16931) /*thick hide*/)
                {
                    // #163 S8 gap fix (b) (AC parity, AiFactory.cpp:358): cat-form feral gets "cc" too.
                    // #408 (AC parity, Bot/Factory/AiFactory.cpp:358): cat-form feral also gets "aoe"
                    // and "feral charge" (FeralChargeDruidStrategy).
                    engine->addStrategiesNoInit("cat", "aoe", "cc", "dps assist", "feral charge", nullptr);
                }
                else
                {
                    // #408 (AC AiFactory.cpp:360): pull + pull back for druid bear tanks.
            // #408 (AC parity, Bot/Factory/AiFactory.cpp:360): bear tanks also get "feral charge".
            engine->addStrategiesNoInit("bear", "tank assist", "pull", "pull back", "feral charge", nullptr);
                }
            }
            else if (spec == Specializations::SPEC_DRUID_GUARDIAN)
            {
                // #408 (AC AiFactory.cpp:360): pull + pull back for druid bear tanks.
            // #408 (AC parity, Bot/Factory/AiFactory.cpp:360): bear tanks also get "feral charge".
            engine->addStrategiesNoInit("bear", "tank assist", "pull", "pull back", "feral charge", nullptr);
            }
            break;
        }
        case CLASS_HUNTER:
        {
            // #408 (AC parity, AiFactory.cpp:363-372): hunter bots used to always run the single
            // "dps" rotation; AC picks a spec strategy from the talent tab. The spec strategy is
            // added with default priority and the pre-existing "dps" rotation is kept below.
            if (spec == Specializations::SPEC_HUNTER_BEAST_MASTERY)
                engine->addStrategy("bm", false);
            else if (spec == Specializations::SPEC_HUNTER_MARKSMANSHIP)
                engine->addStrategy("mm", false);
            else
                engine->addStrategy("surv", false);

            // #380 (AC Bot/Factory/AiFactory.cpp:371): AC also adds "cc" (HunterCcStrategy) to every hunter.
            // #593 (AC Bot/Factory/AiFactory.cpp:371): AC's exact list. The fork-only "dps" (DpsHunterStrategy, every
            // spec's shots as defaults with Explosive Shot at ACTION_HIGH + 1) and "dps debuff" are dropped: they put
            // other specs' shots (Aimed/Chimera/Explosive Shot) on every hunter and outranked the bm/mm/surv rotation,
            // which carries AC's defaults 1:1.
            engine->addStrategiesNoInit("cc", "dps assist", "aoe", "bdps", nullptr);
            break;
        }
        case CLASS_ROGUE:
        {
            // #386 (AC Bot/Factory/AiFactory.cpp:373-379): AC's keys and split - the combat tab runs
            // "combat", Assassination and Subtlety run "assassin" (aliases of this module's "dps"/"melee").
            if (spec == Specializations::SPEC_ROGUE_COMBAT)
                engine->addStrategiesNoInit("combat", nullptr);
            else
                engine->addStrategiesNoInit("assassin", nullptr);

            engine->addStrategiesNoInit("dps assist", "aoe", /*"behind",*/ nullptr);
            break;
        }
        case CLASS_WARLOCK:
        {
            // #408 (AC parity, AC Bot/Factory/AiFactory.cpp:381-389): each spec also gets its
            // default curse sub-strategy; Demonology additionally gets the opt-in meta melee
            // strategy. #381: AC's spec keys "demo"/"destro" (aliases of this module's "demono"/"destru").
            if (spec == Specializations::SPEC_WARLOCK_DEMONOLOGY)
            {
                engine->addStrategy("demo", false);
                engine->addStrategiesNoInit("curse of agony", "meta melee", nullptr);
            }
            else if (spec == Specializations::SPEC_WARLOCK_DESTRUCTION)
            {
                engine->addStrategy("destro", false);
                engine->addStrategiesNoInit("curse of elements", nullptr);
            }
            else
            {
                engine->addStrategy("affli", false);
                engine->addStrategiesNoInit("curse of agony", nullptr);
            }

            // #381 (AC Bot/Factory/AiFactory.cpp:389): AC adds "cc" (WarlockCcStrategy) to every warlock.
            engine->addStrategiesNoInit("dps assist", "dps", "aoe", "cc", nullptr);
            break;
        }
        case CLASS_DEATH_KNIGHT:
        {
            if (spec == Specializations::SPEC_DEATH_KNIGHT_BLOOD)
                // #408 (AC AiFactory.cpp:393): pull + pull back for blood DK tanks.
        engine->addStrategiesNoInit("blood", "tank assist", "pull", "pull back", nullptr);
            else if (spec == Specializations::SPEC_DEATH_KNIGHT_FROST)
                engine->addStrategiesNoInit("frost", "frost aoe", "dps assist", nullptr);
            else
                engine->addStrategiesNoInit("unholy", "unholy aoe", "dps assist", nullptr);
            break;
        }
        case CLASS_MONK:
        {
            // #526: brewmaster gets "pull" + "pull back" like every AC tank (AC Bot/Factory/AiFactory.cpp:324,342,
            // 360,393; "pull" = MonkPullStrategy, Provoke). "dps assist" was added twice for windwalkers.
            if (spec == Specializations::SPEC_MONK_BREWMASTER)
                engine->addStrategiesNoInit("tank", "tank assist", "pull", "pull back", "aoe", nullptr);
            else if (spec == Specializations::SPEC_MONK_MISTWEAVER)
                engine->addStrategiesNoInit("heal", nullptr);
            else
                engine->addStrategiesNoInit("melee", "melee aoe", nullptr);

            engine->addStrategiesNoInit("dps assist", "cure", nullptr);
        }
        break;
    }

    if (PlayerBotSpec::IsTank(player, true))
        engine->addStrategy("tank face", false);
    // #593 (AC Bot/Factory/AiFactory.cpp:404-405): AC adds "behind" when IsMelee(bySpec) && IsDps(bySpec). This
    // module's PlayerBotSpec::IsDps ignores bySpec (it asks the engines' strategy types, which are not built yet
    // here) and PlayerBotSpec::IsRanged treats every paladin spec as melee, so AC's by-spec result is spelled out:
    // melee and neither tank nor healer = rogue, ret, enh, arms/fury, frost/unholy, feral cat, windwalker.
    if (PlayerBotSpec::IsMelee(player, true) && !PlayerBotSpec::IsTank(player, true) && !PlayerBotSpec::IsHeal(player, true))
        engine->addStrategy("behind", false);
    if (PlayerBotSpec::IsHeal(player, true))
    {
        if (sPlayerbotAIConfig->autoSaveMana)
            engine->addStrategy("save mana", false);
        // #163 S8 / #817 (AC parity, AiFactory.cpp:409-412): "healer dps" only on maps that are not in
        // AiPlayerbot.RestrictedHealerDPSMaps while AiPlayerbot.HealerDPSMapRestriction is on (see helper above).
        if (!IsRestrictedHealerDPSMap(player->GetMapId()))
            engine->addStrategy("healer dps", false);
    }

    if (facade->IsRealPlayer() || sRandomPlayerbotMgr->IsRandomBot(player))
    {
        if (!player->GetGroup())
        {
            // change for heal spec
            engine->addStrategy("boost", false);
            engine->addStrategy("dps assist", false);
            engine->removeStrategy("threat", false);
            // engine-
            switch (player->GetClass())
            {
            case CLASS_PRIEST:
            {
                if (spec != Specializations::SPEC_PRIEST_SHADOW)
                {
                    engine->addStrategiesNoInit("holy dps", "shadow debuff", "shadow aoe", nullptr);
                }
                break;
            }
            case CLASS_DRUID:
            {
                if (spec == Specializations::SPEC_DRUID_RESTORATION)
                {
                    // #593 (AC Bot/Factory/AiFactory.cpp:432-437): a solo resto druid only gains AC's "aoe"; its
                    // damage comes from "healer dps". "caster" here was a sibling of "resto" and replaced the heal
                    // rotation outright.
                    engine->addStrategiesNoInit("aoe", nullptr);
                }
                break;
            }
            case CLASS_SHAMAN:
            {
                if (spec == Specializations::SPEC_SHAMAN_RESTORATION)
                {
                    // #593 (AC Bot/Factory/AiFactory.cpp:440-444): AC's exact list (no "bmana").
                    engine->addStrategiesNoInit("caster", "caster aoe", nullptr);
                }
                break;
            }
            case CLASS_PALADIN:
            {
                if (spec == Specializations::SPEC_PALADIN_HOLY)
                {
                    engine->addStrategiesNoInit("dps", "dps assist", "baoe", nullptr);
                }
                break;
            }
            default:
                break;
            }
        }
    }
    if (sRandomPlayerbotMgr->IsRandomBot(player))
        engine->ChangeStrategy(sPlayerbotAIConfig->randomBotCombatStrategies);
    else
        engine->ChangeStrategy(sPlayerbotAIConfig->combatStrategies);

    // The per-map battleground strategy belongs on this engine too - see the note on
    // AddBattlegroundStrategies. Without it a bot in combat has no battleground behaviour at all.
    if (player->InBattleground() && player->GetBattleground())
        AddBattlegroundStrategies(player, engine, false);

    // Phase 1.1 diagnostic (issue #59): prove which class/spec strategies were actually attached to
    // the combat engine. If a level-90 frost mage lists only "default"/"melee"-style strategies here,
    // the spell pipeline never had a chance to run. Logged once per bot.
    // Engines are (re)built on map-update threads (ResetStrategies); with MapUpdate.Threads > 1 the shared set needs the
    // lock (AC pattern: a std::mutex around shared state, e.g. AC WarlockActions.cpp:251).
    static std::set<ObjectGuid> loggedCombatStrategyBots;
    static std::mutex loggedCombatStrategyBotsLock;
    bool logCombatStrategies = false;
    {
        std::lock_guard<std::mutex> lock(loggedCombatStrategyBotsLock);
        logCombatStrategies =
            loggedCombatStrategyBots.size() < 2000 && loggedCombatStrategyBots.insert(player->GetGUID()).second;
    }
    if (logCombatStrategies)
        TC_LOG_DEBUG("playerbots", "[STRAT] %s class=%u level=%u spec=%u combat=%s", player->GetName().c_str(),
                     uint32(player->GetClass()), uint32(player->GetLevel()),
                     uint32(player->GetSpecialization()), engine->ListStrategies().c_str());

    // temporary
    //engine->addStrategy("ranged");
    //engine->addStrategy("melee");
    //engine->addStrategy("say hello");
}

Engine* AiFactory::createCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* aiObjectContext)
{
    Engine* engine = new Engine(facade, aiObjectContext);
    AddDefaultCombatStrategies(player, facade, engine);
    engine->Init();
    return engine;
}

void AiFactory::AddDefaultNonCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* nonCombatEngine)
{
    const Specializations tab = GetPlayerSpecTab(player);

    switch (player->GetClass())
    {
        case CLASS_PRIEST:
            nonCombatEngine->addStrategiesNoInit("dps assist", "cure", nullptr);
            break;
        case CLASS_PALADIN:
            if (tab == Specializations::SPEC_PALADIN_PROTECTION)
            {
                // #408 (AC AiFactory.cpp:519): pull must also be live out of combat so a pull can be *requested*.
                nonCombatEngine->addStrategiesNoInit("bthreat", "tank assist", "pull", "barmor", nullptr);
                if (player->GetLevel() >= 20)
                {
                    nonCombatEngine->addStrategy("bstats", false);
                }
                else
                {
                    // #383 (AC Bot/Factory/AiFactory.cpp:519-524): AC's "bmight" key (alias of "bdps").
                    // AC's level-20+ "bsanc" and holy "bwisdom" stay "bstats"/"bmana": Sanctuary and Wisdom
                    // are not in 5.4.8 (documented on #383/#424).
                    nonCombatEngine->addStrategy("bmight", false);
                }
            }
            else if (tab == Specializations::SPEC_PALADIN_HOLY)
                nonCombatEngine->addStrategiesNoInit("dps assist", "bmana", "bcast", nullptr);
            else
                nonCombatEngine->addStrategiesNoInit("dps assist", "bmight", "baoe", nullptr);  // #383 (AC :528)

            nonCombatEngine->addStrategiesNoInit("cure", nullptr);
            break;
        case CLASS_HUNTER:
            nonCombatEngine->addStrategiesNoInit("bdps", "dps assist", "pet", nullptr);
            break;
        case CLASS_SHAMAN:
            // #408 (AC parity, AC AiFactory.cpp:535-537): AC's shaman non-combat strategy set.
            nonCombatEngine->addStrategiesNoInit("dps assist", "cure", nullptr);
            break;
        case CLASS_MAGE:
            if (tab == Specializations::SPEC_MAGE_ARCANE || tab == Specializations::SPEC_MAGE_FIRE)
                nonCombatEngine->addStrategy("bdps", false);
            else
                nonCombatEngine->addStrategy("bmana", false);

            nonCombatEngine->addStrategiesNoInit("dps assist", "cure", nullptr);
            break;
        case CLASS_DRUID:
            if (tab == Specializations::SPEC_DRUID_FERAL)
            {
                if (player->GetLevel() >= 20 && !player->HasAura(16931) /*thick hide*/)
                {
                    nonCombatEngine->addStrategy("dps assist", false);
                }
                else
                {
                    // #408 (AC AiFactory.cpp:552): pull has to exist on the non-combat engine too.
                    nonCombatEngine->addStrategiesNoInit("tank assist", "pull", nullptr);
                }
            }
            else if (tab == Specializations::SPEC_DRUID_GUARDIAN)
            {
                // #408 (AC AiFactory.cpp:575)
                nonCombatEngine->addStrategiesNoInit("tank assist", "pull", nullptr);
            }
            else
                nonCombatEngine->addStrategiesNoInit("dps assist", "cure", nullptr);
            break;
        case CLASS_WARRIOR:
            if (tab == Specializations::SPEC_WARRIOR_PROTECTION)
                // #408 (AC AiFactory.cpp:552)
                nonCombatEngine->addStrategiesNoInit("tank assist", "pull", nullptr);
            else
                nonCombatEngine->addStrategy("dps assist", false);
            break;
        case CLASS_WARLOCK:
            if (tab == Specializations::SPEC_WARLOCK_AFFLICTION)
            {
                nonCombatEngine->addStrategiesNoInit("felhunter", nullptr);  // #381 (AC :565)
            }
            else if (tab == Specializations::SPEC_WARLOCK_DEMONOLOGY)
            {
                nonCombatEngine->addStrategiesNoInit("felguard", nullptr);  // #381 (AC :567)
            }
            else if (tab == Specializations::SPEC_WARLOCK_DESTRUCTION)
            {
                nonCombatEngine->addStrategiesNoInit("imp", nullptr);  // #381 (AC :569)
            }
            // #163 S8 (AC parity, AiFactory.cpp:571): soulstone the bot itself by default. 5.4.8:
            // Soulstone (20707) is a direct cast since patch 5.0.4, no item/creation step - see the
            // WarlockActions.h comment on UseSoulstoneAction.
            nonCombatEngine->addStrategiesNoInit("dps assist", "ss self", nullptr);
            break;
        case CLASS_DEATH_KNIGHT:
            if (tab == Specializations::SPEC_DEATH_KNIGHT_BLOOD)
                // #523 (AC Bot/Factory/AiFactory.cpp:574-575): blood tanks also get "pull" out of combat.
                nonCombatEngine->addStrategiesNoInit("tank assist", "pull", nullptr);
            else
                nonCombatEngine->addStrategy("dps assist", false);
            break;
        case CLASS_MONK:
            // #388 (MoP-only class, AC pattern of the curing classes above, AC AiFactory.cpp:514-555): Detox out of combat too
            if (tab == Specializations::SPEC_MONK_BREWMASTER)
                nonCombatEngine->addStrategiesNoInit("tank assist", "cure", nullptr);
            else
                nonCombatEngine->addStrategiesNoInit("dps assist", "cure", nullptr);
            break;
        default:
            nonCombatEngine->addStrategy("dps assist", false);
            break;
    }
    if (!player->InBattleground())
    {
        // "pvp" (AttackEnemyPlayersStrategy) engages hostile players. It was dropped from the port,
        // so bots ignored the opposing faction entirely (issue #60).
        // #628 (AC AiFactory.cpp:586): "force rebuff" (AI/ForceRebuff.h) in AC's position.
        nonCombatEngine->addStrategiesNoInit("nc", "food", "chat", "follow", "default", "force rebuff", "quest", "loot",
            "gather", "duel", "pvp", "buff", "mount", "emote", nullptr);
    }

    // AC AiFactory.cpp:618-685.
    if (sRandomPlayerbotMgr->IsRandomBot(player) && !player->InBattleground())
    {
        Player* master = facade->GetMaster();
        // #410 (AC AiFactory.cpp:597-599): let 25% of free bots start duels.
        if (!urand(0, 3))
            nonCombatEngine->addStrategy("start duel", false);

        // #142 (AC AiFactory.cpp:601-602): random bots self-queue for dungeons real players are queued for.
        if (sPlayerbotAIConfig->randomBotJoinLfg)
            nonCombatEngine->addStrategy("lfg", false);

        if (!player->GetGroup() || player->GetGroup()->GetLeaderGUID() == player->GetGUID())
        {
            nonCombatEngine->addStrategy("grind", false);

            if (sPlayerbotAIConfig->enableNewRpgStrategy)
                nonCombatEngine->addStrategy("new rpg", false);
            // AC AiFactory.cpp:618-622 (issue #610): AutoDoQuests selects the old "rpg" strategy; AC's "travel" line is commented out in AC itself.
            else if (sPlayerbotAIConfig->autoDoQuests)
            {
                // nonCombatEngine->addStrategy("travel");
                nonCombatEngine->addStrategy("rpg", false);
            }
            else
                nonCombatEngine->addStrategy("move random", false);

            // Battleground queueing (issue #33)
            if (sPlayerbotAIConfig->randomBotJoinBG)
                nonCombatEngine->addStrategy("bg", false);

            nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig->randomBotNonCombatStrategies);
        }
        else if (facade && master)
        {
            if (GET_PLAYERBOT_AI(master) || sRandomPlayerbotMgr->IsRandomBot(player))
                // #269/#272: AC AiFactory.cpp:648-656, commented out in AC itself (grouped bot, bot/random master):
                // if (sPlayerbotAIConfig.autoDoQuests)
                // {
                //     // nonCombatEngine->addStrategy("travel");
                //     nonCombatEngine->addStrategy("rpg");
                // }
                // else
                // {
                //     nonCombatEngine->addStrategy("move random");
                // }
                nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig->randomBotNonCombatStrategies);
            else
                nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig->nonCombatStrategies);
        }
    }
    else
        nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig->nonCombatStrategies);

    // Objective play, for a bot that is actually inside a match (unchanged).
    if (player->InBattleground() && player->GetBattleground())
        AddBattlegroundStrategies(player, nonCombatEngine, true);

    // #901 (beyond AC): a bot in the Tol Barad war (map 732 battlefield raid) plays the objectives; BattlefieldBots::Update
    // adds/removes the same set live via TolBaradBots::Sync, this covers every engine rebuild while the war runs.
    if (TolBaradBots::InWar(player))
    {
        nonCombatEngine->addStrategy("tolbarad", false);
        nonCombatEngine->addStrategy("pvp", false);
        nonCombatEngine->removeStrategy("new rpg", false);
        nonCombatEngine->removeStrategy("gather", false);
        nonCombatEngine->removeStrategy("rpg", false);
        nonCombatEngine->removeStrategy("grind", false);
        nonCombatEngine->removeStrategy("travel", false);
    }

    // Phase 1.2 diagnostic (issue #60): the "pvp" strategy lives on this engine, so prove it is
    // attached. Logged once per bot, after every strategy has been added.
    static std::set<ObjectGuid> loggedNonCombatStrategyBots;
    static std::mutex loggedNonCombatStrategyBotsLock;  // see loggedCombatStrategyBotsLock
    bool logNonCombatStrategies = false;
    {
        std::lock_guard<std::mutex> lock(loggedNonCombatStrategyBotsLock);
        logNonCombatStrategies =
            loggedNonCombatStrategyBots.size() < 2000 && loggedNonCombatStrategyBots.insert(player->GetGUID()).second;
    }
    if (logNonCombatStrategies)
        TC_LOG_DEBUG("playerbots", "[STRATNC] %s class=%u level=%u noncombat=%s", player->GetName().c_str(),
                     uint32(player->GetClass()), uint32(player->GetLevel()), nonCombatEngine->ListStrategies().c_str());
}

/// Add the strategies a bot needs while it is inside a battleground: the per-map one always, and
/// the shared objective layer only on the non-combat engine (`nonCombat`), which is how upstream
/// splits them.
///
/// Both engines need the per-map strategy. PlayerbotAI runs the COMBAT engine while the bot is in
/// combat and the non-combat engine only otherwise, so a battleground strategy that lives on the
/// non-combat engine alone simply stops existing the moment a bot is attacked. That is not a corner
/// case in a battleground - a flag carrier standing in the enemy base is in combat permanently, and
/// with no battleground actions on the engine that is actually running it never moves again. Every
/// carrier observed in testing froze on the enemy flag stand until it was killed. WarsongStrategy's
/// "player has flag" -> "bg move to objective" at ACTION_EMERGENCY is precisely the trigger that has
/// to outrank fighting, so it has to be on the combat engine to ever fire.
///
/// Branching on the MAP rather than the type id is deliberate. A battleground entered through the
/// Random Battleground queue keeps BATTLEGROUND_RB as its type id (CreateNewBattleground does
/// SetTypeID(originalBgTypeId)), so a type check would leave bots with no tactics at all in exactly
/// the queue most players use.
void AiFactory::AddBattlegroundStrategies(Player* player, Engine* engine, bool nonCombat)
{
    Battleground* bg = player->GetBattleground();
    if (!bg)
        return;

    // Which map has a BGTactics objective layer at all. AC gates its shared "battleground" strategy
    // the same way (AiFactory.cpp:691-692: bgType <= BATTLEGROUND_EY || bgType == BATTLEGROUND_IC).
    // AV and IoC tactics are ported (#393) and BfG/TOK/SSM/DG extend them (#576-#579);
    // SotA has none (AC has no SotA tactics either), so restrict the gate to the maps that actually have a case
    // in BGTactics::Execute's switch. Without this, a bot on an unsupported map got "battleground"
    // anyway, BGTactics::Execute fell into its default: case, called ResetStrategies(), and AiFactory
    // immediately re-added "battleground" - a reset loop on every "often"/"bg active" tick
    // (bg-parity-stage-design.md fix 2).
    char const* mapStrategy = nullptr;
    switch (bg->GetMapId())
    {
        case 30:  // Alterac Valley - AC AiFactory.cpp:475-476/691-693 gates the same way
            mapStrategy = "alterac";
            break;
        case 489:  // Warsong Gulch
        case 726:  // Twin Peaks - same game: BGTactics plays the WSG case on TP data (vPaths_TP, vFlagsTP, TP_*; #674)
            mapStrategy = "warsong";
            break;
        case 529:  // Arathi Basin
            mapStrategy = "arathi";
            break;
        case 566:  // Eye of the Storm
            mapStrategy = "eye";
            break;
        case 628:  // Isle of Conquest - AC AiFactory.cpp:481-482/705-706 ("isle"); tactics ported in #393
            mapStrategy = "isle";
            break;
        // #576-#579: Cataclysm/MoP-only maps, AC's per-map pattern extended (no AC spec).
        case 761:  // Battle for Gilneas
            mapStrategy = "gilneas";
            break;
        case 998:  // Temple of Kotmogu
            mapStrategy = "kotmogu";
            break;
        case 727:  // Silvershard Mines
            mapStrategy = "silvershard";
            break;
        case 1105:  // Deepwind Gorge (5.3 version)
            mapStrategy = "deepwind";
            break;
        // #717 (beyond AC): Strand of the Ancients. AC keeps SotA off "battleground" (AiFactory.cpp:689-690: no SotA
        // tactics); BgTacticsSA.cpp now has a case in every BGTactics switch. With the SotaBeyondAc switch off the
        // map stays off here as well - exactly AC's gate, for an AC-parity 1:1.
        case 607:
            if (SotaBeyondAc::Enabled())
                mapStrategy = "strand";
            break;
        default:
            // Everything else (SotA, and maps with no BGTactics case) has no ported tactics.
            // Leave it off "battleground" entirely instead of looping on ResetStrategies().
            break;
    }

    // This fork's arena early return is gone: AC never puts "battleground" on an arena (see the
    // "do not add for not supported bg or arena" comment at AC AiFactory.cpp:689-691) and instead
    // gives it the separate "arena" strategy + the no-potions combat set (AC :484-490, :708-712).
    // #346 ports that. mapStrategy is null for every arena map, so nothing above puts "battleground"
    // on the engine; this branch is what replaces it.
    bool const inArena = bg->IsArena();

    if (nonCombat)
    {
        if (mapStrategy)
            engine->addStrategy("battleground", false);
        // Engage hostile players in the match too; the per-map tactics layer only handles
        // objectives, so without this a bot walks past an enemy standing next to it (issue #60).
        engine->addStrategy("pvp", false);

        // AC AiFactory.cpp:678-679: the non-combat-in-BG generic set. "force rebuff" is ported (#628,
        // AI/ForceRebuff.h); "attack tagged" is not implemented on this fork (addStrategiesNoInit
        // ignores unknown names, same as upstream); "mount"/"food"/"buff"/"nc"/"dps assist" are, and
        // unlock the behaviour that was missing entirely (issue noted in bg-parity-stage-design.md fix 1).
        engine->addStrategiesNoInit("nc", "chat", "default", "force rebuff", "buff", "food", "mount",
                                     "dps assist", "attack tagged", "emote", nullptr);
        engine->removeStrategy("custom::say", false);
        engine->removeStrategy("travel", false);
        engine->removeStrategy("rpg", false);
        engine->removeStrategy("grind", false);

        // AC AiFactory.cpp:708-712.
        if (inArena)
        {
            engine->addStrategy("arena", false);
            // No mount inside an arena: a mounted bot in a 5v5 is a free kill, and AC drops the
            // strategy here for exactly that reason.
            engine->removeStrategy("mount", false);
        }
    }
    else
    {
        // AC AiFactory.cpp:484-490: an arena gets "arena" and the same set MINUS "potions" (a
        // healthstone/health potion out of combat in an arena is free damage on the enemy team).
        if (inArena)
        {
            engine->addStrategy("arena", false);
            engine->addStrategiesNoInit("boost", "racials", "chat", "default", "aoe", "cast time",
                                         "dps assist", nullptr);
        }
        else
        {
            engine->addStrategiesNoInit("boost", "racials", "chat", "default", "aoe", "potions", "cast time",
                                         "dps assist", nullptr);
        }
        engine->removeStrategy("custom::say", false);
        engine->removeStrategy("flee", false);
        engine->removeStrategy("threat", false);
    }

    TC_LOG_DEBUG("playerbots", "[BGS] %s: in bg map %u -> adding battleground strategies (%s engine)",
                 player->GetName().c_str(), bg->GetMapId(), nonCombat ? "non-combat" : "combat");

    if (mapStrategy)
        engine->addStrategy(mapStrategy, false);
}

Engine* AiFactory::createNonCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* aiObjectContext)
{
    Engine* nonCombatEngine = new Engine(facade, aiObjectContext);

    AddDefaultNonCombatStrategies(player, facade, nonCombatEngine);
    nonCombatEngine->Init();
    return nonCombatEngine;
}

void AiFactory::AddDefaultDeadStrategies(Player* player, PlayerbotAI* const facade, Engine* deadEngine)
{
    (void)facade;  // unused and remove warning
    deadEngine->addStrategiesNoInit("dead", "stay", "chat", "default", "follow", nullptr);

    if (sRandomPlayerbotMgr->IsRandomBot(player) && !player->GetGroup())
    {
        deadEngine->removeStrategy("follow", false);
    }
}

Engine* AiFactory::createDeadEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext)
{
    Engine* deadEngine = new Engine(facade, AiObjectContext);
    AddDefaultDeadStrategies(player, facade, deadEngine);
    deadEngine->Init();
    return deadEngine;
}
