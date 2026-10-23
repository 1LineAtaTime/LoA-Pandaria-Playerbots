/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/NearestGameObjects.cpp) into
 * the Legends of Azeroth Pandaria fork.
 */

#include "NearestGameObjects.h"

// GridObject.h must precede the grid notifiers, or Corpse/AreaTrigger/DynamicObject fail to compile
// their GridObject base (same ordering NearestNpcsValue needs).
#include "GridObject.h"

#include "CellImpl.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"

namespace
{
/// Plain "is it within this many yards" check.
///
/// This core's Trinity::GameObjectInRangeCheck is not that - it tests
/// GameObject::IsAtInteractDistance, which is the object's own usable range rather than a distance,
/// so it would silently drop objects the tactics need to see (banners, flags) while reporting
/// others the bot cannot actually reach.
///
/// #151 (loot-stack-port.md section 2 step 2.5): also requires isSpawned() + a resolvable
/// GameObjectTemplate, matching AC's AnyGameObjectInObjectRangeCheck (Ai/Base/Value/
/// NearestGameObjects.h:16-32). Without this, AddAllLootAction's "nearest game objects" scan (and
/// LootObjectStack::Refresh, which also calls GetGOInfo() unconditionally) would pick up despawned
/// chests. This filter also affects "closest game objects" / "nearest game objects no los" (the BG
/// tactics) - AC-aligned, but re-run a BG smoke test at build/test time (loot-stack-port.md section
/// 5 risk 6).
struct GameObjectWithinRange
{
    GameObjectWithinRange(WorldObject const* source, float range) : _source(source), _range(range) {}

    bool operator()(GameObject* go) const
    {
        return go && go->isSpawned() && go->GetGOInfo() && _source->IsWithinDist(go, _range, false);
    }

    WorldObject const* _source;
    float _range;
};
}  // namespace

GuidVector NearestGameObjects::Calculate()
{
    std::list<GameObject*> targets;

    GameObjectWithinRange check(bot, range);
    Trinity::GameObjectListSearcher<GameObjectWithinRange> searcher(bot, targets, check);
    Cell::VisitAllObjects(bot, searcher, range);

    GuidVector result;
    result.reserve(targets.size());
    for (GameObject* go : targets)
        if (go)
            result.push_back(go->GetGUID());

    return result;
}
