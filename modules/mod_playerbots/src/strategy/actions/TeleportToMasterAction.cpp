/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TeleportToMasterAction.h"

#include "ChatHelper.h"
#include "DBCStores.h"
#include "Event.h"
#include "InstanceSaveMgr.h"
#include "LastMovementValue.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "Timer.h"
#include "VMapFactory.h"
#include "VMapManager2.h"
#include "World.h"

namespace
{
    // Wait before the automatic follow tries a refused destination again, so that a bot that could not enter
    // joins once the reason is gone (a level up, the master leaving again) without running the checks every tick
    constexpr uint32 FOLLOW_TELEPORT_RETRY_DELAY = 10 * IN_MILLISECONDS;

    // Teleports to one destination within FOLLOW_TELEPORT_ATTEMPT_WINDOW before the automatic follow stops
    // trying, e.g. when the core sent the bot to its homebind instead of the master's map
    constexpr uint32 FOLLOW_TELEPORT_MAX_ATTEMPTS = 2;
    constexpr uint32 FOLLOW_TELEPORT_ATTEMPT_WINDOW = 60 * IN_MILLISECONDS;
}

bool TeleportToMasterAction::isPossible()
{
    Player* master = GetMaster();
    return master && master != bot && master->IsInWorld() && !master->IsBeingTeleported() &&
        !bot->IsBeingTeleported();
}

bool TeleportToMasterAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    // The follow strategy's trigger runs this automatically; the "summon" chat command runs it on demand, and
    // then also when the bot is on the master's map but far away
    bool const manual = event.GetSource() == "summon";

    Destination destination;
    destination.mapId = master->GetMapId();
    destination.instanceId = master->GetInstanceId();

    uint32 const now = getMSTime();
    if (attempts && getMSTimeDiff(attemptTime, now) >= FOLLOW_TELEPORT_ATTEMPT_WINDOW)
        attempts = 0;

    if (!(destination == lastDestination) || manual)
    {
        lastDestination = destination;
        attempts = 0;
        refused = false;
        refusedReason.clear();
    }
    else if (refused && getMSTimeDiff(refusedTime, now) < FOLLOW_TELEPORT_RETRY_DELAY)
        return false;
    else if (attempts >= FOLLOW_TELEPORT_MAX_ATTEMPTS)
    {
        Refuse(destination, "I can't get to you, summon me when you need me", false);
        return false;
    }

    // Conditions that pass by themselves (combat, a flight) are only told when asked for
    std::string reason;
    if (!CanLeave(master, reason))
    {
        if (manual)
            botAI->TellMasterNoFacing(reason);
        return false;
    }

    // #985: nothing is changed on the bot until the destination has accepted it. A dead bot is only checked here
    // (ReviveBotWhenSummoned); it is revived after the entry check, so a refused bot is not resurrected in place.
    if (!bot->IsAlive() && !CanRevive(master, reason))
    {
        Refuse(destination, reason, manual);
        return false;
    }

    if (!CanEnter(master, reason))
    {
        Refuse(destination, reason, manual);
        return false;
    }

    if (!bot->IsAlive())
        Revive();

    if (!Teleport(master))
    {
        Refuse(destination, "I can't get to you", manual);
        return false;
    }

    ++attempts;
    attemptTime = now;
    refused = false;
    refusedReason.clear();
    return true;
}

bool TeleportToMasterAction::CanLeave(Player* master, std::string& reason)
{
    MapEntry const* masterMap = sMapStore.LookupEntry(master->GetMapId());

    if (bot->IsInCombat())
        reason = "I'm in combat";
    else if (bot->GetVehicle())
        reason = "I can't leave this vehicle";
    else if (bot->IsInFlight())
        reason = "I'm on a flight path";
    else if (bot->InBattleground() || bot->GetMap()->IsBattlegroundOrArena())
        reason = "I'm in a battleground";
    else if (!masterMap || master->InBattleground() || masterMap->IsBattlegroundOrArena())
        reason = "I can't follow you there";
    else if (!bot->GetGroup() || bot->GetGroup() != master->GetGroup())
        reason = "I'm not in your group";
    else if (!master->IsAlive())
        reason = "I can't come to you while you're dead";
    else
        return true;

    return false;
}

