/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * HARNESS-SPEC-botscenario-v3-module: the .botscenario / .botprobe debug harness, pulled out of
 * NewRpgInfo.h (and therefore out of PlayerbotAI.h, included by most of the module) into its own
 * side table. This header is deliberately tiny and stable - forward declarations and this
 * free-function API only - so a harness/probe change only touches Scenario/BotScenario*.cpp and a
 * relink, instead of recompiling the whole module. All state lives in BotScenario.cpp; nothing here
 * is a class member of anything else.
 */

#ifndef _PLAYERBOT_BOTSCENARIO_H
#define _PLAYERBOT_BOTSCENARIO_H

#include "Define.h"

#include <vector>

class ChatHandler;
class Player;
class PlayerbotAI;
class WorldObject;
class WorldPosition;

// Status machine states for the "new rpg" roaming strategy. The real definition (with its
// enumerators) lives in Utils/PlayerbotAIConfig.h as "enum NewRpgStatus : int"; it is forward
// declared here with the same underlying type so this header does not have to include
// PlayerbotAIConfig.h. Nothing in this header or in BotScenario.cpp needs the enumerator names -
// only the numeric value (as already logged in [TRACE]/[PROBE] as status=<int>).
enum NewRpgStatus : int;

namespace BotScenario
{
    // Cheap global check: true when any scenario or probe is active on this server. Every hook
    // below starts with this, so the fast path (no scenario, no probe - the default for the other
    // ~300 bots) costs one atomic load and nothing else.
    bool AnyActive();

    // w18 (#746 wave): PlayerbotAI::AllowActive - a bot under a running .botscenario (kind != None, not a bare
    // probe) stays active, the way a bot next to a real player does, so harness legs work at the production
    // BotActiveAlone. Test-only: false whenever no scenario runs.
    bool Driving(Player* bot);

    // Hooks (all no-ops for bots without a scenario/probe; each starts with the AnyActive() fast
    // path) --------------------------------------------------------------------------------------

    // PlayerbotAI::UpdateAI, right after the early-return block. Drives [TRACE]/[SCENARIO]/[WANDER]
    // and the [PROBE] tick for this bot.
    void OnUpdate(PlayerbotAI* botAI);
    // PlayerbotAI::Reset(full), after the RPG info is re-created (_rpgInfo = NewRpgInfo()). Re-arms
    // a running scenario across the reset and logs "[SCENARIO] <bot> rearmed after reset map=<m>".
    void OnReset(PlayerbotAI* botAI);
    // NewRpgStatusUpdateAction::Execute: a running .botscenario pins the status until it ends.
    // Returns true -> caller returns false without changing status.
    bool IsStatusLocked(Player* bot);
    // RandomChangeStatus, before the draw: strips out candidates not in the scenario's allow-mask.
    // No-op unless a scenario with an allow-mask set is running for this bot.
    void FilterStatuses(Player* bot, std::vector<NewRpgStatus>& candidates);
    // SelectRandomGrindPos / SelectRandomCampPos / ChooseNpcOrGameObjectToInteract: true when a
    // deterministic scenario wants the nearest candidate instead of a random one.
    bool PickNearest(Player* bot);
    // SelectRandomGrindPos: a scenario started with "grind=x,y,z" gets that point instead of a random
    // grind cell, so both cores send the bot to the same place (the DBs lay out grind cells differently).
    bool GrindPoint(Player* bot, WorldPosition& out);
    // AttackAnythingAction::isUseful: a wander scenario started with "nofight" does not start fights
    // (it still defends itself), so wander stages measure movement only.
    bool NoFight(Player* bot);
    // CheckMountStateAction's flying-mount decision: a goto/wander scenario started with "ground" must
    // never use a flying mount, so a scenario's ground route is not shortcut by flight.
    bool GroundOnly(Player* bot);
    // True while the bot has a scenario or probe: Engine then logs its decision trail (like DebugBotNames).
    bool WantsTrail(Player* bot);
    // PlayerbotAI::UpdateAI's transport-check block, right before the g_nextTransportCheck/
    // nextTransportCheck decrement (research\transport-stage-design.md section 3.2/3.3). True zeroes
    // the caller's timer this tick, making the once-a-minutes-scale check deterministic for a running
    // "transport" scenario's tcheck=tick|now gate. False (including no scenario at all) changes
    // nothing.
    bool TransportCheckDue(Player* bot);
    // bg stage isolation: true while a `bg` stage runs and this bot is not on its roster (BGJoinAction::isUseful).
    bool BgJoinBlocked(Player* bot);
    // MoveRandomNear's deterministic golden-angle sweep. Returns false (caller keeps its normal
    // random pick) unless a deterministic scenario is running for this bot; on true, `angle`/`dist`
    // are set and the scenario's internal pick counter has been advanced.
    bool NextRandomNearAngle(Player* bot, float moveStep, float& angle, float& dist);
    // [WANDER] trace line. No-op unless a scenario is running for this bot. `target` may be null
    // (kind=none, or a coordinate-only pick such as rnd/grind/camp); x/y/z is the target position
    // either way, `step` is MoveRandomNear's step for kind=rnd.
    void LogTarget(Player* bot, char const* kind, WorldObject const* target, float x, float y, float z,
                   float step = 0.0f);
    // Engine: an action's result is known (OK/FAILED/IMPOSSIBLE/USELESS). Probe-only bookkeeping;
    // cheap no-op via the AnyActive() fast path when no probe/scenario is running anywhere.
    void OnAction(Player* bot, char const* action, char const* result);
    // PlayerbotAI::CastSpell's final verdict (a SpellCastResult; SPELL_CAST_OK == 0 on success).
    // Probe-only bookkeeping.
    void OnCast(Player* bot, uint32 spellId, int result);

    // Console/SOAP entry points ------------------------------------------------------------------
    // All argument parsing happens in BotScenario.cpp; these two are the only symbols the command
    // scripts need to link against.
    bool HandleScenarioCommand(ChatHandler* handler, char const* args); // "botscenario ..."
    bool HandleProbeCommand(ChatHandler* handler, char const* args);    // "botprobe ..."
}

#endif
