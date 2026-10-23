/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight bot strategy "cata-hot" - multipliers (issue #673, epic #373). Pattern: AC Ai/Dungeon/HoS/HoSMultipliers.h.
// The multiplier keeps a 100 ms cache: the engine asks it once per queued action, the checks behind it are grid searches.

#ifndef PLAYERBOTS_CATAHOTMULTIPLIERS_H
#define PLAYERBOTS_CATAHOTMULTIPLIERS_H

#include "Define.h"
#include "Multiplier.h"

// While smoke, an orb or a wave needs the bot to move, only this strategy's own moving actions may move it
// (AC HoS KrystallusMultiplier shape).
class CataHotHazardMultiplier : public Multiplier
{
public:
    CataHotHazardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hour of twilight hazards") {}
    float GetValue(Action* action) override;

private:
    bool NeedsMove();

    uint32 _stamp = 0;
    bool _cached = false;
};

// #673 (w65): while the Icy Tomb holds Thrall, a DPS bot that is on the tomb stays on it. Without this, "hot kill icy tomb"
// (ACTION_RAID + 6) switched the bot to the tomb, the next tick it returned FAILED (already the target, AttackActions.cpp:161)
// and "not dps target active" -> "dps assist" switched it back to Arcurion: 527 OK / 527 FAILED, the tomb stayed at 100% and the
// boss took half the damage (76% after 263 s, wipe). Shape of AC Ai/Dungeon/AK/AKMultipliers.cpp:15-29 (ElderNadoxMultiplier:
// DpsAssistAction 0 while the priority add lives).
class CataHotIcyTombMultiplier : public Multiplier
{
public:
    CataHotIcyTombMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hour of twilight icy tomb") {}
    float GetValue(Action* action) override;
};

#endif
