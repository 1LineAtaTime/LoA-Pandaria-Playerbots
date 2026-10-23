/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * HARNESS-SPEC-botscenario-v3-module: implementation. All .botscenario / .botprobe state lives in
 * the side table below (g_bots), keyed by the bot's guid counter, instead of in NewRpgInfo - see
 * BotScenario.h and BotScenarioInternal.h for why. Behaviour (log formats, timings, deterministic
 * picks) is unchanged from HARNESS-SPEC-botscenario-v2; only where the code lives has moved.
 */

#include "BotScenario.h"
#include "BotScenarioInternal.h"

// Playerbots.h/Player.h first: Battleground.h -> RatedPvp.h needs DBC/Item/DB types they pull in.
#include "Playerbots.h"
#include "Player.h"
#include "Battleground.h"
#include "Battlefield.h"      // #900/#898: `.botscenario - bf` (cs_bf.cpp is Console::No)
#include "BattlefieldMgr.h"
#include "BattlegroundAV.h"
#include "BattlegroundMgr.h"
#include "BattlegroundQueue.h"
#include "BattleGroundTactics.h"
#include "BgTacticsSA.h"  // #736(c): `sotadrive` - SotaBeyondAc::SetDriveChanceOverride
#include "BattleGroundJoinAction.h"  // #570: PlayerbotArena::Census for bg queue=natural
#include "BattlegroundWS.h"
#include "BotFactory.h"
#include "CharacterCache.h"
#include "Config.h"
#include "Chat.h"
#include "Corpse.h"
#include "Creature.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "GossipDef.h"
#include "Group.h"
#include "GroupMgr.h"
#include "InstanceSaveMgr.h"  // #811: teleport fail reason (group bind's InstanceSave)
#include "Map.h"
#include "MapManager.h"       // #811: teleport fail reason (IsValidMapCoord, FindMap)
#include "MotionMaster.h"
#include "NewRpgInfo.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "PassiveAI.h"
#include "Pet.h"
#include "Player.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "QuestDef.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "Transport.h"
#include "TransportMgr.h"
#include "TravelMgr.h"
#include "TravelNode.h"
#include "Unit.h"
#include "Vehicle.h"
#include "World.h"
#include "WorldPosition.h"
#include "WorldSession.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

// Wave-29 arena legs: the test-harness arena pin defined in src/server/game/Battlegrounds/BattlegroundMgr.cpp (an
// "All Arenas" draw returns this arena while it is non-zero). Set only by `.botscenario - arenapin` below. No header.
extern uint32 g_harnessArenaPinTypeId;

namespace BotScenario
{
// #743 strat=none: stratA/stratH sentinel - the stage does not pin bgStrategies, so the BG keeps its own per-match roll.
static constexpr uint8 BG_STRAT_UNPINNED = 0xFF;

// #210 S10 P5: forward-declared here, at BotScenario namespace scope (NOT inside the anonymous
// namespace below - an anonymous-namespace-local declaration would shadow this name instead of
// referring to BotScenarioQueue.cpp's definition, and the HandlerRegistrar call below would then
// link against an undefined internal-linkage symbol - this mirrors the AC fork's own copy of this
// hook, where that mistake was caught before it shipped). No shared header touched - see
// BotScenarioQueue.cpp's header for why this .obj needs a forced-link call at all.
void RegisterQueueScenarioHandlers();
bool HandleQueueScenarioCommand(ChatHandler* handler, std::string const& name, std::string const& scenario,
                                 std::istringstream& in);
// #210 S10 P5: the "queuegate on|off" flag (BotScenarioQueue.cpp) - consulted from BgJoinBlocked
// below, same scope-placement reason as the two declarations above.
bool QueueGateOn();

namespace
{
    // --- side table ------------------------------------------------------------------------------
    std::mutex g_mutex;
    std::unordered_map<uint32, ScenarioState> g_bots; // key: bot guid counter
    std::atomic<int> g_active{0};                      // AnyActive() fast path

    // --- test-only config (HARNESS-SPEC-botscenario-v3 section 3), read once, lazily -------------
    uint32 TraceIntervalMs()
    {
        static uint32 const v = sConfigMgr->GetIntDefault("AiPlayerbot.Scenario.TraceIntervalMs", 500);
        return v;
    }

    uint32 ProbeIntervalMsDefault()
    {
        static uint32 const v = sConfigMgr->GetIntDefault("AiPlayerbot.Scenario.ProbeIntervalMs", 1000);
        return v;
    }

    uint32 GotoTimeoutMs()
    {
        static uint32 const v = sConfigMgr->GetIntDefault("AiPlayerbot.Scenario.GotoTimeoutMs", 180000);
        return v;
    }

    float ArriveDistance()
    {
        static float const v = sConfigMgr->GetFloatDefault("AiPlayerbot.Scenario.ArriveDistance", 5.0f);
        return v;
    }

    // --- scenario handler registry (Meyer's singletons: registration order across translation
    // units does not matter) --------------------------------------------------------------------
    std::unordered_map<std::string, ScenarioHandler>& NameRegistry()
    {
        static std::unordered_map<std::string, ScenarioHandler> registry;
        return registry;
    }

    std::unordered_map<uint8, ScenarioHandler>& KindRegistry()
    {
        static std::unordered_map<uint8, ScenarioHandler> registry;
        return registry;
    }

    // --- shared helper: re-apply the RPG status a wander/attack scenario needs (ported from
    // NewRpgInfo::ArmWanderScenario) -------------------------------------------------------------
    void ArmWanderScenario(Player* /*bot*/, PlayerbotAI* botAI, ScenarioState& state)
    {
        NewRpgInfo& info = botAI->GetRpgInfo();

        // Mode "attack" deliberately falls through to the else branch below: G5 wants status IDLE
        // with the lock held, which is exactly what modes other than "free" already get here, so
        // the attack scenario is armed/re-armed through this same function without a special case.
        if (state.kind == ScenarioKind::WanderNpc)
            info.ChangeToWanderNpc();
        else if (state.kind == ScenarioKind::WanderRandom)
            info.ChangeToWanderRandom();
        else
            info.ChangeToIdle();

        // Start every run with fresh MoveFarTo stuck tracking. It is only cleared when the destination
        // changes, and pick=nearest re-selects the same cell, so a stale stuck timer from an earlier
        // stage could teleport the bot seconds into a new one (issue-61-wander-pocket-stall.md).
        info.moveFarPos = WorldPosition();
        info.nearestMoveFarDis = FLT_MAX;
        info.stuckTs = 0;
        info.stuckAttempts = 0;

        state.lock = state.kind != ScenarioKind::WanderFree;
        state.startT = getMSTime();
        state.lastTraceT = 0;
        state.lastQg = ObjectGuid::Empty;
    }

    // ground: the gate in BotCanUseFlyingMount only stops the bot choosing a flying mount; a bot that is
    // already on one when the stage starts would keep flying, so dismount it here.
    void DismountForGround(Player* bot, bool groundOnly)
    {
        if (groundOnly && bot->IsMounted())
        {
            bot->Dismount();
            bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
        }
    }

    // --- goto --------------------------------------------------------------------------------------
    bool ParseGoto(ChatHandler* handler, Player* bot, PlayerbotAI* botAI, std::istringstream& in, ScenarioState& state)
    {
        uint32 mapId = 0;
        float x1, y1, z1, x2, y2, z2;
        if (!(in >> mapId >> x1 >> y1 >> z1 >> x2 >> y2 >> z2))
        {
            handler->SendSysMessage("Usage: .botscenario <botname> goto <map> <x1> <y1> <z1> <x2> <y2> <z2> [ground]");
            return false;
        }

        bool groundOnly = false;
        uint32 destMap = mapId;   // #218: optional map2=<id> - destination on another map (cross-continent boat/zeppelin leg)
        std::string gtok;
        while (in >> gtok)
        {
            if (gtok == "ground")
                groundOnly = true;
            else if (gtok.rfind("map2=", 0) == 0)
            {
                try { destMap = uint32(std::stoul(gtok.substr(5))); } catch (std::exception const&) { }
            }
        }

        WorldPosition const dest(destMap, x2, y2, z2);
        NewRpgInfo& info = botAI->GetRpgInfo();
        bot->TeleportTo(mapId, x1, y1, z1, bot->GetOrientation());
        DismountForGround(bot, groundOnly);
        // Start from a clean combat state (as the attack scenario does): a same-map teleport does not
        // Reset() the AI, so a fight carried over from the previous stage would continue here.
        bot->AttackStop();
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
        botAI->ChangeEngine(BOT_STATE_NON_COMBAT);
        info.ChangeToGoGrind(dest);
        info.SetMoveFarTo(dest);

        state.kind = ScenarioKind::Goto;
        state.startT = getMSTime();
        state.lastTraceT = 0;
        state.destMapId = destMap;
        state.destX = x2;
        state.destY = y2;
        state.destZ = z2;
        state.lock = true;
        state.wanderMs = 0;
        state.allowMask = 0;
        state.pickNearest = false;
        state.pickCount = 0;
        state.attackGuid = 0;
        state.attackEntry = 0;
        state.groundOnly = groundOnly;

        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s start goto map=%u from=(%.1f, %.1f, %.1f) to=(%.1f, %.1f, %.1f)",
            bot->GetName().c_str(), mapId, x1, y1, z1, x2, y2, z2);
        return true;
    }

    bool GotoUpdate(Player* bot, PlayerbotAI* /*botAI*/, ScenarioState& state, uint32 /*now*/, uint32 t)
    {
        bool const arrived = bot->GetMapId() == state.destMapId &&
            bot->GetExactDist(state.destX, state.destY, state.destZ) < ArriveDistance();
        bool const timedOut = t >= GotoTimeoutMs();
        if (!arrived && !timedOut)
            return false;

        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s %s t=%u map=%u x=%.1f y=%.1f z=%.1f",
            bot->GetName().c_str(), arrived ? "arrived" : "timeout", t, bot->GetMapId(),
            bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        return true;
    }

    void RearmGoto(Player* /*bot*/, PlayerbotAI* botAI, ScenarioState& state)
    {
        WorldPosition const dest(state.destMapId, state.destX, state.destY, state.destZ);
        NewRpgInfo& info = botAI->GetRpgInfo();
        info.ChangeToGoGrind(dest);
        info.SetMoveFarTo(dest);
        state.startT = getMSTime();
        state.lastTraceT = 0;
        state.lock = true;
    }

