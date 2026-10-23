/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Isle of Conquest (map 628) play beyond AC mod-playerbots - issue #675 (epic #205), build 14.
//
// Included only by BattleGroundTactics.cpp, BgTacticsIC.cpp and GenericSpellActions.cpp (#675 RamTarget) - NOT by
// BattleGroundTactics.h, which ActionContext.h includes - so a change here rebuilds three translation units.
//
// AC's own IoC tactics (the BATTLEGROUND_IC cases in BattleGroundTactics.cpp, ported 1:1 in #393)
// stay the base. These hooks run ahead of them and return false whenever AC's play applies:
//   SelectObjective  - AC selectObjective's role: pick a "bg objective" (and "bg siege") position.
//   AtObjective      - AC atFlag's role: act on the thing the bot reached (board, launch, plant, use).
//   StaysInOwnKeep   - lets a keep-cannon defender skip AC's IsLockedInsideKeep portal exit.
// See BgTacticsIC.cpp for the mechanics, the sources and the role split.

#ifndef PLAYERBOTS_BGTACTICSIC_H
#define PLAYERBOTS_BGTACTICSIC_H

#include "PositionValue.h"
#include "SharedDefines.h"

class AiObjectContext;
class BattlegroundIC;
class Player;
class PlayerbotAI;
class Unit;

namespace IocBeyondAc
{
    /// Master switch (default on). With it off every hook returns false and the bots play AC's IoC
    /// tactics only - for an AC-parity 1:1. Not a config key (none exists on AC); a harness verb may
    /// flip it.
    bool Enabled();
    void SetEnabled(bool enabled);

    bool SelectObjective(Player* bot, PlayerbotAI* botAI, AiObjectContext* context, BattlegroundIC* bg,
                         TeamId team, uint32 role, PositionMap& posMap);

    bool AtObjective(Player* bot, PlayerbotAI* botAI, AiObjectContext* context, BattlegroundIC* bg,
                     TeamId team, uint32 role);

    bool StaysInOwnKeep(Player* bot, BattlegroundIC* bg, TeamId team, uint32 role);

    /// #675: the enemy a driven IoC Demolisher's Ram (67441) hits now, or nullptr (map 628 only). As SotA's #765
    /// SotaBeyondAc::RamTarget: CastVehicleSpellAction casts "ram" on it in place of "current target", which a driver
    /// never has (GenericSpellActions.cpp VehicleSpellTarget).
    Unit* RamTarget(Player* bot, AiObjectContext* context);
}

#endif
