/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" (Scholomance, epic #373, issue #659). See MopScholoShared.h for sources.

#ifndef PLAYERBOTS_MOPSCHOLOTRIGGERS_H
#define PLAYERBOTS_MOPSCHOLOTRIGGERS_H

#include "MopDungeonShared.h"
#include "MopScholoShared.h"
#include "Trigger.h"

// The bot stands inside (or next to) a live ground-effect carrier: Frigid Grasp, Arcane Bomb marker, Fire Tome,
// Gravity Flux, Soulflame or Dark Blaze. One class, one instance per hazard (see MopScholoTriggerContext.h).
class MopScholoHazardTrigger : public Trigger
{
public:
    MopScholoHazardTrigger(PlayerbotAI* botAI, std::string const name, MopScholomance::Hazard const& hazard)
        : Trigger(botAI, name), _hazard(hazard)
    {
    }

    bool IsActive() override;

private:
    MopScholomance::Hazard const& _hazard;
};

// The bot carries Ice Wrath and a group member stands within its 2.5 yd pulse range.
class MopScholoIceWrathTrigger : public Trigger
{
public:
    MopScholoIceWrathTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo ice wrath") {}
    bool IsActive() override;
};

// A ranged / healer bot stands within 9 yd of another member while Lilian Voss is fighting (Shadow Shiv, 8 yd).
class MopScholoLilianSpreadTrigger : public Trigger
{
public:
    MopScholoLilianSpreadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo lilian spread") {}
    bool IsActive() override;
};

// Lilian's Soul is fixated on this bot.
class MopScholoFixatedTrigger : public Trigger
{
public:
    MopScholoFixatedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo fixated") {}
    bool IsActive() override;
};

// Jandice / Rattlegore / Gandling is casting something worth interrupting. #781: the bots whose interrupt is ready
// take turns (MopDungeon::InterruptGate), the tank included.
class MopScholoInterruptTrigger : public Trigger
{
public:
    MopScholoInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo priority interrupt") {}
    bool IsActive() override;

private:
    MopDungeon::InterruptGate _gate;
};

// There is something to kill instead of the boss: the Phylactery, the real Jandice, the Failed Students.
class MopScholoFocusTrigger : public Trigger
{
public:
    MopScholoFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo focus target") {}
    bool IsActive() override;
};

// Rattlegore is fighting, the bot has no Bone Armor and a Bone Pile is within reach.
// #781 Jandice's Wondrous Rapidity: the tank steps out of the frontal cone (AC Ai/Dungeon/UK/UKTriggers.h:60-72,
// IngvarSmashTankTrigger / IngvarSmashTankReturnTrigger).
class MopScholoRapidityTankTrigger : public Trigger
{
public:
    MopScholoRapidityTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo rapidity tank") {}
    bool IsActive() override;
};

class MopScholoRapidityTankReturnTrigger : public Trigger
{
public:
    MopScholoRapidityTankReturnTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo rapidity tank return") {}
    bool IsActive() override;
};

// #659 Chillheart's Ice Wave: the bot is not ICE_WALL_REACH + ICE_WALL_MARGIN ahead of the wall's front row.
class MopScholoIceWallTrigger : public Trigger
{
public:
    MopScholoIceWallTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo ice wall") {}
    bool IsActive() override;
};

// #659: the tank is fighting Chillheart (wall up) or Jandice (no illusion phase) and stands off its TankSpot.
class MopScholoTankSpotTrigger : public Trigger
{
public:
    MopScholoTankSpotTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo tank spot") {}
    bool IsActive() override;
};

// #806 Harsh Lesson classroom: a healer with a dispel and a loaded Fresh Test Subject (Explosive Pain) next to Expired ones.
class MopScholoHarshLessonTrigger : public Trigger
{
public:
    MopScholoHarshLessonTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo harsh lesson") {}
    bool IsActive() override;
};

// #806: the classroom is clear (door open) and the bot is still far from Gandling.
class MopScholoLessonExitTrigger : public Trigger
{
public:
    MopScholoLessonExitTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo lesson exit") {}
    bool IsActive() override;
};

class MopScholoBoneArmorTrigger : public Trigger
{
public:
    MopScholoBoneArmorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scholo bone armor") {}
    bool IsActive() override;
};

#endif
