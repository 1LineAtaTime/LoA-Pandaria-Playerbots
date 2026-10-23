/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GatherAction.h"

#include "CellImpl.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "LootMgr.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "WorldSession.h"

namespace
{
    /// Facts about a real herb/vein the bot can work, filled in by GetGatheringInfo.
    struct GatheringNode
    {
        uint32 skill = 0;    // SKILL_HERBALISM or SKILL_MINING
        uint32 have = 0;     // the bot's current value in that skill
        uint32 required = 0; // skill demanded by the node's lock (0 = none stored in Lock.dbc)
    };

    /// A real gathering node is a CHEST with a herbalism/mining skill lock and a loot template.
    /// Player::SendLoot picks the gathering skill from the FIRST LOCK_KEY_SKILL case of the lock,
    /// so we mirror that here. Quest pickups such as "Juicy Apple" (entry 203762, lock 259) only
    /// carry a later herbalism case; looking at every case (the old behaviour) wrongly accepted
    /// them, which is what produced the endless `[GATHER] ... (0 items)` and SendLoot errors.
    bool GetGatheringInfo(Player* bot, GameObject* go, GatheringNode& node)
    {
        if (!go || go->GetGoType() != GAMEOBJECT_TYPE_CHEST)
            return false;

        GameObjectTemplate const* info = go->GetGOInfo();
        // No loot template -> nothing to harvest (point checks, depleted or deprecated nodes).
        if (!info || !info->GetLootId())
            return false;

        LockEntry const* lock = sLockStore.LookupEntry(info->GetLockId());
        if (!lock)
            return false;

        uint32 i = 0;
        for (; i < MAX_LOCK_CASE; ++i)
            if (lock->Type[i] == LOCK_KEY_SKILL)
                break;

        if (i == MAX_LOCK_CASE)
            return false;

        uint32 const skill = SkillByLockType(LockType(lock->Index[i]));
        if (skill != SKILL_HERBALISM && skill != SKILL_MINING)
            return false;

        uint32 const have = bot->GetSkillValue(skill);
        uint32 const required = lock->Skill[i];
        // Lock.dbc holds the requirement for the old-world nodes; the Pandaria nodes leave it at 0,
        // so at minimum require the bot to be trained in the profession.
        if (required ? have < required : have == 0)
            return false;

        node.skill = skill;
        node.have = have;
        node.required = required;
        return true;
    }

    struct ChestNearCheck
    {
        ChestNearCheck(WorldObject const* source, float range) : _source(source), _range(range) {}

        bool operator()(GameObject* go) const
        {
            return go && go->GetGoType() == GAMEOBJECT_TYPE_CHEST &&
                   _source->IsWithinDistInMap(go, _range, false);
        }

    private:
        WorldObject const* _source;
        float _range;
    };
}

GameObject* GatherAction::FindGatherableNode()
{
    std::list<GameObject*> objects;
    ChestNearCheck check(bot, 30.0f);
    Trinity::GameObjectListSearcher<ChestNearCheck> searcher(bot, objects, check);
    Cell::VisitAllObjects(bot, searcher, 30.0f);

    GameObject* best = nullptr;
    float bestDist = 0.0f;
    for (GameObject* go : objects)
    {
        if (!go || !go->IsInWorld() || go->getLootState() != GO_READY)
            continue;

        GatheringNode node;
        if (!GetGatheringInfo(bot, go, node))
            continue;

        float const dist = bot->GetExactDist(go);
        if (!best || dist < bestDist)
        {
            best = go;
            bestDist = dist;
        }
    }

    return best;
}

bool GatherAction::isUseful()
{
    _node = FindGatherableNode();
    return _node != nullptr;
}

bool GatherAction::Execute(Event /*event*/)
{
    if (!_node)
        _node = FindGatherableNode();

    if (!_node)
        return false;

    GameObject* node = _node;

    // Walk to the node first; harvest once it is in reach.
    if (!bot->IsWithinDistInMap(node, INTERACTION_DISTANCE))
    {
        if (!node->IsInWorld() || node->GetMapId() != bot->GetMapId())
        {
            _node = nullptr;
            return false;
        }

        if (bot->isMoving())
            return true;

        float x = node->GetPositionX();
        float y = node->GetPositionY();
        float z = node->GetPositionZ();
        // The source of CheckCollisionAndGetValidCoords is fed to PathGenerator, which needs a Unit
        // (it dereferences _sourceUnit), so pass the bot - not the GameObject node.
        if (!node->GetMap()->CheckCollisionAndGetValidCoords(bot, node->GetPositionX(), node->GetPositionY(),
                                                             node->GetPositionZ(), x, y, z))
            return false;

        return MoveTo(node->GetMapId(), x, y, z, false, false, false, true);
    }

    _node = nullptr;

    // Re-validate immediately before harvesting: another bot may have taken it during the walk.
    GatheringNode info;
    if (!node->IsInWorld() || node->getLootState() != GO_READY ||
        node->GetMapId() != bot->GetMapId() || !GetGatheringInfo(bot, node, info))
        return false;

    ObjectGuid const guid = node->GetGUID();
    bot->StopMoving();
    bot->SetFacingToObject(node);

    // Same path the client uses for a herb/vein: open the node as mining-or-herbalism loot, take
    // everything, then release. Player::SendLoot fills the loot from the gameobject loot template.
    bot->SendLoot(guid, LOOT_MINING_OR_HERBALISM);

    // SendLoot bails out without opening anything when the node is no longer lootable; do not
    // touch the stale Loot object in that case.
    if (bot->GetLootGUID() != guid)
        return false;

    Loot& loot = node->loot;
    uint32 items = 0;
    uint32 const maxSlot = loot.GetMaxSlotInLootFor(bot);
    for (uint32 slot = 0; slot < maxSlot; ++slot)
    {
        if (loot.LootItemInSlot(slot, bot))
        {
            bot->StoreLootItem(uint8(slot), &loot);
            ++items;
        }
    }

    bot->GetSession()->DoLootRelease(guid);

    TC_LOG_DEBUG("playerbots", "[GATHER] %s gathered %s (goType=%u entry=%u skill=%u have=%u need=%u, %u items)",
                 bot->GetName().c_str(), node->GetName().c_str(), uint32(node->GetGoType()), node->GetEntry(),
                 info.skill, info.have, info.required, items);
    return true;
}