    // --- wander (npc / random / free) -----------------------------------------------------------
    bool ParseWander(ChatHandler* handler, Player* bot, PlayerbotAI* botAI, std::istringstream& in, ScenarioState& state)
    {
        uint32 mapId = 0, secs = 0;
        float x, y, z;
        if (!(in >> mapId >> x >> y >> z >> secs) || !secs)
        {
            handler->SendSysMessage(
                "Usage: .botscenario <botname> wander <map> <x> <y> <z> <seconds> [npc|random|free] [allow=<n,n,...>] [pick=nearest] [ground]");
            return false;
        }

        std::string mode = "npc";
        uint32 allowMask = 0;
        bool pickNearest = false;
        bool noFight = false;
        bool groundOnly = false;
        bool grindSet = false;
        float grindX = 0.0f, grindY = 0.0f, grindZ = 0.0f;
        std::string tok;
        while (in >> tok)
        {
            if (tok == "npc" || tok == "random" || tok == "free")
                mode = tok;
            else if (tok.rfind("allow=", 0) == 0)
            {
                std::istringstream allowIn(tok.substr(6));
                std::string part;
                while (std::getline(allowIn, part, ','))
                {
                    if (part.empty())
                        continue;
                    try
                    {
                        allowMask |= (1u << uint32(std::stoul(part)));
                    }
                    catch (std::exception const&)
                    {
                        // Malformed allow= token on this debug-only command - ignore it rather than
                        // take down the worldserver on a typo.
                    }
                }
            }
            else if (tok == "pick=nearest")
                pickNearest = true;
            else if (tok == "nofight")
                noFight = true;
            else if (tok == "ground")
                groundOnly = true;
            else if (tok.rfind("grind=", 0) == 0)
            {
                std::istringstream gin(tok.substr(6));
                std::string gx, gy, gz;
                if (std::getline(gin, gx, ',') && std::getline(gin, gy, ',') && std::getline(gin, gz, ','))
                {
                    try
                    {
                        grindX = std::stof(gx);
                        grindY = std::stof(gy);
                        grindZ = std::stof(gz);
                        grindSet = true;
                    }
                    catch (std::exception const&)
                    {
                        // Malformed grind= token on this debug-only command - ignore it.
                    }
                }
            }
        }

        bot->TeleportTo(mapId, x, y, z, bot->GetOrientation());
        DismountForGround(bot, groundOnly);
        // Start from a clean combat state (as the attack scenario does): a same-map teleport does not
        // Reset() the AI, so a fight carried over from the previous stage would continue here.
        bot->AttackStop();
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
        botAI->ChangeEngine(BOT_STATE_NON_COMBAT);

        state.kind = mode == "random" ? ScenarioKind::WanderRandom
                   : mode == "free"   ? ScenarioKind::WanderFree
                                      : ScenarioKind::WanderNpc;
        state.destMapId = mapId;
        state.destX = x;
        state.destY = y;
        state.destZ = z;
        state.wanderMs = secs * IN_MILLISECONDS;
        state.allowMask = allowMask;
        state.pickNearest = pickNearest;
        state.grindSet = grindSet;
        state.noFight = noFight;
        state.groundOnly = groundOnly;
        state.grindX = grindX;
        state.grindY = grindY;
        state.grindZ = grindZ;
        state.pickCount = 0;
        state.attackGuid = 0;
        state.attackEntry = 0;
        ArmWanderScenario(bot, botAI, state);

        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s start wander mode=%s map=%u at=(%.1f, %.1f, %.1f) secs=%u allow=0x%X pick=%s",
            bot->GetName().c_str(), mode.c_str(), mapId, x, y, z, secs, allowMask, pickNearest ? "nearest" : "off");
        return true;
    }

    bool WanderUpdate(Player* bot, PlayerbotAI* /*botAI*/, ScenarioState& state, uint32 /*now*/, uint32 t)
    {
        if (t < state.wanderMs)
            return false;

        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s done t=%u map=%u x=%.1f y=%.1f z=%.1f",
            bot->GetName().c_str(), t, bot->GetMapId(),
            bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        return true;
    }

    void RearmWander(Player* bot, PlayerbotAI* botAI, ScenarioState& state)
    {
        ArmWanderScenario(bot, botAI, state);
    }

    // cds= in [PROBE]: cooldowns still running now. Both cores keep expired entries in their maps until
    // something prunes them, so the raw map size is not comparable across cores.
    uint32 ActiveCooldownCount(Player* bot)
    {
        TimeValue const now = TimeValue::Now();
        uint32 n = 0;
        for (auto const& [spellId, entry] : bot->GetSpellHistory()->GetCooldowns())
            if (entry.CooldownEnd > now || entry.CategoryEnd > now)
                ++n;
        return n;
    }

    // --- attack (issue #129 / G5) ----------------------------------------------------------------
    // The target must be this close to the scenario point, and must be found within the grace period
    // after the scenario starts, or the run ends as "notarget".
    constexpr float ATTACK_ACQUIRE_RANGE = 20.0f;
    constexpr uint32 ATTACK_ACQUIRE_GRACE_MS = 5000;
    // "spawn": where the fresh target goes, this many yards east of the scenario point.
    constexpr float ATTACK_SPAWN_OFFSET = 8.0f;

    // #521 w14: the two cores' rotation dummies (MoP 30527 / AC 31146, both level 83 rank 3, ScriptName
    // npc_training_dummy) run DIFFERENT scripts. AC (src/server/scripts/World/npcs_special.cpp:309-350) is a
    // NullCreatureAI: it takes no damage, never evades, and ends combat with ONE attacker 5 s after that attacker's last
    // hit (auras stay). The fork's (TC, scripts/World/npcs_special.cpp:1637-1757) evades 5 s after the last hit:
    // RemoveAllAuras + CombatStop for everyone (t521 w14 balance druid: opener of moonkin/NS/FF/NG/starfall, no damage in
    // 5 s, evade, 5 s in combat of 40 while w12/w13 got 34-35 s). A harness-spawned training dummy gets this copy of the AC
    // AI on MoP, so the rotation tests fight the same target behaviour on both cores.
    // Deviation: MoP UnitAI::DamageTaken carries no DamageEffectType (AC skips DOT ticks), so DoT ticks refresh the timer here.
    struct ScenarioTrainingDummyAI : public NullCreatureAI
    {
        explicit ScenarioTrainingDummyAI(Creature* creature) : NullCreatureAI(creature) { }

        void JustEnteredCombat(Unit* who) override
        {
            if (who)
                _combatTimer[who->GetGUID()] = 5000;
        }

        void DamageTaken(Unit* attacker, uint32& damage) override
        {
            damage = 0;
            if (!attacker)
                return;
            _combatTimer[attacker->GetGUID()] = 5000;
            // Pet attacks engage the owner via propagation without firing JustEnteredCombat here (AC comment).
            if (Unit* owner = attacker->GetCharmerOrOwner())
                if (me->GetCombatManager().IsInCombatWith(owner))
                    _combatTimer[owner->GetGUID()] = 5000;
        }

        void UpdateAI(uint32 diff) override
        {
            for (auto itr = _combatTimer.begin(); itr != _combatTimer.end();)
            {
                if (itr->second <= int32(diff))
                {
                    // The attacker has not dealt any damage to the dummy for over 5 seconds. End combat (AC).
                    auto const& pveRefs = me->GetCombatManager().GetPvECombatRefs();
                    auto it = pveRefs.find(itr->first);
                    if (it != pveRefs.end())
                        it->second->EndCombat();
                    itr = _combatTimer.erase(itr);
                }
                else
                {
                    itr->second -= int32(diff);
                    ++itr;
                }
            }
        }

    private:
        std::unordered_map<ObjectGuid, int32> _combatTimer;
    };

    // "spawn": a fresh creature of the scenario entry on the ground ATTACK_SPAWN_OFFSET yd east of the
    // scenario point, with no summoner, so it is a plain world creature. Same entry, place and state on
    // both cores, so an attack run never depends on respawn timers or where a wandering spawn happens
    // to be. It despawns by itself when the scenario's time is up.
    Creature* SpawnAttackTarget(Player* bot, ScenarioState const& state)
    {
        float const x = state.destX + state.attackSpawnOffset;
        float const y = state.destY;
        float z = state.destZ + 5.0f;
        bot->UpdateGroundPositionZ(x, y, z);
        Position const pos(x, y, z, float(M_PI));
        Creature* spawned = bot->GetMap()->SummonCreature(state.attackEntry, pos, nullptr, Milliseconds(state.wanderMs));
        // #521 w14: AC's training dummy behaviour on the MoP copy (see ScenarioTrainingDummyAI above).
        if (spawned && spawned->GetScriptName() == "npc_training_dummy")
        {
            spawned->AIM_Initialize(new ScenarioTrainingDummyAI(spawned));
            TC_LOG_DEBUG("playerbots", "[SCENARIO] dummy entry=%u guid=%u ai=ac-training-dummy", state.attackEntry,
                spawned->GetGUID().GetCounter());
        }
        // #636 "unflag": event bosses (Violet Hold, ToC) are NON_ATTACKABLE / immune until their script
        // releases them; clear those so the same summoned boss is attackable on both cores.
        if (spawned && state.attackUnflag)
        {
            uint32 constexpr mask = UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC;
            uint32 const before = spawned->GetUInt32Value(UNIT_FIELD_FLAGS);
            spawned->RemoveUnitFlag(UnitFlags(mask));
            TC_LOG_INFO("playerbots", "[SCENARIO] unflag entry=%u flags before=%#x after=%#x", state.attackEntry, before, spawned->GetUInt32Value(UNIT_FIELD_FLAGS));
            // #569 w31b: the MoP Violet Hold boss scripts evade in JustEngagedWith while their cell door is still
            // closed (boss_erekem.cpp:134-138, boss_ichoron.cpp:107-111, boss_zuramat.cpp:107-111, also moragg,
            // lavanthor, xevozz); AC's have no such check. So "unflag" also opens the door nearest (< 25 yd) to each
            // DB spawn of the entry - its cell - the state the event leaves it in when it releases the boss. Same
            // step on both cores (a no-op where the entry has no DB spawn or no door near it).
            Map* map = bot->GetMap();
            for (auto const& cp : map->GetCreatureBySpawnIdStore())
            {
                Creature* home = cp.second;
                if (!home || home == spawned || home->GetEntry() != state.attackEntry)
                    continue;
                GameObject* door = nullptr;
                float best = 25.0f;
                for (auto const& gp : map->GetGameObjectBySpawnIdStore())
                {
                    GameObject* go = gp.second;
                    if (!go || go->GetGoType() != GAMEOBJECT_TYPE_DOOR)
                        continue;
                    float const d = go->GetExactDist2d(home->GetPositionX(), home->GetPositionY());
                    if (d < best)
                    {
                        best = d;
                        door = go;
                    }
                }
                if (door)
                {
                    uint32 const st = uint32(door->GetGoState());
                    door->SetGoState(GO_STATE_ACTIVE);
                    TC_LOG_INFO("playerbots", "[SCENARIO] unflag entry=%u door entry=%u dist=%.1f state %u -> %u", state.attackEntry,
                        door->GetEntry(), best, st, uint32(door->GetGoState()));
                }
            }
        }
        return spawned;
    }
    bool ParseAttack(ChatHandler* handler, Player* bot, PlayerbotAI* botAI, std::istringstream& in, ScenarioState& state)
    {
        uint32 mapId = 0, entry = 0, secs = 0;
        float x, y, z;
        if (!(in >> mapId >> x >> y >> z >> entry))
        {
            handler->SendSysMessage("Usage: .botscenario <botname> attack <map> <x> <y> <z> <entry> [seconds] [spawn] [unflag] [off=<yd>]");
            return false;
        }
        bool spawn = false;
        bool unflag = false;
        // #827: "off=<yd>" moves the spawned target further out than the default, so a melee bot has to walk to reach it.
        float spawnOffset = ATTACK_SPAWN_OFFSET;
        std::string tok;
        while (in >> tok)
        {
            if (tok == "spawn")
                spawn = true;
            else if (tok == "unflag")
                unflag = true;
            else if (tok.rfind("off=", 0) == 0)
            {
                try
                {
                    spawnOffset = std::stof(tok.substr(4));
                }
                catch (std::exception const&)
                {
                    // Malformed offset on this debug-only command - keep the default.
                }
            }
            else
            {
                try
                {
                    secs = uint32(std::stoul(tok));
                }
                catch (std::exception const&)
                {
                    // Malformed seconds on this debug-only command - keep the default.
                }
            }
        }
        if (!secs)
            secs = 180;

        bot->TeleportTo(mapId, x, y, z, bot->GetOrientation());
        // Start every attack run from the same state on both cores: a same-map teleport does not Reset()
        // the AI, so a bot could still be in its combat engine from an earlier fight.
        bot->AttackStop();
        // #521 w13 (priest holy 0 s in combat, same signature as w10 Bernadela #603): a guardian the bot summoned before
        // this run (a Shadowfiend cast while it fought a wandering mob ~200 yd away) stays behind, so "pet attack" commands
        // it every tick and the run never engages the target. Unsummon every non-pet summon that is not near the scenario
        // point, so each run starts without stray guardians. Pets (hunter/warlock) are kept. Same code on both cores.
        {
            std::vector<TempSummon*> stray;
            for (Unit* controlled : bot->m_Controlled)
                if (controlled && controlled->IsSummon() && !controlled->IsPet() &&
                    (controlled->GetMapId() != mapId || controlled->GetExactDist(x, y, z) > 60.0f))
                    stray.push_back(controlled->ToTempSummon());
            for (TempSummon* summon : stray)
                summon->UnSummon();
            if (!stray.empty())
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s unsummoned %u stray guardian(s) before attack",
                    bot->GetName().c_str(), uint32(stray.size()));
        }
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
        botAI->ChangeEngine(BOT_STATE_NON_COMBAT);

        state.kind = ScenarioKind::Attack;
        state.destMapId = mapId;
        state.destX = x;
        state.destY = y;
        state.destZ = z;
        state.wanderMs = secs * IN_MILLISECONDS;
        state.allowMask = 0;
        state.pickNearest = false;
        state.pickCount = 0;
        state.attackEntry = entry;
        state.attackGuid = 0;
        state.attackSpawn = spawn;
        state.attackUnflag = unflag;
        state.attackSpawnOffset = spawnOffset;
        ArmWanderScenario(bot, botAI, state);

        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s start attack map=%u at=(%.1f, %.1f, %.1f) entry=%u target=%u",
            bot->GetName().c_str(), mapId, x, y, z, entry, 0u);
        return true;
    }

    bool AttackUpdate(Player* bot, PlayerbotAI* /*botAI*/, ScenarioState& state, uint32 /*now*/, uint32 t)
    {
        Creature* atkTarget = state.attackGuid ? ObjectAccessor::GetCreature(*bot, state.attackTarget) : nullptr;
        // w44 (#655 Gadok / #657 Koegler "killed" at 67-68%): a target that cannot be looked up (the tank died and released
        // out of the instance, or the boss despawned/evaded away) is not a kill - end the run as "lost".
        bool const lost = state.attackGuid != 0 && !atkTarget;
        bool const killed = atkTarget && !atkTarget->IsAlive();
        // No living target within ATTACK_ACQUIRE_RANGE a few seconds after landing (e.g. the previous
        // run's kill has not respawned): end the run as "notarget" instead of letting it time out, so
        // the two cores are never compared on different targets.
        bool const noTarget = state.attackGuid == 0 && t >= ATTACK_ACQUIRE_GRACE_MS;
        bool const timedOut = t >= state.wanderMs;
        if (!killed && !lost && !noTarget && !timedOut)
            return false;

        if (state.attackSpawn && atkTarget && atkTarget->IsAlive())
            atkTarget->DespawnOrUnsummon();

        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s %s t=%u map=%u x=%.1f y=%.1f z=%.1f",
            bot->GetName().c_str(), killed ? "killed" : lost ? "lost" : noTarget ? "notarget" : "timeout", t, bot->GetMapId(),
            bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        return true;
    }

    void RearmAttack(Player* bot, PlayerbotAI* botAI, ScenarioState& state)
    {
        ArmWanderScenario(bot, botAI, state);
        // G5: re-acquire rather than reuse the old guid - the target may need re-acquiring after
        // landing, and attackGuid == 0 is what makes UpdateScenarioTrace look it up again.
        state.attackGuid = 0;
    }

    // --- die (death / revive parity, research\death-revive-parity.md section 4) -----------------
    constexpr uint32 DIE_ARM_TIMEOUT_MS = 10000;
    constexpr uint32 DIE_SETTLE_MS = 5000;

    bool ParseDie(ChatHandler* handler, Player* bot, PlayerbotAI* botAI, std::istringstream& in, ScenarioState& state)
    {
        uint32 mapId = 0, entry = 0, secs = 0;
        float x, y, z;
        if (!(in >> mapId >> x >> y >> z >> entry) || !entry)
        {
            handler->SendSysMessage("Usage: .botscenario <botname> die <map> <x> <y> <z> <entry> [seconds] [dcount=<n>] [release=<ms>] [mgr=<ms>]");
            return false;
        }
        DieTrack die;
        std::string tok;
        while (in >> tok)
        {
            try
            {
                if (tok.rfind("dcount=", 0) == 0)       die.count = uint32(std::stoul(tok.substr(7)));
                else if (tok.rfind("release=", 0) == 0) die.releaseAfterMs = uint32(std::stoul(tok.substr(8)));
                else if (tok.rfind("mgr=", 0) == 0)     die.mgrAfterMs = uint32(std::stoul(tok.substr(4)));
                else                                    secs = uint32(std::stoul(tok));
            }
            catch (std::exception const&)
            {
                // Malformed token on this debug-only command - ignore it rather than take down the worldserver.
            }
        }
        if (!secs)
            secs = 300;

        if (!bot->IsAlive())
        {
            bot->ResurrectPlayer(1.0f);
            bot->SpawnCorpseBones();
        }
        bot->DurabilityRepairAll(false, 1.0f, false);
        bot->TeleportTo(mapId, x, y, z, bot->GetOrientation());
        // Start from a clean combat state (as the attack scenario does): a same-map teleport does not
        // Reset() the AI, so a fight carried over from the previous stage would continue here.
        bot->AttackStop();
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
        botAI->ChangeEngine(BOT_STATE_NON_COMBAT);

        state.kind = ScenarioKind::Die;
        state.destMapId = mapId; state.destX = x; state.destY = y; state.destZ = z;
        state.wanderMs = secs * IN_MILLISECONDS;
        state.allowMask = 0; state.pickNearest = false; state.pickCount = 0;
        state.noFight = true;
        state.attackEntry = entry; state.attackGuid = 0; state.attackSpawn = true; state.attackSpawnOffset = ATTACK_SPAWN_OFFSET;
        state.die = die;
        ArmWanderScenario(bot, botAI, state);   // kind Die -> ChangeToIdle, lock = true

        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s start die map=%u at=(%.1f, %.1f, %.1f) entry=%u dcount=%u release=%u mgr=%u",
            bot->GetName().c_str(), mapId, x, y, z, entry, die.count, die.releaseAfterMs, die.mgrAfterMs);
        return true;
    }

    void MarkRevived(Player* bot, DieTrack& d, uint32 t)
    {
        d.reviveT = t;
        d.reviveHp = bot->GetHealthPct();
        d.reviveX = bot->GetPositionX(); d.reviveY = bot->GetPositionY(); d.reviveZ = bot->GetPositionZ();
        d.phase = 3;
    }

    bool DieUpdate(Player* bot, PlayerbotAI* botAI, ScenarioState& s, uint32 /*now*/, uint32 t)
    {
        DieTrack& d = s.die;
        char const* name = bot->GetName().c_str();

        if (d.phase == 0)
        {
            if (t >= DIE_ARM_TIMEOUT_MS)
            {
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s notarget t=%u map=%u x=%.1f y=%.1f z=%.1f", name, t,
                    bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                return true;
            }
            if (bot->GetMapId() != s.destMapId || bot->GetExactDist2d(s.destX, s.destY) > 5.0f || !bot->IsAlive())
                return false;

            Creature* killer = SpawnAttackTarget(bot, s);
            if (!killer)
                return false;                       // retried next tick until DIE_ARM_TIMEOUT_MS
            botAI->GetAiObjectContext()->GetValue<uint32>("death count")->Set(d.count);
            killer->Kill(bot);                      // PvE death: corpse PVE, 10% durability loss
            killer->DespawnOrUnsummon();

            d.deathT = t; d.deathMap = bot->GetMapId();
            d.deathX = bot->GetPositionX(); d.deathY = bot->GetPositionY(); d.deathZ = bot->GetPositionZ();
            d.sawReset = false;   // only a Reset(true) after the death (Revive -> grind teleport) means "mgr"
            d.phase = 1;
            TC_LOG_DEBUG("playerbots", "[DEATH] %s died t=%u map=%u x=%.1f y=%.1f z=%.1f dcount=%u",
                name, t, d.deathMap, d.deathX, d.deathY, d.deathZ, d.count);
            return false;
        }

        if (d.phase == 1 || d.phase == 2)
        {
            if (bot->IsAlive())
                MarkRevived(bot, d, t);
            else if (d.phase == 1)
            {
                if (bot->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
                {
                    d.releaseT = t;
                    d.phase = 2;
                    Corpse* corpse = bot->GetCorpse();
                    uint32 const reclaim = corpse ? bot->GetCorpseReclaimDelay(corpse->GetType() == CORPSE_RESURRECTABLE_PVP) : 0;
                    float const gy = bot->GetMapId() == d.deathMap ? bot->GetExactDist(d.deathX, d.deathY, d.deathZ) : -1.0f;
                    TC_LOG_DEBUG("playerbots", "[DEATH] %s released t=%u after=%u map=%u x=%.1f y=%.1f z=%.1f gydist=%.1f reclaim=%u dcount=%u",
                        name, t, t - d.deathT, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                        gy, reclaim, botAI->GetAiObjectContext()->GetValue<uint32>("death count")->Get());
                }
                else if (d.releaseAfterMs && !d.releaseForced && t - d.deathT >= d.releaseAfterMs)
                {
                    d.releaseForced = true;
                    botAI->DoSpecificAction("auto release", Event(), true);   // the real AutoReleaseSpiritAction
                }
            }
            else // ghost
            {
                Corpse* corpse = bot->GetCorpse();
                if (!d.nearT && corpse && corpse->IsWithinDistInMap(bot, CORPSE_RECLAIM_RADIUS, true))
                {
                    d.nearT = t;
                    TC_LOG_DEBUG("playerbots", "[DEATH] %s corpse-near t=%u run=%u", name, t, t - d.releaseT);
                }
                if (d.mgrAfterMs && !d.mgrForced && t - d.releaseT >= d.mgrAfterMs)
                {
                    d.mgrForced = true;
                    sRandomPlayerbotMgr->Revive(bot);
                }
            }
        }

        if (d.phase == 3)
        {
            if (t - d.reviveT < DIE_SETTLE_MS)
                return false;
            float const toCorpse = std::sqrt((d.reviveX - d.deathX) * (d.reviveX - d.deathX) +
                                             (d.reviveY - d.deathY) * (d.reviveY - d.deathY) +
                                             (d.reviveZ - d.deathZ) * (d.reviveZ - d.deathZ));
            char const* method = (d.sawReset || d.reviveHp >= 99.5f) ? "mgr"
                               : toCorpse <= CORPSE_RECLAIM_RADIUS ? "corpse" : "spirit";
            float const moved = bot->GetMapId() == d.deathMap ? bot->GetExactDist(d.deathX, d.deathY, d.deathZ) : -1.0f;
            TC_LOG_DEBUG("playerbots",
                "[SCENARIO] %s revived t=%u method=%s release=%u run=%u wait=%u revive=%u moved=%.1f hp=%.0f map=%u x=%.1f y=%.1f z=%.1f",
                name, d.reviveT, method, d.releaseT ? d.releaseT - d.deathT : 0,
                (d.nearT && d.releaseT) ? d.nearT - d.releaseT : 0, d.nearT ? d.reviveT - d.nearT : 0,
                d.reviveT - d.deathT, moved, d.reviveHp, bot->GetMapId(),
                bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            return true;
        }

        if (t >= s.wanderMs)
        {
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s timeout t=%u phase=%u map=%u x=%.1f y=%.1f z=%.1f", name, t,
                uint32(d.phase), bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            return true;
        }
        return false;
    }

    // Reset(true) comes from RandomPlayerbotMgr::RandomTeleport (the post-revive grind teleport). Keep the
    // stage clock (unlike wander/attack), hold status IDLE, and record it for method=mgr.
    void RearmDie(Player* /*bot*/, PlayerbotAI* botAI, ScenarioState& state)
    {
        botAI->GetRpgInfo().ChangeToIdle();
        state.lock = true;
        state.die.sawReset = true;
    }

    // --- taxi (flight-master taxi stage, research\taxi-quest-stage-design.md section A.4) ---------
    // AC equivalent for anyone mirroring this handler: AC's harness lives in the same
    // Scenario/BotScenario.cpp file, so the AC-side edit is "add the same handler there" - there is
    // no separate AC function to point at yet. The core calls this ports are AC parity already
    // (sTravelMgr.GetNearestFlightMasterInfo, sTravelNodeMap.FindTaxiPath,
    // NewRpgInfo::ChangeToTravelFlight) - see NewRpgBaseAction.cpp/NewRpgAction.cpp for those.
    constexpr uint32 TAXI_BOARD_TIMEOUT_MS = 60000;
    constexpr uint32 TAXI_SETTLE_MS = 15000;

    // "path=a>b cost=" (A.4): the DBC price of each hop of the resolved node chain, summed. Player
    // ActivateTaxiPathTo debits the whole chain the same way (see ObjectMgr::GetTaxiPath users).
    uint32 TaxiPathCost(std::vector<uint32> const& path)
    {
        uint32 total = 0;
        for (size_t i = 0; i + 1 < path.size(); ++i)
        {
            uint32 pathId = 0, cost = 0;
            sObjectMgr->GetTaxiPath(path[i], path[i + 1], pathId, cost);
            total += cost;
        }
        return total;
    }

    std::string JoinPath(std::vector<uint32> const& path)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < path.size(); ++i)
        {
            if (i)
                oss << '>';
            oss << path[i];
        }
        return oss.str();
    }

    bool ParseTaxi(ChatHandler* handler, Player* bot, PlayerbotAI* botAI, std::istringstream& in, ScenarioState& state)
    {
        uint32 mapId = 0, fromNode = 0, toNode = 0, secs = 0;
        float x, y, z;
        if (!(in >> mapId >> x >> y >> z >> fromNode >> toNode))
        {
            handler->SendSysMessage("Usage: .botscenario <botname> taxi <map> <x> <y> <z> <fromNode> <toNode> [seconds]");
            return false;
        }
        in >> secs;
        if (!secs)
            secs = 240;

        bot->TeleportTo(mapId, x, y, z, bot->GetOrientation());
        // Start from a clean combat state (as the attack/die scenarios do): a same-map teleport does
        // not Reset() the AI, so a fight carried over from the previous stage would continue here.
        bot->AttackStop();
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
        botAI->ChangeEngine(BOT_STATE_NON_COMBAT);

        // A.4 step 2: top up money so Player::ActivateTaxiPathTo never refuses this run for being
        // short - the two-node routes this stage uses cost well under this on both cores.
        uint32 const moneyBefore = bot->GetMoney();
        if (moneyBefore < 10000)
            bot->ModifyMoney(10000 - moneyBefore);

        botAI->GetRpgInfo().ChangeToIdle();

        state.kind = ScenarioKind::Taxi;
        state.destMapId = mapId;
        state.destX = x;
        state.destY = y;
        state.destZ = z;
        state.wanderMs = secs * IN_MILLISECONDS;
        state.allowMask = 0;
        state.pickNearest = false;
        state.pickCount = 0;
        state.noFight = true; // A.4: "attack anything" is not useful for this stage
        state.attackGuid = 0;
        state.attackEntry = 0;
        state.lock = true;
        state.startT = getMSTime();
        state.lastTraceT = 0;
        state.taxi = TaxiTrack{};
        state.taxi.from = fromNode;
        state.taxi.to = toNode;

        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s start taxi map=%u at=(%.1f, %.1f, %.1f) from=%u to=%u money=%u",
            bot->GetName().c_str(), mapId, x, y, z, fromNode, toNode, bot->GetMoney());
        return true;
    }

    bool TaxiUpdate(Player* bot, PlayerbotAI* botAI, ScenarioState& s, uint32 /*now*/, uint32 t)
    {
        TaxiTrack& tx = s.taxi;
        char const* name = bot->GetName().c_str();

        if (t >= s.wanderMs)
        {
            // t= right after the result word, same convention as every other scenario kind's shared
            // "timeout" line (e.g. DieUpdate) - run-scenario-compare.ps1's $endRx relies on it.
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s timeout t=%u phase=%u map=%u x=%.1f y=%.1f z=%.1f",
                name, t, uint32(tx.phase), bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            return true;
        }

        // Phase 0 (arm): wait until the bot is within ArriveDistance() of the start point, then arm
        // the deterministic flight the same way the real random draw would (SelectRandomFlightTaxiNode
        // -> ChangeToTravelFlight), but to the fixed from/to this command asked for.
        if (tx.phase == 0)
        {
            if (bot->GetMapId() != s.destMapId || bot->GetExactDist2d(s.destX, s.destY) > ArriveDistance())
                return false;

            TravelMgr::FlightMasterInfo const* info = sTravelMgr.GetNearestFlightMasterInfo(bot);
            if (!info || info->taxiNodeId != tx.from)
            {
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s nofm t=%u fmnode=%u", name, t, info ? info->taxiNodeId : 0);
                return true;
            }

            std::vector<uint32> path = sTravelNodeMap.FindTaxiPath(tx.from, tx.to);
            if (path.empty())
            {
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s nofm t=%u fmnode=%u", name, t, info->taxiNodeId);
                return true;
            }

            // #225: the resolved route is a node chain, so it costs path.size()-1 legs to fly and
            // the nodes in between are connection points. A single-leg stage (T1/T2) resolves to
            // exactly two nodes, i.e. legCount 1, which phase 2's stopover branch can never match.
            tx.legCount = uint32(path.size() - 1);
            botAI->GetRpgInfo().ChangeToTravelFlight(info->templateEntry, info->pos, path);

            tx.armT = t;
            tx.fmEntry = info->templateEntry;
            tx.fmX = info->pos.GetPositionX();
            tx.fmY = info->pos.GetPositionY();
            tx.fmZ = info->pos.GetPositionZ();
            tx.fmNode = info->taxiNodeId;
            tx.departX = bot->GetPositionX();
            tx.departY = bot->GetPositionY();
            tx.departZ = bot->GetPositionZ();
            tx.phase = 1;

            TC_LOG_DEBUG("playerbots", "[TAXI] %s armed t=%u fm=%u fmnode=%u fmdist=%.1f path=%s cost=%u",
                name, t, tx.fmEntry, tx.fmNode, bot->GetExactDist(tx.fmX, tx.fmY, tx.fmZ), JoinPath(path).c_str(),
                TaxiPathCost(path));
            return false;
        }

        // Phase 1 (walk): wait for the boarding gossip (ActivateTaxiPathTo) to take the bot airborne.
        if (tx.phase == 1)
        {
            float const fmDist = bot->GetExactDist(tx.fmX, tx.fmY, tx.fmZ);
            if (!tx.atFmT && fmDist <= 6.0f)
            {
                tx.atFmT = t;
                tx.mountedAtFm = bot->IsMounted();
                TC_LOG_DEBUG("playerbots", "[TAXI] %s atfm t=%u dist=%.1f mounted=%d", name, t, fmDist,
                    tx.mountedAtFm ? 1 : 0);
            }

            if (bot->IsInFlight())
            {
                tx.boardT = t;
                tx.phase = 2;
                tx.legs = 1; // #225: leg 1 is airborne; phase 2 counts every re-board from here
                TC_LOG_DEBUG("playerbots", "[TAXI] %s boarded t=%u fmdist=%.1f x=%.1f y=%.1f z=%.1f",
                    name, t, fmDist, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                return false;
            }

            int const status = int(botAI->GetRpgInfo().GetStatus());
            if (status != RPG_TRAVEL_FLIGHT || t - tx.armT >= TAXI_BOARD_TIMEOUT_MS)
            {
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s noboard t=%u status=%d", name, t, status);
                return true;
            }

            return false;
        }

        // Phase 2 (flight): the trace hook runs before CanUpdateAI on both cores, so it keeps ticking
        // through MoP's (pre-G-T1) in-flight sleep and the landing time below is exact either way.
        if (tx.phase == 2)
        {
            // #225: a resolved route can have more than one leg. The MoP core flies one leg per
            // FlightPathMovementGenerator and only starts the next one from the *client's*
            // CMSG_MOVE_SPLINE_DONE (TaxiHandler.cpp:339-418), so at a leg boundary the bot is
            // briefly neither flying nor arrived while m_taxi still holds the remaining hops. That
            // is a stopover, not the landing: report it, stay in phase 2. PlayerbotAI::UpdateAI
            // calls ContinueTaxiIfLanded after this hook, so the next leg shows up here as a
            // re-board on a later tick.
            if (tx.waitReboard && bot->IsInFlight())
            {
                tx.waitReboard = false;
                ++tx.legs;
                TC_LOG_DEBUG("playerbots", "[TAXI] %s reboarded t=%u leg=%u of %u x=%.1f y=%.1f z=%.1f",
                    name, t, tx.legs, tx.legCount, bot->GetPositionX(), bot->GetPositionY(),
                    bot->GetPositionZ());
            }

            if (bot->IsInFlight())
                return false;

            // A hop is still pending exactly when m_taxi names a node that is not the destination the
            // command asked for: on the final touchdown the deque is [previous, to], so a pending
            // node other than `to` is a connection point mid-route. legCount == 1 keeps the deque at
            // [from, to] from boarding on, so single-leg stages fall straight through to the
            // untouched landing block below.
            uint32 const nextNode = bot->m_taxi.empty() ? 0 : bot->m_taxi.GetTaxiDestination();
            if (tx.legCount > 1 && nextNode && nextNode != tx.to)
            {
                if (!tx.waitReboard)
                {
                    tx.waitReboard = true;
                    tx.stopoverT = t;
                    ++tx.stopovers;
                    TC_LOG_DEBUG("playerbots", "[TAXI] %s stopover t=%u n=%u next=%u x=%.1f y=%.1f z=%.1f",
                        name, t, tx.stopovers, nextNode, bot->GetPositionX(), bot->GetPositionY(),
                        bot->GetPositionZ());
                }

                if (t - tx.stopoverT >= TAXI_BOARD_TIMEOUT_MS)
                {
                    // #225's failure signature: the route is parked on a connection point with hops
                    // still queued and no re-board - pre-fix, exactly what the bot does there.
                    TC_LOG_DEBUG("playerbots", "[SCENARIO] %s noboard t=%u stopover=%u next=%u legs=%u/%u x=%.1f y=%.1f z=%.1f",
                        name, t, tx.stopovers, nextNode, tx.legs, tx.legCount, bot->GetPositionX(),
                        bot->GetPositionY(), bot->GetPositionZ());
                    return true;
                }

                return false;
            }

            tx.landT = t;
            tx.landX = bot->GetPositionX();
            tx.landY = bot->GetPositionY();
            tx.landZ = bot->GetPositionZ();
            tx.statusAfter = int(botAI->GetRpgInfo().GetStatus());

            float nodeDist = -1.0f;
            if (TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(tx.to))
                nodeDist = bot->GetExactDist(node->x, node->y, node->z);

            TC_LOG_DEBUG("playerbots", "[TAXI] %s landed t=%u flight=%u x=%.1f y=%.1f z=%.1f nodedist=%.1f status=%d",
                name, t, t - tx.boardT, tx.landX, tx.landY, tx.landZ, nodeDist, tx.statusAfter);

            tx.phase = 3;
            return false;
        }

        // Phase 3 (settle, TAXI_SETTLE_MS - longer than passiveDelay so G-T1 shows as a number, not a
        // failure): wait for NewRpgStatusUpdateAction's TRAVEL_FLIGHT -> IDLE transition.
        int const status = int(botAI->GetRpgInfo().GetStatus());
        if (!tx.resolveT && status != RPG_TRAVEL_FLIGHT)
        {
            tx.resolveT = t;
            TC_LOG_DEBUG("playerbots", "[TAXI] %s resolved t=%u after=%u status=%d", name, t, t - tx.landT, status);
        }

        if (t - tx.landT < TAXI_SETTLE_MS)
            return false;

        // #225: multi-leg routes only - one summary line to grep, so a stopover route's pass is
        // "legs=2/2 stopovers=1" and not just the shared "landed" word. legCount == 1 (T1/T2) logs
        // nothing extra and ends byte-identically to before.
        if (tx.legCount > 1)
            TC_LOG_DEBUG("playerbots", "[TAXI] %s route t=%u legs=%u/%u stopovers=%u flight=%u",
                name, t, tx.legs, tx.legCount, tx.stopovers, t - tx.boardT);

        if (tx.resolveT)
        {
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s landed t=%u map=%u x=%.1f y=%.1f z=%.1f",
                name, t, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        }
        else
        {
            // #120 regression: NewRpgTravelFlightAction::Execute still thinks it needs to reach the
            // departure flight master, since the status never resolved off TRAVEL_FLIGHT - "back=" is
            // how close the bot now is to that point, for a quick eyeball of whether it walked toward it.
            float const back = bot->GetExactDist(tx.departX, tx.departY, tx.departZ);
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s unresolved t=%u back=%.1f map=%u x=%.1f y=%.1f z=%.1f",
                name, t, back, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        }
        return true;
    }

    void RearmTaxi(Player* /*bot*/, PlayerbotAI* botAI, ScenarioState& state)
    {
        TaxiTrack& tx = state.taxi;
        // Reset(full) itself early-returns while UNIT_STATE_IN_FLIGHT (PlayerbotAI::Reset), so this
        // only ever runs for phase 0/1 (not armed yet, or armed but still walking) or phase 3 (landed,
        // settling) - never mid-flight.
        if (tx.phase < 3 && tx.fmNode)
        {
            std::vector<uint32> path = sTravelNodeMap.FindTaxiPath(tx.from, tx.to);
            if (!path.empty())
                botAI->GetRpgInfo().ChangeToTravelFlight(tx.fmEntry,
                    WorldPosition(state.destMapId, tx.fmX, tx.fmY, tx.fmZ), path);
        }
        else
            botAI->GetRpgInfo().ChangeToIdle();

        state.lock = true;
    }

    // --- quest (questing stage, research\taxi-quest-stage-design.md section B.4) -------------------
    constexpr uint32 QUEST_ARM_HOLD_MS = 2000;
    constexpr float QUEST_OFFER_RANGE = 80.0f;
    constexpr float QUEST_SPAWN_TRIGGER_DIST = 60.0f;

    void DespawnSurvivors(Player* bot, std::vector<ObjectGuid> const& guids)
    {
        for (ObjectGuid const& guid : guids)
        {
            // #151: spawnGuids can now also hold spawngo= game objects (SpawnQuestTargetGos below).
            if (guid.IsGameObject())
            {
                if (GameObject* go = ObjectAccessor::GetGameObject(*bot, guid))
                    go->Delete();
            }
            else if (Creature* c = ObjectAccessor::GetCreature(*bot, guid))
            {
                if (c->IsAlive())
                    c->DespawnOrUnsummon();
            }
        }
    }

    // Duplicate of NewRpgBaseAction::IsQuestWorthDoing/IsQuestCapableDoing (post G-Q1 fix) for the
    // harness offer-log only: this file has no NewRpgBaseAction instance to call (it is bound to a
    // bot/botAI pair inside strategy/RPG, and BotScenario.cpp is deliberately outside that dependency
    // - see the file banner). Keep this in sync with NewRpgBaseAction.cpp if that logic changes.
    bool QuestOfferable(Player* bot, Quest const* quest)
    {
        if (!quest || bot->GetQuestStatus(quest->GetQuestId()) != QUEST_STATUS_NONE)
            return false;
        if (!bot->CanTakeQuest(quest, false) || !bot->CanAddQuest(quest, false))
            return false;

        int32 const adjLevel = bot->GetQuestLevel(quest);
        if (int32(bot->GetLevel()) > adjLevel + int32(sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF)))
            return false;
        if (quest->IsRepeatable() || quest->IsSeasonal())
            return false;
        if (int32(bot->GetLevel()) + 3 < adjLevel)
            return false;
        if (quest->GetSuggestedPlayers() >= 2)
            return false;

        return true;
    }

    // B.4 step 2: mirrors OrganizeQuestLog's dropQuest lambda (NewRpgBaseAction.cpp), minus the timed
    // quest/PvP-state bookkeeping that only matters for a quest actually being played out.
    // 1:1 parity: AC bots carry 4 x 51809 from PlayerbotFactory::InitBags. MoP bots randomized before that port
    // have none, and a full 16-slot backpack cannot store a single quest item.
    // Identical in both harnesses; a no-op when the bag slots are already filled.
    void EnsureStageBags(Player* bot)
    {
        // CanEquipItem refuses bags while a spell is being cast (CLIENT_LOCKED_OUT) or in combat (NOT_IN_COMBAT),
        // which is the normal state of a caster bot at stage start; clear both first.
        bot->InterruptNonMeleeSpells(false);
        bot->CombatStop(true);
        for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; ++slot)
        {
            if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                continue;
            uint16 dest;
            InventoryResult const res = bot->CanEquipNewItem(slot, dest, 51809, false);
            if (res == EQUIP_ERR_OK)
                bot->EquipNewItem(dest, 51809, true);
            else
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s stage bag slot %u refused: equip result %u", bot->GetName().c_str(), uint32(slot), uint32(res));
        }
    }

    // #698 1:1 gear parity (harness only). MoP's item DB has Cataclysm items at RequiredLevel 77-80 (green <=300, blue
    // <=417, epic <=359) that AC's WotLK DB lacks, so `randomize 80` gave ilvl ~308 vs AC ~184. At level <= 80 cap the
    // pick at the highest ilvl AC's item_template has per quality at RequiredLevel 2-80 (green 187, blue 200, epic 284),
    // through the ported gearScoreLimit (AC PlayerbotFactory.cpp:2446-2454). 0 = no cap (level > 85 or other quality).
    // #373 (Cataclysm 5-mans, levels 81-85; no AC partner exists above 80): the MoP item DB also holds 5.x items at
    // RequiredLevel 81-85 (rare to ilvl 580, epic to 496), which made an 85 bot average ilvl ~425 and the Cata dungeons
    // trivial. Same mechanism, the ceiling being what the Cataclysm 5-mans themselves drop: the highest ilvl per quality
    // in creature_loot_template (+ reference_loot_template) of the 14 Cata dungeon maps (645 643 725 657 670 644 755 568
    // 859 938 939 940 36 33), items RequiredLevel >= 77, on the test DB `world`: green 312, blue 346 (every 4.0 dungeon),
    // epic 359 (4.0 dungeons) / 378 (End Time, Well of Eternity, Hour of Twilight); the one 480 epic in SFK is a stray
    // MoP row and is ignored. The dungeons' own lfg_dungeon_template.requiredItemLevel (226-353) brackets the same range.
    // Levels 86-90 stay uncapped (MoP).
    uint32 WotlkGearScoreCap(uint32 level, uint32 quality)
    {
        if (level > 85)
            return 0;
        bool const cata = level > 80;
        uint32 ilvl = 0;
        switch (quality)
        {
            case ITEM_QUALITY_UNCOMMON: ilvl = cata ? 312 : 187; break;
            case ITEM_QUALITY_RARE: ilvl = cata ? 346 : 200; break;
            case ITEM_QUALITY_EPIC: ilvl = cata ? 378 : 284; break;
            default: return 0;
        }
        return BotFactory::CalcMixedGearScore(ilvl, quality);
    }

    void ResetQuestChain(Player* bot, PlayerbotAI* botAI, uint32 questId)
    {
        if (!questId)
            return;

        if (bot->GetQuestStatus(questId) != QUEST_STATUS_NONE)
        {
            for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
            {
                if (bot->GetQuestSlotQuestId(i) != questId)
                    continue;
                bot->DestroyQuestItems(questId);
                bot->RemoveActiveQuest(questId);
                bot->SetQuestSlot(i, 0);
                break;
            }
        }

        // Leftover objective items from an earlier run (or an earlier life of this random bot) would complete an
        // item quest the moment it is accepted, so destroy them even when the quest is not in the log.
        if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
            for (QuestObjective const& objective : quest->GetObjectives())
                if (objective.Type == QUEST_OBJECTIVE_ITEM && objective.ObjectID > 0)
                    bot->DestroyItemCount(uint32(objective.ObjectID), 9999, true);

        bot->RemoveRewardedQuest(questId);
        botAI->GetLowPriorityQuests().erase(questId);
    }

    // "spawn=n" (B.3 item 1 / B.4): a golden-angle ring of fresh objective-entry creatures around the
    // POI, same offset sequence as BotScenario::NextRandomNearAngle, with no summoner so a run never
    // depends on this quest's DB respawn timers or where a wandering spawn happens to be.
    void SpawnQuestTargets(Player* bot, QuestTrack& q, uint32 entry, WorldPosition const& center, uint32 t,
        uint32 remainingMs)
    {
        for (uint32 i = 0; i < q.spawnN; ++i)
        {
            float const angle = fmod(i * 2.39996f, 2.0f * float(M_PI));
            float const radius = 15.0f + 2.0f * i;
            float x = center.GetPositionX() + radius * std::cos(angle);
            float y = center.GetPositionY() + radius * std::sin(angle);
            float z = center.GetPositionZ() + 5.0f;
            bot->UpdateGroundPositionZ(x, y, z);
            Position const pos(x, y, z, 0.0f);
            if (Creature* c = bot->GetMap()->SummonCreature(entry, pos, nullptr, Milliseconds(remainingMs)))
                q.spawnGuids.push_back(c->GetGUID());
        }
        q.spawned = true;
        TC_LOG_DEBUG("playerbots", "[QUEST] %s spawned t=%u n=%u", bot->GetName().c_str(), t, q.spawnN);
    }

    // #151 (loot-stack-port.md section 2 step 7): "spawngo=<entry>" counterpart of
    // SpawnQuestTargets, for a GO-collect quest stage (e.g. 8463 "Unstable Mana Crystals"). Same
    // golden-angle ring, same no-DB-respawn-timer rationale. GameObject ownership: SummonGameObject
    // on a Player attaches the GO to that player (core:Entities/Object/Object.cpp:2846-2879), which
    // conveniently also bypasses the loot handlers' IsAtInteractDistance check for it
    // (core:Handlers/LootHandler.cpp:91,184,439) - harmless for a single-bot test stage.
    void SpawnQuestTargetGos(Player* bot, QuestTrack& q, uint32 entry, WorldPosition const& center, uint32 t,
        uint32 remainingMs)
    {
        for (uint32 i = 0; i < q.spawnN; ++i)
        {
            float const angle = fmod(i * 2.39996f, 2.0f * float(M_PI));
            float const radius = 15.0f + 2.0f * i;
            float x = center.GetPositionX() + radius * std::cos(angle);
            float y = center.GetPositionY() + radius * std::sin(angle);
            float z = center.GetPositionZ() + 5.0f;
            bot->UpdateGroundPositionZ(x, y, z);
            if (GameObject* go = bot->SummonGameObject(entry, x, y, z, 0.0f, {}, (remainingMs / 1000) + 5))
            {
                // Bug fix (issue #151 follow-up): WorldObject::SummonGameObject's Player-summoner
                // branch (core:Entities/Object/Object.cpp:2846-2879) calls
                // ToUnit()->AddGameObject(go) instead of go->SetSpawnedByDefault(false) (the branch
                // every *non*-player summoner takes). A fresh GameObject's constructor default is
                // m_spawnedByDefault = true (core:Entities/GameObject/GameObject.cpp:53), so with
                // that flag left true and the nonzero one-shot respawnTime above,
                // GameObject::isSpawned() - "(m_respawnTime > 0 && !m_spawnedByDefault) || ..." -
                // evaluates false. Every loot code path this port added or touched gates on
                // isSpawned() (NearestGameObjects.cpp's filter, LootObjectStack::Refresh/
                // GetWorldObject, OpenLootAction::GetOpeningSpell's single-arg overload), all
                // exactly mirroring AC's own gates - so a harness-spawned crate was invisible to the
                // whole pipeline: never entered "nearest game objects"/"available loot", so
                // "loot"/"move to loot"/"open loot" never even targeted it, and quest 8463 sat at 0
                // progress for the full run (the bot could only succeed by accident, wandering next
                // to one of the map's own real DB-spawned crates instead of one of these 6). Force
                // it the same way a non-player summoner already gets, so the ring is loot-visible
                // immediately. GO ownership itself (AddGameObject) is unaffected and still gives the
                // interact-distance bypass noted above.
                go->SetSpawnedByDefault(false);
                q.spawnGuids.push_back(go->GetGUID());
            }
        }
        q.spawned = true;
        TC_LOG_DEBUG("playerbots", "[QUEST] %s spawned t=%u n=%u go=%u", bot->GetName().c_str(), t, q.spawnN, entry);
    }

    bool ParseQuest(ChatHandler* handler, Player* bot, PlayerbotAI* botAI, std::istringstream& in, ScenarioState& state)
    {
        uint32 mapId = 0, questId = 0, secs = 0;
        float x, y, z;
        if (!(in >> mapId >> x >> y >> z >> questId) || !questId)
        {
            handler->SendSysMessage(
                "Usage: .botscenario <botname> quest <map> <x> <y> <z> <questId> [seconds] [clearlog] [spawn=<n>] "
                "[spawnnpc=<entry>] [spawngo=<entry>] [pick=nearest] [reset=<id,..>]");
            return false;
        }

        bool clearLog = false;
        bool pickNearest = false;
        uint32 spawnN = 0;
        uint32 spawnNpcEntry = 0;
        uint32 spawnGoEntry = 0;
        std::vector<uint32> resetIds;
        std::string tok;
        while (in >> tok)
        {
            if (tok == "clearlog")
                clearLog = true;
            else if (tok == "pick=nearest")
                pickNearest = true;
            else if (tok.rfind("spawn=", 0) == 0)
            {
                try { spawnN = uint32(std::stoul(tok.substr(6))); }
                catch (std::exception const&) { /* malformed spawn= on this debug-only command */ }
            }
            // #151: for a drop/collect-from-GO objective, quest->GetObjectives()'s ObjectID is an
            // item id, not a spawnable entry - these override what SpawnQuestTargets(Gos) picks.
            else if (tok.rfind("spawnnpc=", 0) == 0)
            {
                try { spawnNpcEntry = uint32(std::stoul(tok.substr(9))); }
                catch (std::exception const&) { /* malformed spawnnpc= on this debug-only command */ }
            }
            else if (tok.rfind("spawngo=", 0) == 0)
            {
                try { spawnGoEntry = uint32(std::stoul(tok.substr(8))); }
                catch (std::exception const&) { /* malformed spawngo= on this debug-only command */ }
            }
            else if (tok.rfind("reset=", 0) == 0)
            {
                std::istringstream rin(tok.substr(6));
                std::string part;
                while (std::getline(rin, part, ','))
                {
                    if (part.empty())
                        continue;
                    try { resetIds.push_back(uint32(std::stoul(part))); }
                    catch (std::exception const&) { /* malformed reset= token - ignore it */ }
                }
            }
            else
            {
                try { secs = uint32(std::stoul(tok)); }
                catch (std::exception const&) { /* malformed seconds - keep the default */ }
            }
        }
        if (!secs)
            secs = 1200;

        bot->TeleportTo(mapId, x, y, z, bot->GetOrientation());
        bot->AttackStop();
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
        botAI->ChangeEngine(BOT_STATE_NON_COMBAT);

        // B.4 step 2: reset questId + its chain (RewardNextQuest) + any explicit reset= ids, so a
        // rerun always starts from QUEST_STATUS_NONE regardless of what an earlier run left behind.
        ResetQuestChain(bot, botAI, questId);
        if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
            if (uint32 const nextInChain = quest->GetNextQuestInChain())
                ResetQuestChain(bot, botAI, nextInChain);
        for (uint32 id : resetIds)
            ResetQuestChain(bot, botAI, id);

        // B.4 step 3: "clearlog" drops every other active quest so the offer/accept trace only ever
        // shows this stage's own quest activity.
        if (clearLog)
        {
            for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
                if (uint32 const id = bot->GetQuestSlotQuestId(i))
                    ResetQuestChain(bot, botAI, id);
        }

        EnsureStageBags(bot);

        botAI->GetRpgInfo().ChangeToIdle();

        state.kind = ScenarioKind::Quest;
        state.destMapId = mapId;
        state.destX = x;
        state.destY = y;
        state.destZ = z;
        state.wanderMs = secs * IN_MILLISECONDS;
        state.allowMask = 0;
        state.pickNearest = pickNearest; // B.3 item 3: PickNearest(bot) hook (optional NewRpgBaseAction guard)
        state.noFight = false;           // B.4 step 1: the grind strategy must be free to kill
        state.pickCount = 0;
        state.attackGuid = 0;
        state.attackEntry = 0;
        state.lock = true;
        state.startT = getMSTime();
        state.lastTraceT = 0;
        state.quest = QuestTrack{};
        state.quest.questId = questId;
        state.quest.spawnN = spawnN;
        state.quest.spawnNpcEntry = spawnNpcEntry;
        state.quest.spawnGoEntry = spawnGoEntry;
        state.quest.wasAlive = bot->IsAlive();

        TC_LOG_DEBUG("playerbots",
            "[SCENARIO] %s start quest map=%u at=(%.1f, %.1f, %.1f) quest=%u spawn=%u spawnnpc=%u spawngo=%u pick=%s",
            bot->GetName().c_str(), mapId, x, y, z, questId, spawnN, spawnNpcEntry, spawnGoEntry,
            pickNearest ? "nearest" : "off");
        return true;
    }

    bool QuestUpdate(Player* bot, PlayerbotAI* botAI, ScenarioState& s, uint32 /*now*/, uint32 t)
    {
        QuestTrack& q = s.quest;
        char const* name = bot->GetName().c_str();

        bool const alive = bot->IsAlive();
        if (q.wasAlive && !alive)
            ++q.deaths;
        q.wasAlive = alive;

        if (t >= s.wanderMs)
        {
            DespawnSurvivors(bot, q.spawnGuids);
            // t= right after the result word - see the matching note in TaxiUpdate.
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s timeout t=%u phase=%u map=%u x=%.1f y=%.1f z=%.1f",
                name, t, uint32(q.phase), bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            return true;
        }

        Quest const* quest = sObjectMgr->GetQuestTemplate(q.questId);
        if (!quest)
            return false; // nothing sane to do; the global timeout above ends the run

        // Phase 0 (arm): wait for the bot to settle at the start point, log the offer list per giver
        // (B.3 item 2), snapshot the starting quest log, then draw the deterministic DO_QUEST.
        if (q.phase == 0)
        {
            if (bot->GetMapId() != s.destMapId || bot->GetExactDist2d(s.destX, s.destY) > ArriveDistance())
            {
                q.readyT = 0;
                return false;
            }
            if (!q.readyT)
            {
                q.readyT = t;
                return false;
            }
            if (t - q.readyT < QUEST_ARM_HOLD_MS)
                return false;

            GuidVector const possible = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible new rpg targets")->Get();
            for (ObjectGuid const& guid : possible)
            {
                WorldObject* obj = ObjectAccessor::GetWorldObject(*bot, guid);
                if (!obj || bot->GetDistance(obj) > QUEST_OFFER_RANGE)
                    continue;

                bot->PrepareQuestMenu(guid);
                QuestMenu const& menu = bot->PlayerTalkClass->GetQuestMenu();
                if (menu.Empty())
                    continue;

                std::string ids;
                for (uint32 idx = 0; idx < menu.GetMenuItemCount(); ++idx)
                {
                    QuestMenuItem const& item = menu.GetItem(idx);
                    if (!QuestOfferable(bot, sObjectMgr->GetQuestTemplate(item.QuestId)))
                        continue;
                    if (!ids.empty())
                        ids += ',';
                    ids += std::to_string(item.QuestId);
                }

                if (!ids.empty())
                    TC_LOG_DEBUG("playerbots", "[QUEST] %s offer giver=%u q=%s", name, obj->GetEntry(), ids.c_str());
            }

            for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
                if (uint32 const id = bot->GetQuestSlotQuestId(i))
                    q.logSet.insert(id);

            botAI->GetRpgInfo().ChangeToDoQuest(q.questId, quest);
            q.phase = 1;
            return false;
        }

        // Phase 1 (running): diff the quest log every tick against the previous snapshot.
        std::set<uint32> current;
        for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
            if (uint32 const id = bot->GetQuestSlotQuestId(i))
                current.insert(id);

        for (uint32 id : current)
        {
            if (q.logSet.count(id))
                continue;
            if (id == q.questId)
                q.acceptT = t;
            else
                ++q.extraAccepts;
            TC_LOG_DEBUG("playerbots", "[QUEST] %s accept t=%u q=%u", name, t, id);
        }
        for (uint32 id : q.logSet)
        {
            if (current.count(id))
                continue;
            if (bot->GetQuestRewardStatus(id))
            {
                if (id == q.questId)
                    q.rewardT = t;
                TC_LOG_DEBUG("playerbots", "[QUEST] %s reward t=%u q=%u", name, t, id);
            }
            else
                TC_LOG_DEBUG("playerbots", "[QUEST] %s drop t=%u q=%u", name, t, id);
        }
        q.logSet = std::move(current);

        // DoQuest sub-state: POI moves, arrival, and (spawn=n) summoning the objective creatures.
        if (auto const* data = std::get_if<NewRpgInfo::DoQuest>(&botAI->GetRpgInfo().data))
        {
            if (data->questId == q.questId)
            {
                bool const posChanged = data->pos != WorldPosition() &&
                    (data->pos.GetPositionX() != q.lastPoiX || data->pos.GetPositionY() != q.lastPoiY);
                if (posChanged)
                {
                    q.lastPoiX = data->pos.GetPositionX();
                    q.lastPoiY = data->pos.GetPositionY();
                    if (!q.poiT)
                        q.poiT = t;

                    // AC calls this "the blob centroid" (GetQuestPOIPosAndObjectiveIdx's random convex
                    // combination of the blob's points); this fork instead reports the bot's live
                    // distance to that already-sampled point (data->pos) - the centroid itself when
                    // pick=nearest is set (B.3 item 3), a random point on the blob otherwise.
                    float const cdist = bot->GetDistance2d(data->pos.GetPositionX(), data->pos.GetPositionY());
                    TC_LOG_DEBUG("playerbots", "[QUEST] %s poi t=%u idx=%d x=%.1f y=%.1f cdist=%.1f",
                        name, t, data->objectiveIdx, q.lastPoiX, q.lastPoiY, cdist);
                }

                if (data->lastReachPOI && !q.atPoiT)
                {
                    q.atPoiT = t;
                    TC_LOG_DEBUG("playerbots", "[QUEST] %s atpoi t=%u", name, t);
                }

                // Only while an objective is being worked (not the turn-in POI, objectiveIdx -1) and the quest is
                // still incomplete: the AC run once spawned the ring around the ender after completion.
                if (q.spawnN && !q.spawned && data->pos != WorldPosition() && data->objectiveIdx >= 0 &&
                    bot->GetQuestStatus(q.questId) == QUEST_STATUS_INCOMPLETE &&
                    bot->GetExactDist(data->pos) <= QUEST_SPAWN_TRIGGER_DIST)
                {
                    // #151: spawngo=/spawnnpc= override the objective's own ObjectID, which for a
                    // drop/collect-from-GO objective is an item id, not a spawnable entry (see
                    // ParseQuest's usage string).
                    if (q.spawnGoEntry)
                    {
                        SpawnQuestTargetGos(bot, q, q.spawnGoEntry, data->pos, t, s.wanderMs - t);
                    }
                    else
                    {
                        uint32 entry = q.spawnNpcEntry;
                        if (!entry)
                        {
                            for (QuestObjective const& obj : quest->GetObjectives())
                            {
                                // POI index is the 3.3.5 layout: item objectives are 4 + StorageIndex (NewRpgBaseAction LegacyPoiIndex).
                                if ((obj.Type == QUEST_OBJECTIVE_ITEM ? 4 + int32(obj.StorageIndex) : int32(obj.StorageIndex)) == data->objectiveIdx)
                                {
                                    entry = obj.ObjectID;
                                    break;
                                }
                            }
                        }

                        if (entry)
                            SpawnQuestTargets(bot, q, entry, data->pos, t, s.wanderMs - t);
                    }
                }
            }
        }

        // Objective progress (up to the 4 slots QuestTrack::lastCount tracks).
        uint32 objIdx = 0;
        for (QuestObjective const& obj : quest->GetObjectives())
        {
            if (objIdx >= 4)
                break;
            uint32 const n = bot->GetQuestObjectiveCounter(obj.ID);
            if (n != q.lastCount[objIdx])
            {
                if (n > q.lastCount[objIdx])
                    q.kills += (n - q.lastCount[objIdx]);
                if (!q.firstProgT)
                    q.firstProgT = t;
                TC_LOG_DEBUG("playerbots", "[QUEST] %s progress t=%u obj=%d n=%u/%d",
                    name, t, int(obj.StorageIndex), n, obj.Amount);
                q.lastCount[objIdx] = n;
            }
            ++objIdx;
        }

        QuestStatus const qs = bot->GetQuestStatus(q.questId);
        if (qs == QUEST_STATUS_COMPLETE && !q.completeT)
        {
            q.completeT = t;
            TC_LOG_DEBUG("playerbots", "[QUEST] %s complete t=%u", name, t);
        }

        // --- end conditions, checked in this order (B.4) ------------------------------------------
        if (bot->GetQuestRewardStatus(q.questId))
        {
            DespawnSurvivors(bot, q.spawnGuids);
            TC_LOG_DEBUG("playerbots",
                "[SCENARIO] %s questdone t=%u accept=%u atpoi=%u firstprog=%u complete=%u turnin=%u kills=%u deaths=%u extra=%u map=%u x=%.1f y=%.1f z=%.1f",
                name, t, q.acceptT, q.atPoiT, q.firstProgT, q.completeT, q.rewardT, q.kills, q.deaths, q.extraAccepts,
                bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            return true;
        }

        int const status = int(botAI->GetRpgInfo().GetStatus());
        if (status != RPG_DO_QUEST)
        {
            DespawnSurvivors(bot, q.spawnGuids);
            if (qs == QUEST_STATUS_NONE)
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s noaccept t=%u status=%d", name, t, status);
            else
            {
                bool const lowPrio = botAI->GetLowPriorityQuests().count(q.questId) != 0;
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s abandoned t=%u qstatus=%d lowprio=%d",
                    name, t, int(qs), lowPrio ? 1 : 0);
            }
            return true;
        }

        return false;
    }

    void RearmQuest(Player* /*bot*/, PlayerbotAI* botAI, ScenarioState& state)
    {
        QuestTrack& q = state.quest;
        Quest const* quest = q.questId ? sObjectMgr->GetQuestTemplate(q.questId) : nullptr;
        // "Rearm: ChangeToDoQuest again if not yet rewarded" (B.4). Phase 0 (not armed yet) needs no
        // action here - the next tick's own phase-0 check re-evaluates from scratch.
        if (q.phase >= 1 && quest && !botAI->GetBot()->GetQuestRewardStatus(q.questId))
            botAI->GetRpgInfo().ChangeToDoQuest(q.questId, quest);
        else
            botAI->GetRpgInfo().ChangeToIdle();

        state.lock = true;
    }

    // --- transport (research\transport-stage-design.md section 3) --------------------------------
    // Issue #141 item 1: a deterministic 1:1 stage for the boat/zeppelin per-tick boarding check
    // (PlayerbotAI::UpdateAI, AI/PlayerbotAI.cpp:358+). The harness never boards the bot itself - it
    // only teleports it onto the deck; the module's own check (gated by TransportCheckDue below when
    // tcheck != natural) does the AddPassenger/RemovePassenger.
    constexpr uint32 TRANSPORT_DEFAULT_SECS = 900;
    constexpr float TRANSPORT_BOARD_RANGE = 5.0f;      // "missed": still this far from the deck point
                                                        // when stop A's departure passes with no board
    constexpr float TRANSPORT_DRAG_RANGE = 15.0f;      // "dragged": still a passenger this far from
                                                        // dest once T has left stop B
    constexpr uint32 TRANSPORT_DOCK_MARGIN_MS = 45000; // only place the bot if a dock window has at
                                                        // least this much time left

    // Per-core shims (research\transport-stage-design.md section 3.3): the only lines in this
    // section that differ between the MoP and AC BotScenario.cpp.
    Transport* ResolveTransport(Player* bot, uint32 g)
    {
        return ObjectAccessor::GetTransport(*bot, ObjectGuid(HighGuid::Mo_Transport, 0, g));
    }
    uint32 TrPeriod(Transport* t) { return t->GetTransportPeriod(); }
    uint32 TrTimer(Transport* t) { return t->GetTimer() % t->GetTransportPeriod(); } // GetTimer = PathProgress
    uint32 NodeIdx(KeyFrame const& k) { return k.Node->NodeIndex; }
    float NodeX(KeyFrame const& k) { return k.Node->LocX; }
    float NodeY(KeyFrame const& k) { return k.Node->LocY; }
    uint32 DbTransportGuid(uint32 entry)
    {
        QueryResult r = WorldDatabase.PQuery("SELECT guid FROM transports WHERE entry = %u", entry);
        return r ? (*r)[0].GetUInt32() : 0;
    }

    // Static/local transports (elevators, GAMEOBJECT_TYPE_TRANSPORT) are registered through
    // `gameobject`, not the `transports` table DbTransportGuid() above reads, and are deliberately
    // kept OUT of the grid (ObjectMgr.cpp: `AddLocalTransportSpawn` instead of `AddGameobjectToGrid`;
    // Map::AddToMap<Transport> only does AddToWorld()+_transports.insert(), never AddToGrid) - so a
    // grid visitor such as WorldObject::FindNearestGameObject can never see one, regardless of range,
    // phase or grid-load state. Map::_transports
    // is private and a local transport carries no spawn id, so walk the map's own object store
    // instead - the same TypeContainerVisitor idiom GameEventMgr::RunSmartAIScripts already uses.
    struct StaticTransportFinder
    {
        uint32 entry; WorldObject const* ref; Transport* best = nullptr; float bestD = 0.0f;
        template <class T> void Visit(std::unordered_map<ObjectGuid, T*>&) { }
        void Visit(std::unordered_map<ObjectGuid, GameObject*>& m)
        {
            for (auto const& p : m)
            {
                GameObject* go = p.second;
                if (!go->IsInWorld() || go->GetGoType() != GAMEOBJECT_TYPE_TRANSPORT || go->GetEntry() != entry)
                    continue;
                if (!go->GetGUID().IsMOTransport()) // #141 F3: only a real Transport::CreateLocal object
                    continue;
                float d = go->GetExactDist(ref);
                if (!best || d < bestD) { best = go->ToTransport(); bestD = d; }
            }
        }
    };
    Transport* FindStaticTransport(WorldObject* target, uint32 entry)
    {
        StaticTransportFinder finder{entry, target};
        TypeContainerVisitor<StaticTransportFinder, MapStoredObjectTypesContainer> visitor(finder);
        visitor.Visit(target->GetMap()->GetObjectsStore());
        return finder.best;
    }

    // Docked at stop frame i? (both cores: KeyFrame::ArriveTime/DepartureTime/IsStopFrame - MoP
    // TransportMgr.h:38-64, AC :41-67). Templated only so its source text is identical on both cores
    // even though ResolveTransport's return type (Transport* here, MotionTransport* on AC) differs.
    template <class T>
    bool DockedAt(T* t, uint32 i, uint32& leftMs)
    {
        KeyFrame const& k = t->GetKeyFrames()[i];
        uint32 const tm = TrTimer(t);
        if (tm >= k.ArriveTime && tm < k.DepartureTime)
        {
            leftMs = k.DepartureTime - tm;
            return true;
        }
        return false;
    }

    bool ParseTransport(ChatHandler* handler, Player* bot, PlayerbotAI* botAI, std::istringstream& in, ScenarioState& state)
    {
        uint32 mapId = 0, entry = 0, secs = 0;
        float x, y, z, deckX, deckY, deckZ, destX, destY, destZ;
        if (!(in >> mapId >> x >> y >> z >> entry >> deckX >> deckY >> deckZ >> destX >> destY >> destZ))
        {
            handler->SendSysMessage("Usage: .botscenario <botname> transport <map> <x> <y> <z> <entry> <deckX> <deckY> <deckZ> <destX> <destY> <destZ> [seconds] [tcheck=now|tick|natural]");
            return false;
        }

        in >> secs;
        if (!secs)
            secs = TRANSPORT_DEFAULT_SECS;

        uint8 tcheck = 1; // default "now" (section 3.2)
        std::string tok;
        while (in >> tok)
        {
            if (tok == "tcheck=now")
                tcheck = 1;
            else if (tok == "tcheck=tick")
                tcheck = 2;
            else if (tok == "tcheck=natural")
                tcheck = 0;
        }

        uint32 const dbGuid = DbTransportGuid(entry);

        bot->TeleportTo(mapId, x, y, z, bot->GetOrientation());
        DismountForGround(bot, true);
        // Start from a clean combat state (as goto/taxi do): a same-map teleport does not Reset() the
        // AI, so a fight carried over from the previous stage would continue here.
        bot->AttackStop();
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
        botAI->ChangeEngine(BOT_STATE_NON_COMBAT);
        botAI->GetRpgInfo().ChangeToIdle();

        state.kind = ScenarioKind::Transport;
        state.startT = getMSTime();
        state.lastTraceT = 0;
        // The arm/wait point (section 3.2 phase 0); the offboard pier goal lives in state.transport
        // below, since it can be on a different map than this one (stage C).
        state.destMapId = mapId;
        state.destX = x;
        state.destY = y;
        state.destZ = z;
        state.wanderMs = secs * IN_MILLISECONDS;
        state.lock = true;
        state.noFight = true;
        state.groundOnly = true;
        state.allowMask = 0;
        state.pickNearest = false;
        state.pickCount = 0;
        state.attackGuid = 0;
        state.attackEntry = 0;

        state.transport = TransportTrack{};
        state.transport.entry = entry;
        state.transport.dbGuid = dbGuid;
        state.transport.tcheck = tcheck;
        state.transport.tcheckPending = (tcheck == 1);
        state.transport.deckX = deckX;
        state.transport.deckY = deckY;
        state.transport.deckZ = deckZ;
        state.transport.destX = destX;
        state.transport.destY = destY;
        state.transport.destZ = destZ;
        state.transport.lastMap = mapId;

        TC_LOG_DEBUG("playerbots",
            "[SCENARIO] %s start transport map=%u at=(%.1f, %.1f, %.1f) entry=%u dbguid=%u tcheck=%u",
            bot->GetName().c_str(), mapId, x, y, z, entry, dbGuid, uint32(tcheck));
        return true;
    }

    bool TransportUpdate(Player* bot, PlayerbotAI* botAI, ScenarioState& s, uint32 /*now*/, uint32 t)
    {
        TransportTrack& tr = s.transport;
        char const* name = bot->GetName().c_str();

        if (t >= s.wanderMs)
        {
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s timeout t=%u phase=%u map=%u x=%.1f y=%.1f z=%.1f",
                name, t, uint32(tr.phase), bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                bot->GetPositionZ());
            return true;
        }

        // Phase 0 (arm): wait to reach the wait point, then resolve the transport and its two nearest
        // stop keyframes (stop A nearest the wait point, stop B the next one after it).
        // While waiting at the dock (phases 0-1) keep the bot on the wait point: MoP's gather action walks it
        // to a node, AC's does not, and phase 0 must keep looking the
        // transport up until it comes back from its other map.
        if (tr.atWait && tr.phase <= 1 && bot->GetMapId() == s.destMapId &&
            bot->GetExactDist(s.destX, s.destY, s.destZ) > 2.0f * ArriveDistance())
            bot->NearTeleportTo(s.destX, s.destY, s.destZ, bot->GetOrientation());

        if (tr.phase == 0)
        {
            if (bot->GetMapId() != s.destMapId)
                return false;
            if (!tr.atWait)
            {
                if (bot->GetExactDist(s.destX, s.destY, s.destZ) > ArriveDistance())
                    return false;
                tr.atWait = true;
            }

            auto* transport = ResolveTransport(bot, tr.dbGuid);
            if (!transport)
            {
                // (waiting: transport currently on its other map)
                return false; // not on this map yet (a cross-map transport can be on the other side): keep waiting
            }

            KeyFrameVec const& frames = transport->GetKeyFrames();
            bool foundA = false;
            uint32 bestA = 0;
            float bestDist = 0.0f;
            for (uint32 i = 0; i < frames.size(); ++i)
            {
                if (!frames[i].IsStopFrame())
                    continue;
                float const dx = NodeX(frames[i]) - s.destX;
                float const dy = NodeY(frames[i]) - s.destY;
                float const d = dx * dx + dy * dy;
                if (!foundA || d < bestDist)
                {
                    bestDist = d;
                    bestA = i;
                    foundA = true;
                }
            }

            bool foundB = false;
            uint32 bestB = 0;
            if (foundA)
            {
                for (uint32 i = bestA + 1; i < frames.size(); ++i)
                {
                    if (frames[i].IsStopFrame())
                    {
                        bestB = i;
                        foundB = true;
                        break;
                    }
                }
                if (!foundB)
                {
                    for (uint32 i = 0; i <= bestA; ++i)
                    {
                        if (frames[i].IsStopFrame())
                        {
                            bestB = i;
                            foundB = true;
                            break;
                        }
                    }
                }
            }

            if (!foundA || !foundB)
            {
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s notransport t=%u", name, t);
                return true;
            }

            tr.stopA = bestA;
            tr.stopB = bestB;
            tr.armT = t;
            tr.phase = 1;

            uint32 const period = TrPeriod(transport);
            uint32 const legMs = (frames[bestB].ArriveTime >= frames[bestA].DepartureTime)
                ? (frames[bestB].ArriveTime - frames[bestA].DepartureTime)
                : (period - frames[bestA].DepartureTime + frames[bestB].ArriveTime);

            uint32 leftMs = 0;
            uint32 nextArr = 0;
            if (!DockedAt(transport, bestA, leftMs))
            {
                uint32 const tm = TrTimer(transport);
                nextArr = (frames[bestA].ArriveTime >= tm) ? (frames[bestA].ArriveTime - tm)
                                                            : (period - tm + frames[bestA].ArriveTime);
            }

            TC_LOG_DEBUG("playerbots",
                "[TRANSPORT] %s armed t=%u entry=%u period=%u stopA=%u stopB=%u legms=%u nextarr=%u",
                name, t, tr.entry, period, NodeIdx(frames[bestA]), NodeIdx(frames[bestB]), legMs, nextArr);
            return false;
        }

        auto* transport = ResolveTransport(bot, tr.dbGuid);
        if (!transport)
        {
            // Waiting at the dock (phases 0-1): the transport may be on its other map; only a missing
            // transport after the bot was placed on it is a failure.
            if (tr.phase <= 1)
                return false;
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s notransport t=%u", name, t);
            return true;
        }
        KeyFrameVec const& frames = transport->GetKeyFrames();

        // Phase 1 (dock A): wait for a dock window with enough time left, then place the bot on the
        // deck with a near teleport. The module check (not the harness) boards it from here.
        if (tr.phase == 1)
        {
            uint32 leftMs = 0;
            if (!DockedAt(transport, tr.stopA, leftMs) || leftMs < TRANSPORT_DOCK_MARGIN_MS)
                return false;

            float x = tr.deckX, y = tr.deckY, z = tr.deckZ, o = 0.0f;
            transport->CalculatePassengerPosition(x, y, z, &o);
            bot->TeleportTo(bot->GetMapId(), x, y, z, o);

            tr.placeX = x;
            tr.placeY = y;
            tr.placeZ = z;
            tr.placeT = t;
            tr.tcheckPending = (tr.tcheck == 1);
            tr.phase = 2;

            TC_LOG_DEBUG("playerbots", "[TRANSPORT] %s placed t=%u left=%u x=%.1f y=%.1f z=%.1f",
                name, t, leftMs, x, y, z);
            return false;
        }

        // Phase 2 (board): wait for PlayerbotAI::UpdateAI's own check to AddPassenger the bot.
        if (tr.phase == 2)
        {
            if (bot->GetTransport() == transport)
            {
                tr.boardT = t;
                tr.boardLX = bot->GetTransOffsetX();
                tr.boardLY = bot->GetTransOffsetY();
                tr.boardLZ = bot->GetTransOffsetZ();
                tr.phase = 3;

                uint32 leftMs = 0;
                DockedAt(transport, tr.stopA, leftMs);
                TC_LOG_DEBUG("playerbots", "[TRANSPORT] %s boarded t=%u after=%u left=%u lx=%.2f ly=%.2f lz=%.2f",
                    name, t, t - tr.placeT, leftMs, tr.boardLX, tr.boardLY, tr.boardLZ);
                return false;
            }

            uint32 leftMs = 0;
            bool const stillDocked = DockedAt(transport, tr.stopA, leftMs);
            if (!stillDocked && bot->GetExactDist(tr.placeX, tr.placeY, tr.placeZ) > TRANSPORT_BOARD_RANGE)
            {
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s missed t=%u map=%u x=%.1f y=%.1f z=%.1f swim=%d",
                    name, t, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                    bot->IsInWater() ? 1 : 0);
                return true;
            }
            return false;
        }

        // Phase 3 (ride): watch for departure, drift off the boarded local offset, a mid-ride map
        // change, and an early drop (offboard before reaching stop B).
        if (tr.phase == 3)
        {
            if (bot->GetTransport() != transport)
            {
                TC_LOG_DEBUG("playerbots", "[TRANSPORT] %s offboard t=%u where=ride map=%u x=%.1f y=%.1f z=%.1f",
                    name, t, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s dropped t=%u", name, t);
                return true;
            }

            if (!tr.departed && TrTimer(transport) >= frames[tr.stopA].DepartureTime)
            {
                tr.departed = true;
                tr.departT = t;
                TC_LOG_DEBUG("playerbots", "[TRANSPORT] %s departed t=%u", name, t);
            }

            if (bot->GetMapId() != tr.lastMap)
            {
                TC_LOG_DEBUG("playerbots", "[TRANSPORT] %s mapchange t=%u from=%u to=%u passenger=%d",
                    name, t, tr.lastMap, bot->GetMapId(), bot->GetTransport() == transport ? 1 : 0);
                tr.lastMap = bot->GetMapId();
            }

            float const dx = bot->GetTransOffsetX() - tr.boardLX;
            float const dy = bot->GetTransOffsetY() - tr.boardLY;
            float const dz = bot->GetTransOffsetZ() - tr.boardLZ;
            float const drift = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (drift > tr.maxDrift)
                tr.maxDrift = drift;

            uint32 leftB = 0;
            if (tr.departed && DockedAt(transport, tr.stopB, leftB))
            {
                tr.arriveT = t;
                tr.phase = 4;

                float const sx = NodeX(frames[tr.stopB]), sy = NodeY(frames[tr.stopB]);
                float const stopDist = std::sqrt(std::pow(bot->GetPositionX() - sx, 2) + std::pow(bot->GetPositionY() - sy, 2));

                TC_LOG_DEBUG("playerbots", "[TRANSPORT] %s arrived t=%u ride=%u drift=%.2f stopdist=%.1f",
                    name, t, t - tr.departT, tr.maxDrift, stopDist);

                WorldPosition const dest(bot->GetMapId(), tr.destX, tr.destY, tr.destZ);
                NewRpgInfo& info = botAI->GetRpgInfo();
                info.ChangeToGoGrind(dest);
                info.SetMoveFarTo(dest);
            }
            return false;
        }

        // Phase 4 (offboard / ashore).
        bool const stillPassenger = bot->GetTransport() == transport;
        if (!tr.offT && !stillPassenger)
        {
            tr.offT = t;
            TC_LOG_DEBUG("playerbots",
                "[TRANSPORT] %s offboard t=%u after=%u x=%.1f y=%.1f z=%.1f ignorepf=%d swim=%d",
                name, t, t - tr.arriveT, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                bot->HasUnitState(UNIT_STATE_IGNORE_PATHFINDING) ? 1 : 0, bot->IsInWater() ? 1 : 0);
        }

        uint32 leftB = 0;
        bool const stillDockedB = DockedAt(transport, tr.stopB, leftB);
        if (stillPassenger && !stillDockedB && !tr.dragged &&
            bot->GetExactDist(tr.destX, tr.destY, tr.destZ) > TRANSPORT_DRAG_RANGE)
        {
            tr.dragged = true;
            TC_LOG_DEBUG("playerbots", "[TRANSPORT] %s dragged t=%u", name, t);
        }

        if (!stillPassenger && bot->GetExactDist(tr.destX, tr.destY, tr.destZ) < ArriveDistance())
        {
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s ashore t=%u map=%u x=%.1f y=%.1f z=%.1f ignorepf=%d swim=%d",
                name, t, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                bot->HasUnitState(UNIT_STATE_IGNORE_PATHFINDING) ? 1 : 0, bot->IsInWater() ? 1 : 0);
            return true;
        }

        if (!stillPassenger && bot->IsInWater() && bot->GetExactDist(tr.destX, tr.destY, tr.destZ) > ArriveDistance())
        {
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s fell t=%u map=%u x=%.1f y=%.1f z=%.1f",
                name, t, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            return true;
        }

        return false;
    }

    void RearmTransport(Player* /*bot*/, PlayerbotAI* botAI, ScenarioState& state)
    {
        TransportTrack& tr = state.transport;
        if (tr.phase < 4)
            botAI->GetRpgInfo().ChangeToIdle();
        else
        {
            WorldPosition const dest(tr.lastMap, tr.destX, tr.destY, tr.destZ);
            NewRpgInfo& info = botAI->GetRpgInfo();
            info.ChangeToGoGrind(dest);
            info.SetMoveFarTo(dest);
        }

        // Do NOT restart state.startT here: the ride must keep its own clock (section 3.3).
        state.lock = true;
    }

    // --- battleground (research\bg-parity-stage-design.md section 3) -----------------------------
    // A 1:1 WSG (later AB/EotS) harness stage. Only the anchor (bots=<n1,...>'s first name) carries a
    // side-table entry; ParseBg preps and queues every named member once, and BgUpdate then reads the
    // whole match - the Battleground object and every member - off the anchor's own map/AI update
    // thread (section 3.4). No hook in any other module file: pinning goes through public AI-context
    // values ("bg role") and the extern bgStrategies map, exactly as section 3.6 asks.
    // "bgnoform"/"bgnoanchor": not everyone in by then. Raised from the design doc's 90000: v16-bg-wsg-1
    // showed the *first* (already-invited) batch alone can take up to ~90.5 s to accept on MoP (natural
    // relevance-based AI latency - bg status = 3.0, see research doc section 2.1/section 4 fix #6, out
    // of scope here), before the missing-invite root cause below is even fixed. 180 s gives the slower
    // core's natural accept latency room without masking a real no-invite failure, which now surfaces
    // in seconds via the per-member invited/queued diagnostics instead of only at the timeout.
    constexpr uint32 BG_ENTER_TIMEOUT_MS = 180000;
    // #570 queue=natural: one census tick (BgCheckInterval, 35 s on the test servers) + the bots' own "bg join"
    // cadence + party gathering + the queue pop, twice over for margin.
    constexpr uint32 BG_NATURAL_ENTER_TIMEOUT_MS = 600000;
    constexpr uint32 BG_DEFAULT_SAMPLE_MS = 5000;
    constexpr uint32 BG_QUEUE_KICK_MS = 2000;       // re-issue ScheduleQueueUpdate at most this often
    constexpr uint32 BG_STATIC_WINDOW_MS = 30000;   // "static": alive, in progress, < 2 yd/window
    constexpr float BG_STATIC_MOVE_YD = 2.0f;
    constexpr float BG_GATECLIP_MARGIN_YD = 10.0f;  // added on top of the BG's own GetStartMaxDist()
    // Wave-29 arena legs (#391/#709): a member more than this far (3D) from its team's start point while the match is
    // IN_PROGRESS has left its start room ("leftstart"). Every arena start point is 60-82 yd from the centre
    // ArenaTactics::moveToCenter walks to; Ring of Valor's are pits ~27 yd below it (only ~10 yd away in 2D).
    constexpr float BG_ARENA_LEFTSTART_YD = 20.0f;

    char const* BgTeamLetter(uint8 team) { return team == TEAM_ALLIANCE ? "A" : "H"; }

    int FindBgMember(BgTrack const& bg, ObjectGuid guid)
    {
        if (guid.IsEmpty())
            return -1;
        for (size_t i = 0; i < bg.members.size(); ++i)
            if (bg.members[i] == guid)
                return int(i);
        return -1;
    }

    // Per-core shims (section 3.6): the only lines in this section that differ between the MoP and AC
    // BotScenario.cpp, the same pattern the transport stage's ResolveTransport/TrPeriod/... use.
    PvPDifficultyEntry const* BgBracket(uint32 mapId, uint32 level)
    {
        return sDBCManager.GetBattlegroundBracketByLevel(mapId, level);
    }

    void BgStartPos(Battleground* bgObj, uint8 team, float& x, float& y)
    {
        float z = 0.0f, o = 0.0f;
        // w29: GetTeamStartLoc takes a Team (ALLIANCE/HORDE), not a TeamId - uint32(TEAM_ALLIANCE)=0 resolved to the Horde
        // start, so every Alliance BG start-room/gate-clip distance was measured from the wrong base (see ArenaStartPos)
        bgObj->GetTeamStartLoc(team == TEAM_ALLIANCE ? ALLIANCE : HORDE, x, y, z, o);
    }

    // --- wave-29 arena legs (#391/#709/#437/#632): arena-only helpers. AC's twin defines the same names with its own
    // core API; everything that calls them is identical on both trees.
    // The team's start point in 3D. 5.4.8's GetTeamStartLoc takes a Team (ALLIANCE/HORDE), not a TeamId: passing a
    // TeamId resolves TEAM_ALLIANCE (0) to the Horde start (GetTeamIndexByTeamId, Battleground.h:515).
    void ArenaStartPos(Battleground* bgObj, uint8 team, float& x, float& y, float& z)
    {
        float o = 0.0f;
        bgObj->GetTeamStartLoc(team == TEAM_ALLIANCE ? ALLIANCE : HORDE, x, y, z, o);
    }

    // The arena the All-Arenas draw produced: 5.4.8 keeps BATTLEGROUND_AA in GetTypeID() and the drawn arena in
    // GetTypeID(true) (BattlegroundMgr.cpp CreateNewBattleground: SetTypeID / SetRandomTypeID).
    uint32 ArenaDrawnType(Battleground* bgObj) { return uint32(bgObj->GetTypeID(true)); }
    bool ArenaIsRated(Battleground* bgObj) { return bgObj->IsRated(); }
    char const* ArenaWinner(Battleground* bgObj)
    {
        uint8 const w = bgObj->GetWinner();
        return w == WINNER_ALLIANCE ? "A" : w == WINNER_HORDE ? "H" : "none";
    }

    // Roster members alive / dead on the arena map, per team (index = TeamId), and the team sizes.
    // A member resolved off the anchor's map, or anywhere when the anchor itself is not there (#709: the anchor can be
    // the one that vanished), and only while it is in this arena instance.
    Player* ArenaMember(BgTrack const& bg, std::vector<Player*> const& live, size_t i, uint32 instanceId)
    {
        Player* m = i < live.size() && live[i] ? live[i] : ObjectAccessor::FindPlayer(bg.members[i]);
        return m && m->InArena() && m->GetBattlegroundId() == instanceId ? m : nullptr;
    }

    void ArenaCounts(BgTrack const& bg, std::vector<Player*> const& live, uint32 instanceId, uint32 aliveN[2],
        uint32 deadN[2], uint32 sizeN[2])
    {
        aliveN[0] = aliveN[1] = deadN[0] = deadN[1] = sizeN[0] = sizeN[1] = 0;
        for (size_t i = 0; i < bg.members.size(); ++i)
        {
            uint8 const team = bg.memberTeam[i] == TEAM_ALLIANCE ? 0 : 1;
            ++sizeN[team];
            if (Player* m = ArenaMember(bg, live, i, instanceId))
                ++(m->IsAlive() ? aliveN : deadN)[team];
        }
    }

    // How an arena ended, from the core's winner and the last IN_PROGRESS sample. Wave 30: counted at STATUS_WAIT_LEAVE,
    // every end read "time"/"left" - EndBattleground has already resurrected the dead (MoP Battleground.cpp:1101-1104,
    // AC :883-886) and members are already leaving. The cores' arena time limits never name a winner (MoP :416-419
    // WINNER_NONE, AC :305-307 PVP_TEAM_NEUTRAL), so: "time" - no winner; "left" - a member of the losing team vanished
    // (#709) or was not on the map at the last sample; "kill" - otherwise (CheckArenaWinConditions saw every loser dead;
    // the final kill and the end fall in one core update, between two samples, so the sampled deaths= can be short).
    char const* ArenaEndReason(BgTrack const& bg, char const* winner)
    {
        if (!std::strcmp(winner, "anchorout"))
            return "anchorout";
        if (winner[0] != 'A' && winner[0] != 'H')
            return "time";
        uint8 const loser = winner[0] == 'A' ? 1 : 0;
        uint32 size = 0;
        bool vanished = false;
        for (size_t i = 0; i < bg.members.size(); ++i)
        {
            if ((bg.memberTeam[i] == TEAM_ALLIANCE ? 0 : 1) != loser)
                continue;
            ++size;
            if (i < bg.vanishedLogged.size() && bg.vanishedLogged[i])
                vanished = true;
        }
        if (vanished || (bg.arenaCounted && bg.arenaAlive[loser] + bg.arenaDead[loser] < size))
            return "left";
        return "kill";
    }

    // alive/dead per team are the LAST IN_PROGRESS SAMPLE (before EndBattleground resurrects the dead); deaths= is the
    // sampled death count of the match. winner "anchorout" = the anchor left a running arena (no verdict).
    void LogArenaEnd(char const* anchorName, BgTrack const& bg, uint32 now, char const* winner)
    {
        TC_LOG_DEBUG("playerbots",
            "[BGSTAGE] %s arenaend winner=%s reason=%s fightMs=%u aliveA=%u aliveH=%u deadA=%u deadH=%u deaths=%u/%u "
            "leftstart=%u/%u engageA=%d engageH=%d",
            anchorName, winner, ArenaEndReason(bg, winner), bg.gatesSeen ? getMSTimeDiff(bg.gatesT, now) : 0u,
            bg.arenaAlive[0], bg.arenaAlive[1], bg.arenaDead[0], bg.arenaDead[1], bg.deaths[0], bg.deaths[1],
            bg.leftStartCount, uint32(bg.members.size()), bg.engageT[0], bg.engageT[1]);
    }

    // --- AV adapter: the AV-specific reads,
    // per-core. MoP: the new BattlegroundAV getters added in step 2. AC's twin defines the same
    // function names against its own existing public getters plus FillInitialWorldStates for
    // reinforcements - everything below this section (LogAvEnd, AvBgUpdate) is identical on both
    // trees and only calls these.
    uint32 const AV_CREATURE_A_CAPTAIN = AV_CPLACE_TRIGGER16;
    uint32 const AV_CREATURE_H_CAPTAIN = AV_CPLACE_TRIGGER18;
    // AC's AV_CPLACE_A_BOSS / H_BOSS constants are not in this core's BG_AV_ObjectPlaces enum - see
    // the identical comment and derivation in strategy/bg/BattleGroundTactics.cpp.
    uint32 const AV_CREATURE_A_BOSS = AV_CPLACE_MAX + 60;
    uint32 const AV_CREATURE_H_BOSS = AV_CPLACE_MAX + 122;

    struct AvNodeSnapshot
    {
        uint8 state = 0;        // BG_AV_States: 0 neutral/standing, 1 assaulted, 2 destroyed, 3 controlled
        uint8 ownerLetter = 0;  // 0 = neutral, 1 = Alliance, 2 = Horde
    };

    AvNodeSnapshot AvNode(Battleground* bgObj, uint8 node)
    {
        BattlegroundAV* av = static_cast<BattlegroundAV*>(bgObj);
        BG_AV_NodeInfo const& info = av->GetAVNodeInfo(node);
        AvNodeSnapshot snap;
        snap.state = uint8(info.State);
        snap.ownerLetter = info.Owner == ALLIANCE ? 1 : info.Owner == HORDE ? 2 : 0;
        return snap;
    }

    bool AvCaptainAlive(Battleground* bgObj, uint8 side)  // 0 = Alliance, 1 = Horde
    {
        return static_cast<BattlegroundAV*>(bgObj)->IsCaptainAlive(side);
    }

    uint8 AvMineOwnerLetter(Battleground* bgObj, uint8 mineIdx)  // 0 = north, 1 = south
    {
        uint32 const owner = static_cast<BattlegroundAV*>(bgObj)->GetMineOwner(mineIdx);
        return owner == ALLIANCE ? 1 : owner == HORDE ? 2 : 0;
    }

    bool AvReinforcements(Battleground* bgObj, int32& allianceReinf, int32& hordeReinf)
    {
        BattlegroundAV* av = static_cast<BattlegroundAV*>(bgObj);
        allianceReinf = av->GetTeamReinforcements(0);
        hordeReinf = av->GetTeamReinforcements(1);
        return true;
    }

    bool AvBossAlive(Battleground* bgObj, uint8 side)  // 0 = Alliance boss (Vanndar), 1 = Horde boss (Drek'Thar)
    {
        Creature* boss = bgObj->GetBGCreature(side == 0 ? AV_CREATURE_A_BOSS : AV_CREATURE_H_BOSS);
        return boss && boss->IsAlive();
    }

    uint8 AvBossHealthPct(Battleground* bgObj, uint8 side)
    {
        Creature* boss = bgObj->GetBGCreature(side == 0 ? AV_CREATURE_A_BOSS : AV_CREATURE_H_BOSS);
        if (!boss || !boss->GetMaxHealth())
            return 0;
        return uint8(std::min<uint64>(100, (100ull * boss->GetHealth()) / boss->GetMaxHealth()));
    }

    // v18-bg-wsg-1 escalation: 20 solo AddGroup entries share the server's single global per-
    // (bgType,bracket) queue with every other bot's own natural BGJoinAction traffic (BgJoinBlocked
    // only stops a non-roster bot from *starting* a new join, not from an existing natural queue/BG
    // instance elsewhere absorbing top-up invites). CheckNormalMatch (BattlegroundQueue.cpp) only
    // creates a new instance at the bracket minimum, and once it leaves WAIT_JOIN,
    // Battleground::GetFreeSlotsForTeam's IN_PROGRESS branch throttles further invites to a strict
    // side-balancing trickle that can permanently strand the last few queued solo entries. Both are
    // core (non-module) behaviour, identical on both trees, and out of reach from Scenario files.
    //
    // CheckPremadeMatch runs BEFORE CheckNormalMatch on both cores (BattlegroundQueue.cpp) and matches
    // two premade GroupQueueInfo entries - one per faction - atomically in a single pass, inviting each
    // in full up to MaxPlayersPerTeam. So queue each faction as a real (bot-led) Group instead: a
    // 10-Alliance and a 10-Horde group, each queued through the same tail BGJoinAction::JoinQueue uses
    // for solo entries (strategy/bg/BattleGroundJoinAction.cpp:203-253), just with isPremade=true and a
    // real Group* argument to AddGroup.
    //
    // The group is disbanded again immediately after AddGroup snapshots its membership into
    // GroupQueueInfo::Players - nothing further needs it (Battleground manages its own internal BG-raid
    // group once players actually enter, Battleground.cpp SetBgRaid), and every step here runs inside
    // one synchronous .botscenario command with no bot AI tick in between, so no bot is ever exposed to
    // a fellow-bot "master" via PlayerbotAI::UpdateAI's FindNewMaster (see AI/PlayerbotAI.cpp:1202-1203 -
    // that guard only skips a *group member* candidate while InBattleground(), which none of these are
    // yet at queue time; disbanding before the first AI tick sidesteps the gap instead of relying on it).
    // Group::CanJoinBattlegroundQueue rejects GetMembersCount() > BattlemasterListEntry::maxGroupSize with
    // ERR_BATTLEGROUND_NONE. That is -1 on AC but 0 on MoP, where the harness's `!err` reads it as success.
    // maxGroupSize is 10 (WSG) / 15 (AB, EotS) on AC's WotLK DBC but 5 on MoP's 5.4.8 DBC. Capping the seed
    // group to it split a MoP WSG team into a 5-man premade plus 5 solos, and one queue pass then built two
    // 5v5 instances (v28/v30). QueueFactionGroupForBg
    // therefore seeds with max(maxGroupSize, team size): one team-sized premade per faction on both cores,
    // AC's own path. BgMaxGroupSize is the DBC value; the solo overflow only runs if a roster is bigger.
    uint32 BgMaxGroupSize(BattlegroundTypeId bgTypeId, Battleground* bgTemplate)
    {
        if (BattlemasterListEntry const* bmEntry = sBattlemasterListStore.LookupEntry(uint32(bgTypeId)))
            if (bmEntry->maxGroupSize)
                return bmEntry->maxGroupSize;
        return bgTemplate->GetMinPlayersPerTeam(); // defensive fallback if the DBC lookup ever fails
    }

    uint8 ArenaQueueType(BattlegroundTypeId bgTypeId, size_t rosterSize)
    {
        return BattlegroundMgr::IsArenaType(bgTypeId) &&
            (rosterSize / 2 == ARENA_TYPE_2v2 || rosterSize / 2 == ARENA_TYPE_3v3 ||
                rosterSize / 2 == ARENA_TYPE_5v5) ? uint8(rosterSize / 2) : 0;
    }

    // #631: a stopped/ended queue=natural stage must not leave its rated-bracket hold in the census until the
    // next ResetCensus pass (BgCheckInterval) - non-roster captains take it the moment BgJoinBlocked lets go.
    void ReleaseNaturalArenaHold(BgTrack const& bg)
    {
        if (!bg.naturalQueue)
            return;
        BattlegroundTypeId const bgTypeId = BattlegroundTypeId(bg.bgTypeId);
        BattlegroundQueueTypeId const queueTypeId =
            BattlegroundMgr::BGQueueTypeId(bgTypeId, ArenaQueueType(bgTypeId, bg.members.size()));
        PlayerbotArena::Census(uint32(queueTypeId), bg.bracketId).activeRatedArenaQueue = 0;
    }
    // Mirrors the tail of BGJoinAction::JoinQueue (strategy/bg/BattleGroundJoinAction.cpp:203-253) for
    // one member - used only for the part of a faction's roster beyond BgMaxGroupSize() above, which
    // cannot be queued as part of the premade group at all. These land in the *normal* queue and ride
    // BgUpdate's existing queue-kick loop (ScheduleQueueUpdate -> FillPlayersToBG) into the same
    // instance the premade seed group creates, while it is still in WAIT_JOIN.
    bool QueueSoloMemberForBg(Player* member, BattlegroundTypeId bgTypeId, size_t rosterSize, PvPDifficultyEntry const* bracketEntry)
    {
        if (!member->IsInWorld() || member->InBattleground())
            return false;

        Battleground* bgTemplate = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bgTemplate)
            return false;

        BattlegroundQueueTypeId const queueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, ArenaQueueType(bgTypeId, rosterSize));
        if (member->InBattlegroundQueueForBattlegroundQueueType(queueTypeId))
            return false;
        if (!member->GetBGAccessByLevel(bgTypeId))
            return false;
        if (!member->HasFreeBattlegroundQueueId())
            return false;
        if (!member->CanJoinToBattleground(bgTemplate))
            return false;

        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
        // #544: an arena entry must carry its arena type - with 0 the core's forced rated pass
        // (BattlegroundMgr.cpp:143-153) files the result under GetSlotByType(0) = PVP_SLOT_RATED_BG.
        GroupQueueInfo* ginfo = bgQueue.AddGroup(member, nullptr, bgTypeId, bracketEntry,
            ArenaQueueType(bgTypeId, rosterSize), false, false, 0, 0);
        if (!ginfo)
            return false;

        uint32 const avgWaitTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
        uint32 const queueSlot = member->AddBattlegroundQueueId(queueTypeId);

        WorldPacket data;
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bgTemplate, member, queueSlot, STATUS_WAIT_QUEUE,
            avgWaitTime, 0, 0);
        member->GetSession()->SendPacket(&data);
        return true;
    }

    // Returns one ok flag per teamMembers[i] (not a single faction-wide bool - see v23-bg-ab above: the
    // premade seed and the solo overflow can now fail independently, and the coordinator asked for the
    // per-member queued/missing bookkeeping to reflect that accurately instead of a blanket "joinfailed").
    std::vector<uint8> QueueFactionGroupForBg(std::vector<Player*> const& teamMembers, BattlegroundTypeId bgTypeId,
        size_t rosterSize, PvPDifficultyEntry const* bracketEntry)
    {
        std::vector<uint8> ok(teamMembers.size(), 0);
        if (teamMembers.empty())
            return ok;

        // A member that left an earlier BG run mid-match carries Deserter (26013, saved with the character) and
        // cannot queue; clear it on both cores so every 1:1 run starts from the same state (v43 AC bgnoform).
        for (Player* member : teamMembers)
            if (member)
                member->RemoveAurasDueToSpell(26013);

        Battleground* bgTemplate = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bgTemplate)
            return ok;

        uint32 const cap = std::max(BgMaxGroupSize(bgTypeId, bgTemplate), bgTemplate->GetMaxPlayersPerTeam());
        size_t const groupSize = std::min(teamMembers.size(), size_t(cap));

        Player* leader = teamMembers[0];
        if (leader->IsInWorld() && !leader->InBattleground())
        {
            Group* group = new Group();
            group->Create(leader);
            sGroupMgr->AddGroup(group);
            for (size_t i = 1; i < groupSize; ++i)
                group->AddMember(teamMembers[i]);

            BattlegroundQueueTypeId const queueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, ArenaQueueType(bgTypeId, teamMembers.size() * 2));
            GroupJoinBattlegroundResult const err =
                group->CanJoinBattlegroundQueue(bgTemplate, queueTypeId, ArenaQueueType(bgTypeId, teamMembers.size() * 2), bgTemplate->GetMaxPlayersPerTeam(), false, 0);

            bool groupOk = false;
            if (!err && leader->HasFreeBattlegroundQueueId())
            {
                BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
                // Wave-29 arena skirmish legs: AC queues a skirmish party with isPremade=false (AC
                // HandleBattlemasterJoinArena, BattleGroundHandler.cpp:870), so it waits in the normal queue and is
                // matched by the unrated pass. As a premade it sat in the premade queue, where the forced rated pass
                // (BattlegroundMgr.cpp:143-153, every Arena.RatedUpdateTimer) took it as a RATED match (#544).
                bool const isPremade = !ArenaQueueType(bgTypeId, teamMembers.size() * 2) &&
                    groupSize >= bgTemplate->GetMinPlayersPerTeam();
                // #544: pass the arena type (0 for a battleground). With 0, an arena premade is matched by the core's
                // forced rated pass (BattlegroundMgr.cpp:143-153) and its result lands in rated_pvp_info slot 3
                // (GetSlotByType(0) = PVP_SLOT_RATED_BG, ArenaTeam.cpp:43/90) instead of the bracket's own slot.
                if (GroupQueueInfo* ginfo = bgQueue.AddGroup(leader, group, bgTypeId, bracketEntry,
                        ArenaQueueType(bgTypeId, teamMembers.size() * 2), false, isPremade, 0, 0))
                {
                    uint32 const avgWaitTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
                    for (size_t i = 0; i < groupSize; ++i)
                    {
                        Player* member = teamMembers[i];
                        uint32 const queueSlot = member->AddBattlegroundQueueId(queueTypeId);
                        WorldPacket data;
                        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bgTemplate, member, queueSlot,
                            STATUS_WAIT_QUEUE, avgWaitTime, 0, 0);
                        member->GetSession()->SendPacket(&data);
                    }
                    groupOk = true;
                }
            }

            group->Disband();

            TC_LOG_DEBUG("playerbots",
                "[BG] %s-led group of %u (cap %u) queued for battleground %u as premade (queue %u, bracket %u) ok=%d err=%d",
                leader->GetName().c_str(), uint32(groupSize), cap, uint32(bgTypeId), uint32(queueTypeId),
                uint32(bracketEntry->GetBracketId()), groupOk ? 1 : 0, int(err));

            if (groupOk)
                for (size_t i = 0; i < groupSize; ++i)
                    ok[i] = 1;
            else
            {
                // AV parity: AC's
                // Group::CanJoinBattlegroundQueue rejects a group bigger than the BG's maxGroupSize
                // (5 on AV) with ERR_BATTLEGROUND_NONE (-1), so a team-sized AV premade never queues
                // at all on AC. The same check returns 0 (success) on MoP, so groupOk is normally
                // already true here and this branch is a no-op on this core - kept identical on both
                // trees per the harness rule. Fall every member of the failed premade back to the same
                // solo path the overflow beyond groupSize already uses.
                for (size_t i = 0; i < groupSize; ++i)
                    ok[i] = QueueSoloMemberForBg(teamMembers[i], bgTypeId, rosterSize, bracketEntry) ? 1 : 0;
            }
        }

        // Beyond the DBC's maxGroupSize cap: solo entries into the normal queue (no-op range when the
        // whole roster fit in the premade group - e.g. every WSG run so far).
        for (size_t i = groupSize; i < teamMembers.size(); ++i)
            ok[i] = QueueSoloMemberForBg(teamMembers[i], bgTypeId, rosterSize, bracketEntry) ? 1 : 0;

        return ok;
    }

    bool ParseBg(ChatHandler* handler, Player* bot, PlayerbotAI* /*botAI*/, std::istringstream& in, ScenarioState& state)
    {
        uint32 bgTypeIdRaw = 0, secs = 0;
        if (!(in >> bgTypeIdRaw >> secs) || !secs)
        {
            handler->SendSysMessage(
                "Usage: .botscenario <botname> bg <bgTypeId> <seconds> bots=<n1,n2,...> [strat=<a>,<h>|none] "
                "[roles=index|natural] [fast=<ms>] [sample=<ms>] [trail=anchor|all] [queue=natural] [onlyteam=A|H] [aqtimeout=1]");
            return false;
        }

        std::vector<std::string> botNames;
        uint8 stratA = 0, stratH = 0;
        bool rolesIndex = true;
        bool naturalQueue = false;
        int8 onlyTeam = -1;      // wave-29 #632 leg: onlyteam=A|H
        bool aqTimeout = false;  // wave-29 #632 leg: aqtimeout=1
        uint32 fastMs = 0, sampleMs = BG_DEFAULT_SAMPLE_MS;
        std::string tok;
        while (in >> tok)
        {
            if (tok.rfind("bots=", 0) == 0)
            {
                std::istringstream bin(tok.substr(5));
                std::string part;
                while (std::getline(bin, part, ','))
                    if (!part.empty())
                        botNames.push_back(part);
            }
            else if (tok == "strat=none")
                stratA = stratH = BG_STRAT_UNPINNED;  // #743: leave the BG's own roll (ToK weights) alone
            else if (tok.rfind("strat=", 0) == 0)
            {
                std::istringstream sin(tok.substr(6));
                std::string a, h;
                if (std::getline(sin, a, ',') && std::getline(sin, h, ','))
                {
                    try { stratA = uint8(std::stoul(a)); stratH = uint8(std::stoul(h)); }
                    catch (std::exception const&) { /* malformed strat= token on this debug-only command - keep 0,0 */ }
                }
            }
            else if (tok == "roles=natural")
                rolesIndex = false;
            else if (tok == "roles=index")
                rolesIndex = true;
            else if (tok == "queue=natural")
                naturalQueue = true;
            else if (tok == "onlyteam=A" || tok == "onlyteam=H")
                onlyTeam = int8(tok.back() == 'A' ? TEAM_ALLIANCE : TEAM_HORDE);
            else if (tok == "aqtimeout=1")
                aqTimeout = true;
            else if (tok.rfind("fast=", 0) == 0)
            {
                try { fastMs = uint32(std::stoul(tok.substr(5))); } catch (std::exception const&) {}
            }
            else if (tok.rfind("sample=", 0) == 0)
            {
                try { sampleMs = uint32(std::stoul(tok.substr(7))); } catch (std::exception const&) {}
                if (!sampleMs)
                    sampleMs = BG_DEFAULT_SAMPLE_MS;
            }
            else if (tok == "trail=all")
                handler->SendSysMessage("botscenario: bg trail=all is not implemented - only the anchor's trail runs");
            // "trail=anchor" is the only implemented mode (the default) - accepted and ignored.
        }

        if (botNames.empty() || botNames.front() != bot->GetName())
        {
            handler->SendSysMessage("botscenario: bg bots=<list> must start with the anchor's own name");
            return false;
        }
        if (botNames.size() < 4 || (botNames.size() % 2) != 0)
        {
            handler->SendSysMessage("botscenario: bg bots=<list> must be an even-sized roster (equal team halves)");
            return false;
        }

        BattlegroundTypeId const bgTypeId = BattlegroundTypeId(bgTypeIdRaw);
        Battleground* bgTemplate = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bgTemplate)
        {
            handler->SendSysMessage("botscenario: bg unknown bgTypeId");
            return false;
        }
        uint32 const mapId = bgTemplate->GetMapId();

        // #570 queue=natural: the roster queues itself through its own BGJoinAction (rated arena: a leader
        // gathers its party and sends the arena join). Only the all-arenas template takes it - that is the
        // template the natural path queues under (BGTemplateId(2v2/3v3/5v5) = BATTLEGROUND_AA).
        if (naturalQueue && (bgTypeId != BATTLEGROUND_AA || !ArenaQueueType(bgTypeId, botNames.size())))
        {
            handler->SendSysMessage("botscenario: bg queue=natural needs bgTypeId 6 (all arenas) and a 4/6/10 roster");
            return false;
        }

        // Wave-29 arena legs: an arena the harness queues itself (a skirmish) also goes through the All-Arenas
        // template, as the natural rated one does; `.botscenario - arenapin <bgTypeId>` picks the map, identically on
        // both cores. A single arena's own type id is refused (AC cannot queue it; on 5.4.8 it was taken by the
        // forced rated pass, #544).
        bool const arenaStage = BattlegroundMgr::IsArenaType(bgTypeId);
        if (arenaStage && (bgTypeId != BATTLEGROUND_AA || !ArenaQueueType(bgTypeId, botNames.size())))
        {
            handler->SendSysMessage("botscenario: bg arena stages need bgTypeId 6 (all arenas; pick the map with "
                "`.botscenario - arenapin <bgTypeId>`) and a 4/6/10 roster");
            return false;
        }
        if ((onlyTeam >= 0 || aqTimeout) && !naturalQueue)
        {
            handler->SendSysMessage("botscenario: bg onlyteam=/aqtimeout= need queue=natural");
            return false;
        }

        // --- resolve + validate every member (section 3.2 step 3) --------------------------------
        struct Member { Player* player; PlayerbotAI* ai; };
        std::vector<Member> members;
        members.reserve(botNames.size());
        for (std::string const& n : botNames)
        {
            Player* p = ObjectAccessor::FindPlayerByName(n);
            PlayerbotAI* ai = p ? GET_PLAYERBOT_AI(p) : nullptr;
            if (!p || !ai)
            {
                handler->SendSysMessage(("botscenario: bg member '" + n + "' is not an online bot").c_str());
                return false;
            }
            if (p->InBattleground())
            {
                handler->SendSysMessage(("botscenario: bg member '" + n + "' is already in a battleground").c_str());
                return false;
            }
            members.push_back({p, ai});
        }

        PvPDifficultyEntry const* bracket0 = BgBracket(mapId, members.front().player->GetLevel());
        if (!bracket0)
        {
            handler->SendSysMessage("botscenario: bg anchor has no PvP bracket for this map");
            return false;
        }
        for (Member const& m : members)
        {
            PvPDifficultyEntry const* d = BgBracket(mapId, m.player->GetLevel());
            if (!d || d->GetBracketId() != bracket0->GetBracketId())
            {
                handler->SendSysMessage(("botscenario: bg member '" + m.player->GetName() +
                    "' is not in the anchor's PvP bracket for this map").c_str());
                return false;
            }
        }

        // --- prep every member: leave group, drop any BG queue, resurrect, dismount, clean combat -
        std::vector<ObjectGuid> memberGuids;
        std::vector<std::string> memberNames;
        std::vector<uint8> memberTeam;
        memberGuids.reserve(members.size());
        memberNames.reserve(members.size());
        memberTeam.reserve(members.size());
        for (size_t i = 0; i < members.size(); ++i)
        {
            Player* m = members[i].player;

            m->RemoveFromGroup();

            for (uint8 slot = 0; slot < PLAYER_MAX_BATTLEGROUND_QUEUES; ++slot)
            {
                BattlegroundQueueTypeId const qId = m->GetBattlegroundQueueTypeId(slot);
                if (qId == BATTLEGROUND_QUEUE_NONE)
                    continue;
                m->RemoveBattlegroundQueueId(qId);
                sBattlegroundMgr->GetBattlegroundQueue(qId).RemovePlayer(m->GetGUID(), true);
            }

            if (!m->IsAlive())
            {
                m->ResurrectPlayer(1.0f);
                m->SpawnCorpseBones();
            }
            m->CombatStopWithPets(true);
            DismountForGround(m, true);
            // 1:1 runs start from the same state on both cores (the same rule the generic dispatcher
            // applies to the anchor after parse() returns, HandleScenarioCommand below): full health
            // and mana for every member, not just the anchor, so nobody starts the match already
            // spending its first seconds eating/drinking.
            m->SetFullHealth();
            if (m->GetPowerType() == POWER_MANA)
                m->SetPower(POWER_MANA, m->GetMaxPower(POWER_MANA));

            uint8 const team = uint8(m->GetTeamId());
            memberGuids.push_back(m->GetGUID());
            memberNames.push_back(m->GetName());
            memberTeam.push_back(team);

            TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s member i=%u name=%s team=%s class=%u level=%u",
                bot->GetName().c_str(), uint32(i), m->GetName().c_str(), BgTeamLetter(team), m->GetClass(),
                m->GetLevel());
        }

        // --- queue each faction as one premade group (see QueueFactionGroupForBg for why) ----------
        std::vector<Player*> allianceMembers, hordeMembers;
        for (size_t i = 0; i < members.size(); ++i)
            (memberTeam[i] == TEAM_ALLIANCE ? allianceMembers : hordeMembers).push_back(members[i].player);
        if (allianceMembers.empty() || hordeMembers.empty())
        {
            handler->SendSysMessage("botscenario: bg roster must have at least one member per faction");
            return false;
        }

        // Per-member (not per-faction) since v23-bg-ab: a roster bigger than the BG's DBC maxGroupSize
        // (Arathi Basin's 15 on AC) splits into a premade seed plus a solo overflow - see
        // QueueFactionGroupForBg - and the two can succeed/fail independently.
        // #570 queue=natural: nothing is queued here. The roster is registered below, so BgJoinBlocked admits
        // only roster bots to the natural BGJoinAction (as leaders and as party members), and BgUpdate keeps
        // the roster's rated bracket open the way a waiting real player would.
        std::vector<uint8> const allianceOk = naturalQueue ? std::vector<uint8>(allianceMembers.size(), 1)
            : QueueFactionGroupForBg(allianceMembers, bgTypeId, members.size(), bracket0);
        std::vector<uint8> const hordeOk = naturalQueue ? std::vector<uint8>(hordeMembers.size(), 1)
            : QueueFactionGroupForBg(hordeMembers, bgTypeId, members.size(), bracket0);
        std::vector<uint8> queuedOk(members.size(), 0);
        if (naturalQueue)
            TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s natural queue: roster of %u registered, bots queue themselves",
                bot->GetName().c_str(), uint32(members.size()));
        {
            size_t ai = 0, hi = 0;
            for (size_t i = 0; i < members.size(); ++i)
            {
                bool const ok = memberTeam[i] == TEAM_ALLIANCE ? allianceOk[ai++] : hordeOk[hi++];
                queuedOk[i] = ok ? 1 : 0;
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s queued name=%s team=%s ok=%d",
                    bot->GetName().c_str(), members[i].player->GetName().c_str(), BgTeamLetter(memberTeam[i]), ok ? 1 : 0);
            }
        }
        // Kept as a defensive no-op safety net (CheckPremadeMatch above should already invite the whole
        // premade seed in one pass, and any solo overflow rides this same re-issue from BgUpdate too):
        // BgUpdate's forming phase re-issues this on a timer while anyone is missing.
        uint8 const arenaType = ArenaQueueType(bgTypeId, members.size());
        if (!naturalQueue)  // #570: a natural run's queue updates come from the bots' own arena joins
            sBattlegroundMgr->ScheduleQueueUpdate(0, arenaType, BattlegroundMgr::BGQueueTypeId(bgTypeId, arenaType), bgTypeId,
                bracket0->GetBracketId());

        state.kind = ScenarioKind::Bg;
        state.startT = getMSTime();
        state.lastTraceT = 0;
        state.destMapId = mapId;
        state.wanderMs = secs * IN_MILLISECONDS;
        state.lock = false;   // BG strategies own the bot, not the "new rpg" status machine
        state.noFight = false;
        state.groundOnly = false;
        state.allowMask = 0;
        state.pickNearest = false;
        state.pickCount = 0;
        state.attackGuid = 0;
        state.attackEntry = 0;

        state.bg = BgTrack{};
        state.bg.bgTypeId = uint32(bgTypeId);
        state.bg.bracketId = uint8(bracket0->GetBracketId());
        state.bg.members = memberGuids;
        state.bg.memberNames = memberNames;
        state.bg.memberTeam = memberTeam;
        state.bg.stratA = stratA;
        state.bg.stratH = stratH;
        state.bg.naturalQueue = naturalQueue;
        state.bg.rolesIndex = rolesIndex;
        state.bg.fastMs = fastMs;
        state.bg.sampleMs = sampleMs;
        state.bg.queuedOk = queuedOk;
        state.bg.invited.assign(members.size(), 0);
        state.bg.invitedT.assign(members.size(), 0);
        state.bg.enterT.assign(members.size(), 0);
        state.bg.wasAlive.assign(members.size(), 1);
        state.bg.deathAtT.assign(members.size(), 0);
        state.bg.lastX.assign(members.size(), 0.0f);
        state.bg.lastY.assign(members.size(), 0.0f);
        state.bg.staticMs.assign(members.size(), 0);
        state.bg.gateclipLogged.assign(members.size(), 0);
        state.bg.arena = naturalQueue || arenaStage;
        state.bg.onlyTeam = onlyTeam;
        state.bg.aqTimeout = aqTimeout;
        state.bg.leftStart.assign(members.size(), 0);
        state.bg.vanishedLogged.assign(members.size(), 0);

        TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s queued n=%u t0=0 bgtype=%u map=%u",
            bot->GetName().c_str(), uint32(members.size()), uint32(bgTypeId), mapId);
        return true;
    }

    void LogBgEndLine(char const* anchorName, BgTrack const& bg, uint32 t, uint32 scoreA, uint32 scoreH,
        char const* winner, char const* reason)
    {
        TC_LOG_DEBUG("playerbots",
            "[SCENARIO] %s bgdone t=%u winner=%s dur=%u caps=%u-%u pickups=%u/%u returns=%u/%u fckills=%u/%u "
            "deaths=%u/%u firstPickup=%u firstCap=%u entered=%u/%u enterMs=%u mountedAtGate=%u staticSec=%u "
            "gateclip=%u reason=%s",
            anchorName, t, winner, t, scoreA, scoreH, bg.pickups[0], bg.pickups[1], bg.returns[0], bg.returns[1],
            bg.fckills[0], bg.fckills[1], bg.deaths[0], bg.deaths[1], bg.firstPickupT, bg.firstCapT,
            bg.enteredCount, uint32(bg.members.size()), bg.formedT, bg.mountedAtGate, bg.staticSecTotal,
            bg.gateclipCount, reason);
    }

    void LogBgEnd(char const* anchorName, BgTrack const& bg, Battleground* bgObj, uint32 t, char const* reason)
    {
        uint8 const winnerRaw = bgObj->GetWinner();
        LogBgEndLine(anchorName, bg, t, bgObj->GetTeamScore(TEAM_ALLIANCE), bgObj->GetTeamScore(TEAM_HORDE),
            winnerRaw == WINNER_ALLIANCE ? "A" : winnerRaw == WINNER_HORDE ? "H" : "none", reason);
    }

    // AV's own end line: AV has no score/flag state
    // worth reusing from LogBgEndLine (BattlegroundAV keeps reinforcements in its own private
    // m_Team_Scores, never the base Battleground::m_TeamScores LogBgEndLine reads), so it gets its own
    // format entirely.
    void LogAvEnd(char const* anchorName, BgTrack const& bg, uint32 t, char const* winner, char const* reason)
    {
        AvTrack const& av = bg.av;
        TC_LOG_DEBUG("playerbots",
            "[SCENARIO] %s bgdone t=%u winner=%s reason=%s dur=%u reinf=%d-%d towersDown=%u/%u gyCaps=%u/%u "
            "assaults=%u/%u defends=%u/%u captainKillT=%u/%u bossKillT=%u firstAssault=%u firstTowerDown=%u "
            "mineCaps=%u/%u deaths=%u/%u npcDeaths=%u/%u entered=%u/%u enterMs=%u mountedAtGate=%u staticSec=%u "
            "gateclip=%u belowMin=%u",
            anchorName, t, winner, reason, t, av.reinf[0], av.reinf[1], av.towersDown[0], av.towersDown[1],
            av.gyCaps[0], av.gyCaps[1], av.assaults[0], av.assaults[1], av.defends[0], av.defends[1],
            av.captainKillT[0], av.captainKillT[1], av.bossKillT, av.firstAssaultT, av.firstTowerDownT,
            av.mineCaps[0], av.mineCaps[1], bg.deaths[0], bg.deaths[1], av.npcDeaths[0], av.npcDeaths[1],
            bg.enteredCount, uint32(bg.members.size()), bg.formedT, bg.mountedAtGate, bg.staticSecTotal,
            bg.gateclipCount, uint32(av.belowMin ? 1 : 0));
    }

    // Alterac Valley's own BgUpdate branch. Called from BgUpdate
    // once forming/gates/outsider/mountedAtGate/timeout are already handled identically to every other
    // BG type; everything from here on is AV-specific, since AV has no flags, no score worth reading
    // off the base Battleground class, and its own boss/reinforcements end condition instead of
    // STATUS_WAIT_LEAVE-by-score.
    bool AvBgUpdate(Player* bot, ScenarioState& s, Battleground* bgObj, std::vector<Player*> const& live,
        uint32 now, uint32 t)
    {
        BgTrack& bg = s.bg;
        AvTrack& av = bg.av;
        char const* anchorName = bot->GetName().c_str();
        BattlegroundStatus const status = bgObj->GetStatus();

        // Only evaluate the boss/reinforcements end condition once the match is actually running - a
        // boss "not found yet" during STATUS_WAIT_JOIN (before SetupBattleground's creatures have all
        // spawned) must never read as "boss already dead".
        bool const inMatch = status == STATUS_IN_PROGRESS || status == STATUS_WAIT_LEAVE;
        bool const bossAAlive = !inMatch || AvBossAlive(bgObj, 0);
        bool const bossHAlive = !inMatch || AvBossAlive(bgObj, 1);
        bool const reinfKnown = inMatch && av.reinf[0] >= 0 && av.reinf[1] >= 0;
        bool const byBoss = inMatch && (!bossAAlive || !bossHAlive);
        bool const byReinf = inMatch && !byBoss && reinfKnown && (av.reinf[0] <= 0 || av.reinf[1] <= 0);

        // --- natural end: boss kill, reinforcements at 0, or the core's own STATUS_WAIT_LEAVE --------
        if (status == STATUS_WAIT_LEAVE || byBoss || byReinf)
        {
            // #171: re-read reinforcements before deciding, so the winner/reason reflect the
            // freshest count instead of whatever the last periodic (15s) sample cached.
            int32 a = av.reinf[0], h = av.reinf[1];
            if (AvReinforcements(bgObj, a, h))
            {
                av.reinf[0] = a;
                av.reinf[1] = h;
            }

            uint8 const winnerRaw = bgObj->GetWinner();
            char const* winner = winnerRaw == WINNER_ALLIANCE ? "A" : winnerRaw == WINNER_HORDE ? "H" : "none";
            char const* reason = (!bossAAlive || !bossHAlive) ? "boss" :
                                  (av.reinf[0] <= 0 || av.reinf[1] <= 0) ? "reinf" : "core";
            if (byBoss && !av.bossKillT)
                av.bossKillT = t;
            LogAvEnd(anchorName, bg, t, winner, reason);
            return true;
        }

        // --- periodic sample: strat/role pins, fast prep, edges, [BGSTAGE] av sample line ------------
        if (bg.lastSampleT && getMSTimeDiff(bg.lastSampleT, now) < bg.sampleMs)
            return false;
        bg.lastSampleT = now;

        // strat=<a>,<h>: pin the team strategy every sample while WAIT_JOIN or IN_PROGRESS.
        if (bg.stratA != BG_STRAT_UNPINNED && (status == STATUS_WAIT_JOIN || status == STATUS_IN_PROGRESS))
        {
            BGStrategyData& d = bgStrategies[bgObj->GetInstanceID()];
            d.allianceStrategy = bg.stratA;
            d.hordeStrategy = bg.stratH;
        }

        // fast=<ms>: once, on the first WAIT_JOIN sample.
        if (bg.fastMs && !bg.fastApplied && status == STATUS_WAIT_JOIN)
        {
            bgObj->SetStartDelayTime(int32(bg.fastMs));
            bg.fastApplied = true;
        }

        // roles=index (default): pin "bg role" = i % 10 every sample, same as every other BG type.
        if (bg.rolesIndex)
        {
            for (size_t i = 0; i < live.size(); ++i)
                if (live[i])
                    if (PlayerbotAI* ai = GET_PLAYERBOT_AI(live[i]))
                        ai->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(uint32(i % 10));
        }

        if (status != STATUS_IN_PROGRESS)
            return false;  // nothing else to sample before the gates open

        // Reinforcements: 15 s cadence on both trees (section 3.4 point 5 - AC's FillInitialWorldStates
        // has the SendMineWorldStates side effect the doc calls out; MoP's getter has none, but the
        // same cadence keeps the code, and the sample line, identical on both trees).
        if (!av.lastReinfT || getMSTimeDiff(av.lastReinfT, now) >= 15000)
        {
            av.lastReinfT = now;
            int32 a = av.reinf[0], h = av.reinf[1];
            if (AvReinforcements(bgObj, a, h))
            {
                av.reinf[0] = a;
                av.reinf[1] = h;
            }
        }

        // --- node edges: assault / towerdown / defend / gy capture -----------------------------------
        // Node home teams are fixed by BG_AV_Nodes' layout on both cores: 0-2 and 7-10 start
        // Alliance-owned, 4-6 and 11-14 start Horde-owned, 3 (Snowfall) is neutral with no fixed home
        // team. "assault"/"defend"/"towerdown" attribute to the fixed attacker/home side; a Snowfall
        // assault has no fixed attacker and logs team=? (a documented simplification - see the port's
        // final report).
        std::string towerStr, gyStr;
        for (uint8 node = 0; node < 15; ++node)
        {
            AvNodeSnapshot const snap = AvNode(bgObj, node);
            bool const isTower = node >= 7;
            bool const isSnowfall = node == 3;
            uint8 const homeLetter = isSnowfall ? 0 : ((node <= 2 || (node >= 7 && node <= 10)) ? 1 : 2);

            if (av.nodeSeeded)
            {
                uint8 const prevState = av.nodeState[node];
                uint8 const prevOwner = av.nodeOwnerLetter[node];

                if (snap.state == 1 && prevState != 1)
                {
                    uint8 const attacker = snap.ownerLetter; // #168: Owner = the assaulting team at the assault
                    if (attacker)
                        ++av.assaults[attacker - 1];
                    if (!av.firstAssaultT)
                        av.firstAssaultT = t;
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s assault node=%u team=%s t=%u",
                        anchorName, node, attacker == 1 ? "A" : attacker == 2 ? "H" : "?", t);
                }
                else if (isTower && snap.state == 2 && prevState != 2)
                {
                    if (homeLetter)
                    {
                        ++av.towersDown[homeLetter - 1];
                        if (!av.firstTowerDownT)
                            av.firstTowerDownT = t;
                    }
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s towerdown node=%u team=%s t=%u",
                        anchorName, node, homeLetter == 1 ? "A" : "H", t);
                }
                // #168: an assaulted node always goes 1 -> 3 (controlled). Same owner = the assaulter took
                // it (graveyard capture); owner flipped back = the defenders retook it (defend).
                else if (!isTower && snap.state == 3 && prevState == 1 && snap.ownerLetter == prevOwner &&
                         snap.ownerLetter != 0)
                {
                    ++av.gyCaps[snap.ownerLetter - 1];
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s gycap node=%u team=%s t=%u",
                        anchorName, node, snap.ownerLetter == 1 ? "A" : "H", t);
                }
                else if (snap.state == 3 && prevState == 1 && snap.ownerLetter != prevOwner && snap.ownerLetter != 0)
                {
                    ++av.defends[snap.ownerLetter - 1];
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s defend node=%u team=%s t=%u",
                        anchorName, node, snap.ownerLetter == 1 ? "A" : "H", t);
                }
            }

            av.nodeState[node] = snap.state;
            av.nodeOwnerLetter[node] = snap.ownerLetter;

            char const letter = isTower ? (snap.state == 2 ? 'D' : snap.state == 1 ? 'A' : 'S')
                                         : (snap.ownerLetter == 1 ? 'A' : snap.ownerLetter == 2 ? 'H' : 'N');
            (isTower ? towerStr : gyStr) += letter;
            if (!isTower && node == 3)
                gyStr += ' ';
        }
        av.nodeSeeded = true;

        // --- mine edges --------------------------------------------------------------------------
        for (uint8 mineIdx = 0; mineIdx < 2; ++mineIdx)
        {
            uint8 const owner = AvMineOwnerLetter(bgObj, mineIdx);
            if (owner != av.mineOwnerLetterPrev[mineIdx] && owner != 0)
            {
                ++av.mineCaps[owner - 1];
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s minecap mine=%s team=%s t=%u",
                    anchorName, mineIdx == 0 ? "N" : "S", owner == 1 ? "A" : "H", t);
            }
            av.mineOwnerLetterPrev[mineIdx] = owner;
        }

        // --- captain / boss edges ------------------------------------------------------------------
        bool const captAlive[2] = { AvCaptainAlive(bgObj, 0), AvCaptainAlive(bgObj, 1) };
        for (uint8 side = 0; side < 2; ++side)
        {
            if (av.captainAlivePrev[side] && !captAlive[side])
            {
                av.captainKillT[side] = t;
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s captainkill team=%s t=%u",
                    anchorName, side == 0 ? "A" : "H", t);
            }
            av.captainAlivePrev[side] = captAlive[side];
        }

        bool const bossAliveNow[2] = { bossAAlive, bossHAlive };
        for (uint8 side = 0; side < 2; ++side)
        {
            if (av.bossAlivePrev[side] && !bossAliveNow[side])
            {
                if (!av.bossKillT)
                    av.bossKillT = t;
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s bosskill team=%s t=%u",
                    anchorName, side == 0 ? "A" : "H", t);
            }
            av.bossAlivePrev[side] = bossAliveNow[side];
        }

        // --- per-member death/res/static/gateclip, alive counts and zone bands ---------------------
        uint32 aliveA = 0, aliveH = 0;
        uint32 bandAllianceHalf = 0, bandMiddle = 0, bandHordeHalf = 0;
        uint32 presentA = 0, presentH = 0;
        for (size_t i = 0; i < live.size(); ++i)
        {
            Player* m = live[i];
            if (!m)
                continue;

            (bg.memberTeam[i] == TEAM_ALLIANCE ? presentA : presentH)++;

            bool const alive = m->IsAlive();
            if (alive)
            {
                (bg.memberTeam[i] == TEAM_ALLIANCE ? aliveA : aliveH)++;
                float const x = m->GetPositionX();
                if (x > 0.0f)
                    ++bandAllianceHalf;
                else if (x >= -300.0f)
                    ++bandMiddle;
                else
                    ++bandHordeHalf;
            }

            if (av.npcAttacked.size() < live.size())
                av.npcAttacked.resize(live.size(), 0);
            if (alive)
            {
                bool npcAttacker = false;
                for (Unit* attacker : m->getAttackers())
                    if (attacker && attacker->GetTypeId() == TYPEID_UNIT)
                    {
                        npcAttacker = true;
                        break;
                    }
                av.npcAttacked[i] = npcAttacker ? 1 : 0;
            }

            if (bg.wasAlive[i] && !alive)
            {
                bg.deathAtT[i] = t;
                ++bg.deaths[bg.memberTeam[i]];

                // npcDeath (section 3.5): book the death as an NPC kill when a hostile creature was
                // among the member's attackers at the death edge - the AV NPC-damage difference
                // between cores (§3.5) should be read alongside this, not from bare death counts.
                // #168: death runs CombatStop -> RemoveAllAttackers, so read the last sample's snapshot
                bool const npcKill = av.npcAttacked[i] != 0;
                if (npcKill)
                    ++av.npcDeaths[bg.memberTeam[i]];

                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s death name=%s team=%s t=%u npc=%d",
                    anchorName, m->GetName().c_str(), BgTeamLetter(bg.memberTeam[i]), t, npcKill ? 1 : 0);
            }
            else if (!bg.wasAlive[i] && alive)
            {
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s res name=%s deadMs=%u",
                    anchorName, m->GetName().c_str(), t - bg.deathAtT[i]);
            }
            bg.wasAlive[i] = alive ? 1 : 0;

            // static, with the AV defender exemption (section 3.4): a bot within 20 yd of its own
            // "bg objective" position is legitimately standing at a node it holds or is assaulting,
            // not idle.
            bool exempt = false;
            if (PlayerbotAI* ai = GET_PLAYERBOT_AI(m))
            {
                PositionInfo objPos = ai->GetAiObjectContext()->GetValue<PositionMap&>("position")->Get()["bg objective"];
                if (objPos.isSet() && m->GetDistance(objPos.x, objPos.y, objPos.z) < 20.0f)
                    exempt = true;
            }

            bool const casting = m->GetCurrentSpell(CURRENT_GENERIC_SPELL) || m->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
            float const dx = m->GetPositionX() - bg.lastX[i], dy = m->GetPositionY() - bg.lastY[i];
            bool const moved = (dx * dx + dy * dy) >= (BG_STATIC_MOVE_YD * BG_STATIC_MOVE_YD);
            bg.lastX[i] = m->GetPositionX();
            bg.lastY[i] = m->GetPositionY();

            if (alive && !moved && !casting && !exempt)
            {
                bg.staticMs[i] += bg.sampleMs;
                bg.staticSecTotal += bg.sampleMs / 1000;
                if (bg.staticMs[i] >= BG_STATIC_WINDOW_MS && (bg.staticMs[i] % BG_STATIC_WINDOW_MS) < bg.sampleMs)
                {
                    ++bg.staticEvents;
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s static name=%s sec=%u",
                        anchorName, m->GetName().c_str(), bg.staticMs[i] / 1000);
                }
            }
            else
                bg.staticMs[i] = 0;

            // gateclip (#68/#97): same check as every other BG type.
            if (status == STATUS_WAIT_JOIN && !bg.gateclipLogged[i])
            {
                float sx = 0.0f, sy = 0.0f;
                BgStartPos(bgObj, bg.memberTeam[i], sx, sy);
                float const gx = m->GetPositionX() - sx, gy = m->GetPositionY() - sy;
                float const radius = bgObj->GetStartMaxDist() + BG_GATECLIP_MARGIN_YD;
                if ((gx * gx + gy * gy) > (radius * radius))
                {
                    bg.gateclipLogged[i] = 1;
                    ++bg.gateclipCount;
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s gateclip name=%s", anchorName, m->GetName().c_str());
                }
            }
        }

        if (!av.belowMin && (presentA < bgObj->GetMinPlayersPerTeam() || presentH < bgObj->GetMinPlayersPerTeam()))
        {
            av.belowMin = true;
            TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s belowmin t=%u a=%u h=%u", anchorName, t, presentA, presentH);
        }

        TC_LOG_DEBUG("playerbots",
            "[BGSTAGE] %s sample t=%u reinf=%d-%d gy=%s twr=%s mines=%c%c capt=A%uH%u bossA=%u/%u%% bossH=%u/%u%% "
            "aliveA=%u aliveH=%u bandA=%u bandM=%u bandH=%u pcA=%u pcH=%u",
            anchorName, t, av.reinf[0], av.reinf[1], gyStr.c_str(), towerStr.c_str(),
            av.mineOwnerLetterPrev[0] == 1 ? 'A' : av.mineOwnerLetterPrev[0] == 2 ? 'H' : 'N',
            av.mineOwnerLetterPrev[1] == 1 ? 'A' : av.mineOwnerLetterPrev[1] == 2 ? 'H' : 'N',
            captAlive[0] ? 1u : 0u, captAlive[1] ? 1u : 0u, bossAAlive ? 1u : 0u, uint32(AvBossHealthPct(bgObj, 0)),
            bossHAlive ? 1u : 0u, uint32(AvBossHealthPct(bgObj, 1)), aliveA, aliveH, bandAllianceHalf, bandMiddle,
            bandHordeHalf, bgObj->GetPlayersCountByTeam(ALLIANCE), bgObj->GetPlayersCountByTeam(HORDE));

        return false;
    }

    bool BgUpdate(Player* bot, PlayerbotAI* /*botAI*/, ScenarioState& s, uint32 now, uint32 t)
    {
        BgTrack& bg = s.bg;
        char const* anchorName = bot->GetName().c_str();

        // Resolve every member's live Player* off the anchor's own map/world (section 3.4) - the
        // anchor (index 0) is `bot` itself, already resolved by the caller.
        std::vector<Player*> live(bg.members.size(), nullptr);
        for (size_t i = 0; i < bg.members.size(); ++i)
            live[i] = ObjectAccessor::GetPlayer(*bot, bg.members[i]);

        BattlegroundQueueTypeId const queueTypeId = BattlegroundMgr::BGQueueTypeId(
            BattlegroundTypeId(bg.bgTypeId), ArenaQueueType(BattlegroundTypeId(bg.bgTypeId), bg.members.size()));

        // --- forming: not every member has entered the instance yet (section 3.2 step 4) ---------
        if (!bg.formed)
        {
            // #570 queue=natural: hold the roster's rated bracket open the way a waiting real player does
            // (AC RandomPlayerbotMgr.cpp:1012-1017). The census rebuilds its entries every pass, so re-assert it
            // every update until the match has formed.
            // #631: hold only while a roster member is neither invited nor inside, the same test the census applies
            // to a real player (AC RandomPlayerbotMgr.cpp:1017) - once everyone is invited/inside, nothing holds it.
            if (bg.naturalQueue)
            {
                bool waiting = false;
                for (Player* m : live)
                    if (m && !m->InBattleground() && !m->IsInvitedForBattlegroundQueueType(queueTypeId))
                        waiting = true;
                if (waiting)
                    PlayerbotArena::Census(uint32(queueTypeId), bg.bracketId).activeRatedArenaQueue = 1;
            }

            for (size_t i = 0; i < live.size(); ++i)
            {
                if (bg.enterT[i])
                    continue;
                Player* m = live[i];
                if (!m)
                    continue;

                // Invited (per-member diagnostic, independent of "entered" below): flips once
                // BattlegroundQueue::FillPlayersToBG/InviteGroupToBG picks this member's GroupQueueInfo.
                // A member stuck at invited=0 past the timeout never got a slot in any pass; invited=1
                // but never "entered" means the invite landed but this member's own AI never accepted
                // it (busy/relevance-starved - see the timeout comment above).
                if (!bg.invited[i] && m->IsInvitedForBattlegroundQueueType(queueTypeId))
                {
                    bg.invited[i] = 1;
                    bg.invitedT[i] = t;
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s invited name=%s team=%s t=%u",
                        anchorName, m->GetName().c_str(), BgTeamLetter(bg.memberTeam[i]), t);
                }

                if (m->InBattleground() && (bg.arena ? m->InArena() : uint32(m->GetBattlegroundTypeId()) == bg.bgTypeId))
                {
                    bg.enterT[i] = t ? t : 1; // 1 disambiguates "entered at t=0" from "not yet entered"
                    ++bg.enteredCount;
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s entered name=%s team=%s acceptMs=%u",
                        anchorName, m->GetName().c_str(), BgTeamLetter(bg.memberTeam[i]), t);
                }
            }

            if (bg.enteredCount == live.size())
            {
                bg.formed = true;
                bg.formedT = t;
                uint32 aCount = 0, hCount = 0;
                for (uint8 team : bg.memberTeam)
                    (team == TEAM_ALLIANCE ? aCount : hCount)++;
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s formed a=%u h=%u enterMs=%u", anchorName, aCount, hCount, t);
            }
            else if (t >= (bg.naturalQueue ? BG_NATURAL_ENTER_TIMEOUT_MS : BG_ENTER_TIMEOUT_MS))
            {
                // Per-member failure reason (what the coordinator asked to see directly in the log,
                // rather than only inferred from entered=n/N): queued but never invited (still stuck
                // behind the bracket-minimum cap), or invited but this member's own AI never accepted it.
                for (size_t i = 0; i < live.size(); ++i)
                {
                    if (bg.enterT[i])
                        continue;
                    char const* reason = !bg.queuedOk[i] ? "joinfailed" : !bg.invited[i] ? "noinvite" : "noaccept";
                    // Name from the cached memberNames (captured at queue time), not a live lookup: a
                    // member who went offline before the timeout (natural random-bot login churn) would
                    // otherwise resolve to null here and print a raw GUID instead of a name.
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s missing name=%s team=%s queued=%d invited=%d reason=%s t=%u",
                        anchorName, bg.memberNames[i].c_str(),
                        BgTeamLetter(bg.memberTeam[i]), int(bg.queuedOk[i]), int(bg.invited[i]), reason, t);
                }
                if (!bg.gatesSeen)
                {
                    TC_LOG_DEBUG("playerbots", "[SCENARIO] %s bgnoform entered=%u/%u",
                        anchorName, bg.enteredCount, uint32(live.size()));
                    return true;
                }
                // The match already started (gates open): keep measuring with whoever entered instead
                // of aborting only one side; EnteredN in the report records the shortfall.
                bg.formed = true;
                bg.formedT = t;
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s formed partial entered=%u/%u enterMs=%u", anchorName, bg.enteredCount, uint32(live.size()), t);
            }
            else if (!bg.naturalQueue &&  // #570: no harness queue entries to kick in a natural run
                (!bg.lastQueueKickT || getMSTimeDiff(bg.lastQueueKickT, now) >= BG_QUEUE_KICK_MS))
            {
                // Defensive safety net, not the primary fix (v18-bg-wsg-1): ParseBg now queues each
                // faction as one premade group specifically so CheckPremadeMatch invites everyone in a
                // single pass and this normally never has anything left to do. Kept in case a premade
                // ever falls back to the plain queue (e.g. a below-minimum roster) - forces another
                // BattlegroundQueueUpdate pass so FillPlayersToBG can top up. Cheap and idempotent
                // (ScheduleQueueUpdate dedupes by queue/bgType/bracket), safe to re-issue on a timer.
                bg.lastQueueKickT = now;
                // Wave-29: the queue's arena type, not 0 - with 0 an arena pass ran with Min/MaxPlayersPerTeam = 0
                // (BattlegroundQueueUpdate: arenas take both from arenaType) and could start an empty arena.
                sBattlegroundMgr->ScheduleQueueUpdate(0, BattlegroundMgr::BGArenaType(queueTypeId), queueTypeId,
                    BattlegroundTypeId(bg.bgTypeId), BattlegroundBracketId(bg.bracketId));
            }
        }

        Player* anchorPlayer = live.empty() ? nullptr : live[0]; // members[0] is always the anchor
        bool const anchorIn = anchorPlayer && anchorPlayer->InBattleground();

        // Resolve the shared Battleground* from any member already inside, not just the anchor - all
        // members land on the same instance, and this lets gates/sample/outsider tracking (and the
        // queue-kick loop's timeout branch) keep working even on a run where the anchor itself happens
        // to be one of the slower acceptors rather than one of the first batch.
        Player* observer = anchorIn ? anchorPlayer : nullptr;
        if (!observer)
            for (Player* m : live)
                if (m && m->InBattleground()) { observer = m; break; }

        if (!anchorIn)
        {
            // Only when no member is inside any more: bots leave the moment the match ends (AC at relevance
            // 100), so the anchor can be gone while others still see STATUS_WAIT_LEAVE and the final score.
            // Wave-29 arena legs: once an arena's gates have opened and no roster member is left on the anchor's map,
            // report the end now instead of after the enter timeout (bots leave an arena the moment it ends).
            if ((t >= (bg.naturalQueue ? BG_NATURAL_ENTER_TIMEOUT_MS : BG_ENTER_TIMEOUT_MS) || (bg.arena && bg.gatesSeen)) &&
                !observer)
            {
                // All bots get TIME_TO_AUTOREMOVE in the same update when the match ends by score, so
                // STATUS_WAIT_LEAVE can fall between two samples: report the end from the last sampled score
                if (bg.gatesSeen)
                {
                    // Members are resolved on the anchor's map only, so an anchor moved off the BG map makes
                    // everyone look gone: report "left" only when the instance is really over, else mark the
                    // run invalid ("anchorout")
                    Battleground* still = bg.instanceId
                        ? sBattlegroundMgr->GetBattleground(bg.instanceId,
                            // #570: search every type for a natural arena, as AC must (AC files an all-arenas
                            // instance under its real arena type); identical on both cores.
                            bg.arena ? BATTLEGROUND_TYPE_NONE : BattlegroundTypeId(bg.bgTypeId)) : nullptr;
                    bool const over = !still || still->GetStatus() != STATUS_IN_PROGRESS;
                    char const* winner = bg.score[0] > bg.score[1] ? "A" : bg.score[1] > bg.score[0] ? "H" : "none";
                    if (bg.arena)
                    {
                        // Wave-29: an arena has no score - the core's winner while the instance still exists, else the
                        // team that still had a live member at the last sample.
                        winner = still && still->GetStatus() == STATUS_WAIT_LEAVE ? ArenaWinner(still)
                            : !bg.arenaCounted ? "none"
                            : !bg.arenaAlive[0] && bg.arenaAlive[1] ? "H" : !bg.arenaAlive[1] && bg.arenaAlive[0] ? "A" : "none";
                        LogArenaEnd(anchorName, bg, now, over ? winner : "anchorout");
                    }
                    LogBgEndLine(anchorName, bg, t, bg.score[0], bg.score[1], over ? winner : "none",
                        over ? "left" : "anchorout");
                    return true;
                }
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s bgnoanchor t=%u", anchorName, t);
                return true;
            }
            if (!observer)
                return false; // nobody has landed on the BG map yet at all
        }

        Battleground* bgObj = observer ? observer->GetBattleground() : nullptr;
        if (!bgObj)
            return false;
        bg.instanceId = bgObj->GetInstanceID();

        // Wave-29 arena legs: which arena the draw produced (the leg checks it against its arenapin), once.
        if (bg.arena && !bg.arenaLogged)
        {
            bg.arenaLogged = true;
            TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s arena map=%u type=%u size=%u rated=%u instance=%u", anchorName,
                bgObj->GetMapId(), ArenaDrawnType(bgObj), uint32(bgObj->GetArenaType()), ArenaIsRated(bgObj) ? 1u : 0u,
                bgObj->GetInstanceID());
        }

        // AutoReleaseSpiritAction adds 1 to "death count" on every tick a BG ghost waits, not once per
        // death, and above 10 SpiritHealerAction picks a start-zone graveyard and teleports the ghost off
        // the BG map (AC). Pin it to 0 for members inside, as the die
        // stage pins it, so both cores stay on the BG resurrection path for the whole match.
        for (Player* m : live)
            if (m && m->InBattleground())
                if (PlayerbotAI* ai = GET_PLAYERBOT_AI(m))
                    ai->GetAiObjectContext()->GetValue<uint32>("death count")->Set(0);

        // Outsider guard (section 3.2 step 4: "outsiders cannot join ... that is not a valid run").
        // Re-issuing ScheduleQueueUpdate above to fix the min-fill stall reopens the window the doc
        // warned about: while free slots remain, any other random bot whose own shouldJoinBg() still
        // sees room can be invited into one of them by that same top-up pass. Checked only while
        // forming - once full there are no free slots left for this WSG-sized (20 = 2*max) roster.
        if (!bg.formed && !bg.outsiderSeen)
        {
            for (auto const& kv : bgObj->GetPlayers())
            {
                if (FindBgMember(bg, kv.first) >= 0)
                    continue;
                bg.outsiderSeen = true;
                Player* outsider = ObjectAccessor::GetPlayer(*bot, kv.first);
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s outsider name=%s t=%u",
                    anchorName, outsider ? outsider->GetName().c_str() : kv.first.ToString().c_str(), t);
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s bgoutsider t=%u", anchorName, t);
                return true;
            }
        }

        // --- gates: WAIT_JOIN -> IN_PROGRESS is t=0 for every [BGSTAGE] line below (section 3.4) --
        BattlegroundStatus const status = bgObj->GetStatus();
        if (!bg.gatesSeen)
        {
            if (status == STATUS_WAIT_JOIN)
                bg.gatesT = now + uint32(std::max(0, bgObj->GetStartDelayTime())); // live re-estimate
            else if (status == STATUS_IN_PROGRESS || status == STATUS_WAIT_LEAVE)
            {
                bg.gatesSeen = true;
                bg.gatesT = now;
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s gates t=0", anchorName);
            }
        }

        if (!bg.mountedLogged && status == STATUS_IN_PROGRESS)
        {
            uint32 mountedA = 0, mountedH = 0;
            for (size_t i = 0; i < live.size(); ++i)
                if (live[i] && live[i]->IsAlive() && live[i]->IsMounted())
                    (bg.memberTeam[i] == TEAM_ALLIANCE ? mountedA : mountedH)++;
            TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s mountedAtGate a=%u h=%u", anchorName, mountedA, mountedH);
            bg.mountedAtGate = mountedA + mountedH;
            bg.mountedLogged = true;
        }

        // --- harness safety-net timeout (section 3.6: "time out at seconds") ---------------------
        if (t >= s.wanderMs)
        {
            if (bg.bgTypeId == 1 && bg.gatesSeen)
            {
                AvTrack const& av = bg.av;
                char const* winner = av.reinf[0] > av.reinf[1] ? "A" : av.reinf[1] > av.reinf[0] ? "H" : "none";
                LogAvEnd(anchorName, bg, t, winner, "time");
                return true;
            }
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s timeout t=%u", anchorName, t);
            return true;
        }

        // --- Alterac Valley: everything past this point is BG-type-agnostic setup (forming, gates,
        // outsiders, mountedAtGate, the timeout above); AV has no flags/score of its own and its own
        // boss/reinforcements end condition, so it branches into its own function here instead of the
        // generic WAIT_LEAVE/flag-edges/sample logic below.
        if (bg.bgTypeId == uint32(BATTLEGROUND_AV))
            return AvBgUpdate(bot, s, bgObj, live, now, t);

        // --- natural end of match: STATUS_WAIT_LEAVE ---------------------------------------------
        if (status == STATUS_WAIT_LEAVE)
        {
            // WSG ends at 3 captures, AB/EotS at 1600 resources.
            uint32 const maxScore = bg.bgTypeId == uint32(BATTLEGROUND_WS) ? uint32(BG_WS_MAX_TEAM_SCORE) : 1600u;
            bool const byScore = uint32(bgObj->GetTeamScore(TEAM_ALLIANCE)) >= maxScore ||
                                  uint32(bgObj->GetTeamScore(TEAM_HORDE)) >= maxScore;
            if (bg.arena)
            {
                // Wave-29 arena legs: an arena ends by kills (or the core's time limit), never by score.
                // Wave 30: judged from the last IN_PROGRESS sample, not from now (see ArenaEndReason).
                char const* winner = ArenaWinner(bgObj);
                LogArenaEnd(anchorName, bg, now, winner);
                LogBgEnd(anchorName, bg, bgObj, t, ArenaEndReason(bg, winner));
                return true;
            }
            LogBgEnd(anchorName, bg, bgObj, t, byScore ? "score" : "time");
            return true;
        }

        // --- periodic sample: strat/role pins, fast prep, edges, static, [BGSTAGE] sample line ----
        if (bg.lastSampleT && getMSTimeDiff(bg.lastSampleT, now) < bg.sampleMs)
            return false;
        bg.lastSampleT = now;

        // strat=<a>,<h>: pin the team strategy every sample while WAIT_JOIN or IN_PROGRESS (section 3.3).
        if (bg.stratA != BG_STRAT_UNPINNED && (status == STATUS_WAIT_JOIN || status == STATUS_IN_PROGRESS))
        {
            BGStrategyData& d = bgStrategies[bgObj->GetInstanceID()];
            d.allianceStrategy = bg.stratA;
            d.hordeStrategy = bg.stratH;
        }

        // fast=<ms>: once, on the first WAIT_JOIN sample.
        if (bg.fastMs && !bg.fastApplied && status == STATUS_WAIT_JOIN)
        {
            bgObj->SetStartDelayTime(int32(bg.fastMs));
            bg.fastApplied = true;
        }

        // roles=index (default): pin "bg role" = i % 10 on every member's own AI context every sample,
        // overriding the urand(0,9) roll at accept and the 1-2% re-roll in resetObjective.
        if (bg.rolesIndex)
        {
            for (size_t i = 0; i < live.size(); ++i)
                if (live[i])
                    if (PlayerbotAI* ai = GET_PLAYERBOT_AI(live[i]))
                        ai->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(uint32(i % 10));
        }

        // --- flag / score edges (section 3.4; index 0 = Alliance's flag, 1 = Horde's) -------------
        uint32 const curScoreA = bgObj->GetTeamScore(TEAM_ALLIANCE);
        uint32 const curScoreH = bgObj->GetTeamScore(TEAM_HORDE);
        bool const scoreChangedThisSample = curScoreA != bg.score[0] || curScoreH != bg.score[1];
        // EotS has one flag and its GetFlagPickerGUID ignores the team: check index 0 only and book each
        // event on the carrier's team
        bool const singleFlag = bg.bgTypeId == uint32(BATTLEGROUND_EY);
        for (uint8 flagTeam = 0; flagTeam < (singleFlag ? 1 : 2); ++flagTeam)
        {
            ObjectGuid const carrier = bgObj->GetFlagPickerGUID(int32(flagTeam));
            if (carrier == bg.lastFc[flagTeam])
                continue;

            int const idx = FindBgMember(bg, bg.lastFc[flagTeam].IsEmpty() ? carrier : bg.lastFc[flagTeam]);
            std::string const who = idx >= 0 && live[idx] ? live[idx]->GetName() : "?";
            uint8 const evTeam = singleFlag && idx >= 0 && live[idx] ? uint8(live[idx]->GetTeamId()) : flagTeam;

            if (bg.lastFc[flagTeam].IsEmpty() && !carrier.IsEmpty())
            {
                ++bg.pickups[evTeam];
                if (!bg.firstPickupT)
                    bg.firstPickupT = t;
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s pickup team=%s name=%s t=%u",
                    anchorName, BgTeamLetter(evTeam), who.c_str(), t);
            }
            else if (!bg.lastFc[flagTeam].IsEmpty() && carrier.IsEmpty())
            {
                bool const carrierDead = idx >= 0 && live[idx] && !live[idx]->IsAlive();
                if (scoreChangedThisSample)
                {
                    if (!bg.firstCapT)
                        bg.firstCapT = t;
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s capture team=%s name=%s score=%u-%u t=%u",
                        anchorName, BgTeamLetter(evTeam), who.c_str(), curScoreA, curScoreH, t);
                }
                else if (carrierDead)
                {
                    ++bg.fckills[evTeam];
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s fckill team=%s name=%s t=%u",
                        anchorName, BgTeamLetter(evTeam), who.c_str(), t);
                }
                else
                {
                    ++bg.returns[evTeam];
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s return team=%s t=%u", anchorName, BgTeamLetter(evTeam), t);
                }
            }
            else if (!bg.lastFc[flagTeam].IsEmpty() && !carrier.IsEmpty())
            {
                // Handed from one carrier to another inside one sample (drop and re-pickup between two
                // samples): book the old carrier's death as an fckill and the new carrier's pickup
                if (idx >= 0 && live[idx] && !live[idx]->IsAlive())
                {
                    ++bg.fckills[evTeam];
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s fckill team=%s name=%s t=%u",
                        anchorName, BgTeamLetter(evTeam), who.c_str(), t);
                }
                int const newIdx = FindBgMember(bg, carrier);
                std::string const newWho = newIdx >= 0 && live[newIdx] ? live[newIdx]->GetName() : "?";
                uint8 const newTeam = singleFlag && newIdx >= 0 && live[newIdx] ? uint8(live[newIdx]->GetTeamId()) : flagTeam;
                ++bg.pickups[newTeam];
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s pickup team=%s name=%s t=%u",
                    anchorName, BgTeamLetter(newTeam), newWho.c_str(), t);
            }
            bg.lastFc[flagTeam] = carrier;
        }
        bg.score[0] = curScoreA;
        bg.score[1] = curScoreH;

        // --- per-member death/res, static, gateclip; and the alive/mounted counts the sample line needs
        uint32 aliveA = 0, aliveH = 0, mountedNow = 0;
        int32 const tArena = int32(now - bg.gatesT);
        for (size_t i = 0; i < live.size(); ++i)
        {
            Player* m = live[i];
            // Wave-29 arena legs (#709): a member that entered the arena but is no longer on its map while the match
            // runs - the "opponent vanished, the match could not end" symptom. Logged once per member.
            if (bg.arena && status == STATUS_IN_PROGRESS && bg.enterT[i] && !bg.vanishedLogged[i] &&
                !ArenaMember(bg, live, i, bgObj->GetInstanceID()))
            {
                bg.vanishedLogged[i] = 1;
                Player* farPlr = ObjectAccessor::FindPlayer(bg.members[i]);
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s vanished name=%s team=%s map=%d t=%d", anchorName,
                    bg.memberNames[i].c_str(), BgTeamLetter(bg.memberTeam[i]), farPlr ? int32(farPlr->GetMapId()) : -1, tArena);
            }
            if (!m)
                continue;

            bool const alive = m->IsAlive();
            if (alive)
            {
                (bg.memberTeam[i] == TEAM_ALLIANCE ? aliveA : aliveH)++;
                if (m->IsMounted())
                    ++mountedNow;
            }

            if (bg.wasAlive[i] && !alive)
            {
                bg.deathAtT[i] = t;
                ++bg.deaths[bg.memberTeam[i]];
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s death name=%s team=%s t=%u",
                    anchorName, m->GetName().c_str(), BgTeamLetter(bg.memberTeam[i]), t);
            }
            else if (!bg.wasAlive[i] && alive)
            {
                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s res name=%s deadMs=%u",
                    anchorName, m->GetName().c_str(), t - bg.deathAtT[i]);
            }
            bg.wasAlive[i] = alive ? 1 : 0;

            // static (section 3.4): alive, in progress, moved < 2 yd since the previous sample, not
            // casting, not currently a flag carrier (a stand-off flag holder is "at its own stand").
            bool const isCarrier = bg.lastFc[0] == m->GetGUID() || bg.lastFc[1] == m->GetGUID();
            bool const casting = m->GetCurrentSpell(CURRENT_GENERIC_SPELL) || m->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
            float const dx = m->GetPositionX() - bg.lastX[i], dy = m->GetPositionY() - bg.lastY[i];
            bool const moved = (dx * dx + dy * dy) >= (BG_STATIC_MOVE_YD * BG_STATIC_MOVE_YD);
            bg.lastX[i] = m->GetPositionX();
            bg.lastY[i] = m->GetPositionY();

            if (alive && status == STATUS_IN_PROGRESS && !moved && !casting && !isCarrier)
            {
                bg.staticMs[i] += bg.sampleMs;
                bg.staticSecTotal += bg.sampleMs / 1000;
                if (bg.staticMs[i] >= BG_STATIC_WINDOW_MS && (bg.staticMs[i] % BG_STATIC_WINDOW_MS) < bg.sampleMs)
                {
                    ++bg.staticEvents;
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s static name=%s sec=%u",
                        anchorName, m->GetName().c_str(), bg.staticMs[i] / 1000);
                }
            }
            else
                bg.staticMs[i] = 0;

            // gateclip (#68/#97): outside the team's own start radius during WAIT_JOIN, logged once
            // per member per match.
            if (status == STATUS_WAIT_JOIN && !bg.gateclipLogged[i])
            {
                float sx = 0.0f, sy = 0.0f;
                BgStartPos(bgObj, bg.memberTeam[i], sx, sy);
                float const gx = m->GetPositionX() - sx, gy = m->GetPositionY() - sy;
                float const radius = bgObj->GetStartMaxDist() + BG_GATECLIP_MARGIN_YD;
                if ((gx * gx + gy * gy) > (radius * radius))
                {
                    bg.gateclipLogged[i] = 1;
                    ++bg.gateclipCount;
                    TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s gateclip name=%s", anchorName, m->GetName().c_str());
                }
            }

            // Wave-29 arena legs (#391/#709): left the start room, and the first sample each team had a member
            // attacking an enemy roster member ("engage").
            if (bg.arena && status == STATUS_IN_PROGRESS && alive && m->InArena())
            {
                if (!bg.leftStart[i])
                {
                    float sx = 0.0f, sy = 0.0f, sz = 0.0f;
                    ArenaStartPos(bgObj, bg.memberTeam[i], sx, sy, sz);
                    float const d = m->GetExactDist(sx, sy, sz);
                    if (d > BG_ARENA_LEFTSTART_YD)
                    {
                        bg.leftStart[i] = 1;
                        ++bg.leftStartCount;
                        TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s leftstart name=%s team=%s t=%d dist=%.1f", anchorName,
                            m->GetName().c_str(), BgTeamLetter(bg.memberTeam[i]), tArena, d);
                    }
                }
                uint8 const teamIdx = bg.memberTeam[i] == TEAM_ALLIANCE ? 0 : 1;
                if (bg.engageT[teamIdx] < 0)
                    if (Unit* victim = m->GetVictim())
                        if (Player* vp = victim->ToPlayer())
                        {
                            int const vi = FindBgMember(bg, vp->GetGUID());
                            if (vi >= 0 && bg.memberTeam[vi] != bg.memberTeam[i])
                            {
                                bg.engageT[teamIdx] = std::max(0, tArena);
                                TC_LOG_DEBUG("playerbots", "[BGSTAGE] %s engage team=%s name=%s victim=%s t=%d", anchorName,
                                    BgTeamLetter(bg.memberTeam[i]), m->GetName().c_str(), vp->GetName().c_str(), tArena);
                            }
                        }
            }
        }
        if (bg.arena && status == STATUS_IN_PROGRESS)
        {
            uint32 sizeN[2];
            ArenaCounts(bg, live, bgObj->GetInstanceID(), bg.arenaAlive, bg.arenaDead, sizeN);
            bg.arenaCounted = true;
        }

        std::string const fcA = bg.lastFc[0].IsEmpty() ? std::string("-") : std::to_string(bg.lastFc[0].GetCounter());
        std::string const fcH = bg.lastFc[1].IsEmpty() ? std::string("-") : std::to_string(bg.lastFc[1].GetCounter());
        int32 const tRebased = int32(now - bg.gatesT);
        TC_LOG_DEBUG("playerbots",
            "[BGSTAGE] %s sample t=%d score=%u-%u fcA=%s fcH=%s aliveA=%u aliveH=%u mounted=%u",
            anchorName, tRebased, curScoreA, curScoreH, fcA.c_str(), fcH.c_str(), aliveA, aliveH, mountedNow);

        return false;
    }

    void RearmBg(Player* /*bot*/, PlayerbotAI* /*botAI*/, ScenarioState& state)
    {
        // BG strategies (the "bg"/"warsong"/etc engine strategies) own the bot, not the "new rpg"
        // status machine, so there is nothing to re-arm on the RPG side. Do NOT restart state.startT
        // (the match keeps its own clock, as the transport ride does) and leave lock=false, exactly
        // as ParseBg set it.
        state.lock = false;
    }

    // --- registration --------------------------------------------------------------------------
    struct HandlerRegistrar
    {
        HandlerRegistrar()
        {
            RegisterScenarioHandler(ScenarioKind::Goto, {"goto", &ParseGoto, &GotoUpdate, &RearmGoto});
            RegisterScenarioHandler(ScenarioKind::WanderNpc, {"wander", &ParseWander, &WanderUpdate, &RearmWander});
            RegisterScenarioHandler(ScenarioKind::WanderRandom, {"wander", &ParseWander, &WanderUpdate, &RearmWander});
            RegisterScenarioHandler(ScenarioKind::WanderFree, {"wander", &ParseWander, &WanderUpdate, &RearmWander});
            RegisterScenarioHandler(ScenarioKind::Attack, {"attack", &ParseAttack, &AttackUpdate, &RearmAttack});
            RegisterScenarioHandler(ScenarioKind::Die, {"die", &ParseDie, &DieUpdate, &RearmDie});
            RegisterScenarioHandler(ScenarioKind::Taxi, {"taxi", &ParseTaxi, &TaxiUpdate, &RearmTaxi});
            RegisterScenarioHandler(ScenarioKind::Quest, {"quest", &ParseQuest, &QuestUpdate, &RearmQuest});
            RegisterScenarioHandler(ScenarioKind::Transport, {"transport", &ParseTransport, &TransportUpdate, &RearmTransport});
            RegisterScenarioHandler(ScenarioKind::Bg, {"bg", &ParseBg, &BgUpdate, &RearmBg});
            // #207: forces BotScenarioVehicle.cpp's .obj to link (see RegisterVehicleScenarioHandlers'
            // own comment in BotScenarioInternal.h - a self-registering static in that file's own
            // anonymous namespace was silently dropped by the linker since nothing else referenced it).
            RegisterVehicleScenarioHandlers();

            // #210 S10 P5: same reason, for BotScenarioQueue.cpp - see the forward declaration's
            // comment near the top of this file.
            RegisterQueueScenarioHandlers();

            // #201 (#613): same reason, for BotScenarioDuel.cpp (the "duelspot" scenario).
            RegisterDuelScenarioHandlers();
        }
    } const g_handlerRegistrar;

    // --- per-tick trace/end-condition + [PROBE] emission ----------------------------------------

    // Mirrors PlayerbotAI::UpdateAI's scenario block: attack-target acquisition (if due), the
    // [TRACE] line, then the per-kind end-condition check (which logs "[SCENARIO] ... word ..."
    // itself). Returns true when the scenario ended this tick.
    bool UpdateScenarioTrace(Player* bot, PlayerbotAI* botAI, ScenarioState& local, uint32 now)
    {
        uint32 const t = getMSTimeDiff(local.startT, now);

        // HARNESS-SPEC-botscenario-v2 G5 (#129): the teleport that starts an attack scenario is
        // asynchronous, so the nearest-creature lookup cannot happen in the command handler - it
        // runs here, on the first trace tick after landing. Done before the [TRACE] line so a bot
        // that acquires this tick shows target= immediately. Only the AI value is set -
        // bot->Attack() is never called, since the point of the scenario is to see whether the AI
        // engages by itself.
        // Only once the bot has landed at the scenario point: before that, the search would run
        // around wherever the bot was standing when the command arrived.
        if (local.kind == ScenarioKind::Attack && !local.attackGuid && bot->GetMapId() == local.destMapId &&
            bot->GetExactDist2d(local.destX, local.destY) < ATTACK_ACQUIRE_RANGE)
        {
            Creature* victim = local.attackSpawn ? SpawnAttackTarget(bot, local)
                                                 : bot->FindNearestCreature(local.attackEntry, ATTACK_ACQUIRE_RANGE, true);
            if (victim)
            {
                local.attackGuid = victim->GetGUID().GetCounter();
                local.attackTarget = victim->GetGUID();
                // Hand the target to the AI the way the game does ("prioritized targets" -> attackers -> dps
                // target -> non-combat "dps assist" -> Attack()), not by setting "current target", which no
                // trigger reads on its own on either core.
                botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({victim->GetGUID()});
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s target acquired entry=%u target=%u dist=%.1f",
                    bot->GetName().c_str(), local.attackEntry, local.attackGuid, bot->GetExactDist(victim));
            }
        }

        // section 3.6: a 20-member bg scenario would otherwise flood the log with one generic
        // [TRACE] line per anchor tick (500 ms default) on top of its own [BGSTAGE] lines, which
        // already sample at sampleMs; BgUpdate below logs its own "sample" line at that cadence.
        if (local.kind != ScenarioKind::Bg)
        {
            // Normalised to the AC numbering: shared values as-is, NULL = 19, MoP-only = 100 + value.
            int mgen = int(bot->GetMotionMaster()->GetCurrentMovementGeneratorType());
            if (mgen == NULL_MOTION_TYPE)
                mgen = 19;
            else if (mgen == SPLINE_CHAIN_MOTION_TYPE || mgen == FACE_MOTION_TYPE)
                mgen += 100;
            Unit* curTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
            Unit* victim = bot->GetVictim();
            TC_LOG_DEBUG("playerbots",
                "[TRACE] %s t=%u map=%u x=%.1f y=%.1f z=%.1f o=%.2f moving=%d mgen=%d status=%d target=%u victim=%u",
                bot->GetName().c_str(), t, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                bot->GetPositionZ(), bot->GetOrientation(), bot->isMoving() ? 1 : 0, mgen,
                int(botAI->GetRpgInfo().GetStatus()), curTarget ? curTarget->GetGUID().GetCounter() : 0,
                victim ? victim->GetGUID().GetCounter() : 0);
        }

        ScenarioHandler const* h = FindScenarioHandler(local.kind);
        return h && h->update(bot, botAI, local, now, t);
    }

    void AppendFlag(std::string& flags, bool cond, char const* name)
    {
        if (!cond)
            return;
        if (!flags.empty())
            flags += ',';
        flags += name;
    }

    // spdaura= in [PROBE] (#573/#575): every active run-speed aura as spellId:amount, so a FAST row
    // (speed >= 12 on foot) shows whether a real speed buff (Sprint, the BG Speed buff, Spirit Walk...)
    // is behind it, or a run rate left at mount speed with no aura at all. These are the six aura types
    // Unit::UpdateSpeed(MOVE_RUN) reads on both cores, mounted and unmounted. '-' when none. Only for
    // a bot's own probe (botAI set): that runs on the bot's own map thread, while a masterprobe row can
    // come from another map's tick, and the aura lists are not safe to walk from there ("na").
    std::string SpeedAuraList(Player* bot)
    {
        static AuraType const speedAuras[] = {SPELL_AURA_MOD_INCREASE_SPEED, SPELL_AURA_MOD_SPEED_ALWAYS,
            SPELL_AURA_MOD_SPEED_NOT_STACK, SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED,
            SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS, SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK};
        std::string out;
        for (AuraType const type : speedAuras)
        {
            for (AuraEffect const* eff : bot->GetAuraEffectsByType(type))
            {
                if (!out.empty())
                    out += ',';
                out += std::to_string(eff->GetId()) + ':' + std::to_string(eff->GetAmount());
            }
        }
        return out.empty() ? "-" : out;
    }

    void EmitProbe(Player* bot, PlayerbotAI* botAI, ScenarioState const& state, uint32 now)
    {
        // A real player registered via masterprobe has no PlayerbotAI (issue #206); botAI is
        // then null and every AI-context-dependent field below degrades to its "none" value.
        NewRpgInfo const* info = botAI ? &botAI->GetRpgInfo() : nullptr;
        uint32 const startT = state.kind != ScenarioKind::None ? state.startT : state.probeStartT;
        uint32 const t = getMSTimeDiff(startT, now);

        std::string flags;
        AppendFlag(flags, bot->IsFlying(), "fly");
        AppendFlag(flags, bot->IsInWater(), "swim");
        AppendFlag(flags, bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR), "fall");
        AppendFlag(flags, bot->IsMounted(), "mounted");
        AppendFlag(flags, bot->IsInFlight(), "taxi");
        AppendFlag(flags, bot->GetTransport() != nullptr, "transport");
        AppendFlag(flags, bot->GetVehicleBase() != nullptr, "vehicle");
        AppendFlag(flags, bot->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GHOST), "ghost");
        AppendFlag(flags, !bot->IsAlive(), "dead");
        AppendFlag(flags, bot->IsInCombat(), "incombat");
        AppendFlag(flags, bot->GetCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr ||
                           bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr, "casting");
        AppendFlag(flags, bot->HasUnitState(UNIT_STATE_STUNNED), "stunned");
        AppendFlag(flags, bot->HasUnitState(UNIT_STATE_ROOT), "rooted");
        // #837: Spirit of Redemption - alive and rooted, but dead for every harness purpose (the priest dies when it ends)
        AppendFlag(flags, bot->HasSpiritOfRedemptionAura(), "sor");
        if (flags.empty())
            flags = "-";

        char transportBuf[16] = "-";
        if (Transport* transport = bot->GetTransport())
            snprintf(transportBuf, sizeof(transportBuf), "%u", transport->GetEntry());

        char vehicleBuf[16] = "-";
        if (Unit* vehicleBase = bot->GetVehicleBase())
            snprintf(vehicleBuf, sizeof(vehicleBuf), "%u", vehicleBase->GetEntry());

        Powers const ptype = bot->GetPowerType();
        float pwrPct = 0.0f;
        if (int32 const maxPower = bot->GetMaxPower(ptype))
            pwrPct = 100.0f * float(bot->GetPower(ptype)) / float(maxPower);

        char destBuf[64] = "-";
        char rpgnpcBuf[16] = "-";
        if (info)
        {
            if (auto const* grind = std::get_if<NewRpgInfo::GoGrind>(&info->data))
                snprintf(destBuf, sizeof(destBuf), "%.1f,%.1f,%.1f", grind->pos.GetPositionX(), grind->pos.GetPositionY(),
                    grind->pos.GetPositionZ());
            else if (auto const* camp = std::get_if<NewRpgInfo::GoCamp>(&info->data))
                snprintf(destBuf, sizeof(destBuf), "%.1f,%.1f,%.1f", camp->pos.GetPositionX(), camp->pos.GetPositionY(),
                    camp->pos.GetPositionZ());
            else if (auto const* wanderNpc = std::get_if<NewRpgInfo::WanderNpc>(&info->data))
            {
                if (wanderNpc->npcOrGo)
                    if (WorldObject* npc = ObjectAccessor::GetWorldObject(*bot, wanderNpc->npcOrGo))
                        snprintf(rpgnpcBuf, sizeof(rpgnpcBuf), "%u", npc->GetEntry());
            }
            else if (auto const* flight = std::get_if<NewRpgInfo::TravelFlight>(&info->data))
            {
                // A.4: dest= is the departure flight master's position, rpgnpc= its creature_template entry.
                snprintf(destBuf, sizeof(destBuf), "%.1f,%.1f,%.1f", flight->flightMasterPos.GetPositionX(),
                    flight->flightMasterPos.GetPositionY(), flight->flightMasterPos.GetPositionZ());
                if (flight->flightMasterEntry)
                    snprintf(rpgnpcBuf, sizeof(rpgnpcBuf), "%u", flight->flightMasterEntry);
            }
            else if (auto const* doQuest = std::get_if<NewRpgInfo::DoQuest>(&info->data))
            {
                // B.4: dest= is the current POI position (turn-in included, ObjectiveIndex -1).
                if (doQuest->pos != WorldPosition())
                    snprintf(destBuf, sizeof(destBuf), "%.1f,%.1f,%.1f", doQuest->pos.GetPositionX(),
                        doQuest->pos.GetPositionY(), doQuest->pos.GetPositionZ());
            }
        }

        char targetBuf[32] = "-";
        if (botAI)
            if (Unit* curTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get())
                snprintf(targetBuf, sizeof(targetBuf), "%u:%u", curTarget->GetEntry(), curTarget->GetGUID().GetCounter());

        char victimBuf[32] = "-";
        if (Unit* victim = bot->GetVictim())
            snprintf(victimBuf, sizeof(victimBuf), "%u:%u", victim->GetEntry(), victim->GetGUID().GetCounter());

        // #569 (#643): thp=<hp%>:<max hp> of the AI target (else the melee victim), appended last, so a 1:1
        // dungeon run reads the boss's runtime max HP, its hp% curve and the kill time on both cores alike.
        char thpBuf[32] = "-";
        {
            Unit* hpUnit = botAI ? botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() : nullptr;
            if (!hpUnit)
                hpUnit = bot->GetVictim();
            if (hpUnit)
                snprintf(thpBuf, sizeof(thpBuf), "%.0f:%u", hpUnit->GetHealthPct(), hpUnit->GetMaxHealth());
        }

        char castBuf[16] = "-";
        if (Spell* spell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            snprintf(castBuf, sizeof(castBuf), "%u", spell->GetSpellInfo()->Id);
        else if (Spell* channeled = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            snprintf(castBuf, sizeof(castBuf), "%u", channeled->GetSpellInfo()->Id);

        char lastActBuf[128] = "-";
        if (!state.lastActionName.empty())
            snprintf(lastActBuf, sizeof(lastActBuf), "%s:%s", state.lastActionName.c_str(), state.lastActionResult.c_str());

        char lastCastBuf[32] = "-";
        if (state.hasLastCast)
            snprintf(lastCastBuf, sizeof(lastCastBuf), "%u:%d", state.lastCastSpell, state.lastCastResult);

        char groupBuf[16] = "-";
        if (Group* group = bot->GetGroup())
            snprintf(groupBuf, sizeof(groupBuf), "%u", group->GetMembersCount());

        char bgBuf[16] = "-";
        if (Battleground* bg = bot->GetBattleground())
            snprintf(bgBuf, sizeof(bgBuf), "%d", int(bg->GetTypeID()));

        char const* inst = "world";
        if (Map* map = bot->GetMap())
        {
            if (map->IsBattleArena())
                inst = "arena";
            else if (map->IsBattleground())
                inst = "bg";
            else if (map->IsRaid())
                inst = "raid";
            else if (map->IsDungeon())
                inst = "dungeon";
        }

        int mgen = int(bot->GetMotionMaster()->GetCurrentMovementGeneratorType());
        if (mgen == NULL_MOTION_TYPE)
            mgen = 19;
        else if (mgen == SPLINE_CHAIN_MOTION_TYPE || mgen == FACE_MOTION_TYPE)
            mgen += 100;

        // master=/mdist= (issue #172): the follow-distance metric a real-player 1:1 test reads from
        // the server side, since the client only sees its own EVT pos trail. Null for a masterprobe
        // target (issue #206): a real player has no PlayerbotAI, so no "master" of its own.
        char masterBuf[64] = "-";
        float mdist = -1.0f;
        if (Player* master = botAI ? botAI->GetMaster() : nullptr)
        {
            snprintf(masterBuf, sizeof(masterBuf), "%s", master->GetName().c_str());
            mdist = bot->GetDistance(master);
        }

        // rspd= / spdaura= (#575 FAST split): the raw run-speed rate (speed= is this times the base
        // 7.0) and the speed auras behind it - see SpeedAuraList. Appended last so older parsers that
        // stop at loot= keep working.
        std::string const spdAura = botAI ? SpeedAuraList(bot) : std::string("na");

        // #201 (#613): duel=<state>:<opp|-> dspot=<idx|-> (BotScenarioDuel.cpp), appended last.
        std::string duelProbe, dspotProbe;
        DuelProbeFields(bot, botAI, duelProbe, dspotProbe);

        // #756: atk=<entry:guid:P|S|T|C,...> up to 3 attackers (P player, S pet/summon, T totem, C creature), appended last
        std::string atkBuf;
        {
            uint32 n = 0;
            for (Unit* a : bot->getAttackers())
            {
                if (!a || n++ >= 3)
                    break;
                char const kind = a->GetTypeId() == TYPEID_PLAYER ? 'P' : a->IsTotem() ? 'T' : (a->IsSummon() || a->IsPet() || a->IsGuardian()) ? 'S' : 'C';
                if (!atkBuf.empty())
                    atkBuf += ',';
                atkBuf += std::to_string(a->GetEntry()) + ":" + std::to_string(a->GetGUID().GetCounter()) + ":" + kind;
            }
            if (atkBuf.empty())
                atkBuf = "-";
        }

        // #319: tseen=<entry>:<ticks> | - . The latch (ScenarioState::tlatchEntry/Ticks, set on every UpdateAI tick
        // in OnUpdate and cleared when a row is emitted) says whether the bot was aboard a transport at ANY tick
        // since the previous row, so a short aboard window between two samples still shows. A masterprobe row
        // has no per-tick latch and falls back to the instantaneous transport (ticks 0). Appended last.
        std::string tseenBuf = "-";
        {
            uint32 tEntry = state.tlatchEntry;
            if (!tEntry)
                if (Transport* tNow = bot->GetTransport())
                    tEntry = tNow->GetEntry();
            if (tEntry)
                tseenBuf = std::to_string(tEntry) + ":" + std::to_string(state.tlatchTicks);
        }

        // loot=<entry|-> : the "loot target" AI value is not implemented on this fork (see
        // DistanceValue.cpp / PlayerbotAI.cpp, all commented out) - always '-' (see report).
        TC_LOG_DEBUG("playerbots",
            "[PROBE] %s t=%u map=%u zone=%u area=%u x=%.1f y=%.1f z=%.1f o=%.2f speed=%.2f moving=%d mgen=%d "
            "flags=%s transport=%s vehicle=%s "
            "hp=%.0f pwr=%.0f ptype=%d lvl=%u status=%d dest=%s rpgnpc=%s "
            "target=%s victim=%s attackers=%u cast=%s cds=%u "
            "lastact=%s lastcast=%s group=%s bg=%s inst=%s master=%s mdist=%.1f loot=- rspd=%.2f spdaura=%s duel=%s dspot=%s iid=%u seat=%d thp=%s atk=%s tseen=%s",
            bot->GetName().c_str(), t, bot->GetMapId(), bot->GetZoneId(), bot->GetAreaId(),
            bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation(),
            bot->GetSpeed(MOVE_RUN), bot->isMoving() ? 1 : 0, mgen,
            flags.c_str(), transportBuf, vehicleBuf,
            bot->GetHealthPct(), pwrPct, int(ptype), bot->GetLevel(), info ? int(info->GetStatus()) : -1, destBuf, rpgnpcBuf,
            targetBuf, victimBuf, uint32(bot->getAttackers().size()), castBuf,
            ActiveCooldownCount(bot),
            lastActBuf, lastCastBuf, groupBuf, bgBuf, inst, masterBuf, mdist,
            bot->GetSpeedRate(MOVE_RUN), spdAura.c_str(), duelProbe.c_str(), dspotProbe.c_str(),
            bot->GetInstanceId(), bot->GetVehicle() ? int(bot->GetTransSeat()) : -1,  // #206 T8/T9: seat=/iid= appended last
            thpBuf, atkBuf.c_str(), tseenBuf.c_str());  // #319 tseen= appended after atk=; #569: thp= appended last; #756 atk= after it
    }

    // #209 S9 round 6: masterprobe (a real player's [PROBE] line) was piggy-backed on a single
    // grouped bot's own AI tick (see the "if (Player* master = botAI->GetMaster())" block in
    // OnUpdate below). That pointer is fragile: PlayerbotAI::UpdateAIGroupMaster-equivalent logic
    // clears a random bot's master the instant bot->GetGroup() is null - the server's own
    // autonomous random-bot-pool behaviour can drop a group at any time, independent of anything
    // the test harness/driver did. Observed live (AC, same code shape - see the AC copy of this
    // file): exactly one [PROBE] <master> line right after grouping, then total silence for the
    // rest of a 45s+ run, because that one bot's master pointer went stale a tick later. Fix: an
    // independent sweep that emits every masterprobe-enabled REAL PLAYER (no PlayerbotAI of its
    // own) directly via ObjectAccessor::FindPlayer, with no group/master relationship involved at
    // all. It runs from whichever bot's OnUpdate happens to fire next - with ~100 random bots
    // ticking, that is many times a second, far more often than the 1s probe interval needs - so
    // masterprobe now emits reliably regardless of any one bot's group state. A bot's OWN probe
    // is untouched: it is skipped here (GET_PLAYERBOT_AI(p) check) and keeps firing from its own
    // reliable per-bot-tick path further down in OnUpdate.
    void MasterProbeSweep(uint32 now)
    {
        // Rate-limit the sweep itself (not the per-entry interval, which is still enforced below):
        // no need to scan g_bots on every single one of ~100 bots' ticks.
        static std::atomic<uint32> lastSweep{0};
        uint32 prev = lastSweep.load(std::memory_order_relaxed);
        if (prev && getMSTimeDiff(prev, now) < 250)
            return;
        if (!lastSweep.compare_exchange_strong(prev, now))
            return; // another thread's tick already claimed this sweep window

        // Collect the DUE entries WITHOUT stamping them. Stamping here claimed the probe interval for
        // entries this sweep then declines to emit - a real bot is skipped just below, because its own
        // tick owns it - and since this sweep's own rate limit (250 ms) is finer than the probe interval
        // (ProbeIntervalMsDefault(), 1000 ms) it would win that race nearly every time. That silenced the
        // bot's own [PROBE] line, i.e. the exact opposite of what the comment above promises. Only the
        // entries this sweep actually emits are stamped now, after the emit.
        std::vector<std::pair<uint32, ScenarioState>> due;
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            for (auto& entry : g_bots)
            {
                ScenarioState& st = entry.second;
                if (!st.probeEnabled)
                    continue;
                if (!st.lastProbeT || getMSTimeDiff(st.lastProbeT, now) >= st.probeIntervalMs)
                    due.emplace_back(entry.first, st);
            }
        }

        std::vector<uint32> claimed;
        for (auto& item : due)
        {
            ObjectGuid const guid(HighGuid::Player, item.first);
            Player* p = ObjectAccessor::FindPlayer(guid);
            if (!p || GET_PLAYERBOT_AI(p))
                continue; // a real bot: its own tick's doProbe path below owns it - do NOT stamp it here
            if (p->IsBeingTeleported())
                continue; // same guard the piggy-backed path applies; a mid-teleport sample is meaningless
            EmitProbe(p, nullptr, item.second, now);
            claimed.push_back(item.first);
        }

        if (!claimed.empty())
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            for (uint32 claimedGuid : claimed)
            {
                auto it = g_bots.find(claimedGuid);
                if (it != g_bots.end())
                    it->second.lastProbeT = now;
            }
        }
    }
} // anonymous namespace