bool TeleportToMasterAction::CanRevive(Player* master, std::string& reason)
{
    // As AzerothCore's AiPlayerbot.ReviveBotWhenSummoned: 0 never, 1 if the master is alive and out of combat,
    // 2 always
    uint32 const mode = sPlayerbotAIConfig->reviveBotWhenSummoned;
    bool const revive = mode >= 2 || (mode == 1 && master->IsAlive() && !master->IsInCombat());
    if (!revive)
    {
        reason = mode == 1 ? "I'm dead, I'll come when you're out of combat" : "I'm dead, revive me first";
        return false;
    }

    return true;
}

void TeleportToMasterAction::Revive()
{
    bot->ResurrectPlayer(1.0f, false);
    bot->SpawnCorpseBones();
}

bool TeleportToMasterAction::CanEnter(Player* master, std::string& reason)
{
    uint32 const mapId = master->GetMapId();
    uint32 const instanceId = master->GetInstanceId();

    MapEntry const* entry = sMapStore.LookupEntry(mapId);
    if (!entry)
        return false;

    // Open world maps have no entry rules. A bot already in the master's instance moves within it, and must not
    // be checked: InstanceMap::CanEnter asserts on a player who is already in the map.
    if (!entry->IsDungeon() || (bot->GetMapId() == mapId && bot->GetInstanceId() == instanceId))
        return true;

    // MapManager::CanPlayerEnter refuses a dead player whose corpse is outside the instance (and spawns its corpse
    // bones as a side effect), so a dead bot cannot be checked without being revived first. The revive happens only
    // after this check passes, hence a dead bot does not follow into a different instance: it is told so instead.
    if (!bot->IsAlive())
    {
        reason = "I'm dead, revive me first";
        return false;
    }

    // Lockouts, difficulty, raid group, instance full, encounter in progress, then level, items, quests and
    // achievements (Player::Satisfy); the same checks Player::TeleportTo makes again. A bot in another copy of the
    // map may be in the group's bound copy that CanPlayerEnter checks, so it is flagged as Teleport() flags it.
    bool const sameMapId = bot->GetMapId() == mapId;
    if (sameMapId)
        bot->SetForcedTeleportFar(true);

    bool const canEnter = sMapMgr->CanPlayerEnter(mapId, bot);

    if (sameMapId)
        bot->SetForcedTeleportFar(false);

    if (!canEnter)
    {
        reason = GetEntryRefusal(entry);
        return false;
    }

    // The core puts the bot into the instance its permanent bind or else the group's bind points at
    // (MapInstanced::CreateInstanceForPlayer), or opens a new one. Refuse rather than enter another copy alone.
    // In an LFG group only the group's bind counts.
    Group* group = bot->GetGroup();
    bool const isLfgGroup = group && group->isLFGGroup();
    Difficulty const difficulty = bot->GetDifficulty(entry->IsRaid());
    InstancePlayerBind* playerBind = isLfgGroup ? nullptr : bot->GetBoundInstance(mapId, difficulty);
    InstanceSave* save = playerBind ? playerBind->save : nullptr;
    if ((!playerBind || !playerBind->perm) && group)
    {
        if (InstanceGroupBind* groupBind = group->GetBoundInstance(entry))
            save = groupBind->save;
    }

    if (!save || save->GetInstanceId() != instanceId)
    {
        std::string const mapName = entry->name;
        reason = playerBind && playerBind->perm ? "I'm saved to another instance of " + mapName
                                                : "I can't get into your instance of " + mapName;
        return false;
    }

    return true;
}

