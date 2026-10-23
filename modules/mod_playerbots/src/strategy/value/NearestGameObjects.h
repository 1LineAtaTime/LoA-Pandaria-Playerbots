/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/NearestGameObjects.h and the
 * BgRoleValue from PvpValues.h) into the Legends of Azeroth Pandaria fork.
 *
 * "closest game objects", "nearest game objects no los" and "bg role" were all used by the ported
 * battleground tactics but registered by none of this fork's contexts. AI_VALUE and
 * context->GetValue<T>(name) both end in ->Get() on whatever GetValue returns, so an unregistered
 * name is a null dereference, not a miss - "bg role" crashed the worldserver the first time a bot
 * walked to its Warsong Gulch start position. Same defect class as "nearest npcs" and the flag
 * carrier values.
 */

#ifndef _PLAYERBOT_NEARESTGAMEOBJECTS_H
#define _PLAYERBOT_NEARESTGAMEOBJECTS_H

#include "PlayerbotAIConfig.h"
#include "Value.h"

class PlayerbotAI;

/// Every game object within `range` of the bot.
class NearestGameObjects : public ObjectGuidListCalculatedValue
{
public:
    NearestGameObjects(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->sightDistance,
                       bool /*ignoreLos*/ = false, std::string const name = "nearest game objects")
        : ObjectGuidListCalculatedValue(botAI, name, 1 * 1000), range(range)
    {
    }

protected:
    GuidVector Calculate() override;

private:
    float range;
};

/// A bot's assigned job inside a battleground. Set by the tactics, not calculated - it only needs
/// to persist between decisions, which is what ManualSetValue is for.
class BgRoleValue : public ManualSetValue<uint32>
{
public:
    BgRoleValue(PlayerbotAI* botAI) : ManualSetValue<uint32>(botAI, 0, "bg role") {}
};

#endif
