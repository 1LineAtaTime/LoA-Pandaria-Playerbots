/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/PossibleRpgTargetsValue.cpp)
 * into the Legends of Azeroth Pandaria fork.
 */

#include "PossibleRpgTargetsValue.h"

#include "GridObject.h"
#include "CellImpl.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "TravelMgr.h"

#include <algorithm>
#include <unordered_set>

// #272 (AC PossibleRpgTargetsValue.cpp:17-38)
std::vector<uint32> const PossibleRpgTargetsValue::allowedNpcFlags = {
    UNIT_NPC_FLAG_INNKEEPER,
    UNIT_NPC_FLAG_GOSSIP,
    UNIT_NPC_FLAG_QUESTGIVER,
    UNIT_NPC_FLAG_FLIGHTMASTER,
    UNIT_NPC_FLAG_BANKER,
    UNIT_NPC_FLAG_GUILD_BANKER,
    UNIT_NPC_FLAG_TRAINER_CLASS,
    UNIT_NPC_FLAG_TRAINER_PROFESSION,
    UNIT_NPC_FLAG_VENDOR_AMMO,
    UNIT_NPC_FLAG_VENDOR_FOOD,
    UNIT_NPC_FLAG_VENDOR_POISON,
    UNIT_NPC_FLAG_VENDOR_REAGENT,
    UNIT_NPC_FLAG_AUCTIONEER,
    UNIT_NPC_FLAG_STABLEMASTER,
    UNIT_NPC_FLAG_PETITIONER,
    UNIT_NPC_FLAG_TABARDDESIGNER,
    UNIT_NPC_FLAG_BATTLEMASTER,
    UNIT_NPC_FLAG_TRAINER,
    UNIT_NPC_FLAG_VENDOR,
    UNIT_NPC_FLAG_REPAIR,
};
PossibleRpgTargetsValue::PossibleRpgTargetsValue(PlayerbotAI* botAI, float range)
    : NearestUnitsValue(botAI, "possible rpg targets", range, true)
{
}