// ---------------------------------------------------------------------------------------------
// Registry (declared in BotScenarioInternal.h)
// ---------------------------------------------------------------------------------------------
void RegisterScenarioHandler(ScenarioKind kind, ScenarioHandler const& handler)
{
    KindRegistry()[uint8(kind)] = handler;
    NameRegistry()[handler.name] = handler; // wander's 3 kinds register the same name 3x - harmless
}

ScenarioHandler const* FindScenarioHandlerByName(std::string const& name)
{
    auto const it = NameRegistry().find(name);
    return it == NameRegistry().end() ? nullptr : &it->second;
}

ScenarioHandler const* FindScenarioHandler(ScenarioKind kind)
{
    if (kind == ScenarioKind::None)
        return nullptr;
    auto const it = KindRegistry().find(uint8(kind));
    return it == KindRegistry().end() ? nullptr : &it->second;
}

// ---------------------------------------------------------------------------------------------
// Public API (declared in BotScenario.h)
// ---------------------------------------------------------------------------------------------
bool AnyActive()
{
    return g_active.load(std::memory_order_relaxed) > 0;
}

void OnUpdate(PlayerbotAI* botAI)
{
    if (!AnyActive())
        return;

    Player* bot = botAI->GetBot();
    if (!bot || bot->IsBeingTeleported())
        return;

    uint32 const guidLow = bot->GetGUID().GetCounter();
    uint32 const now = getMSTime();

    // #209 round 6: independent masterprobe sweep - see MasterProbeSweep's comment above. Runs
    // first and unconditionally (any bot's tick can drive it), so a masterprobe target still gets
    // its [PROBE] line even if the specific grouped-bot piggyback just below has gone stale.
    MasterProbeSweep(now);
    DuelSweep(now); // #201 (#613): due [DUEL] SAMPLE rows + decline fallback

    // masterprobe for a real player (issue #206): a real player has no PlayerbotAI of its own, so
    // it never reaches this function as "bot" and the EmitProbe call below never fires for it. A
    // grouped bot's own tick is the only hook available for a real master; piggy-back the
    // master's probe on it here, rate-limited on the master's own ScenarioState so several bots
    // following the same master do not duplicate the line. Kept as a fast/early emission right
    // after grouping - MasterProbeSweep above is the reliability backstop for when this pointer
    // goes stale (#209 round 6 finding).
    if (Player* master = botAI->GetMaster())
    {
        if (master != bot && !master->IsBeingTeleported())
        {
            uint32 const masterGuidLow = master->GetGUID().GetCounter();
            ScenarioState masterLocal;
            bool masterDoProbe = false;
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                auto mit = g_bots.find(masterGuidLow);
                if (mit != g_bots.end() && mit->second.probeEnabled &&
                    (!mit->second.lastProbeT || getMSTimeDiff(mit->second.lastProbeT, now) >= mit->second.probeIntervalMs))
                {
                    mit->second.lastProbeT = now;
                    masterDoProbe = true;
                    masterLocal = mit->second;
                }
            }
            if (masterDoProbe)
                EmitProbe(master, nullptr, masterLocal, now);
        }
    }

    // #319: latch "aboard a transport on this tick" (applied under the lock below, cleared when a row is
    // emitted), so a short aboard window between two probe samples still shows up as tseen=. Read before
    // the lock: it is a plain pointer read, no game-state change.
    uint32 aboardEntry = 0;
    if (Transport* aboardTransport = bot->GetTransport())
        aboardEntry = aboardTransport->GetEntry();

    ScenarioState local;
    bool doTrace = false;
    bool doProbe = false;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = g_bots.find(guidLow);
        if (it == g_bots.end())
            return;

        ScenarioState& st = it->second;
        if (aboardEntry)
        {
            st.tlatchEntry = aboardEntry;
            ++st.tlatchTicks;
        }
        if (st.kind != ScenarioKind::None &&
            (!st.lastTraceT || getMSTimeDiff(st.lastTraceT, now) >= TraceIntervalMs()))
        {
            st.lastTraceT = now;
            doTrace = true;
        }
        if (st.probeEnabled &&
            (!st.lastProbeT || getMSTimeDiff(st.lastProbeT, now) >= st.probeIntervalMs))
        {
            st.lastProbeT = now;
            doProbe = true;
        }

        if (!doTrace && !doProbe)
            return;

        local = st; // work from a copy - no game-API calls while holding the lock
        if (doProbe)
        {
            st.tlatchEntry = 0; // #319: this row consumes the latch
            st.tlatchTicks = 0;
        }
    }

    bool const scenarioEnded = doTrace && UpdateScenarioTrace(bot, botAI, local, now);

    if (doProbe)
        EmitProbe(bot, botAI, local, now);

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = g_bots.find(guidLow);
        if (it == g_bots.end())
            return;

        if (scenarioEnded)
        {
            it->second.kind = ScenarioKind::None;
            it->second.lock = false;
            it->second.wanderMs = 0;
            it->second.allowMask = 0;
            it->second.pickNearest = false;
            it->second.pickCount = 0;
            it->second.attackGuid = 0;
            it->second.attackEntry = 0;

            if (!it->second.IsActive())
            {
                g_bots.erase(it);
                g_active.fetch_sub(1, std::memory_order_relaxed);
            }
        }
        else if (doTrace)
        {
            // Only the attack-acquire step (UpdateScenarioTrace) can have changed anything that
            // needs to persist across ticks.
            it->second.attackGuid = local.attackGuid;
            it->second.attackTarget = local.attackTarget;
            it->second.die = local.die;     // death-stage progress lives in the per-tick copy
            it->second.taxi = local.taxi;   // taxi-stage progress lives in the per-tick copy
            it->second.quest = local.quest; // quest-stage progress lives in the per-tick copy
            it->second.transport = local.transport; // transport-stage progress lives in the per-tick copy
            it->second.bg = local.bg;               // bg-stage progress lives in the per-tick copy
            it->second.drive = local.drive;         // drive-scenario progress lives in the per-tick copy (#207)
            it->second.hotrod = local.hotrod;       // hotrod-scenario progress lives in the per-tick copy (#207)
        }
    }

    // #631: game-API side of the end, after g_mutex is released - drop a queue=natural stage's rated-bracket hold.
    if (scenarioEnded && local.kind == ScenarioKind::Bg)
        ReleaseNaturalArenaHold(local.bg);
}

