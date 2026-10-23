/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// End Time bot strategy "cata-et" - multipliers (issue #671, epic #373). Pattern: AC Ai/Dungeon/HoS/HoSMultipliers.h.
// Each multiplier keeps a 100 ms cache: the engine asks it once per queued action, the checks behind it are grid searches.

#ifndef PLAYERBOTS_CATAETMULTIPLIERS_H
#define PLAYERBOTS_CATAETMULTIPLIERS_H

#include "Define.h"
#include "Multiplier.h"
#include "ObjectGuid.h"

// While a ground hazard / blade path / flarecore / lava / breath needs the bot to move, only this strategy's own moving
// actions may move it (AC HoS KrystallusMultiplier shape).
class CataEtHazardMultiplier : public Multiplier
{
public:
    CataEtHazardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "end time hazards") {}
    float GetValue(Action* action) override;

private:
    bool NeedsMove();

    uint32 _stamp = 0;
    bool _cached = false;
};

// Calling of the Highborne (w62, #671): while all eight Risen Ghouls stand, every attack goes to the designated ghoul (no other
// ghoul, not the immune Sylvanas); once one is dead (the breach) nobody attacks a ghoul and only this strategy's moves and
// target picks run, so the bots stay on the breach spot until Sacrifice.
class CataEtSylvanasCallMultiplier : public Multiplier
{
public:
    CataEtSylvanasCallMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "echo of sylvanas call") {}
    float GetValue(Action* action) override;

private:
    enum class CallState : uint8
    {
        None,
        Kill,
        Breach
    };

    CallState GetState();

    uint32 _stamp = 0;
    CallState _cached = CallState::None;
    ObjectGuid _designated;
};

// Echo of Tyrande's five-pool event (w62, #671): the bots hold the Pool of Moonlight - no movement but this strategy's own
// (target picks still run: AttackAction moves nobody, the core's melee swing does not chase).
class CataEtTyrandeEventMultiplier : public Multiplier
{
public:
    CataEtTyrandeEventMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "echo of tyrande event") {}
    float GetValue(Action* action) override;

private:
    uint32 _stamp = 0;
    bool _cached = false;
};

// Murozond (w67, #671): the bot that walks to the Hourglass of Time ("et use hourglass", ACTION_RAID + 6) was pulled back by
// reach melee / set behind / Killing Spree for ~40 s (rogue Midand, 7 x "et use hourglass - OK" between leg log lines 739412 and
// 779049) while the mage lay dead from the first 20 s. While NeedsHourglass picks this bot, only this strategy's moves run.
class CataEtHourglassMultiplier : public Multiplier
{
public:
    CataEtHourglassMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "murozond hourglass walk") {}
    float GetValue(Action* action) override;

private:
    uint32 _stamp = 0;
    bool _cached = false;
};

// Echo of Tyrande (w65, #671): the class interrupt triggers kick any cast, so Kick / Pummel / Counterspell went on Moonbolt (cast
// every 3 s, boss_echo_of_tyrande.cpp:428-431) and were on cooldown for Stardust, the 50k hit on every player (9 Stardusts landed,
// 16 hits; "et tyrande interrupt" 4 OK / 266 FAILED). While she is attackable the class interrupts wait for Stardust.
class CataEtTyrandeKickMultiplier : public Multiplier
{
public:
    CataEtTyrandeKickMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "echo of tyrande kicks") {}
    float GetValue(Action* action) override;
};

// Echo of Tyrande (w68, #671): "reach spell" (SpellDistance 28.5 + combat reaches) pulled the mage from the band to 35.7 yd, into
// the outer Eye's gaze (52k), and back out again - a ping-pong with "et tyrande ranged band". "reach party member to heal" and
// "combat formation move" walk the same chords. While she is attackable a ranged bot's position belongs to the band alone
// (every 40 yd spell reaches her from 42 yd: 40 + 1.5 + her 1.8 combat reach). AC shape: GD / MgT multipliers zeroing
// ReachTargetAction / CombatFormationMoveAction during a positional mechanic (GDMultipliers.cpp:52-65, MgTMultipliers.cpp:174-215).
class CataEtTyrandeBandMultiplier : public Multiplier
{
public:
    CataEtTyrandeBandMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "echo of tyrande ranged band") {}
    float GetValue(Action* action) override;

private:
    uint32 _stamp = 0;
    bool _cached = false;
};

#endif
