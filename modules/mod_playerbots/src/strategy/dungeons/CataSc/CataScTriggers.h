/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: The Stonecore (epic #373, issue #662). See CataScShared.h for sources.

#ifndef PLAYERBOTS_CATASCTRIGGERS_H
#define PLAYERBOTS_CATASCTRIGGERS_H

#include "CataScShared.h"
#include "Trigger.h"

// Slabhide's Lava Fissure / Azil's Gravity Well: the bot stands in (or right next to) the ground effect creature
class CataScHazardTrigger : public Trigger
{
public:
    CataScHazardTrigger(PlayerbotAI* botAI, std::string const name, CataSc::HazardId hazard)
        : Trigger(botAI, name, 200), _hazard(hazard)
    {
    }

    bool IsActive() override;

private:
    CataSc::HazardId _hazard;
};

// Slabhide's stalactite warning circle / Corborus' Crystal Barrage: the bot stands in the DynamicObject
class CataScDynHazardTrigger : public Trigger
{
public:
    CataScDynHazardTrigger(PlayerbotAI* botAI, std::string const name, CataSc::DynHazardId hazard)
        : Trigger(botAI, name, 200), _hazard(hazard)
    {
    }

    bool IsActive() override;

private:
    CataSc::DynHazardId _hazard;
};

// Ozruk is casting Shatter and the bot is inside the 15 yd blast
class CataScShatterTrigger : public Trigger
{
public:
    CataScShatterTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata sc shatter") {}
    bool IsActive() override;
};

// Ozruk is casting Ground Slam and the bot (not his target) stands in front of him
class CataScGroundSlamTrigger : public Trigger
{
public:
    CataScGroundSlamTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata sc ground slam") {}
    bool IsActive() override;
};

// Slabhide's Sand Blast cone (instant, so this is positional): the bot (not his target) stands in front of him
class CataScSandBlastTrigger : public Trigger
{
public:
    CataScSandBlastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata sc sand blast") {}
    bool IsActive() override;
};

// Add priority for the DPS: Crystal Shards (Corborus), Devout Followers while Azil is shielded
class CataScFocusTrigger : public Trigger
{
public:
    CataScFocusTrigger(PlayerbotAI* botAI, std::string const name, CataSc::FocusId focus)
        : Trigger(botAI, name), _focus(focus)
    {
    }

    bool IsActive() override;

private:
    CataSc::FocusId _focus;
};

#endif
