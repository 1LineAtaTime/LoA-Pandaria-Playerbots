/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Strand of the Ancients (map 607) play beyond AC mod-playerbots - issue #717 (epic #205), after the Isle of
// Conquest pattern (BgTacticsIC.h, #675).
//
// Included only by BattleGroundTactics.cpp, BgTacticsSA.cpp and AiFactory.cpp (NOT by BattleGroundTactics.h,
// which ActionContext.h includes), so a change here rebuilds three translation units.
//
// AC has no SotA tactics at all: AC queues and seeds SotA (AC BattleGroundJoinAction.cpp:477/:847,
// RandomPlayerbotMgr.cpp:1244) but never gives a bot in it the "battleground" strategy (AC AiFactory.cpp:689-690
// gates it on bgType <= BATTLEGROUND_EY || bgType == BATTLEGROUND_IC), and AC Bot/PlayerbotAI.cpp:4181 leaves the
// siege shot "implemented for IoC (extend for SotA if needed)". So these hooks ARE the SotA objective layer:
//   StartSpot        - AC moveToStart's role during both warm-ups (WAIT_JOIN).
//   SelectObjective  - AC selectObjective's role: the "bg objective" (and "bg siege") position.
//   AtObjective      - AC atFlag's role: act on the thing the bot reached (board, capture, plant, relic).
//   ObserveEnd       - the round/match result evidence line.
// See BgTacticsSA.cpp for the mechanics, the sources and the role split.

#ifndef PLAYERBOTS_BGTACTICSSA_H
#define PLAYERBOTS_BGTACTICSSA_H

#include "PositionValue.h"
#include "SharedDefines.h"

class AiObjectContext;
class BattlegroundSA;
class Player;
class PlayerbotAI;
class Unit;
struct Position;

namespace SotaBeyondAc
{
    /// Master switch (default on). With it off AiFactory gives a SotA bot no "battleground"/"strand" strategy
    /// (exactly AC's gate) and every hook returns false - for an AC-parity 1:1. Not a config key (none exists on
    /// AC); a harness verb may flip it.
    bool Enabled();
    void SetEnabled(bool enabled);

    /// #736(c) - beyond AC (owner 2026-10-14): the chance (0-100) that a demolisher gets a bot driver, one deterministic
    /// roll per demolisher life. pct < 0 = AiPlayerbot.BgSotaVehicleDriveChance (default 70). Set by the harness verb
    /// `.botscenario - sotadrive <pct>|off` (BotScenario.cpp); setting it drops the rolls already made.
    void SetDriveChanceOverride(int32 pct);
    int32 DriveChanceOverride();
    uint32 EffectiveDriveChance();

    /// WAIT_JOIN (round-one boats and the round-two warm-up): true with `spot` set = walk there (defenders to
    /// their cannon or gate); false = stay where the bot is (attackers on the boats; #750: a defender that boarded its
    /// cannon, stands at its gate post, or has just jumped off the wall here).
    bool StartSpot(Player* bot, BattlegroundSA* bg, TeamId team, uint32 role, Position& spot);

    bool SelectObjective(Player* bot, PlayerbotAI* botAI, AiObjectContext* context, BattlegroundSA* bg,
                         TeamId team, uint32 role, PositionMap& posMap);

    bool AtObjective(Player* bot, PlayerbotAI* botAI, AiObjectContext* context, BattlegroundSA* bg,
                     TeamId team, uint32 role);

    /// STATUS_WAIT_LEAVE: one "[SOTA] result" line per match.
    void ObserveEnd(Player* bot, BattlegroundSA* bg);

    /// #765: the enemy a driven Battleground Demolisher's Ram (60206) hits now, or nullptr. CastVehicleSpellAction
    /// casts "ram" on it in place of "current target", which a driver never has (GenericSpellActions.cpp).
    Unit* RamTarget(Player* bot, AiObjectContext* context);
}

#endif
