/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots (Mgr/Item/LootObjectStack.cpp, issue #151). 5.4.8 adaptations are
 * called out at each site below.
 */

#include "LootObjectStack.h"

#include "DBCStores.h"
#include "GameObject.h"
#include "LootMgr.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SharedDefines.h"

#include <algorithm>
#include <limits>

#define MAX_LOOT_OBJECT_COUNT 200

namespace
{
    // AC PlayerbotAI::HasSkill(SkillType) (Bot/PlayerbotAI.cpp:5427) has no equivalent on this
    // fork's PlayerbotAI (issue #151 keeps PlayerbotAI.h/.cpp untouched outside the loot-target
    // reset and CastSpell GO-target hookups - see AI/PlayerbotAI.cpp). Player::HasSkill exists on
    // this core (Entities/Player/Player.h:2485), so this is a same-behaviour local helper.
    bool BotHasSkill(Player* bot, uint32 skillId)
    {
        return skillId != SKILL_NONE && bot->HasSkill(skillId) && bot->GetSkillValue(skillId) > 0;
    }
}

LootTarget::LootTarget(ObjectGuid guid) : guid(guid), asOfTime(time(nullptr)) {}

LootTarget::LootTarget(LootTarget const& other)
{
    guid = other.guid;
    asOfTime = other.asOfTime;
}

LootTarget& LootTarget::operator=(LootTarget const& other)
{
    if ((void*)this == (void*)&other)
        return *this;

    guid = other.guid;
    asOfTime = other.asOfTime;

    return *this;
}

bool LootTarget::operator<(LootTarget const& other) const { return guid < other.guid; }

void LootTargetList::shrink(time_t fromTime)
{
    for (std::set<LootTarget>::iterator i = begin(); i != end();)
    {
        if (i->asOfTime <= fromTime)
            erase(i++);
        else
            ++i;
    }
}

LootObject::LootObject(Player* bot, ObjectGuid guid) : guid(), skillId(SKILL_NONE), reqSkillValue(0), reqItem(0)
{
    Refresh(bot, guid);
}

void LootObject::Refresh(Player* bot, ObjectGuid lootGUID)
{
    skillId = SKILL_NONE;
    reqSkillValue = 0;
    reqItem = 0;
    guid.Clear();

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return;

    // AC: creature->getDeathState() == DeathState::Corpse -> this core spells the enum value CORPSE
    // directly (Entities/Unit/Unit.h:326-333).
    Creature* creature = botAI->GetCreature(lootGUID);
    if (creature && creature->getDeathState() == CORPSE)
    {
        // AC: HasFlag(UNIT_DYNAMIC_FLAGS, ...) -> this core's update field is
        // OBJECT_FIELD_DYNAMIC_FLAGS (core:Handlers/LootHandler.cpp:546 clears the same flag with
        // that name on release).
        // 5.4.8 difference: this core sets UNIT_FLAG_SKINNABLE the moment a
        // skinnable creature dies (core:Entities/Creature/Creature.cpp:1681-1685) and UNIT_DYNFLAG_LOOTABLE even
        // for an empty loot; AC sets SKINNABLE only once the loot is gone and LOOTABLE only for a non-empty loot.
        // Taken verbatim, every fresh beast corpse became a skinning target that a non-skinner could never loot.
        // So: a corpse with loot left is a plain loot target; only an emptied one is a skinning target.
        bool const hasLoot =
            creature->HasFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE) && !creature->loot.isLooted();
        if (hasLoot)
            guid = lootGUID;

        if (!hasLoot && creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
        {
            skillId = creature->GetCreatureTemplate()->GetRequiredLootSkill();
            uint32 targetLevel = creature->GetLevel();
            reqSkillValue = targetLevel < 10 ? 1 : targetLevel < 20 ? (targetLevel - 10) * 10 : targetLevel * 5;
            if (BotHasSkill(bot, skillId) && bot->GetSkillValue(skillId) >= reqSkillValue)
                guid = lootGUID;
        }

        return;
    }

    GameObject* go = botAI->GetGameObject(lootGUID);
    if (go && go->isSpawned() && go->GetGoState() == GO_STATE_READY)
    {
        // #156: herb/mining nodes are no longer skipped here. GatherStrategy now queues them through
        // "add gathering loot" exactly as AC does (AC Ai/Base/Strategy/LootNonCombatStrategy.cpp:19-23,
        // AC LootObjectStack.cpp has no such skip), so this stack is their only owner again.
        bool onlyHasQuestItems = true;
        bool hasAnyQuestItems = false;
        bool neededQuestItem = false;

        // AC: sObjectMgr->GetGameObjectQuestItemList(entry) returns a vector built from the
        // `gameobject_questitem` table; on 5.4.8 those ids live directly on the template as a fixed
        // array (core:Entities/GameObject/GameObjectData.h:29,42).
        GameObjectTemplate const* info = go->GetGOInfo();
        for (uint32 i = 0; i < MAX_GAMEOBJECT_QUEST_ITEMS; ++i)
        {
            uint32 itemId = info->questItems[i];
            if (!itemId)
                continue;

            hasAnyQuestItems = true;

            if (IsNeededForQuest(bot, itemId))
            {
                // A gathering node can also drop a needed quest item; gathering yields both, so
                // keep reading the lock below to set skillId rather than bailing here.
                this->guid = lootGUID;
                neededQuestItem = true;
            }

            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            if (!proto)
                continue;

            if (proto->Class != ITEM_CLASS_QUEST)
                onlyHasQuestItems = false;
        }

        // Retrieve the correct loot table entry
        uint32 lootEntry = go->GetGOInfo()->GetLootId();
        if (lootEntry == 0)
            return;

        // Check the main loot template
        if (LootTemplate const* lootTemplate = LootTemplates_Gameobject.GetLootFor(lootEntry))
        {
            Loot loot;
            // AC: lootTemplate->Process(loot, LootTemplates_Gameobject, 1, bot) -> this core's
            // Process(loot, rate, lootmode, groupId, player) expands LootTemplates_Reference itself
            // (core:Loot/LootMgr.h:281), so AC's manual reference-template walk afterwards is
            // redundant here and dropped.
            lootTemplate->Process(loot, true, 1, 0, bot);

            for (LootItem const& item : loot.items)
            {
                uint32 itemId = item.itemid;
                if (!itemId)
                    continue;

                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (!proto)
                    continue;

                if (proto->Class != ITEM_CLASS_QUEST)
                {
                    onlyHasQuestItems = false;
                    break;
                }
            }
        }

        // If gameobject has only quest items that bot doesn't need, skip it.
        if (!neededQuestItem && hasAnyQuestItems && onlyHasQuestItems)
            return;

        // Otherwise, loot it.
        guid = lootGUID;

        uint32 goId = go->GetEntry();
        uint32 lockId = go->GetGOInfo()->GetLockId();
        LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
        if (!lockInfo)
            return;

        for (uint8 i = 0; i < 8; ++i)
        {
            switch (lockInfo->Type[i])
            {
                case LOCK_KEY_ITEM:
                    if (lockInfo->Index[i] > 0)
                    {
                        reqItem = lockInfo->Index[i];
                        guid = lootGUID;
                    }
                    break;

                case LOCK_KEY_SKILL:
                    if (goId == 13891 || goId == 19535)  // Serpentbloom
                    {
                        this->guid = lootGUID;
                    }
                    else if (SkillByLockType(LockType(lockInfo->Index[i])) > 0)
                    {
                        skillId = SkillByLockType(LockType(lockInfo->Index[i]));
                        reqSkillValue = std::max((uint32)1, lockInfo->Skill[i]);
                        guid = lootGUID;
                    }
                    break;

                case LOCK_KEY_NONE:
                    guid = lootGUID;
                    break;
            }
        }
    }
}

bool LootObject::IsNeededForQuest(Player* bot, uint32 itemId)
{
    // AC walks RequiredItemId/RequiredItemCount over the quest log directly; this core's objective
    // model equivalent is Player::HasQuestForItem (core:Entities/Player/Player.cpp:18083), which
    // walks Quest::Objectives of type QUEST_OBJECTIVE_ITEM against GetQuestObjectiveCounter and
    // already covers ItemDrop - the same semantics as AC's loop (loot-stack-port.md section 2 step 1).
    return bot->HasQuestForItem(itemId);
}

WorldObject* LootObject::GetWorldObject(Player* bot)
{
    Refresh(bot, guid);

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return nullptr;

    Creature* creature = botAI->GetCreature(guid);
    if (creature && creature->getDeathState() == CORPSE && creature->IsInWorld())
        return creature;

    GameObject* go = botAI->GetGameObject(guid);
    if (go && go->isSpawned() && go->IsInWorld())
        return go;

    return nullptr;
}

LootObject::LootObject(LootObject const& other)
{
    guid = other.guid;
    skillId = other.skillId;
    reqSkillValue = other.reqSkillValue;
    reqItem = other.reqItem;
}

bool LootObject::IsLootPossible(Player* bot)
{
    if (IsEmpty() || !bot)
        return false;

    WorldObject* worldObj = GetWorldObject(bot);
    if (!worldObj)
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;

    if (reqItem && !bot->HasItemCount(reqItem, 1))
        return false;

    if (abs(worldObj->GetPositionZ() - bot->GetPositionZ()) > INTERACTION_DISTANCE - 2.0f)
        return false;

    Creature* creature = botAI->GetCreature(guid);
    if (creature && creature->getDeathState() == CORPSE)
    {
        // AC: bot->isAllowedToLoot(creature) -> this core spells it IsAllowedToLoot
        // (core:Entities/Player/Player.cpp:19394). It also folds in
        // IsDamageEnoughForLootingAndReward and HasPendingBind (5.4.8-only content; keep as-is).
        if (!bot->IsAllowedToLoot(creature) && skillId != SKILL_SKINNING)
            return false;
    }

    // Prevent bot from running to chests that are unlootable (e.g. Gunship Armory before completing
    // the event) or on respawn time
    GameObject* go = botAI->GetGameObject(guid);
    // AC: GAMEOBJECT_FLAGS -> this core's update field is GAMEOBJECT_FIELD_FLAGS.
    if (go && (go->HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE) || !go->isSpawned()))
        return false;

    // Conditional objects (quest chests, goobers, ...) are gated client-side on quest state.
    // A bot has no client, so make the same call the server makes for one.
    if (go && go->HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_INTERACT_COND) && !go->ActivateToQuest(bot))
        return false;

    if (skillId == SKILL_NONE)
        return true;

    if (skillId == SKILL_FISHING)
        return false;

    if (!BotHasSkill(bot, skillId))
        return false;

    if (!reqSkillValue)
        return true;

    uint32 skillValue = uint32(bot->GetSkillValue(skillId));
    if (reqSkillValue > skillValue)
        return false;

    if (skillId == SKILL_MINING && !bot->HasItemCount(756, 1) && !bot->HasItemCount(778, 1) &&
        !bot->HasItemCount(1819, 1) && !bot->HasItemCount(1893, 1) && !bot->HasItemCount(1959, 1) &&
        !bot->HasItemCount(2901, 1) && !bot->HasItemCount(9465, 1) && !bot->HasItemCount(20723, 1) &&
        !bot->HasItemCount(40772, 1) && !bot->HasItemCount(40892, 1) && !bot->HasItemCount(40893, 1))
    {
        return false;  // Bot is missing a mining pick
    }

    if (skillId == SKILL_SKINNING && !bot->HasItemCount(7005, 1) && !bot->HasItemCount(40772, 1) &&
        !bot->HasItemCount(40893, 1) && !bot->HasItemCount(12709, 1) && !bot->HasItemCount(19901, 1))
    {
        return false;  // Bot is missing a skinning knife
    }

    return true;
}

