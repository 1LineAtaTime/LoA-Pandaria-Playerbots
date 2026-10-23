/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/PossibleRpgTargetsValue.h)
 * into the Legends of Azeroth Pandaria fork.
 */

#ifndef _PLAYERBOT_POSSIBLERPGTARGETSVALUE_H
#define _PLAYERBOT_POSSIBLERPGTARGETSVALUE_H

#include "NearestUnitsValue.h"
#include "PlayerbotAIConfig.h"
#include "SharedDefines.h"
#include "Value.h"

class PlayerbotAI;

// #272 (AC Ai/Base/Value/PossibleRpgTargetsValue.h:17-27): the OLD "rpg" strategy's target list. The New* classes
// below are the "new rpg" ones and are untouched.
class PossibleRpgTargetsValue : public NearestUnitsValue
{
public:
    PossibleRpgTargetsValue(PlayerbotAI* botAI, float range = 70.0f);

    static std::vector<uint32> const allowedNpcFlags;

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

/// Interactable NPCs near the bot (quest givers, vendors, trainers, bankers, auctioneers, ...),
/// sorted nearest first. This is what the "new rpg" actions walk between, so it is the value that
/// makes bots visit NPCs rather than just wander.
class PossibleNewRpgTargetsValue : public NearestUnitsValue
{
public:
    PossibleNewRpgTargetsValue(PlayerbotAI* botAI, float range = 150.0f);

    static std::vector<uint32> const allowedNpcFlags;

    GuidVector Calculate() override;

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;

private:
    float defaultRange;
};

/// Quest-giver game objects near the bot, sorted nearest first.
class PossibleNewRpgGameObjectsValue : public ObjectGuidListCalculatedValue
{
public:
    PossibleNewRpgGameObjectsValue(PlayerbotAI* botAI, float range = 150.0f, bool ignoreLos = true)
        : ObjectGuidListCalculatedValue(botAI, "possible new rpg game objects"), range(range), ignoreLos(ignoreLos)
    {
    }

    static std::vector<GameobjectTypes> const allowedGOTypes;

    GuidVector Calculate() override;

private:
    float range;
    bool ignoreLos;
};

#endif