void OnReset(PlayerbotAI* botAI)
{
    if (!AnyActive())
        return;

    Player* bot = botAI->GetBot();
    if (!bot)
        return;

    uint32 const guidLow = bot->GetGUID().GetCounter();

    ScenarioState local;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = g_bots.find(guidLow);
        if (it == g_bots.end() || it->second.kind == ScenarioKind::None)
            return; // no running scenario to rearm; a standalone probe holds no RPG-status state
        local = it->second;
    }

    if (ScenarioHandler const* h = FindScenarioHandler(local.kind))
        h->rearm(bot, botAI, local);

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = g_bots.find(guidLow);
        if (it != g_bots.end())
            it->second = local;
    }

    TC_LOG_DEBUG("playerbots", "[SCENARIO] %s rearmed after reset map=%u", bot->GetName().c_str(), bot->GetMapId());
}

bool Driving(Player* bot)
{
    if (!AnyActive())
        return false;

    std::lock_guard<std::mutex> lockGuard(g_mutex);
    auto const it = g_bots.find(bot->GetGUID().GetCounter());
    return it != g_bots.end() && it->second.kind != ScenarioKind::None;
}

bool IsStatusLocked(Player* bot)
{
    if (!AnyActive())
        return false;

    ScenarioKind kind = ScenarioKind::None;
    bool lock = false;
    {
        std::lock_guard<std::mutex> lockGuard(g_mutex);
        auto const it = g_bots.find(bot->GetGUID().GetCounter());
        if (it == g_bots.end())
            return false;
        kind = it->second.kind;
        lock = it->second.lock;
    }

    if (!lock)
        return false;

    // A.4: the taxi stage still needs NewRpgStatusUpdateAction's own TRAVEL_FLIGHT -> IDLE transition
    // (data->inFlight && !bot->IsInFlight()) to run, or landing would never show up (the #120
    // regression the stage is measuring would be permanently hidden behind a pinned status instead of
    // producing "unresolved"). Read the RPG status via GET_PLAYERBOT_AI(bot) outside g_mutex - it is
    // PlayerbotAI-owned state, not the scenario side table's, and this is always called from
    // NewRpgStatusUpdateAction::Execute on the bot's own map/AI update thread.
    if (kind == ScenarioKind::Taxi)
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
            if (botAI->GetRpgInfo().GetStatus() == RPG_TRAVEL_FLIGHT)
                return false;

    return true;
}

