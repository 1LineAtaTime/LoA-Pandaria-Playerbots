/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DTKActions.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"

// AC Ai/Dungeon/DTK/DTKActions.cpp:10 (CorpseExplodeSpreadAction::Execute)
bool CorpseExplodeSpreadAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "trollgore");
    if (!boss) { return false; }

    float distance = 5.0f;
    float distanceExtra = 2.0f;
    GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
    for (auto i = corpses.begin(); i != corpses.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_DRAKKARI_INVADER)
        {
            float currentDistance = bot->GetExactDist2d(unit);
            if (currentDistance < distance + distanceExtra)
                return MoveAway(unit, distance + distanceExtra - currentDistance);
        }
    }
    return false;
}

// AC Ai/Dungeon/DTK/DTKActions.cpp:31 (AvoidArcaneFieldAction::Execute)
bool AvoidArcaneFieldAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "novos the summoner");
    if (!boss) { return false; }

    float distance = 12.0f;  // 11 unit radius, 1 unit added as buffer
    if (bot->GetExactDist2d(boss) < distance)
    {
        return MoveAway(boss, distance - bot->GetExactDist2d(boss));
    }
    return false;
}

// AC Ai/Dungeon/DTK/DTKActions.cpp:44 (NovosDefaultPositionAction::isUseful)
// 5.4.8: WorldObject::GetDistance(Position const&) exists (src/server/game/Entities/Object/Object.h:481).
bool NovosDefaultPositionAction::isUseful()
{
    // Distance to tether to centre of room
    float threshold = 15.0f;
    return bot->GetDistance(NOVOS_PARTY_POSITION) > threshold;
}
// AC Ai/Dungeon/DTK/DTKActions.cpp:50 (NovosDefaultPositionAction::Execute)
// 5.4.8: MoveNear(mapId,x,y,z,distance,priority) is on the fork (strategy/actions/MovementActions.h:51);
// ObjectGuid::operator! (ObjectGuid.h:251) keeps `!bot->GetTarget()` valid.
bool NovosDefaultPositionAction::Execute(Event /*event*/)
{
    float clusterDistance = 4.0f;
    // Only reposition if we're not killing anything
    if (!bot->GetTarget())
    {
        return MoveNear(bot->GetMap()->GetId(),
            NOVOS_PARTY_POSITION.GetPositionX(),
            NOVOS_PARTY_POSITION.GetPositionY(),
            NOVOS_PARTY_POSITION.GetPositionZ(),
            clusterDistance, MovementPriority::MOVEMENT_NORMAL);
    }
    return false;
}

// AC Ai/Dungeon/DTK/DTKActions.cpp:65 (NovosTargetPriorityAction::Execute)
// 5.4.8: AC botAI->IsDps/IsMelee/IsTank(...) -> static PlayerBotSpec::IsX(...) (AI/PlayerbotSpec.h:11-19).
bool NovosTargetPriorityAction::Execute(Event /*event*/)
{
    // TODO: This can be improved, some parts are still buggy.
    // But it works for now and this fight is very easy

    // Designate a dps char to handle the stairs adds.
    // This is probably better as a melee, so just pick the first
    // melee dps in the party. If none exist, pick the first ranged.

    // TODO: Switch to botAI->Index instead, cleaner
    Player* stairsDps = nullptr;
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Player* groupMember = botAI->GetPlayer(member);
        if (!groupMember) { continue; }

        if (PlayerBotSpec::IsDps(groupMember))
        {
            if (PlayerBotSpec::IsMelee(groupMember))
            {
                // Found our first melee dps, grab handle and break
                stairsDps = groupMember;
                break;
            }
            else
            {
                // Ranged dps, only set if none already assigned.
                // Don't break, we want to keep searching for a melee instead.
                if (!stairsDps)
                    stairsDps = groupMember;
            }
        }
    }

    Unit* selectedTargets[2] = {nullptr, nullptr};
    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit) { continue; }
        uint32 creatureId = unit->GetEntry();

        // Tank priority:
        // Hulking Corpse -> Crystal Handler
        if (PlayerBotSpec::IsTank(bot))
        {
            if (creatureId == NPC_HULKING_CORPSE)
                selectedTargets[0] = unit;

            else if (creatureId == NPC_CRYSTAL_HANDLER)
                selectedTargets[1] = unit;

        }
        // Dedicated stairs dps is assigned.
        // Priority: Risen Shadowcaster -> Fetid Troll Corpse
        else if (stairsDps && bot == stairsDps)
        {
            if (creatureId == NPC_RISEN_SHADOWCASTER)
            {
                if (!selectedTargets[0] || bot->GetDistance(unit) < bot->GetDistance(selectedTargets[0]) - 5.0f)
                    selectedTargets[0] = unit;

            }
            else if (creatureId == NPC_FETID_TROLL_CORPSE)
            {
                if (!selectedTargets[1] || bot->GetDistance(unit) < bot->GetDistance(selectedTargets[1]) - 5.0f)
                    selectedTargets[1] = unit;
            }
        }
        // All other dps priority:
        // Crystal Handler -> Hulking Corpse
        else if (PlayerBotSpec::IsDps(bot))
        {
            if (creatureId == NPC_CRYSTAL_HANDLER)
                selectedTargets[0] = unit;

            else if (creatureId == NPC_HULKING_CORPSE)
                selectedTargets[1] = unit;
        }
    }

    for (Unit* primaryTarget : selectedTargets)
    {
        // Attack the first valid target in the priority list
        if (primaryTarget)
        {
            if (AI_VALUE(Unit*, "current target") != primaryTarget)
            {
                // bot->Yell(primaryTarget->GetName(), LANG_UNIVERSAL);
                return Attack(primaryTarget);
            }
            // Don't continue loop here, the target exists so we don't
            // want to move down the prio list. We just don't need to send attack
            // command again, just return false and exit the loop that way
            return false;
        }
    }
    return false;
}
