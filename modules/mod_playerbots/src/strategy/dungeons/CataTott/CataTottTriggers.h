/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (epic #373, issue #661). See CataTottShared.h for sources.

#ifndef PLAYERBOTS_CATATOTTTRIGGERS_H
#define PLAYERBOTS_CATATOTTTRIGGERS_H

#include "CataTottShared.h"
#include "Trigger.h"

// The bot stands in (or right next to) a ground effect: Geyser, vortex, Dark Fissure, Mind Fog, Earth Shards,
// Blight of Ozumat pool. One class, one instance per HazardId.
class CataTottHazardTrigger : public Trigger
{
public:
    CataTottHazardTrigger(PlayerbotAI* botAI, std::string const name, CataTott::HazardId hazard)
        : Trigger(botAI, name, 200), _hazard(hazard)
    {
    }

    bool IsActive() override;

private:
    CataTott::HazardId _hazard;
};

// Naz'jar's Tempest Witch: the bot carries a Lightning Surge charge and must stay away from the group
class CataTottSurgeTrigger : public Trigger
{
public:
    CataTottSurgeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata tott lightning surge") {}
    bool IsActive() override;
};

// Add priority for the DPS: Waterspout adds (Naz'jar), Faceless Sappers, the adds on Neptulon (DPS + tank) and
// Ozumat himself in stage 3
class CataTottFocusTrigger : public Trigger
{
public:
    CataTottFocusTrigger(PlayerbotAI* botAI, std::string const name, CataTott::FocusId focus)
        : Trigger(botAI, name), _focus(focus)
    {
    }

    bool IsActive() override;

private:
    CataTott::FocusId _focus;
};

// A unit in reach is casting Shock Blast / Brain Spike / Chain Lightning / Lava Bolt / Shadow Bolt / Veil of Shadow
class CataTottInterruptTrigger : public Trigger
{
public:
    CataTottInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata tott interrupt") {}
    bool IsActive() override;
};

#endif