// #272 (AC PossibleRpgTargetsValue.cpp:42-47): Acore -> Trinity searcher, and Cell::VisitAllObjects, exactly as
// PossibleNewRpgTargetsValue::FindUnits below does on this fork.
void PossibleRpgTargetsValue::FindUnits(std::list<Unit*>& targets)
{
    Trinity::AnyUnitInObjectRangeCheck u_check(bot, range);
    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

// AC PossibleRpgTargetsValue.cpp:49-86. 5.4.8 adaptations: unit->IsPlayer() -> GetTypeId() == TYPEID_PLAYER and
// HasNpcFlag(f) -> HasFlag(UNIT_FIELD_NPC_FLAGS, f), as PossibleNewRpgTargetsValue::AcceptUnit does;
// ServerFacade::instance() -> sServerFacade (Utils/ServerFacade.h:28); sPlayerbotAIConfig.tooCloseDistance ->
// sPlayerbotAIConfig->tooCloseDistance (Utils/PlayerbotAIConfig.h:75).
bool PossibleRpgTargetsValue::AcceptUnit(Unit* unit)
{
    if (!unit || !unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
        return false;

    if (unit->IsHostileTo(bot) || unit->GetTypeId() == TYPEID_PLAYER)
        return false;

    if (sServerFacade->GetDistance2d(bot, unit) <= sPlayerbotAIConfig->tooCloseDistance)
        return false;

    if (unit->HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER))
        return false;

    for (uint32 npcFlag : allowedNpcFlags)
    {
        if (unit->HasFlag(UNIT_FIELD_NPC_FLAGS, npcFlag))
            return true;
    }

    TravelTarget* travelTarget = context->GetValue<TravelTarget*>("travel target")->Get();
    if (travelTarget && travelTarget->getDestination() &&
        static_cast<uint32>(travelTarget->getDestination()->getEntry()) == unit->GetEntry())
        return true;

    if (urand(1, 100) < 25 && unit->IsFriendlyTo(bot))
        return true;

    if (urand(1, 100) < 5)
        return true;

    return false;
}

std::vector<uint32> const PossibleNewRpgTargetsValue::allowedNpcFlags = {
    UNIT_NPC_FLAG_INNKEEPER,
    UNIT_NPC_FLAG_GOSSIP,
    UNIT_NPC_FLAG_QUESTGIVER,
    UNIT_NPC_FLAG_FLIGHTMASTER,
    UNIT_NPC_FLAG_BANKER,
    UNIT_NPC_FLAG_GUILD_BANKER,
    UNIT_NPC_FLAG_TRAINER_CLASS,
    UNIT_NPC_FLAG_TRAINER_PROFESSION,
    UNIT_NPC_FLAG_VENDOR_AMMO,
    UNIT_NPC_FLAG_VENDOR_FOOD,
    UNIT_NPC_FLAG_VENDOR_POISON,
    UNIT_NPC_FLAG_VENDOR_REAGENT,
    UNIT_NPC_FLAG_AUCTIONEER,
    UNIT_NPC_FLAG_STABLEMASTER,
    UNIT_NPC_FLAG_PETITIONER,
    UNIT_NPC_FLAG_TABARDDESIGNER,
    UNIT_NPC_FLAG_BATTLEMASTER,
    UNIT_NPC_FLAG_TRAINER,
    UNIT_NPC_FLAG_VENDOR,
    UNIT_NPC_FLAG_REPAIR,
};

// Sparse starting zones where the default scan range does not reach enough NPCs for WANDER_NPC
// (which wants at least 3). Upstream lists the 3.3.5 ones; the MoP starting area is added here.
static std::unordered_set<uint32> const rpgRangeOverrideAreaIds = {
    3526,  // Ammen Vale
    2117,  // Deathknell
    5833,  // The Wandering Isle - Wu Song Village
};

PossibleNewRpgTargetsValue::PossibleNewRpgTargetsValue(PlayerbotAI* botAI, float range)
    : NearestUnitsValue(botAI, "possible new rpg targets", range, true), defaultRange(range)
{
}

GuidVector PossibleNewRpgTargetsValue::Calculate()
{
    if (rpgRangeOverrideAreaIds.count(bot->GetAreaId()) && defaultRange < 200.0f)
        range = 200.0f;
    else
        range = defaultRange;

    std::list<Unit*> targets;
    FindUnits(targets);

    std::vector<std::pair<ObjectGuid, float>> guidDistancePairs;
    for (Unit* unit : targets)
    {
        if (AcceptUnit(unit) && (ignoreLos || bot->IsWithinLOSInMap(unit)))
            guidDistancePairs.emplace_back(unit->GetGUID(), bot->GetExactDist(unit));
    }

    // Unlike the base class, sort by distance: the rpg actions take the first entry that they can
    // interact with, so "first" should mean "closest".
    std::sort(guidDistancePairs.begin(), guidDistancePairs.end(),
              [](auto const& a, auto const& b) { return a.second < b.second; });

    GuidVector results;
    results.reserve(guidDistancePairs.size());
    for (auto const& pair : guidDistancePairs)
        results.push_back(pair.first);

    return results;
}

void PossibleNewRpgTargetsValue::FindUnits(std::list<Unit*>& targets)
{
    Trinity::AnyUnitInObjectRangeCheck u_check(bot, range);
    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

bool PossibleNewRpgTargetsValue::AcceptUnit(Unit* unit)
{
    if (!unit || !unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
        return false;

    if (unit->IsHostileTo(bot) || unit->GetTypeId() == TYPEID_PLAYER)
        return false;

    if (unit->HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER))
        return false;

    for (uint32 npcFlag : allowedNpcFlags)
        if (unit->HasFlag(UNIT_FIELD_NPC_FLAGS, npcFlag))
            return true;

    return false;
}

std::vector<GameobjectTypes> const PossibleNewRpgGameObjectsValue::allowedGOTypes = {
    GAMEOBJECT_TYPE_QUESTGIVER,
};

namespace
{
    /// Plain "within range of the bot" check. LoA's Trinity::GameObjectInRangeCheck measures
    /// interact distance against the object's box instead, which is not what we want here.
    struct GameObjectNearCheck
    {
        GameObjectNearCheck(WorldObject const* source, float range) : _source(source), _range(range) {}

        bool operator()(GameObject* go) const { return _source->IsWithinDistInMap(go, _range, false); }

    private:
        WorldObject const* _source;
        float _range;
    };
}

GuidVector PossibleNewRpgGameObjectsValue::Calculate()
{
    std::list<GameObject*> targets;
    GameObjectNearCheck u_check(bot, range);
    Trinity::GameObjectListSearcher<GameObjectNearCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);

    std::vector<std::pair<ObjectGuid, float>> guidDistancePairs;
    for (GameObject* go : targets)
    {
        if (!go || !go->IsInWorld())
            continue;

        if (std::find(allowedGOTypes.begin(), allowedGOTypes.end(), go->GetGoType()) == allowedGOTypes.end())
            continue;

        if (!ignoreLos && !bot->IsWithinLOSInMap(go))
            continue;

        guidDistancePairs.emplace_back(go->GetGUID(), bot->GetExactDist(go));
    }

    std::sort(guidDistancePairs.begin(), guidDistancePairs.end(),
              [](auto const& a, auto const& b) { return a.second < b.second; });

    GuidVector results;
    results.reserve(guidDistancePairs.size());
    for (auto const& pair : guidDistancePairs)
        results.push_back(pair.first);

    return results;
}