// #201 (#613): the duel harness (BotScenarioDuel.cpp) logs [DUEL] lines only for tracked players and uses
// the tracked entry's own clock, so its t= matches that entry's [PROBE] rows.
bool ScenarioClock(ObjectGuid guid, uint32& t)
{
    if (!AnyActive())
        return false;
    uint32 startT = 0;
    {
        std::lock_guard<std::mutex> lockGuard(g_mutex);
        auto const it = g_bots.find(guid.GetCounter());
        if (it == g_bots.end())
            return false;
        startT = it->second.kind != ScenarioKind::None ? it->second.startT : it->second.probeStartT;
    }
    t = getMSTimeDiff(startT, getMSTime());
    return true;
}

void FilterStatuses(Player* bot, std::vector<NewRpgStatus>& candidates)
{
    if (!AnyActive())
        return;

    uint32 allowMask = 0;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto const it = g_bots.find(bot->GetGUID().GetCounter());
        if (it == g_bots.end() || it->second.kind == ScenarioKind::None || !it->second.allowMask)
            return;
        allowMask = it->second.allowMask;
    }

    candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
        [allowMask](NewRpgStatus s) { return !(allowMask & (1u << uint32(s))); }), candidates.end());
}

bool WantsTrail(Player* bot)
{
    // The per-tick decision trail (Engine "A:<action> - <result>" lines) is on for every bot under a
    // scenario or probe, so a test bot needs no AiPlayerbot.DebugBotNames entry (and no restart).
    if (!AnyActive())
        return false;
    std::lock_guard<std::mutex> lock(g_mutex);
    auto const it = g_bots.find(bot->GetGUID().GetCounter());
    return it != g_bots.end() && (it->second.kind != ScenarioKind::None || it->second.probeEnabled);
}