bool LootObjectStack::Add(ObjectGuid guid)
{
    if (availableLoot.size() >= MAX_LOOT_OBJECT_COUNT)
    {
        availableLoot.shrink(time(nullptr) - 30);
    }

    if (availableLoot.size() >= MAX_LOOT_OBJECT_COUNT)
    {
        availableLoot.clear();
    }

    if (!availableLoot.insert(guid).second)
        return false;

    return true;
}

void LootObjectStack::Remove(ObjectGuid guid)
{
    LootTargetList::iterator i = availableLoot.find(guid);
    if (i != availableLoot.end())
        availableLoot.erase(i);
}

void LootObjectStack::Clear() { availableLoot.clear(); }

bool LootObjectStack::CanLoot(float maxDistance)
{
    LootObject nearest = GetNearest(maxDistance);
    return !nearest.IsEmpty();
}

LootObject LootObjectStack::GetLoot(float maxDistance)
{
    LootObject nearest = GetNearest(maxDistance);
    return nearest.IsEmpty() ? LootObject() : nearest;
}

LootObject LootObjectStack::GetNearest(float maxDistance)
{
    availableLoot.shrink(time(nullptr) - 30);

    LootObject nearest;
    float nearestDistance = std::numeric_limits<float>::max();

    LootTargetList safeCopy(availableLoot);
    for (LootTargetList::iterator i = safeCopy.begin(); i != safeCopy.end(); i++)
    {
        ObjectGuid guid = i->guid;

        WorldObject* worldObj = ObjectAccessor::GetWorldObject(*bot, guid);
        if (!worldObj)
            continue;

        float distance = bot->GetDistance(worldObj);

        if (distance >= nearestDistance || (maxDistance && distance > maxDistance))
            continue;

        LootObject lootObject(bot, guid);

        if (!lootObject.IsLootPossible(bot))
            continue;

        nearestDistance = distance;
        nearest = lootObject;
    }

    return nearest;
}