std::string const TeleportToMasterAction::GetEntryRefusal(MapEntry const* entry)
{
    std::string const mapName = entry->name;
    uint32 const mapId = entry->MapID;

    Group* group = bot->GetGroup();
    if (entry->IsRaid() && entry->Expansion() >= EXPANSION_MISTS_OF_PANDARIA && (!group || !group->isRaidGroup()) &&
        !sWorld->getBoolConfig(CONFIG_INSTANCE_IGNORE_RAID))
        return "I need a raid group to enter " + mapName;

    Difficulty difficulty = bot->GetDifficulty(entry->IsRaid());
    if (!sDBCManager.GetDownscaledMapDifficultyData(mapId, difficulty))
        return mapName + " has no such difficulty";

    // The access requirement checks of Player::Satisfy, which only reports them to the bot's own client
    if (AccessRequirement const* ar = sObjectMgr->GetAccessRequirement(mapId, bot->GetDifficulty(entry->IsRaid())))
    {
        if (!sWorld->getBoolConfig(CONFIG_INSTANCE_IGNORE_LEVEL))
        {
            if (ar->levelMin && bot->GetLevel() < ar->levelMin)
                return "I need to be level " + std::to_string(ar->levelMin) + " to enter " + mapName;

            if (ar->levelMax && bot->GetLevel() > ar->levelMax)
                return "I'm too high level to enter " + mapName;
        }

        uint32 missingItem = 0;
        if (ar->item)
        {
            if (!bot->HasItemCount(ar->item) && (!ar->item2 || !bot->HasItemCount(ar->item2)))
                missingItem = ar->item;
        }
        else if (ar->item2 && !bot->HasItemCount(ar->item2))
            missingItem = ar->item2;

        if (missingItem)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(missingItem);
            return "I need " + (proto ? ChatHelper::FormatItem(proto) : "an item") + " to enter " + mapName;
        }

        uint32 const quest = bot->GetTeam() == ALLIANCE ? ar->quest_A : ar->quest_H;
        if (quest && !bot->GetQuestRewardStatus(quest))
        {
            Quest const* questTemplate = sObjectMgr->GetQuestTemplate(quest);
            return "I need to complete " + (questTemplate ? ChatHelper::FormatQuest(questTemplate) : "a quest") +
                " to enter " + mapName;
        }

        // Player::Satisfy checks the achievement on the group leader
        if (ar->achievement)
        {
            Player* leader = group ? ObjectAccessor::FindConnectedPlayer(group->GetLeaderGUID()) : bot;
            if (!leader || !leader->HasAchieved(ar->achievement))
                return "Our group leader needs an achievement to enter " + mapName;
        }
    }

    InstanceSave* save = bot->GetInstanceSave(mapId, entry->IsRaid());
    if (!bot->CheckInstanceCount(save ? save->GetInstanceId() : 0))
        return "I've entered too many instances in the last hour";

    return "I can't enter " + mapName + " now: I may be saved to another instance, or it is full or in combat";
}

bool TeleportToMasterAction::Teleport(Player* master)
{
    uint32 const mapId = master->GetMapId();
    uint32 const instanceId = master->GetInstanceId();
    float const masterX = master->GetPositionX();
    float const masterY = master->GetPositionY();
    float const masterZ = master->GetPositionZ();
    float const orientation = master->GetOrientation();
    float const height = master->GetCollisionHeight();

    // A spot beside the master in line of sight, as AzerothCore's SummonAction::Teleport: the bot's follow angle
    // first, then the angles to either side of it. Only the static geometry is checked, since the master's map
    // and its dynamic objects may be updated by another thread; the master's own spot is the fallback.
    float x = masterX;
    float y = masterY;
    float const followAngle = GetFollowAngle();
    float const distance = sPlayerbotAIConfig->followDistance;
    VMAP::VMapManager2* vmapManager = VMAP::VMapFactory::createOrGetVMapManager();
    for (uint32 i = 0; i < 8; ++i)
    {
        float const offset = static_cast<float>((i + 1) / 2) * static_cast<float>(M_PI) / 4.0f;
        float const angle = followAngle + (i % 2 ? offset : -offset);
        float const spotX = masterX + std::cos(angle) * distance;
        float const spotY = masterY + std::sin(angle) * distance;
        if (vmapManager->isInLineOfSight(mapId, masterX, masterY, masterZ + height, spotX, spotY, masterZ + height,
                VMAP::ModelIgnoreFlags::Nothing))
        {
            x = spotX;
            y = spotY;
            break;
        }
    }

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    AI_VALUE(LastMovement&, "last movement").clear();

    // Another copy of the same map: a teleport by map id alone would stay in the bot's own copy
    bool const sameMap = bot->GetMapId() == mapId;
    bool const otherInstance = sameMap && bot->GetInstanceId() != instanceId;
    if (otherInstance)
        bot->SetForcedTeleportFar(true);

    bool const teleported = bot->TeleportTo(mapId, x, y, masterZ, orientation);

    if (otherInstance)
        bot->SetForcedTeleportFar(false);

    if (!teleported)
        return false;

    // A pet comes along on a near teleport if it is close enough to be kept; on a far teleport the core
    // unsummons it and summons it again on arrival
    if (sameMap && !otherInstance)
    {
        if (Pet* pet = bot->GetPet())
            pet->NearTeleportTo(x, y, masterZ, orientation);
    }

    return true;
}

void TeleportToMasterAction::Refuse(Destination const& destination, std::string const& reason, bool manual)
{
    // Tell each reason once per destination; a manual summon always gets an answer
    if (manual || reason != refusedReason)
        botAI->TellMasterNoFacing(reason);

    lastDestination = destination;
    refused = true;
    refusedTime = getMSTime();
    refusedReason = reason;
}