bool BgJoinBlocked(Player* bot)
{
    if (!bot)
        return false;

    // #210 S10 P5 "queuegate": a blanket gate for a 1:1 LFG/BG run - every bot the harness has not
    // registered (any scenario/probe, not just a "bg" stage roster) is refused, regardless of
    // whether a `bg` stage itself is running. Checked ahead of AnyActive()'s fast path below,
    // since queuegate can be the only thing active on the server.
    if (QueueGateOn())
    {
        // Block EVERY bot's own BG self-queue: a probed/registered roster bot that queues itself
        // then refuses its leader's invite (PlayerbotSecurity InBattlegroundQueue gate), and group
        // joins by a real leader do not go through this action at all.
        return true;
    }

    // A `bg` stage fills one instance with a fixed roster; a random bot outside it queueing through its own
    // BGJoinAction would take a free slot (the run then ends `bgoutsider`). Identical on both cores.
    if (!AnyActive())
        return false;
    std::lock_guard<std::mutex> lock(g_mutex);
    // w19: a bot under any non-BG scenario (rotation attack, goto, wander, quest, ...) never queues itself mid-leg -
    // at 1000 bots the queues pop within the leg and the bot left its dummy (MoP unholy DK, 0 s in combat).
    {
        auto const self = g_bots.find(bot->GetGUID().GetCounter());
        if (self != g_bots.end() && self->second.kind != ScenarioKind::None && self->second.kind != ScenarioKind::Bg)
            return true;
    }
    bool anyBg = false;
    for (auto const& [low, st] : g_bots)
    {
        if (st.kind != ScenarioKind::Bg)
            continue;
        anyBg = true;
        for (size_t k = 0; k < st.bg.members.size(); ++k)
            if (st.bg.members[k] == bot->GetGUID())
                // wave-29 #632 leg: onlyteam=A|H keeps the other half of the roster out of the queue
                return st.bg.onlyTeam >= 0 && st.bg.memberTeam[k] != uint8(st.bg.onlyTeam);
    }
    return anyBg;
}

// #632 roster guard: true while a `bg` stage holds this bot in its roster. The arena-queue "waited too
// long" leave (BGStatusAction::LeaveArenaQueueIfWaitedTooLong / AC BattleGroundJoinAction.cpp:925-962)
// must not drop a roster captain out of a queue=natural wait (up to 600 s). Harness-only; defined in
// namespace BotScenario, declared by the caller (no BotScenario.h change). Identical on both cores.
bool BotScenarioHoldsArenaRoster(Player* bot)
{
    if (!bot || !AnyActive())
        return false;
    std::lock_guard<std::mutex> lock(g_mutex);
    for (auto const& [low, st] : g_bots)
    {
        if (st.kind != ScenarioKind::Bg || st.bg.aqTimeout)  // wave-29 #632 leg: aqtimeout=1 lets the queue time out
            continue;
        for (ObjectGuid const& g : st.bg.members)
            if (g == bot->GetGUID())
                return true;
    }
    return false;
}

bool TransportCheckDue(Player* bot)
{
    if (!AnyActive() || !bot)
        return false;
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_bots.find(bot->GetGUID().GetCounter());
    if (it == g_bots.end() || it->second.kind != ScenarioKind::Transport)
        return false;
    TransportTrack& tr = it->second.transport;
    if (tr.tcheck == 2 && tr.phase >= 2) // tick
        return true;
    if (tr.tcheck == 1 && tr.tcheckPending && !bot->IsBeingTeleported() &&
        bot->GetExactDist(tr.placeX, tr.placeY, tr.placeZ) < 3.0f) // now
    {
        tr.tcheckPending = false;
        return true;
    }
    return false;
}

bool NoFight(Player* bot)
{
    if (!AnyActive())
        return false;
    std::lock_guard<std::mutex> lock(g_mutex);
    auto const it = g_bots.find(bot->GetGUID().GetCounter());
    return it != g_bots.end() && it->second.kind != ScenarioKind::None && it->second.noFight;
}

bool GroundOnly(Player* bot)
{
    if (!AnyActive())
        return false;
    std::lock_guard<std::mutex> lock(g_mutex);
    auto const it = g_bots.find(bot->GetGUID().GetCounter());
    return it != g_bots.end() && it->second.kind != ScenarioKind::None && it->second.groundOnly;
}

bool GrindPoint(Player* bot, WorldPosition& out)
{
    if (!AnyActive())
        return false;
    std::lock_guard<std::mutex> lock(g_mutex);
    auto const it = g_bots.find(bot->GetGUID().GetCounter());
    if (it == g_bots.end() || it->second.kind == ScenarioKind::None || !it->second.grindSet)
        return false;
    ScenarioState const& s = it->second;
    out = WorldPosition(s.destMapId, s.grindX, s.grindY, s.grindZ);
    return true;
}

bool PickNearest(Player* bot)
{
    if (!AnyActive())
        return false;
    std::lock_guard<std::mutex> lock(g_mutex);
    auto const it = g_bots.find(bot->GetGUID().GetCounter());
    return it != g_bots.end() && it->second.kind != ScenarioKind::None && it->second.pickNearest;
}

bool NextRandomNearAngle(Player* bot, float moveStep, float& angle, float& dist)
{
    if (!AnyActive())
        return false;

    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_bots.find(bot->GetGUID().GetCounter());
    if (it == g_bots.end() || it->second.kind == ScenarioKind::None || !it->second.pickNearest)
        return false;

    // HARNESS-SPEC-botscenario-v2 G3.4: deterministic pick - a golden-angle sweep advanced by
    // pickCount instead of a random angle/distance, so repeated calls are reproducible.
    angle = fmod(it->second.pickCount * 2.39996f, 2 * static_cast<float>(M_PI));
    dist = moveStep;
    ++it->second.pickCount;
    return true;
}

void LogTarget(Player* bot, char const* kind, WorldObject const* target, float x, float y, float z, float step)
{
    if (!AnyActive())
        return;

    uint32 const guidLow = bot->GetGUID().GetCounter();
    uint32 startT = 0;
    bool const isQg = kind && !strcmp(kind, "qg");
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = g_bots.find(guidLow);
        if (it == g_bots.end() || it->second.kind == ScenarioKind::None)
            return;

        startT = it->second.startT;

        if (isQg)
        {
            // issue #61: de-dupe kind=qg while the bot stays at the same quest giver, so staying
            // put across several ticks logs it once, not on every Execute().
            ObjectGuid const guid = target ? target->GetGUID() : ObjectGuid::Empty;
            if (guid == it->second.lastQg)
                return;
            it->second.lastQg = guid;
        }
    }

    PlayerbotAI* rpgAI = GET_PLAYERBOT_AI(bot);
    int const status = rpgAI ? int(rpgAI->GetRpgInfo().GetStatus()) : 0;

    TC_LOG_DEBUG("playerbots",
        "[WANDER] %s t=%u kind=%s status=%d entry=%u guid=%u step=%.0f bx=%.1f by=%.1f bz=%.1f tx=%.1f ty=%.1f "
        "tz=%.1f dist=%.1f",
        bot->GetName().c_str(), getMSTimeDiff(startT, getMSTime()), kind, status,
        target ? target->GetEntry() : 0, target ? uint32(target->GetGUID().GetCounter()) : 0, step,
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), x, y, z, bot->GetExactDist(x, y, z));
}

void OnAction(Player* bot, char const* action, char const* result)
{
    if (!AnyActive())
        return;

    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_bots.find(bot->GetGUID().GetCounter());
    if (it == g_bots.end() || !it->second.probeEnabled)
        return;

    it->second.lastActionName = action ? action : "";
    it->second.lastActionResult = result ? result : "";
}

void OnCast(Player* bot, uint32 spellId, int result)
{
    if (!AnyActive())
        return;

    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_bots.find(bot->GetGUID().GetCounter());
    if (it == g_bots.end() || !it->second.probeEnabled)
        return;

    it->second.hasLastCast = true;
    it->second.lastCastSpell = spellId;
    it->second.lastCastResult = result;
}

    // #811 (w49/w51 Scarlet Halls normal, level-36 bots): Vlanko and Lyndelius failed every `teleport` into map 1001 while
    // the other two bots entered iid 7. Their saved dungeon difficulty was HEROIC (characters.instance_mode_mask & 0xF = 2,
    // left by an earlier heroic run), and MoP's Group::AddMember copies the group's difficulty only to members at level >= 70
    // (Group.cpp:496, LEVELREQUIREMENT_HEROIC). AC's Group::AddMember (Group.cpp:506) copies it at any level. CanPlayerEnter
    // then read the HEROIC access_requirement (level_min 90) and Player::Satisfy (Player.cpp:20756) refused without a log.
    // A far teleport of a grouped player into a dungeon first takes the group's difficulty, the state AC's AddMember already
    // gives every member, so both cores enter with the group's difficulty. Returns "" or " diff=<old>-><new>" for the reply.
    static std::string AlignGroupDifficulty(Player* p, uint32 mapId)
    {
        MapEntry const* entry = sMapStore.LookupEntry(mapId);
        Group* group = p->GetGroup();
        if (!entry || !entry->IsDungeon() || !group || group->isBGGroup() || group->isBFGroup() || p->GetMapId() == mapId)
            return std::string();
        bool const raid = entry->IsRaid();
        Difficulty const want = raid ? group->GetRaidDifficulty() : group->GetDungeonDifficulty();
        Difficulty const have = p->GetDifficulty(raid);
        if (have == want)
            return std::string();
        if (raid)
        {
            p->SetRaidDifficulty(want);
            p->SendRaidDifficulty(want);
        }
        else
        {
            p->SetDungeonDifficulty(want);
            p->SendDungeonDifficulty(want);
        }
        return " diff=" + std::to_string(uint32(have)) + "->" + std::to_string(uint32(want));
    }

    // #811: why Player::TeleportTo returned false. Read-only re-check of its gates in its own order: Player.cpp:1856-1880
    // (coords, BG map, expansion), the far-teleport branch (MapManager::CanPlayerEnter MapManager.cpp:255-380:
    // instance_template, map difficulty, raid group, ghost/corpse, group-bound InstanceMap::CanEnter Map.cpp:3524,
    // instances per hour, Player::Satisfy access_requirement). Gates not re-checked (disables, the start-zone race gates
    // at Player.cpp:1989-1997, Terrace) fall through to "unknown".
    static std::string TeleportFailReason(Player* p, uint32 mapId, float x, float y, float z)
    {
        MapEntry const* entry = sMapStore.LookupEntry(mapId);
        if (!entry || !MapManager::IsValidMapCoord(mapId, x, y, z))
            return "invalid-map-or-coord";
        if (!p->InBattleground() && entry->IsBattlegroundOrArena())
            return "bg-map-without-bg";
        if (p->GetSession()->Expansion() < entry->Expansion())
            return "expansion account=" + std::to_string(uint32(p->GetSession()->Expansion())) + " map=" + std::to_string(entry->Expansion());
        if (p->GetMapId() == mapId || !entry->IsDungeon())
            return "unknown(not-an-instance-gate)";
        if (!sObjectMgr->GetInstanceTemplate(mapId))
            return "no-instance_template";
        bool const raid = entry->IsRaid();
        Difficulty diff = p->GetDifficulty(raid);
        std::string const d = " diff=" + std::to_string(uint32(diff));
        Difficulty down = diff;
        if (!sDBCManager.GetDownscaledMapDifficultyData(mapId, down) && mapId != 1112)
            return "no-map-difficulty" + d;
        if (p->IsGameMaster())
            return "unknown(gm)" + d;
        Group* group = p->GetGroup();
        if (raid && entry->Expansion() >= EXPANSION_MISTS_OF_PANDARIA && (!group || !group->isRaidGroup()) &&
            !sWorld->getBoolConfig(CONFIG_INSTANCE_IGNORE_RAID))
            return "raid-group-only";
        if (!p->IsAlive() && p->HasCorpse())
        {
            uint32 corpseMap = p->GetCorpseLocation().GetMapId();
            while (corpseMap && corpseMap != mapId)
            {
                InstanceTemplate const* it = sObjectMgr->GetInstanceTemplate(corpseMap);
                corpseMap = it ? it->Parent : 0;
            }
            if (!corpseMap)
                return "ghost-corpse-elsewhere corpseMap=" + std::to_string(p->GetCorpseLocation().GetMapId());
        }
        if (group)
            if (InstanceGroupBind* bind = group->GetBoundInstance(entry))
                if (bind->save)
                    if (Map* m = sMapMgr->FindMap(mapId, bind->save->GetInstanceId()))
                        if (InstanceMap* im = m->ToInstanceMap())
                        {
                            std::string const iid = " iid=" + std::to_string(im->GetInstanceId());
                            if (im->GetPlayersCountExceptGMs() >= im->GetMaxPlayers())
                                return "instance-full" + iid + " players=" + std::to_string(im->GetPlayersCountExceptGMs()) + "/" +
                                       std::to_string(im->GetMaxPlayers());
                            if (im->IsRaid() && im->GetInstanceScript() && im->GetInstanceScript()->IsEncounterInProgress())
                                return "encounter-in-progress" + iid;
                            Map::PlayerList const& pl = im->GetPlayers();
                            for (Map::PlayerList::const_iterator i = pl.begin(); i != pl.end(); ++i)
                                if (Player* in = i->GetSource())
                                {
                                    if (in->IsGameMaster())
                                        continue;
                                    if (in->GetGroup() != group)
                                        return "instance-holds-other-group" + iid + " first=" + in->GetName();
                                    break;
                                }
                        }
        if (!group || !group->isLFGGroup())
        {
            InstanceSave* save = p->GetInstanceSave(mapId, raid);
            if (!p->CheckInstanceCount(save ? save->GetInstanceId() : 0) && !p->isDead() && diff != DUNGEON_DIFFICULTY_CHALLENGE)
                return "too-many-instances-per-hour";
        }
        if (AccessRequirement const* ar = sObjectMgr->GetAccessRequirement(mapId, diff))
        {
            if (!sWorld->getBoolConfig(CONFIG_INSTANCE_IGNORE_LEVEL))
            {
                if (ar->levelMin && p->GetLevel() < ar->levelMin)
                    return "access_requirement level_min=" + std::to_string(uint32(ar->levelMin)) + " > level " +
                           std::to_string(uint32(p->GetLevel())) + d;
                if (ar->levelMax && p->GetLevel() > ar->levelMax)
                    return "access_requirement level_max=" + std::to_string(uint32(ar->levelMax)) + d;
            }
            if ((ar->item && !p->HasItemCount(ar->item) && (!ar->item2 || !p->HasItemCount(ar->item2))) ||
                (!ar->item && ar->item2 && !p->HasItemCount(ar->item2)))
                return "access_requirement item=" + std::to_string(ar->item ? ar->item : ar->item2) + d;
            uint32 const q = p->GetTeam() == ALLIANCE ? ar->quest_A : ar->quest_H;
            if (q && !p->GetQuestRewardStatus(q))
                return "access_requirement quest=" + std::to_string(q) + d;
            if (ar->achievement)
            {
                Player* leader = group ? ObjectAccessor::FindConnectedPlayer(group->GetLeaderGUID()) : p;
                if (!leader || !leader->HasAchieved(ar->achievement))
                    return "access_requirement achievement=" + std::to_string(ar->achievement) + " (leader)" + d;
            }
        }
        return "unknown" + d;
    }

    // "aiprobe" argument helper (issue #222). The qualifier is the *rest* of the argument string, so
    // it may contain spaces ("self target"); the surrounding double quotes a player types are dropped
    // (client chat keeps them in the raw arg string) and "-" / "_" both mean "no qualifier", for
    // clients that cannot pass an empty argument.
    static std::string AiprobeQualifier(std::istringstream& in)
    {
        std::string rest;
        std::getline(in, rest);
        size_t const first = rest.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return std::string();
        size_t const last = rest.find_last_not_of(" \t\r\n");
        rest = rest.substr(first, last - first + 1);
        if (rest.size() >= 2 && rest.front() == '"' && rest.back() == '"')
            rest = rest.substr(1, rest.size() - 2);
        if (rest == "-" || rest == "_")
            rest.clear();
        return rest;
    }

bool HandleScenarioCommandImpl(ChatHandler* handler, char const* args)
{
    EnsureDuelHarnessScript(); // #201 (#613): idempotent; registers the [DUEL] PlayerScript on first use
    std::istringstream in(args ? args : "");
    std::string name, scenario;
    in >> name >> scenario;

    // "login": brings a specific random bot online for a 1:1 test when this restart's random subset
    // left it offline, so the test no longer depends on hunting for a bot pair that happens to be up
    // on both cores. Handled before the online-player lookup below, since the whole point is that the
    // bot is *not* online yet - unlike goto/wander/attack/stop, which all need a live Player already.
    if (scenario == "login")
    {
        if (name.empty())
        {
            handler->SendSysMessage("Usage: .botscenario <botname> login");
            return false;
        }

        if (ObjectAccessor::FindPlayerByName(name))
        {
            handler->SendSysMessage(("botscenario: " + name + " already online").c_str());
            return true;
        }

        ObjectGuid const guid = sCharacterCache->GetCharacterGuidByName(name);
        if (guid.IsEmpty())
        {
            handler->SendSysMessage(("botscenario: no character named '" + name + "'").c_str());
            return false;
        }

        uint32 const accountId = sCharacterCache->GetCharacterAccountIdByGuid(guid);
        if (!accountId || !sPlayerbotAIConfig->IsInRandomAccountList(accountId))
        {
            handler->SendSysMessage(("botscenario: " + name + " is not a random bot").c_str());
            return false;
        }

        uint32 const guidLow = guid.GetCounter();

        // Same event values RandomPlayerbotMgr::AddRandomBots uses to bring a bot into rotation
        // (RandomPlayerbotMgr.cpp ~line 241: SetEventValue(guid, "add", 1, ...); SetEventValue(guid,
        // "logout", 0, 0);), via the public SetValue wrapper since SetEventValue itself is private to
        // that class. SetValue's validIn is always AiPlayerbot.MaxRandomBotInWorldTime
        // (RandomPlayerbotMgr::SetValue -> SetEventValue(bot, type, value,
        // sPlayerbotAIConfig->maxRandomBotInWorldTime, data)), comfortably above
        // AiPlayerbot.MinRandomBotInWorldTime, so ProcessBot(uint32) never sees "add" go invalid and
        // log this bot straight back out.
        sRandomPlayerbotMgr->SetValue(guidLow, "add", 1);
        sRandomPlayerbotMgr->SetValue(guidLow, "logout", 0);

        // Same call RandomPlayerbotMgr::ProcessBot(uint32) makes to bring an "add"-flagged bot with no
        // live Player online (RandomPlayerbotMgr.cpp ~line 313: AddPlayerBot(botGUID, 0);). Login is
        // asynchronous (queued DB holder + world-thread callback), so this only requests it - the
        // caller polls characters.online afterwards.
        // In the manager's current-bot set first, so IsRandomBot() is true at login and the bot gets the
        // random-bot strategies (without it the bot came online with no "new rpg" and stood still).
        sRandomPlayerbotMgr->AddCurrentBotForTest(guid.GetCounter());
        sRandomPlayerbotMgr->AddPlayerBot(guid, 0);

        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s login requested guid=%u account=%u", name.c_str(), guidLow, accountId);
        handler->SendSysMessage(("botscenario: " + name + " login requested").c_str());
        return true;
    }

    // #201 (#613) (BotScenarioDuel.cpp): duelinfo/duelhp/startduel accept any online player (bot or real).
    if (scenario == "duelinfo" || scenario == "duelhp" || scenario == "startduel")
        return HandleDuelScenarioCommand(handler, name, scenario, in);

    // #793 (BotScenarioSpellWatch.cpp): `.botscenario - spellwatch <id,id,...> on|off | off | list` - server-global spell
    // cast/hit/aura probe ([SPELLWATCH] lines), no player needed.
    if (scenario == "spellwatch")
        return HandleSpellWatchCommand(handler, in);

    // #900/#898: `.botscenario - bf <battleId> start|stop|switch|timer <seconds>|state|ws <id,id,...>` - SOAP-callable twin of
    // the `.bf` GM commands (cs_bf.cpp registers them Console::No; that stays as is). battleId 1 = Wintergrasp, 21 = Tol Barad.
    //   start = StartBattle(); stop = EndBattle(true); switch = EndBattle(false); timer = HandleBattlefieldTimer's body
    //   (SetTimer(sec * IN_MILLISECONDS) + SendInitWorldStatesToAll()); state = one BF line (see below).
    // Capture points are not reachable (Battlefield::m_capturePoints / GetCapturePoint are protected and cs_bf.cpp is core):
    // capturepoints=n/a. players_in_war = players in the zone that sit in a battlefield raid group (GetGroupPlayer).
    // ws: battlefield world states are only sent as packets (Battlefield::SendUpdateWorldState), never stored: n/a.
    if (scenario == "bf")
    {
        uint32 battleId = 0;
        std::string verb;
        in >> battleId >> verb;
        Battlefield* bf = battleId ? sBattlefieldMgr->GetBattlefieldByBattleId(battleId) : nullptr;
        if (!bf || verb.empty())
        {
            handler->SendSysMessage("botscenario: usage: botscenario - bf <battleId> start|stop|switch|timer <seconds>|state|ws <id,..> (no such battlefield or missing verb)");
            return false;
        }
        if (verb == "start")
            bf->StartBattle();
        else if (verb == "stop")
            bf->EndBattle(true);
        else if (verb == "switch")
            bf->EndBattle(false);
        else if (verb == "timer")
        {
            uint32 seconds = 0;
            if (!(in >> seconds))
            {
                handler->SendSysMessage("botscenario: usage: botscenario - bf <battleId> timer <seconds>");
                return false;
            }
            bf->SetTimer(seconds * IN_MILLISECONDS);
            bf->SendInitWorldStatesToAll();
        }
        else if (verb == "ws")
        {
            handler->PSendSysMessage("BF id=%u ws=n/a (battlefield world states are packet-only, not stored server-side)", battleId);
            return true;
        }
        else if (verb != "state")
        {
            handler->SendSysMessage("botscenario: usage: botscenario - bf <battleId> start|stop|switch|timer <seconds>|state|ws <id,..>");
            return false;
        }
        uint32 inZone[2] = { 0, 0 };
        uint32 inWar[2] = { 0, 0 };
        for (auto const& kv : ObjectAccessor::GetPlayers())
        {
            Player* p = kv.second;
            if (!p || !p->IsInWorld() || p->GetZoneId() != bf->GetZoneId())
                continue;
            uint32 const team = p->GetTeamId() == TEAM_HORDE ? 1 : 0;
            ++inZone[team];
            if (bf->GetGroupPlayer(p->GetGUID(), TeamId(team)))
                ++inWar[team];
        }
        handler->PSendSysMessage("BF id=%u enabled=%u war=%u state=%d timer_s=%u defender=%u attacker=%u capturepoints=n/a players_in_war_A=%u _H=%u zone_A=%u _H=%u",
            battleId, bf->IsEnabled() ? 1u : 0u, bf->IsWarTime() ? 1u : 0u, int32(bf->GetState()), bf->GetTimer() / IN_MILLISECONDS,
            uint32(bf->GetDefenderTeam()), uint32(bf->GetAttackerTeam()), inWar[0], inWar[1], inZone[0], inZone[1]);
        TC_LOG_DEBUG("playerbots", "[SCENARIO] bf %u %s", battleId, verb.c_str());
        return true;
    }

    // #736(c) (BgTacticsSA.cpp DriveAllowed, beyond AC): `.botscenario - sotadrive <0-100>|off` - server-global override
    // of AiPlayerbot.BgSotaVehicleDriveChance for the SotA leg (0 = no bot drives, 100 = every demolisher may get one);
    // off = back to the config key. MoP only (AC has no SotA tactics).
    if (scenario == "sotadrive")
    {
        std::string arg;
        in >> arg;
        int32 pct = -1;
        if (arg != "off")
        {
            char* end = nullptr;
            long const value = std::strtol(arg.c_str(), &end, 10);
            if (arg.empty() || !end || *end || value < 0 || value > 100)
            {
                handler->SendSysMessage("botscenario: usage: botscenario - sotadrive <0-100>|off");
                return false;
            }
            pct = int32(value);
        }
        SotaBeyondAc::SetDriveChanceOverride(pct);
        TC_LOG_DEBUG("playerbots", "[SCENARIO] sotadrive override=%d chance=%u", SotaBeyondAc::DriveChanceOverride(),
            SotaBeyondAc::EffectiveDriveChance());
        handler->PSendSysMessage("botscenario: sotadrive override=%d chance=%u", SotaBeyondAc::DriveChanceOverride(),
            SotaBeyondAc::EffectiveDriveChance());
        return true;
    }

    // Wave-29 arena legs (#391/#709/#437/#632) - arena-specific verbs, same text on both cores:
    //   `.botscenario - arenapin <bgTypeId|0>`: every "All Arenas" draw on this server returns that arena (0 = off).
    //       Server-global, so arena legs on one core run one at a time and end with `arenapin 0`.
    //   `.botscenario <name> bgstatus`: CMSG_BATTLEFIELD_STATUS from <name>'s own session (what a client's PvP frame
    //       sends), so the server answers with that player's queue status packets - the #632 WAIT_QUEUE path.
    if (scenario == "arenapin")
    {
        uint32 typeId = 0;
        in >> typeId;
        if (!typeId)
        {
            g_harnessArenaPinTypeId = 0;
            TC_LOG_DEBUG("playerbots", "[SCENARIO] arenapin off");
            handler->SendSysMessage("botscenario: arenapin off");
            return true;
        }
        Battleground* arena = typeId == uint32(BATTLEGROUND_AA) ? nullptr
            : sBattlegroundMgr->GetBattlegroundTemplate(BattlegroundTypeId(typeId));
        if (!arena || !arena->IsArena())
        {
            handler->PSendSysMessage("botscenario: arenapin %u refused - no arena template with that id on this server "
                "(disabled or not an arena)", typeId);
            return false;
        }
        g_harnessArenaPinTypeId = typeId;
        TC_LOG_DEBUG("playerbots", "[SCENARIO] arenapin %u map=%u", typeId, arena->GetMapId());
        handler->PSendSysMessage("botscenario: arenapin %u map=%u (All Arenas draws pinned)", typeId, arena->GetMapId());
        return true;
    }
    if (scenario == "bgstatus")
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target || !target->GetSession())
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }
        uint32 queues = 0;
        for (uint8 slot = 0; slot < PLAYER_MAX_BATTLEGROUND_QUEUES; ++slot)
            if (target->GetBattlegroundQueueTypeId(slot) != BATTLEGROUND_QUEUE_NONE)
                ++queues;
        WorldPacket packet(CMSG_BATTLEFIELD_STATUS);
        target->GetSession()->HandleBattlefieldStatusOpcode(packet);
        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s bgstatus queues=%u", name.c_str(), queues);
        handler->PSendSysMessage("botscenario: bgstatus sent for %s (queues=%u)", name.c_str(), queues);
        return true;
    }

    // #830 (MoP-only, BotScenarioQueue.cpp): lfgreward - the random-dungeon reward LFGMgr would give for an LFG entry.
    // #831 (MoP-only, BotScenarioQueue.cpp): pvpscale - a synthetic hit/heal through the damage scripts (any-level PvP scaling).
    if (scenario == "lfgreward" || scenario == "pvpscale")
        return HandleQueueScenarioCommand(handler, name, scenario, in);

    // #210 S10 P5 (BotScenarioQueue.cpp): spec/bgtest/bgend/lfgstate/queuegate/clearqueue/leavegroup/
    // unaura/bgqueueinfo. Each does its own player lookup, so this needs nothing resolved by either
    // the block above or the botAI gate below.
    if (scenario == "spec" || scenario == "bgtest" || scenario == "bgend" || scenario == "lfgstate" ||
        scenario == "queuegate" || scenario == "clearqueue" || scenario == "leavegroup" || scenario == "lfgjoin" || scenario == "lfgleave" || scenario == "role" ||
        scenario == "acceptinvite" || scenario == "unbind" || scenario == "instinfo" || scenario == "unaura" || scenario == "bgqueueinfo" ||
        scenario == "additem" || scenario == "strategy" || scenario == "imbueinfo" || scenario == "dismount" || scenario == "setfly" ||
        scenario == "hasitem" || scenario == "questreward" || scenario == "bosshp" || scenario == "abinfo" || scenario == "useitem" || scenario == "despawnnear" || scenario == "encounter" || scenario == "engage" ||
        scenario == "cansee" ||  // #519
        scenario == "dgstate" || scenario == "dgdropcart" || scenario == "dgpick" || scenario == "dgtake" || scenario == "ratedbg")  // #895 ratedbg; #797 (MoP-only Deepwind Gorge cart verbs, BotScenarioQueue.cpp)
        return HandleQueueScenarioCommand(handler, name, scenario, in);

    // --- S1b (issue #172): harness verbs that accept a non-bot (real-socket) player. These run
    // BEFORE the botAI gate below because they operate on any online character, bot or real, and the
    // headless client's Python side reads their reply straight out of the SOAP HTTP response body
    // (SendSysMessage on a console ChatHandler), no chat round-trip needed.
    // w19 (#746): `.botscenario <name[,name...]> save` saves only these characters. .saveall saved all 1000 players
    // per call (~163k statements on MoP, ~95 s of the single CharacterDatabase worker) and the harness polled it,
    // which timed out bot and real-player logins. Offline names are counted and skipped.
    if (scenario == "save")
    {
        uint32 saved = 0, missing = 0;
        std::istringstream names(name);
        std::string one;
        while (std::getline(names, one, ','))
        {
            if (Player* target = one.empty() ? nullptr : ObjectAccessor::FindPlayerByName(one))
            {
                target->SaveToDB(false);
                ++saved;
            }
            else
                ++missing;
        }
        handler->PSendSysMessage("botscenario: saved %u (not online %u)", saved, missing);
        return true;
    }

    if (scenario == "npcguid" || scenario == "npcpos" || scenario == "goguid" || scenario == "nearcorpse" || scenario == "kill" ||
        scenario == "board" || scenario == "equip" || scenario == "randomize" || scenario == "learn" || scenario == "spawngo" || scenario == "unlearn" || scenario == "refresh" || scenario == "masterprobe" ||
        scenario == "teleport" || scenario == "bossstate" || scenario == "transportinfo" || scenario == "questadd" ||
        scenario == "pvp" || scenario == "sethp" || scenario == "setmana" || scenario == "castself" || scenario == "addaura" || scenario == "npcaura" || scenario == "npccast" || scenario == "hasspell" ||
        scenario == "npcstate" || scenario == "threatinfo" || scenario == "talentinfo" || scenario == "spellcount" || scenario == "rpgtarget")
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        // #448: "npcpos <entry>": position of the nearest live creature of that entry (200 yd), for drivers that must follow
        // an escort NPC (t438 L08 shadow_miner adaptive mode).
        if (scenario == "npcpos")
        {
            uint32 entry = 0;
            in >> entry;
            Creature* c = entry ? target->FindNearestCreature(entry, 200.0f, true) : nullptr;
            if (!c)
            {
                handler->PSendSysMessage("botscenario: npcpos entry=%u none", entry);
                return false;
            }
            // #170: vehbase = raw guid of the vehicle the creature is seated on (0 = not a passenger), so a driver can tell a
            // seated accessory from a stale one standing at the old seat spot.
            Unit* const vehBase = c->GetVehicleBase();
            handler->PSendSysMessage("botscenario: npcpos entry=%u guid=%llu x=%.1f y=%.1f z=%.1f vehbase=%llu vehalive=%d", entry,
                (unsigned long long)c->GetGUID().GetRawValue(), c->GetPositionX(), c->GetPositionY(), c->GetPositionZ(),
                (unsigned long long)(vehBase ? vehBase->GetGUID().GetRawValue() : 0), (vehBase && vehBase->IsAlive()) ? 1 : 0);
            return true;
        }

        if (scenario == "npcguid")
        {
            uint32 entry = 0;
            in >> entry;
            if (!entry)
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> npcguid <entry>");
                return false;
            }
            Creature* found = target->FindNearestCreature(entry, 100.0f, true);
            if (!found)
            {
                handler->PSendSysMessage("botscenario: npcguid entry=%u guid=0 (none within 100y of %s)", entry, name.c_str());
                return false;
            }
            handler->PSendSysMessage("botscenario: npcguid entry=%u guid=%llu",
                entry, (unsigned long long)found->GetGUID().GetRawValue());
            return true;
        }

        // #708 "goguid <entry> [range]": raw guid of the nearest spawned GameObject of that entry (default 8 yd), the
        // GameObject twin of npcguid. The bomb-summoned Kaja'mite Chunks 195492 are dynamic spawns (no DB guid), so a
        // headless leg can only learn their guid here; headless.py caches it as "go" for `cast 6478 go`.
        if (scenario == "goguid")
        {
            uint32 entry = 0;
            float range = 8.0f;
            in >> entry;
            if (!(in >> range) || range <= 0.0f)
                range = 8.0f;
            if (!entry)
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> goguid <entry> [range]");
                return false;
            }
            GameObject* foundGo = target->FindNearestGameObject(entry, range);
            if (!foundGo)
            {
                handler->PSendSysMessage("botscenario: goguid entry=%u guid=0 (none within %.1fy of %s)", entry, range, name.c_str());
                return false;
            }
            handler->PSendSysMessage("botscenario: goguid entry=%u guid=%llu dist=%.1f",
                entry, (unsigned long long)foundGo->GetGUID().GetRawValue(), target->GetDistance(foundGo));
            return true;
        }

        if (scenario == "nearcorpse")
        {
            Corpse* corpse = target->GetCorpse();
            if (!corpse)
            {
                handler->PSendSysMessage("botscenario: nearcorpse guid=0 (%s has no corpse)", name.c_str());
                return false;
            }
            handler->PSendSysMessage("botscenario: nearcorpse guid=%llu", (unsigned long long)corpse->GetGUID().GetRawValue());
            return true;
        }

        // #636 "castself <spellId>": the player casts the spell on itself, triggered (ignores reagents/cost).
        // Neither headless client can cast, and SOAP cannot cast as a player; used for the Oculus drake
        // (Amber Essence 49461) on the headless master.
        // #521/#388/#382/#522 "addaura <spellId>": put the spell's aura on the character directly (Unit::AddAura, no cast).
        // castself cannot apply a hostile debuff to oneself (Serpent Sting, Curse of the Elements, Faerie Fire, Fear
        // all came back aura=0 on both cores in wave 30), so the cure/break-fear legs set their debuff with this.
        if (scenario == "addaura")
        {
            uint32 spellId = 0;
            in >> spellId;
            bool const ok = !in.fail() && spellId && sSpellMgr->GetSpellInfo(spellId) && target->AddAura(spellId, target);
            handler->PSendSysMessage("botscenario: addaura %s spell=%u %s", name.c_str(), spellId, ok ? "ok" : "failed");
            return ok;
        }

        // #894 "npcaura <entry> <spellId> [range=60] [remove]": put (or remove) a spell aura directly on the nearest creature of
        // <entry> (Unit::AddAura, no cast, so no combat): roots/stuns a waypoint patroller out of combat for the stall test.
        // #680 "npccast <entry> <spellId> [triggered] [range=100]": the nearest live creature of <entry> casts the spell on the
        // named player (LCT: force Husam's Bad Intentions 83113 onto the headless master, which never enters the threat list).
        if (scenario == "npccast")
        {
            uint32 entry = 0, spellId = 0;
            float range = 100.0f;
            bool triggered = false;
            std::string arg;
            in >> entry >> spellId;
            while (in >> arg)
            {
                if (arg == "triggered")
                    triggered = true;
                else if (arg.rfind("range=", 0) == 0)
                    range = float(atof(arg.c_str() + 6));
            }
            Creature* npc = (entry && spellId && sSpellMgr->GetSpellInfo(spellId)) ? target->FindNearestCreature(entry, range) : nullptr;
            if (!npc)
            {
                handler->PSendSysMessage("botscenario: npccast entry=%u spell=%u none alive within %.0f of %s", entry, spellId, range, name.c_str());
                return false;
            }
            // #680 (w106): the second proof cast hit a dead master (SPELL_FAILED_TARGETS_DEAD) and still answered as sent;
            // CastSpell returns nothing, so refuse the cases that cannot work instead of reporting them as a cast.
            if (!target->IsAlive() || !npc->IsAlive())
            {
                handler->PSendSysMessage("botscenario: npccast entry=%u spell=%u refused (%s dead)", entry, spellId,
                    !target->IsAlive() ? name.c_str() : "caster");
                return false;
            }
            npc->CastSpell(target, spellId, triggered);
            handler->PSendSysMessage("botscenario: npccast entry=%u spell=%u on %s %s guid=%llu dist=%.1f", entry, spellId, name.c_str(),
                triggered ? "triggered" : "normal", (unsigned long long)npc->GetGUID().GetRawValue(), target->GetDistance(npc));
            return true;
        }

        if (scenario == "npcaura")
        {
            uint32 entry = 0, spellId = 0;
            float range = 60.0f;
            std::string arg;
            bool remove = false;
            in >> entry >> spellId;
            while (in >> arg)
            {
                if (arg == "remove")
                    remove = true;
                else if (arg.rfind("range=", 0) == 0)
                    range = float(atof(arg.c_str() + 6));
            }
            Creature* npc = (entry && spellId && sSpellMgr->GetSpellInfo(spellId)) ? target->FindNearestCreature(entry, range) : nullptr;
            if (!npc)
            {
                handler->PSendSysMessage("botscenario: npcaura entry=%u spell=%u none within %.0f of %s", entry, spellId, range, name.c_str());
                return false;
            }
            if (remove)
                npc->RemoveAurasDueToSpell(spellId);
            else
                npc->AddAura(spellId, npc);
            handler->PSendSysMessage("botscenario: npcaura entry=%u spell=%u %s guid=%llu pos=%.1f,%.1f,%.1f aura=%u rooted=%u stunned=%u combat=%u",
                entry, spellId, remove ? "removed" : "added", (unsigned long long)npc->GetGUID().GetRawValue(), npc->GetPositionX(),
                npc->GetPositionY(), npc->GetPositionZ(), npc->HasAura(spellId) ? 1u : 0u, npc->HasUnitState(UNIT_STATE_ROOT) ? 1u : 0u,
                npc->HasUnitState(UNIT_STATE_STUNNED) ? 1u : 0u, npc->IsInCombat() ? 1u : 0u);
            return true;
        }

        if (scenario == "castself")
        {
            uint32 spellId = 0;
            in >> spellId;
            bool const sp = !in.fail() && spellId && sSpellMgr->GetSpellInfo(spellId);
            if (sp)
                target->CastSpell(target, spellId, true);
            handler->PSendSysMessage("botscenario: castself %s spell=%u %s", name.c_str(), spellId, sp ? "ok" : "failed");
            return sp;
        }

        if (scenario == "kill")
        {
            if (target->IsAlive())
                target->Kill(target);
            handler->SendSysMessage(("botscenario: killed " + name).c_str());
            return true;
        }

        // #527 test seam: "sethp <pct>" / "setmana <pct>" set current health / mana to pct (0-100) of
        // the maximum on a live character. A bot's login refresh refills it, so a food/drink 1:1 must
        // lower hp/mana AFTER login on both cores; no SOAP command does that. sethp never goes below
        // 1 (it does not kill: use "kill") and is refused on a dead character; setmana is refused when
        // the character has no mana pool. Same reply text on both trees.
        // hasspell <id> (#624, read-only, same reply on both trees): does the character know the spell / carry the aura.
        if (scenario == "hasspell")
        {
            uint32 id = 0;
            if (!(in >> id) || !id)
            {
                handler->PSendSysMessage("botscenario: usage: botscenario <player> hasspell <spellId>");
                return false;
            }
            handler->PSendSysMessage("botscenario: %s hasspell %u spell=%u aura=%u", name.c_str(), id, uint32(target->HasSpell(id)), uint32(target->HasAura(id)));
            return true;
        }

        // lane G2 (read-only probes, identical reply text on both cores):
        //  npcstate <entry> [range=100]   one line per live creature of that entry (#469, #453)
        //  threatinfo <entry|guid> [range]  the creature's threat list (#500)
        //  talentinfo / spellcount          (#533)   rpgtarget get|clear|set <guid>  (#609)
        if (scenario == "npcstate")
        {
            uint32 entry = 0;
            float range = 100.0f;
            in >> entry;
            if (!(in >> range) || range <= 0.0f)
                range = 100.0f;
            if (!entry)
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> npcstate <entry> [range=100]");
                return false;
            }
            std::list<Creature*> found;
            target->GetCreatureListWithEntryInGrid(found, entry, range);
            char buf[512];
            std::snprintf(buf, sizeof(buf), "botscenario: npcstate entry=%u range=%.0f count=%u", entry, range, uint32(found.size()));
            handler->SendSysMessage(buf);
            uint32 shown = 0;
            for (Creature* c : found)
            {
                if (++shown > 40)
                    break;
                Unit* v = c->GetVictim();
                std::snprintf(buf, sizeof(buf),
                    "botscenario: npcstate guid=%llu spawnId=%llu x=%.1f y=%.1f z=%.1f alive=%d hp=%llu/%llu incombat=%d evade=%d victim=%u/%llu",
                    (unsigned long long)c->GetGUID().GetRawValue(), (unsigned long long)c->GetSpawnId(), c->GetPositionX(),
                    c->GetPositionY(), c->GetPositionZ(), c->IsAlive() ? 1 : 0, (unsigned long long)c->GetHealth(),
                    (unsigned long long)c->GetMaxHealth(), c->IsInCombat() ? 1 : 0, c->IsInEvadeMode() ? 1 : 0,
                    v ? uint32(v->GetEntry()) : 0u, (unsigned long long)(v ? v->GetGUID().GetRawValue() : 0));
                handler->SendSysMessage(buf);
            }
            return true;
        }

        if (scenario == "threatinfo")
        {
            unsigned long long arg = 0;
            float range = 100.0f;
            in >> arg;
            if (!(in >> range) || range <= 0.0f)
                range = 100.0f;
            if (!arg)
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> threatinfo <entry|guid> [range]");
                return false;
            }
            Creature* c = arg > 0xFFFFFFFFull ? ObjectAccessor::GetCreature(*target, ObjectGuid(uint64(arg)))
                                              : target->FindNearestCreature(uint32(arg), range, true);
            if (!c)
            {
                char nb[128];
                std::snprintf(nb, sizeof(nb), "botscenario: threatinfo arg=%llu none", arg);
                handler->SendSysMessage(nb);
                return false;
            }
            char buf[512];
            ThreatManager& mgr = c->GetThreatManager();
            std::snprintf(buf, sizeof(buf), "botscenario: threatinfo entry=%u guid=%llu size=%u", uint32(c->GetEntry()),
                (unsigned long long)c->GetGUID().GetRawValue(), uint32(mgr.GetThreatListSize()));
            handler->SendSysMessage(buf);
            uint32 shown = 0;
            for (ThreatReference const* ref : mgr.GetUnsortedThreatList())
            {
                if (++shown > 40)
                    break;
                Unit* who = ref->GetVictim();
                std::snprintf(buf, sizeof(buf), "botscenario: threatinfo ref name=%s guid=%llu threat=%.1f online=%d",
                    who ? who->GetName().c_str() : "?", (unsigned long long)(who ? who->GetGUID().GetRawValue() : 0),
                    ref->GetThreat(), ref->IsOnline() ? 1 : 0);
                handler->SendSysMessage(buf);
            }
            return true;
        }

        // talentinfo: free = unspent points, used = spent (MoP: Player::GetUsedTalentCount; AC: talent entries known in the
        // active spec), spec = MoP specialization id (AC 3.3.5 has none: 0), activespec = dual-spec index, ids = talent ids
        // (MoP PlayerTalentMap key = talent id; AC key = talent spell id).
        if (scenario == "talentinfo")
        {
            std::string ids;
            uint32 n = 0;
            if (PlayerTalentMap const* tm = target->GetTalentMap(target->GetActiveSpec()))
                for (auto const& kv : *tm)
                {
                    if (!kv.second || kv.second->state == PLAYERSPELL_REMOVED)
                        continue;
                    ++n;
                    ids += (ids.empty() ? "" : ",") + std::to_string(kv.first);
                }
            uint32 const usedP = target->GetUsedTalentCount();
            uint32 const allP = target->CalculateTalentsPoints();
            uint32 const freeP = allP > usedP ? allP - usedP : 0;
            uint32 const specId = uint32(target->GetSpecialization());
            char buf[1024];
            std::snprintf(buf, sizeof(buf), "botscenario: %s talentinfo free=%u used=%u spec=%u activespec=%u talents=%u ids=%s",
                name.c_str(), freeP, usedP, specId, uint32(target->GetActiveSpec()), n, ids.c_str());
            handler->SendSysMessage(buf);
            return true;
        }

        // spellcount: total = spellbook entries not removed; active = of those, shown in the spellbook (Active flag);
        // nonpassive = of those, not passive (what #533 compares).
        if (scenario == "spellcount")
        {
            uint32 total = 0, active = 0, nonpassive = 0;
            for (auto const& kv : target->GetSpellMap())
            {
                if (!kv.second || kv.second->state == PLAYERSPELL_REMOVED)
                    continue;
                ++total;
                if (!kv.second->active)
                    continue;
                ++active;
                SpellInfo const* si = sSpellMgr->GetSpellInfo(kv.first);
                if (si && !si->IsPassive())
                    ++nonpassive;
            }
            char buf[256];
            std::snprintf(buf, sizeof(buf), "botscenario: %s spellcount total=%u active=%u nonpassive=%u", name.c_str(), total, active, nonpassive);
            handler->SendSysMessage(buf);
            return true;
        }

        if (scenario == "rpgtarget")
        {
            std::string op;
            in >> op;
            PlayerbotAI* rai = GET_PLAYERBOT_AI(target);
            if (!rai)
            {
                handler->SendSysMessage(("botscenario: " + name + " rpgtarget refused (not a bot)").c_str());
                return false;
            }
            GuidPosition cur = rai->GetAiObjectContext()->GetValue<GuidPosition>("rpg target")->Get();
            if (op == "clear")
                rai->GetAiObjectContext()->GetValue<GuidPosition>("rpg target")->Set(GuidPosition());
            else if (op == "set")
            {
                unsigned long long raw = 0;
                in >> raw;
                WorldObject* wo = raw ? ObjectAccessor::GetWorldObject(*target, ObjectGuid(uint64(raw))) : nullptr;
                if (!wo)
                {
                    char nb[160];
                    std::snprintf(nb, sizeof(nb), "botscenario: %s rpgtarget set guid=%llu not found", name.c_str(), raw);
                    handler->SendSysMessage(nb);
                    return false;
                }
                rai->GetAiObjectContext()->GetValue<GuidPosition>("rpg target")->Set(GuidPosition(wo));
            }
            else if (op != "get")
            {
                handler->SendSysMessage("botscenario: usage: botscenario <bot> rpgtarget get|clear|set <guid>");
                return false;
            }
            if (op != "get")
                cur = rai->GetAiObjectContext()->GetValue<GuidPosition>("rpg target")->Get();
            char buf[384];
            std::snprintf(buf, sizeof(buf), "botscenario: %s rpgtarget %s guid=%llu entry=%u map=%u x=%.1f y=%.1f z=%.1f", name.c_str(),
                op.c_str(), (unsigned long long)cur.GetRawValue(), uint32(cur.GetEntry()), uint32(cur.getMapId()), cur.getX(), cur.getY(), cur.getZ());
            handler->SendSysMessage(buf);
            return true;
        }

        if (scenario == "sethp" || scenario == "setmana")
        {
            bool const hp = scenario == "sethp";
            int32 pct = -1;
            if (!(in >> pct) || pct < 0 || pct > 100)
            {
                handler->PSendSysMessage("botscenario: usage: botscenario <player> %s <pct 0-100>", scenario.c_str());
                return false;
            }
            if (!target->IsAlive())
            {
                handler->PSendSysMessage("botscenario: %s %s refused (dead)", name.c_str(), scenario.c_str());
                return false;
            }
            if (hp)
            {
                uint32 const maxHp = uint32(target->GetMaxHealth());
                uint32 val = uint32(uint64(maxHp) * uint32(pct) / 100);
                if (val < 1)
                    val = 1;
                target->SetHealth(val);
                handler->PSendSysMessage("botscenario: %s sethp %d hp=%u/%u", name.c_str(), pct,
                    uint32(target->GetHealth()), uint32(target->GetMaxHealth()));
                return true;
            }
            uint32 const maxMana = uint32(target->GetMaxPower(POWER_MANA));
            if (!maxMana)
            {
                handler->PSendSysMessage("botscenario: %s setmana refused (no mana)", name.c_str());
                return false;
            }
            target->SetPower(POWER_MANA, uint32(uint64(maxMana) * uint32(pct) / 100));
            handler->PSendSysMessage("botscenario: %s setmana %d mana=%u/%u", name.c_str(), pct,
                uint32(target->GetPower(POWER_MANA)), uint32(target->GetMaxPower(POWER_MANA)));
            return true;
        }

        // #376 test seam: this realm is PvE, so a bot teleported into an outdoor-PvP zone does not
        // auto-flag (Player::pvpInfo.IsInHostileArea is false outside a PvP realm / capital /
        // Wintergrasp - Player.cpp:8006-8016) and no harness verb could set the flag. "pvp" (default)
        // and "pvp off" drive UpdatePvP/DisablePvP, exactly what the core does at Player.cpp:24340-24348.
        if (scenario == "pvp")
        {
            std::string token;
            in >> token;
            bool const enable = token != "off";
            if (enable)
                target->UpdatePvP(true);
            else
                target->DisablePvP();
            handler->SendSysMessage(("botscenario: pvp " + name + (enable ? " on" : " off")).c_str());
            return true;
        }

        // "teleport <map> <x> <y> <z>": place any online character (a real test player too) at an exact
        // point server-side. Headless client walking can be snapped back by anticheat (AC) or be imprecise,
        // so tests that need the player inside a trigger box use this (#206 T9).
        // #569 w31b: "bossstate <id> <state>" sets an encounter state of the player's instance (InstanceScript::SetBossState,
        // EncounterState 0 NOT_STARTED .. 3 DONE). For a leg that pulls a boss whose script requires an earlier encounter
        // done (fork HoS Sjonnir: CheckRequiredBosses needs DATA_BRANN_EVENT=2 DONE, instance_halls_of_stone.cpp:173-189,
        // else boss_sjonnir.cpp:90-94 evades). Same id and step on both cores.
        if (scenario == "bossstate")
        {
            uint32 id = 0, st = 0;
            in >> id >> st;
            InstanceScript* inst = target->GetInstanceScript();
            if (in.fail() || !inst || st > uint32(TO_BE_DECIDED))
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> bossstate <id> <state 0-5> (player in an instance)");
                return false;
            }
            uint32 const before = uint32(inst->GetBossState(id));
            bool const ok = inst->SetBossState(id, EncounterState(st));
            handler->PSendSysMessage("botscenario: bossstate %s id=%u %u -> %u (%s, now %u)", target->GetName().c_str(), id, before, st,
                ok ? "set" : "unchanged", uint32(inst->GetBossState(id)));
            return true;
        }

        if (scenario == "teleport")
        {
            uint32 mapId = 0;
            float x = 0.0f, y = 0.0f, z = 0.0f;
            in >> mapId >> x >> y >> z;
            if (in.fail())
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> teleport <map> <x> <y> <z>");
                return false;
            }
            // #811: grouped + far + dungeon -> the group's difficulty first (AlignGroupDifficulty), and a refusal names its gate
            // (TeleportFailReason), on the reply's first line, which the dungeon driver prints as "TELEPORT FAILED <bot>: ...".
            std::string const aligned = AlignGroupDifficulty(target, mapId);
            bool const ok = target->TeleportTo(mapId, x, y, z, target->GetOrientation());
            std::string const why = ok ? std::string() : " reason=" + TeleportFailReason(target, mapId, x, y, z);
            handler->PSendSysMessage("botscenario: teleport %s map=%u (%.2f, %.2f, %.2f) %s%s%s", name.c_str(), mapId, x, y, z,
                                     ok ? "ok" : "failed", aligned.c_str(), why.c_str());
            return ok;
        }

        if (scenario == "board")
        {
            uint32 entry = 0;
            in >> entry;
            uint32 const dbGuid = entry ? DbTransportGuid(entry) : 0;
            Transport* transport = dbGuid ? ResolveTransport(target, dbGuid) : nullptr;
            if (!transport)
            {
                handler->PSendSysMessage("botscenario: board: no live transport with entry %u", entry);
                return false;
            }

            // S9 T7 (issue #206): optional transport-local deck offset, "board <entry> [lx ly lz]".
            // Without it, behaviour is unchanged: the target lands at the transport's own origin,
            // which for a hollow model (e.g. the Stranglethorn turtle) is inside the hull, not on
            // the deck.
            float lx = 0.0f, ly = 0.0f, lz = 0.0f;
            bool const hasOffset = static_cast<bool>(in >> lx >> ly >> lz);
            if (!hasOffset)
                lx = ly = lz = 0.0f;

            float x = lx, y = ly, z = lz, o = 0.0f;
            if (hasOffset)
                // Local deck offset -> world position, the same transform
                // Transport::UpdatePassengerPositions uses every tick.
                transport->CalculatePassengerPosition(x, y, z, &o);
            else
                transport->GetPosition(x, y, z, o);

            if (target->GetMapId() != transport->GetMapId())
                target->TeleportTo(transport->GetMapId(), x, y, z, o);
            else
                target->NearTeleportTo(x, y, z, o);

            target->SetTransport(transport);
            if (hasOffset)
            {
                // Set BEFORE AddPassenger, so UpdatePassengerPositions holds the target on the deck
                // from the very first tick instead of at local (0,0,0). AddPassenger's withAll=false
                // branch (Transport.cpp ~582-589) sets the transport guid itself but never writes
                // transport.pos, which is the bug this offset works around.
                target->m_movementInfo.transport.guid = transport->GetGUID();
                target->m_movementInfo.transport.pos.Relocate(lx, ly, lz, 0.0f);
            }
            transport->AddPassenger(target);
            handler->PSendSysMessage("botscenario: board entry=%u guid=%llu world=(%.2f,%.2f,%.2f) local=(%.2f,%.2f,%.2f)",
                entry, (unsigned long long)transport->GetGUID().GetRawValue(), x, y, z, lx, ly, lz);
            return true;
        }

        // S9 (#206) walk-on tooling: a read-only transport-pose query for a driver polling until
        // a platform is docked - no teleport, no SetTransport/AddPassenger on the target, unlike
        // `board` above. Tries the moving-transport path first (DbTransportGuid/ResolveTransport,
        // the same pair `board` uses), then falls back to a live grid search for a static
        // transport (elevator, GAMEOBJECT_TYPE_TRANSPORT): those are never in the `transports`
        // world DB table and have no per-map spawn-id index (a local/static transport's
        // GameObject is never loaded through the normal DB-spawn path, so it never gets a
        // spawn-id the way a persistent GameObject does), so a range search anchored at the
        // target is the only way to find one.
        if (scenario == "transportinfo")
        {
            uint32 entry = 0;
            in >> entry;
            if (!entry)
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> transportinfo <entry> [lx ly lz]");
                return false;
            }

            uint32 const dbGuid = DbTransportGuid(entry);
            auto* movingTransport = dbGuid ? ResolveTransport(target, dbGuid) : nullptr;
            Transport* transport = movingTransport;
            bool isStatic = false;
            if (!transport)
            {
                // Grid search can never see a static transport (see FindStaticTransport's comment
                // above) - walk the map's own object store instead.
                if (Transport* t = FindStaticTransport(target, entry))
                {
                    transport = t;
                    isStatic = true;
                }
            }
            if (!transport)
            {
                handler->PSendSysMessage("botscenario: transportinfo: no live transport with entry %u near %s", entry, name.c_str());
                return false;
            }

            // Optional transport-local offset (same math as `board`'s [lx ly lz]): reports the
            // world point of a chosen deck spot without touching the target at all.
            float lx = 0.0f, ly = 0.0f, lz = 0.0f;
            bool const hasOffset = static_cast<bool>(in >> lx >> ly >> lz);
            float x = lx, y = ly, z = lz, o = 0.0f;
            if (hasOffset)
                transport->CalculatePassengerPosition(x, y, z, &o);
            else
                transport->GetPosition(x, y, z, o);

            // Docked/progress state: moving transports (boats/zeppelins) have arrive/depart
            // keyframe windows (DockedAt, used by the `transport` scenario above); static ones
            // (elevators) don't - report the GameObject's go-state instead.
            char stateBuf[96] = "-";
            if (!isStatic && movingTransport)
            {
                bool anyDocked = false;
                auto const& frames = movingTransport->GetKeyFrames();
                for (uint32 i = 0; i < frames.size() && !anyDocked; ++i)
                {
                    uint32 leftMs = 0;
                    if (DockedAt(movingTransport, i, leftMs))
                    {
                        snprintf(stateBuf, sizeof(stateBuf), "docked=1 node=%u leftms=%u", NodeIdx(frames[i]), leftMs);
                        anyDocked = true;
                    }
                }
                if (!anyDocked)
                    snprintf(stateBuf, sizeof(stateBuf), "docked=0");
            }
            else
            {
                snprintf(stateBuf, sizeof(stateBuf), "gostate=%d progress=%u", int(transport->GetGoState()),
                    transport->GetGOValue()->Transport.PathProgress);
            }

            // Animation period the core uses for the path-progress modulus (elevator cycle length).
            uint32 const periodMs = transport->GetTransportPeriod();
            size_t const stateLen = strlen(stateBuf);
            if (periodMs && stateLen + 24 < sizeof(stateBuf))
                snprintf(stateBuf + stateLen, sizeof(stateBuf) - stateLen, " period=%u", periodMs);

            handler->PSendSysMessage("botscenario: transportinfo entry=%u guid=%llu kind=%s map=%u world=(%.2f,%.2f,%.2f,%.2f) %s",
                entry, (unsigned long long)transport->GetGUID().GetRawValue(), isStatic ? "static" : "moving",
                transport->GetMapId(), x, y, z, o, stateBuf);
            return true;
        }

        if (scenario == "equip")
        {
            std::string set;
            in >> set;
            uint32 quality = 2; // uncommon/green: a deterministic mid-tier default
            if (set == "blue")
                quality = 3;
            else if (set == "epic" || set == "purple")
                quality = 4;
            else if (set == "green" || set.empty())
                quality = 2;
            else
                quality = uint32(atoi(set.c_str()));
            // #641: bags first (AC bots always carry 4 x 51809 from InitBags; MoP bots randomized before that port have
            // none, so a full backpack refuses every conjured/looted item). Same call on both trees; no-op when filled.
            EnsureStageBags(target);
            // #698: `equip <quality> [nocap]`; nocap drops the WotLK (level <= 80) / Cataclysm (81-85, #373) gear-score cap.
            std::string capArg;
            in >> capArg;
            uint32 const gsCap = capArg == "nocap" ? 0 : WotlkGearScoreCap(target->GetLevel(), quality);
            BotFactory factory(target, target->GetLevel(), quality, gsCap);
            factory.InitEquipment(true);
            handler->PSendSysMessage("botscenario: %s equipped (quality %u) gscap=%u", name.c_str(), quality, gsCap);
            return true;
        }

        // #592 `randomize <level>`: re-initialise a relevelled bot at <level> (gear, talents, spells, pet)
        // the way the random-bot randomize does (RandomPlayerbotMgr::Randomize full path), then push its
        // next scheduled randomize out so login maintenance does not redo it.
        if (scenario == "randomize")
        {
            uint32 lvl = 0;
            if (!(in >> lvl) || lvl < 1 || lvl > 90)
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> randomize <level>");
                return false;
            }
            target->GiveLevel(lvl);
            // #698: `randomize <level> [nocap]`. Same quality and score limit as BotFactory's config defaults
            // (BotFactory.cpp:51-57), plus the WotLK (level <= 80) / Cataclysm (81-85, #373) gear-score cap unless nocap is given.
            std::string capArg;
            in >> capArg;
            uint32 const quality = uint32(sConfigMgr->GetIntDefault("AiPlayerbot.RandomGearQualityLimit", 3));
            uint32 const cfgScore = uint32(sConfigMgr->GetIntDefault("AiPlayerbot.RandomGearScoreLimit", 0));
            uint32 gsLimit = cfgScore == 0 ? 0 : BotFactory::CalcMixedGearScore(cfgScore, quality);
            uint32 const gsCap = capArg == "nocap" ? 0 : WotlkGearScoreCap(lvl, quality);
            if (gsCap && (!gsLimit || gsCap < gsLimit))
                gsLimit = gsCap;
            BotFactory factory(target, lvl, quality, gsLimit);
            factory.Randomize(false);
            // push the next login/periodic randomize far out so it cannot lift the bot back (#592)
            sRandomPlayerbotMgr->ScheduleRandomize(target->GetGUID().GetCounter(), 62208000);
            handler->PSendSysMessage("botscenario: %s randomized at level %u (ilvl avg %.1f) gscap=%u", name.c_str(), lvl, target->GetAverageItemLevel(), gsLimit);
            return true;
        }

        // lane G (#377 P3 / #530): "spawngo <entry> [dist=<yd>] [secs=<s>]" summons a game object <dist> yd (default 30) in front
        // of <player>, owned by it and loot-visible (same SetSpawnedByDefault(false) as the quest spawngo= ring). Replaces the
        // driver's 'say .gobject add temp', which got no reply. "unlearn <spellId>" / "unlearn mounts" removes a spell, or every
        // mount spell, so a bot can hold a mount ITEM with no mount spell (CheckMountStateAction mount-item fallback).
        if (scenario == "spawngo")
        {
            uint32 entry = 0, secs = 600;
            float dist = 30.0f;
            in >> entry;
            std::string kv;
            while (in >> kv)
            {
                if (kv.rfind("dist=", 0) == 0)
                    dist = float(atof(kv.c_str() + 5));
                else if (kv.rfind("secs=", 0) == 0)
                    secs = uint32(atoi(kv.c_str() + 5));
            }
            if (!entry)
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> spawngo <entry> [dist=<yd>] [secs=<s>]");
                return false;
            }
            float const o = target->GetOrientation();
            float x = target->GetPositionX() + dist * std::cos(o);
            float y = target->GetPositionY() + dist * std::sin(o);
            float z = target->GetPositionZ() + 5.0f;
            target->UpdateGroundPositionZ(x, y, z);
            GameObject* const go = target->SummonGameObject(entry, x, y, z, 0.0f, G3D::Quat(), secs);
            char buf[200];
            if (!go)
            {
                std::snprintf(buf, sizeof(buf), "botscenario: spawngo entry=%u failed", entry);
                handler->SendSysMessage(buf);
                return false;
            }
            go->SetSpawnedByDefault(false);
            std::snprintf(buf, sizeof(buf), "botscenario: spawngo entry=%u guid=%llu x=%.1f y=%.1f z=%.1f dist=%.1f", entry,
                (unsigned long long)go->GetGUID().GetRawValue(), x, y, z, dist);
            handler->SendSysMessage(buf);
            return true;
        }

        if (scenario == "unlearn")
        {
            std::string what;
            in >> what;
            uint32 removed = 0;
            if (what == "mounts")
            {
                std::vector<uint32> ids;
                for (auto const& sp : target->GetSpellMap())
                    if (SpellInfo const* si = sSpellMgr->GetSpellInfo(sp.first))
                        if (si->Effects[0].ApplyAuraName == SPELL_AURA_MOUNTED)
                            ids.push_back(sp.first);
                for (uint32 id : ids)
                {
                    target->RemoveSpell(id);
                    ++removed;
                }
            }
            else if (uint32 id = uint32(atoi(what.c_str())))
            {
                target->RemoveSpell(id);
                removed = 1;
            }
            else
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> unlearn <spellId>|mounts");
                return false;
            }
            char buf[120];
            std::snprintf(buf, sizeof(buf), "botscenario: %s unlearned %u", name.c_str(), removed);
            handler->SendSysMessage(buf);
            return true;
        }

        if (scenario == "learn")
        {
            uint32 spellId = 0;
            in >> spellId;
            if (!spellId)
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> learn <spellId>");
                return false;
            }
            target->LearnSpell(spellId, false);
            handler->PSendSysMessage("botscenario: %s learned spell %u", name.c_str(), spellId);
            return true;
        }

        // #707 `refresh`: run the random-bot refresh now (RandomPlayerbotMgr::Refresh, which the 1-5 h teleport
        // cycle, a revive and an LFG join call), so a 1:1 can check its bag clear without waiting for the schedule.
        if (scenario == "refresh")
        {
            sRandomPlayerbotMgr->Refresh(target);
            handler->PSendSysMessage("botscenario: %s refreshed", name.c_str());
            return true;
        }

        // #277 `questadd <questId>`: the observation instrument for #253 (a SECOND AddQuest
        // while objective counters are live). Mirrors scripts/Commands/cs_quest.cpp:52-101
        // HandleQuestAdd with NO guard added on purpose - the whole point is to reproduce the
        // double-add, so there is deliberately no "already in the quest log" check here. And
        // Player::CanAddQuest (Player.cpp:16038) would not catch it anyway: it checks the quest log
        // for a free SLOT and the source item, and never whether the quest is already active, so a
        // re-add sails through and re-runs `m_questObjectiveStatus[objective.ID] = 0`
        // (Player.cpp:16366) over live counters. Read-only otherwise: no mail, no teleport, no
        // items, and - unlike the real command - no CompleteQuest, so nothing is ever rewarded.
        if (scenario == "questadd")
        {
            if (!target)
            {
                // Contract guard: the shared S1b player lookup above already refused a missing
                // name, so this is belt-and-braces, but the headless driver greps for the
                // QUESTADD-prefixed token rather than the shared message.
                handler->PSendSysMessage("QUESTADD error=no-such-bot player=%s", name.c_str());
                return false;
            }

            uint32 questId = 0;
            if (!(in >> questId) || !questId)
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> questadd <questId>");
                return false;
            }

            Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
            if (!quest)
            {
                handler->PSendSysMessage("QUESTADD error=no-such-quest quest=%u", questId);
                return false;
            }

            // CanAddQuest is asked ONCE: it also pushes a "you cannot add this quest" message to
            // the player when it refuses, and calling it for the report and again for the
            // condition below would say that twice.
            bool const canAdd = target->CanAddQuest(quest, true);
            QuestObjectives const& objectives = quest->Objectives;

            // `opened` = the player's quest-log rows in QUEST_STATUS_INCOMPLETE (accepted, not
            // yet handed in), counted before the AddQuest so the header line carries the value the
            // before-set is measured against. `openedAfter` on the last line is the same count
            // afterwards, so a re-add that reopened the log is visible next to the counters it
            // just zeroed.
            auto countIncomplete = [](Player* who)
            {
                uint32 n = 0;
                for (auto const& row : who->getQuestStatusMap())
                    if (row.second.Status == QUEST_STATUS_INCOMPLETE)
                        ++n;
                return n;
            };

            uint32 const openedBefore = countIncomplete(target);
            // cantake = prerequisites/level/class/race gate (Player::CanTakeQuest); canadd only checks log space.
            bool const canTake = target->CanTakeQuest(quest, false);
            handler->PSendSysMessage("QUESTADD quest=%u level=%u canadd=%u cantake=%u objectives=%u opened=%u",
                questId, uint32(target->GetLevel()), uint32(canAdd), uint32(canTake), uint32(objectives.size()),
                openedBefore);

            for (QuestObjective const& questObjective : objectives)
                handler->PSendSysMessage("QUESTADD objective=%u before=%u", questObjective.ID,
                    target->GetQuestObjectiveCounter(questObjective.ID));

            if (canAdd)
                target->AddQuestAndCheckCompletion(quest, nullptr);

            for (QuestObjective const& questObjective : objectives)
                handler->PSendSysMessage("QUESTADD objective=%u after=%u", questObjective.ID,
                    target->GetQuestObjectiveCounter(questObjective.ID));

            handler->PSendSysMessage("QUESTADD quest=%u status=%u openedAfter=%u", questId,
                uint32(target->GetQuestStatus(questId)), countIncomplete(target));
            return true;
        }

        // masterprobe: same probe machinery as .botprobe (below), but reachable through
        // .botscenario for a non-bot target and named for the master=/mdist= fields the periodic
        // [PROBE] line below now always carries.
        if (scenario == "masterprobe")
        {
            std::string mode;
            in >> mode;
            std::lock_guard<std::mutex> lock(g_mutex);
            uint32 const guidLow2 = target->GetGUID().GetCounter();
            ScenarioState& st = g_bots[guidLow2];
            bool const wasActive = st.IsActive();
            st.botName = target->GetName();
            st.probeEnabled = (mode != "off");
            if (st.probeEnabled)
            {
                st.probeIntervalMs = ProbeIntervalMsDefault();
                st.probeStartT = getMSTime();
                st.lastProbeT = 0;
            }
            if (!wasActive && st.probeEnabled)
                g_active.fetch_add(1, std::memory_order_relaxed);
            else if (wasActive && !st.probeEnabled && !st.IsActive())
                g_active.fetch_sub(1, std::memory_order_relaxed);
            handler->PSendSysMessage("botscenario: %s masterprobe %s", name.c_str(), st.probeEnabled ? "on" : "off");
            return true;
        }
    }

    Player* bot = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    if (!botAI)
    {
        handler->SendSysMessage(("botscenario: no online bot named '" + name + "'").c_str());
        return false;
    }

    uint32 const guidLow = bot->GetGUID().GetCounter();

    if (scenario == "stop")
    {
        uint32 t = 0;
        bool wasRunning = false;
        std::vector<ObjectGuid> questSpawns; // B.4: "despawn the survivors on end or stop"
        BgTrack releasedBg;                  // #631: queue=natural hold to drop once g_mutex is released
        bool releaseBg = false;
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            auto it = g_bots.find(guidLow);
            if (it != g_bots.end() && it->second.kind != ScenarioKind::None)
            {
                wasRunning = true;
                t = getMSTimeDiff(it->second.startT, getMSTime());
                if (it->second.kind == ScenarioKind::Quest)
                    questSpawns = it->second.quest.spawnGuids;
                if (it->second.kind == ScenarioKind::Bg && it->second.bg.naturalQueue)
                {
                    releasedBg = it->second.bg;
                    releaseBg = true;
                }
                it->second.kind = ScenarioKind::None;
                it->second.lock = false;
                it->second.wanderMs = 0;
                it->second.allowMask = 0;
                it->second.pickNearest = false;
                it->second.pickCount = 0;
                it->second.attackGuid = 0;
                it->second.attackEntry = 0;

                if (!it->second.IsActive())
                {
                    g_bots.erase(it);
                    g_active.fetch_sub(1, std::memory_order_relaxed);
                }
            }
        }

        if (wasRunning)
        {
            // Game-API calls happen after g_mutex is released, same rule OnUpdate follows.
            DespawnSurvivors(bot, questSpawns);
            DuelScenarioStopped(bot); // #201 (#613): drop the scan / rematch overrides
            if (releaseBg)
                ReleaseNaturalArenaHold(releasedBg); // #631
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s stopped t=%u", bot->GetName().c_str(), t);
            handler->SendSysMessage(("botscenario: " + name + " stopped").c_str());
        }
        else
            handler->SendSysMessage(("botscenario: " + name + " has no running scenario").c_str());
        return true;
    }

    // "aiprobe" (issue #222): one-shot, read-only inspection of the bot's AI object context. The
    // null-deref being pinned down here is the "chi" gate of CastUpliftOnPartyAction::isUseful, which
    // used to read GetValue<Unit*>("") with no qualifier - an unregistered key, so the returned
    // pointer was null and the caller dereferenced it. This verb shows, without ever touching the
    // null pointer, whether the bare lookup is null and the qualified one ("self target") resolves.
    if (scenario == "aiprobe")
    {
        std::string valueName;
        in >> valueName;
        if (valueName.empty())
        {
            handler->SendSysMessage("botscenario: usage: botscenario <bot> aiprobe <valuename> [qualifier]");
            return false;
        }

        std::string const qualifier = AiprobeQualifier(in);
        AiObjectContext* ctx = botAI->GetAiObjectContext();

        // Null-ness only - Get() on either pointer is never called unless it is non-null.
        bool const unitBareNull = ctx->GetValue<Unit*>("") == nullptr;
        bool const unitQualifiedNull = ctx->GetValue<Unit*>(qualifier) == nullptr;

        Value<uint8>* const value = ctx->GetValue<uint8>(valueName, qualifier);
        char valueBuf[32];
        // SAFETY (#222): a Qualified value's Calculate() reads GetValue<Unit*>(qualifier); that
        // lookup is null for an empty qualifier, and dereferencing it is the very crash this issue
        // is about. Never call Get() unless the qualifier is non-empty AND its Unit* lookup resolves.
        if (value && !qualifier.empty() && !unitQualifiedNull)
            snprintf(valueBuf, sizeof(valueBuf), "%u", unsigned(value->Get()));
        else
            strcpy(valueBuf, "NULL");

        TC_LOG_DEBUG("playerbots",
            "[SCENARIO] AIPROBE %s name=%s qualifier=%s unitBare=%s unitQualified=%s value=%s",
            bot->GetName().c_str(), valueName.c_str(), qualifier.c_str(),
            unitBareNull ? "null" : "ptr", unitQualifiedNull ? "null" : "ptr", valueBuf);
        handler->PSendSysMessage("botscenario: AIPROBE %s name=%s qualifier=%s unitBare=%s unitQualified=%s value=%s",
            bot->GetName().c_str(), valueName.c_str(), qualifier.c_str(),
            unitBareNull ? "null" : "ptr", unitQualifiedNull ? "null" : "ptr", valueBuf);
        return true;
    }

    // "dismisspet" (issue #221 harness precondition): one-shot, unsummons the bot's current pet so a run
    // can start from a known no-pet state. Without it every run inherits whatever pet the previous session
    // left in character_pet, and Player::SummonPet then returns early out of LoadPetFromDB
    // (Player.cpp:29726-29745) instead of taking the fresh-create path, so the FollowMovementGenerator
    // re-anchor gets measured on a stale pet rather than on the one summonpet just made. PET_REMOVE_DISMISS
    // is the no-save removal, so it also drops the character_pet row.
    // Scenario-neutral on purpose: it never reads or writes g_bots and never enters the state machine,
    // so a scenario that is already running is not disturbed.
    if (scenario == "dismisspet")
    {
        Pet* pet = bot->GetPet();
        std::string const petName = pet ? pet->GetName() : std::string("none");
        uint32 const petGuidLow = pet ? pet->GetGUID().GetCounter() : 0;
        uint32 const petEntry = pet ? pet->GetEntry() : 0;
        Position const ppos = pet ? pet->GetWorldLocation() : bot->GetWorldLocation();

        // Reasons are single tokens so the reply stays machine-greppable.
        char const* reason = nullptr;
        if (!pet)
            reason = "no-pet";
        else
        {
            bot->RemovePet(PET_REMOVE_DISMISS);
            // Re-read rather than trusting the call: Player::RemovePet returns early for an already-removed
            // pet (Player.cpp:22338) and leaves GetPet() intact, which is the refusal we want to see.
            if (bot->GetPet())
                reason = "remove-refused";
        }

        TC_LOG_DEBUG("playerbots",
            "[SCENARIO] %s dismisspet pet=%s entry=%u guid=%u petpos=(%.2f, %.2f, %.2f) reason=%s",
            bot->GetName().c_str(), petName.c_str(), petEntry, petGuidLow,
            ppos.GetPositionX(), ppos.GetPositionY(), ppos.GetPositionZ(), reason ? reason : "none");
        handler->PSendSysMessage("botscenario: %s dismisspet pet=%s entry=%u guid=%u petpos=(%.2f, %.2f, %.2f) reason=%s",
            bot->GetName().c_str(), petName.c_str(), petEntry, petGuidLow,
            ppos.GetPositionX(), ppos.GetPositionY(), ppos.GetPositionZ(), reason ? reason : "none");
        return reason == nullptr;
    }

    // "summonpet" (issue #221 harness precondition): one-shot, gives the bot a pet/guardian through the
    // same core call the module's own pet bootstrap uses (BotFactory::InitPet -> Player::SummonPet).
    // Scenario-neutral on purpose: it never reads or writes g_bots and never enters the state machine,
    // so a scenario that is already running is not disturbed.
    if (scenario == "summonpet")
    {
        uint32 entry = 0;
        in >> entry;
        if (!entry)
        {
            handler->SendSysMessage("botscenario: usage: botscenario <bot> summonpet <creatureEntry>");
            return false;
        }

        // Reasons are single tokens so the reply stays machine-greppable.
        char const* reason = nullptr;
        if (bot->GetPet())
            reason = "already-has-pet";
        else if (!sObjectMgr->GetCreatureTemplate(entry))
            reason = "no-such-creature";

        if (!reason)
        {
            Position const pos = bot->GetWorldLocation();
            // MoP signature: last arg is despwtime, passed as 0 by SpellEffects.cpp:3496.
            bot->SummonPet(entry, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(),
                           pos.GetOrientation(), 0);
        }

        // Re-read the pet instead of trusting SummonPet's return value: this core returns nullptr on the
        // *success* path (Player::SummonPet, Player.cpp:29744) when the pet loads from DB.
        Pet* pet = bot->GetPet();
        if (!pet && !reason)
            reason = "summon-failed";

        std::string const petName = pet ? pet->GetName() : std::string("none");
        uint32 const petGuidLow = pet ? pet->GetGUID().GetCounter() : 0;

        TC_LOG_DEBUG("playerbots",
            "[SCENARIO] %s summonpet entry=%u pet=%s guid=%u reason=%s",
            bot->GetName().c_str(), entry, petName.c_str(), petGuidLow, reason ? reason : "none");
        handler->PSendSysMessage("botscenario: %s summonpet entry=%u pet=%s guid=%u reason=%s",
            bot->GetName().c_str(), entry, petName.c_str(), petGuidLow, reason ? reason : "none");
        return reason == nullptr;
    }

    // "petinfo" (issue #221 harness): one-shot, read-only dump of the bot's pet position and its distance
    // to the master. FollowMovementGenerator's NOPATH tolerance and the >20 yd re-anchor have no
    // server-side log of their own, so a pet trail can only be built by asking for one sample at a time
    // (one SOAP round trip per call). Scenario-neutral on purpose: it never reads or writes g_bots and
    // never enters the state machine, so a scenario that is already running is not disturbed.
    if (scenario == "petinfo")
    {
        Pet* pet = bot->GetPet();
        std::string const petName = pet ? pet->GetName() : std::string("none");
        uint32 const petGuidLow = pet ? pet->GetGUID().GetCounter() : 0;
        uint32 const petEntry = pet ? pet->GetEntry() : 0;
        uint32 const petLevel = pet ? pet->GetLevel() : 0;
        uint32 const petMap = pet ? pet->GetMapId() : 0;
        Position const bpos = bot->GetWorldLocation();
        Position const ppos = pet ? pet->GetWorldLocation() : bpos;
        float const dist = pet ? bot->GetDistance(pet) : -1.0f;
        int const combat = (bot->IsInCombat() || (pet && pet->IsInCombat())) ? 1 : 0;

        // #251: the reported symptom is flight-only, so the sample has to carry the flight state that separates
        // it from the ground-mount case, plus the two numbers the dismissal sinks are measured on. The pet's
        // 2D distance is the one Pet::Update (Pet.cpp:559) and Player::Update (Player.cpp:1517) compare against
        // the map visibility range before removing the pet for good; the 3D distance is what the follow
        // re-anchor trips on. Pet health answers "did it come back on landing", and the pet GUID being
        // 0 with pet=none is the (a) despawn signature. NOT sampled, and therefore NOT claimed: an aura
        // count, and the isPossessed() state that both dismissal sinks also test - so inrange3d=0 alone
        // does not by itself identify the sink.
        float const dist2d = pet ? bot->GetDistance2d(pet) : -1.0f;
        uint32 const petHealth = pet ? pet->GetHealth() : 0;
        uint32 const petMaxHealth = pet ? pet->GetMaxHealth() : 0;
        uint32 const visRange = uint32(bot->GetMap()->GetVisibilityRange());
        // inrange3d is the test Pet::Update (Pet.cpp:559) and Player::Update (Player.cpp:1517) actually dismiss
        // the pet on - IsWithinDistInMap defaults to is3D=true (Object.h:495) - so 0 here is the #251 signature.
        int const inRange3d = (pet && pet->IsWithinDistInMap(bot, float(visRange))) ? 1 : 0;
        char const* flight = bot->IsInFlight() ? "taxi" :
            (bot->IsFlying() ? "flying" : (bot->IsFalling() ? "falling" : "ground"));
        int const petDead = (pet && !pet->IsAlive()) ? 1 : 0;
        float const dz = pet ? (bpos.GetPositionZ() - ppos.GetPositionZ()) : 0.0f;

        TC_LOG_DEBUG("playerbots",
            "[SCENARIO] %s petinfo pet=%s entry=%u level=%u guid=%u map=%u bot=(%.2f, %.2f, %.2f) "
            "petpos=(%.2f, %.2f, %.2f) dist=%.2f dist2d=%.2f dz=%.2f combat=%d flight=%s mounted=%d inflight=%d "
            "flying=%d visrange=%u inrange3d=%d petdead=%d pethealth=%u/%u",
            bot->GetName().c_str(), petName.c_str(), petEntry, petLevel, petGuidLow, petMap,
            bpos.GetPositionX(), bpos.GetPositionY(), bpos.GetPositionZ(),
            ppos.GetPositionX(), ppos.GetPositionY(), ppos.GetPositionZ(), dist, dist2d, dz, combat, flight,
            bot->IsMounted() ? 1 : 0, bot->IsInFlight() ? 1 : 0, bot->IsFlying() ? 1 : 0, visRange,
            inRange3d, petDead, petHealth, petMaxHealth);
        handler->PSendSysMessage("botscenario: %s petinfo pet=%s entry=%u level=%u guid=%u map=%u "
            "bot=(%.2f, %.2f, %.2f) petpos=(%.2f, %.2f, %.2f) dist=%.2f dist2d=%.2f dz=%.2f combat=%d flight=%s "
            "mounted=%d inflight=%d flying=%d visrange=%u inrange3d=%d petdead=%d pethealth=%u/%u",
            bot->GetName().c_str(), petName.c_str(), petEntry, petLevel, petGuidLow, petMap,
            bpos.GetPositionX(), bpos.GetPositionY(), bpos.GetPositionZ(),
            ppos.GetPositionX(), ppos.GetPositionY(), ppos.GetPositionZ(), dist, dist2d, dz, combat, flight,
            bot->IsMounted() ? 1 : 0, bot->IsInFlight() ? 1 : 0, bot->IsFlying() ? 1 : 0, visRange,
            inRange3d, petDead, petHealth, petMaxHealth);
        return pet != nullptr;
    }

    // "factioninfo" (issue #228 harness): one-shot, read-only dump of the server-side state behind the
    // client's /follow gate. A real player's /follow on a playerbot is refused with
    // ERR_INVALID_FOLLOW_TARGET (342), and the client's gate needs (A) a TYPE_PLAYER target, (B)
    // UNIT_FIELD_CHARMED_BY == 0, (C) an equal FactionTemplateEntry.ourMask, and (D) a friendly
    // reaction. None of the four is logged server-side, so this prints the bot's own values rather than
    // leaving them to be guessed at. Scenario-neutral on purpose: it never reads or writes g_bots and
    // never enters the state machine, so a scenario that is already running is not disturbed.
    if (scenario == "factioninfo")
    {
        // (C) GetFactionTemplateEntry() is a DBC lookup that can legitimately fail, so every read of
        // ft is guarded. Its key field is ID, not Entry.
        FactionTemplateEntry const* const ft = bot->GetFactionTemplateEntry();
        uint32 const faction = bot->GetFaction();
        uint32 const ftId = ft ? ft->ID : 0;
        uint32 const ourMask = ft ? ft->ourMask : 0;

        // (B) The charm field holds a packed GUID, so read it through the same accessor the core's own
        // IsCharmed() uses instead of reinterpreting the raw uint32 pair.
        ObjectGuid const charmerGuid = bot->GetCharmerGUID();
        ObjectGuid const charmerOrOwnerGuid = bot->GetCharmerOrOwnerGUID();
        char charmedBy[48] = "none";
        if (charmerGuid)
            snprintf(charmedBy, sizeof(charmedBy), "%s", charmerGuid.ToString().c_str());
        char charmerOrOwner[48] = "none";
        if (charmerOrOwnerGuid)
            snprintf(charmerOrOwner, sizeof(charmerOrOwner), "%s", charmerOrOwnerGuid.ToString().c_str());

        // (D) The reaction the *master's* client would compute for this bot - the direction the follow
        // gate actually reads. A real player is its own master (PlayerbotAI::IsRealPlayer), so "self"
        // is a distinct state rather than a reaction worth printing. The master null check is
        // load-bearing, not defensive noise: the core's GetReactionTo dereferences its argument
        // unconditionally (Unit.cpp:6837) and MoP is the only tree that guards it.
        Player* const master = botAI->GetMaster();
        int reaction = -1;
        char masterBuf[48];
        if (!master)
            snprintf(masterBuf, sizeof(masterBuf), "%s", "none");
        else if (master == bot)
            snprintf(masterBuf, sizeof(masterBuf), "%s%s", master->GetName().c_str(), "(self)");
        else
        {
            snprintf(masterBuf, sizeof(masterBuf), "%s", master->GetName().c_str());
            reaction = int(master->GetReactionTo(bot));
        }

        // GetTeamId() only: the AC core has no Player::GetTeam() to pair it with, so using one and not
        // the other is what keeps this block identical across both trees.
        uint32 const team = uint32(bot->GetTeamId());

        TC_LOG_DEBUG("playerbots",
            "[SCENARIO] %s factioninfo isplayer=%d race=%u class=%u level=%u faction=%u ftid=%u "
            "ourmask=%u charmedby=%s charm=%d charmerorowner=%s ctrlbyplayer=%d team=%u master=%s reaction=%d",
            bot->GetName().c_str(), bot->IsPlayer() ? 1 : 0, unsigned(bot->GetRace()),
            unsigned(bot->GetClass()), unsigned(bot->GetLevel()), faction, ftId, ourMask,
            charmedBy, bot->IsCharmed() ? 1 : 0, charmerOrOwner,
            bot->IsControlledByPlayer() ? 1 : 0, team, masterBuf, reaction);
        handler->PSendSysMessage("botscenario: %s factioninfo isplayer=%d race=%u class=%u level=%u "
            "faction=%u ftid=%u ourmask=%u charmedby=%s charm=%d charmerorowner=%s ctrlbyplayer=%d "
            "team=%u master=%s reaction=%d",
            bot->GetName().c_str(), bot->IsPlayer() ? 1 : 0, unsigned(bot->GetRace()),
            unsigned(bot->GetClass()), unsigned(bot->GetLevel()), faction, ftId, ourMask,
            charmedBy, bot->IsCharmed() ? 1 : 0, charmerOrOwner,
            bot->IsControlledByPlayer() ? 1 : 0, team, masterBuf, reaction);
        return true;
    }

    ScenarioHandler const* h = FindScenarioHandlerByName(scenario);
    if (!h)
    {
        handler->SendSysMessage("botscenario: unknown scenario (login, goto, wander, attack, die, taxi, quest, transport, bg, stop)");
        return false;
    }

    ScenarioState state;
    state.botName = bot->GetName();
    if (!h->parse(handler, bot, botAI, in, state))
        return false;

    // 1:1 runs start from the same state on both cores: full health and full mana (otherwise a bot
    // that starts hurt spends its first seconds eating, and the two runs diverge from t=0).
    bot->SetFullHealth();
    if (bot->GetPowerType() == POWER_MANA)
        bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        ScenarioState& st = g_bots[guidLow];
        bool const wasActive = st.IsActive();
        bool const hadProbe = st.probeEnabled;
        uint32 const keepInterval = st.probeIntervalMs;

        // A running scenario always enables the probe too (HARNESS-SPEC-botscenario-v3 section 4),
        // at whatever interval an independently-toggled probe already had, or the config default.
        state.probeEnabled = true;
        state.probeIntervalMs = hadProbe ? keepInterval : ProbeIntervalMsDefault();
        state.probeStartT = state.startT;
        state.lastProbeT = 0;

        st = state;
        if (!wasActive)
            g_active.fetch_add(1, std::memory_order_relaxed);
    }

    handler->SendSysMessage(("botscenario: " + name + " " + scenario + " started").c_str());
    return true;
}

bool HandleProbeCommandImpl(ChatHandler* handler, char const* args)
{
    EnsureDuelHarnessScript(); // #201 (#613)
    std::istringstream in(args ? args : "");
    std::string name;
    in >> name;

    if (name == "list")
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (g_bots.empty())
        {
            handler->SendSysMessage("botprobe: no active scenarios or probes");
            return true;
        }

        for (auto const& entry : g_bots)
        {
            ScenarioState const& st = entry.second;
            ScenarioHandler const* h = FindScenarioHandler(st.kind);
            std::string line = "botprobe: " + st.botName +
                " scenario=" + (h ? h->name : "none") +
                " probe=" + (st.probeEnabled ? ("on(" + std::to_string(st.probeIntervalMs) + "ms)") : "off");
            handler->SendSysMessage(line.c_str());
        }
        return true;
    }

    std::string mode;
    in >> mode;

    Player* bot = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    if (!botAI)
    {
        handler->SendSysMessage(("botprobe: no online bot named '" + name + "'").c_str());
        return false;
    }

    uint32 const guidLow = bot->GetGUID().GetCounter();

    if (mode == "on")
    {
        uint32 intervalMs = 0;
        in >> intervalMs;
        if (!intervalMs)
            intervalMs = ProbeIntervalMsDefault();

        std::lock_guard<std::mutex> lock(g_mutex);
        ScenarioState& st = g_bots[guidLow];
        bool const wasActive = st.IsActive();
        st.botName = bot->GetName();
        st.probeEnabled = true;
        st.probeIntervalMs = intervalMs;
        st.probeStartT = getMSTime();
        st.lastProbeT = 0;
        if (!wasActive)
            g_active.fetch_add(1, std::memory_order_relaxed);

        handler->SendSysMessage(("botprobe: " + name + " probe on").c_str());
        return true;
    }

    if (mode == "off")
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = g_bots.find(guidLow);
        if (it != g_bots.end())
        {
            it->second.probeEnabled = false;
            if (!it->second.IsActive())
            {
                g_bots.erase(it);
                g_active.fetch_sub(1, std::memory_order_relaxed);
            }
        }

        handler->SendSysMessage(("botprobe: " + name + " probe off").c_str());
        return true;
    }

    handler->SendSysMessage("botprobe: usage: botprobe <bot> on [intervalMs] | botprobe <bot> off | botprobe list");
    return false;
}

// #256: AC ChatCommand.cpp:332-341 sets SentErrorMessage whenever a command returns false, so a negative
// result is SOAP 500 there. MoP ExecuteNode (ChatCommand.cpp:258-269) does not, so the same result was
// HTTP 200 plus an appended syntax line. Setting the flag here (harness verbs only) makes both cores answer alike.
bool HandleScenarioCommand(ChatHandler* handler, char const* args)
{
    bool const ok = HandleScenarioCommandImpl(handler, args);
    if (!ok && handler)
        handler->SetSentErrorMessage(true);
    return ok;
}

bool HandleProbeCommand(ChatHandler* handler, char const* args)
{
    bool const ok = HandleProbeCommandImpl(handler, args);
    if (!ok && handler)
        handler->SetSentErrorMessage(true);
    return ok;
}

} // namespace BotScenario
