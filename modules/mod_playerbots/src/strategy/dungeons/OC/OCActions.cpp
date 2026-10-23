/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "OCActions.h"
#include "Group.h"
#include "LastSpellCastValue.h"
#include "Map.h"
#include "MoveSplineInit.h"
#include "OCTriggers.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellHistory.h"
#include "Vehicle.h"
#include "WorldSession.h"
#include <map>

namespace
{
    // 5.4.8 ADAPTATION: AC Unit::AddSpellCooldown(spellId, 0, ms) / Unit::HasSpellCooldown(spellId) on the
    // drake (AC OCActions.cpp:210,229,231,251,254,311,318). The fork's Unit has neither (Player::AddSpellCooldown
    // Player.h:2151; Creature::HasSpellCooldown Creature.h:329 only knows the creature's own cooldown map, not
    // SpellHistory). PlayerbotAI::CanCastVehicleSpell reads the vehicle base's SpellHistory
    // (AI/PlayerbotAI.cpp:3723), so both are mapped onto it:
    //  - add: SpellHistory::AddCooldown(spellId, itemId, ms, category, categoryMs) (SpellHistory.h:79, .cpp:403-419)
    //  - has: !SpellHistory::IsReady(spellId) (SpellHistory.h:71, .cpp:173-198; HasCooldown is private, :123)
    // Unit::GetSpellHistory is Unit.h:2587.
    void AddDrakeSpellCooldown(Unit* vehicleBase, uint32 spellId, uint32 cooldownMs)
    {
        if (vehicleBase && vehicleBase->GetSpellHistory())
            vehicleBase->GetSpellHistory()->AddCooldown(spellId, 0, cooldownMs, 0, 0);
    }

    bool HasDrakeSpellCooldown(Unit* vehicleBase, uint32 spellId)
    {
        return vehicleBase && vehicleBase->GetSpellHistory() && !vehicleBase->GetSpellHistory()->IsReady(spellId);
    }

    // 5.4.8 ADAPTATION: AC PlayerbotAI::GetAllPlayersInGroup() (AC Bot/PlayerbotAI.h:458,
    // Bot/PlayerbotAI.cpp:2964-2986) is not on the fork's PlayerbotAI (AI/PlayerbotAI.h), so the identical
    // group walk is done here (fork Group::GetFirstMember / GroupReference::next / GetSource, as in
    // AI/PlayerbotAI.cpp:489-509 GetGroupSlotIndex).
    std::vector<Player*> GetAllPlayersInGroupOf(Player* bot)
    {
        std::vector<Player*> members;

        Group* group = bot->GetGroup();
        if (!group)
            return members;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member)
                continue;

            members.push_back(member);
        }

        return members;
    }

    // 5.4.8 ADAPTATION: AC MotionMaster::MoveForwards(Unit* target, float dist) is an AzerothCore core method
    // (AC src/server/game/Movement/MotionMaster.cpp:399-423, MotionMaster.h:238) that the fork's MotionMaster
    // does not have (src/server/game/Movement/MotionMaster.h:140-202). This is the same body on fork APIs:
    // Map::CanReachPositionAndGetValidCoords(source, x&, y&, z&, failOnCollision, failOnSlopes) (Map.h:660)
    // and Movement::MoveSplineInit MoveTo(x,y,z,generatePath) / SetFacing(Unit const*) / Launch()
    // (src/server/game/Movement/Spline/MoveSplineInit.h:53,76,87). Like AC's it moves 'owner' to the point
    // target + dist * (direction from target to owner), facing the target; the sign/length are the caller's.
    void MoveForwardsOf(Unit* owner, Unit* target, float dist)
    {
        // like movebackwards, but without the inversion
        if (!target)
            return;

        Position const& pos = target->GetPosition();
        float angle = target->GetAngle(owner);
        G3D::Vector3 point;
        point.x = pos.m_positionX + dist * cosf(angle);
        point.y = pos.m_positionY + dist * sinf(angle);
        point.z = pos.m_positionZ;

        if (!owner->GetMap()->CanReachPositionAndGetValidCoords(owner, point.x, point.y, point.z, true, true))
            return;

        Movement::MoveSplineInit init(owner);
        init.MoveTo(point.x, point.y, point.z, false);
        init.SetFacing(target);
        init.Launch();
    }
}

// AC Ai/Dungeon/OC/OCActions.cpp:12
bool AvoidUnstableSphereAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "drakos the interrogator");
    if (!boss) { return false; }

    float radius = 12.0f;
    float extraDistance = 1.0f;
    Unit* closestSphere = nullptr;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_UNSTABLE_SPHERE && !unit->isMoving())
        {
            if (!closestSphere || bot->GetExactDist2d(unit) < bot->GetExactDist2d(closestSphere))
            {
                closestSphere = unit;
            }
        }
    }

    if (closestSphere && bot->GetExactDist2d(closestSphere) < radius + extraDistance)
    {
        return MoveAway(closestSphere, fmin(3.0f, bot->GetExactDist2d(closestSphere) - radius + extraDistance));
    }

    return false;
}

// AC Ai/Dungeon/OC/OCActions.cpp:42
bool MountDrakeAction::isPossible() { return bot->GetMapId() == OCULUS_MAP_ID; }
// AC Ai/Dungeon/OC/OCActions.cpp:43
bool MountDrakeAction::Execute(Event /*event*/)
{
    std::map<int32, int32> drakeAssignments;
    // Composition can be adjusted - both 3/1/1 and 2/2/1 are good default comps
    // {Amber, Emerald, Ruby}
    std::vector<uint8> composition = {2, 2, 1};
    // std::vector<uint8> composition = {3, 1, 1};
    int32 myIndex = botAI->GetGroupSlotIndex(bot);

    Player* master = botAI->GetMaster();
    if (!master) { return false; }
    Unit* vehicle = master->GetVehicleBase();
    if (!vehicle) { return false; }

    // Subtract the player's chosen mount type from the composition so player can play whichever they prefer
    switch (vehicle->GetEntry())
    {
        case NPC_AMBER_DRAKE:
            composition[0]--;
            break;
        case NPC_EMERALD_DRAKE:
            composition[1]--;
            break;
        case NPC_RUBY_DRAKE:
            composition[2]--;
            break;
    }

    std::vector<Player*> players = GetAllPlayersInGroupOf(bot);
    for (Player* player : players)
    {
        // 5.4.8 ADAPTATION: AC also checks player->IsDuringRemoveFromWorld(); MoP has no such member
        // (same note as AI/PlayerbotAI.cpp:3687 for IsValidUnit), so IsInWorld() alone is used.
        if (!player || !player->IsInWorld())
            continue;

        WorldSession* session = player->GetSession();
        if (!session || !session->IsBot())
            continue;

        int slot = botAI->GetGroupSlotIndex(player);
        if (slot < 0)
            continue;

        for (uint8 i = 0; i < composition.size(); ++i)
        {
            if (composition[i] > 0)
            {
                drakeAssignments[slot] = DRAKE_ITEMS[i];
                composition[i]--;
                break;
            }
        }
    }

    // Correct/update the drake items in inventories incase assignments have changed
    for (int64_t itemId : DRAKE_ITEMS)
    {
        Item* item = bot->GetItemByEntry(itemId);
        if (!item) { continue; }

        if (itemId == drakeAssignments[myIndex])
        {
            // Use our assigned drake
            // 5.4.8 ADAPTATION: AC UseItemAuto(item) = UseItem(item, ObjectGuid::Empty, nullptr)
            // (AC UseItemAction.cpp:60); the fork's UseItemAction has the self-only UseItem(Item*)
            // (strategy/actions/UseItemAction.h:33) which casts the item's on-use spell at the bot.
            return UseItem(item);
        }
        // Else assigned drake is different, destroy old drake
        uint32 count = 1;
        bot->DestroyItemCount(item, count, true);
        break;
    }

    // Bot does not have the correct drake item
    bot->AddItem(drakeAssignments[myIndex], 1);
    return false;
}

// AC Ai/Dungeon/OC/OCActions.cpp:118
bool DismountDrakeAction::Execute(Event /*event*/)
{
    if (bot->GetVehicle())
    {
        bot->ExitVehicle();
        return true;
    }
    return false;
}

// AC Ai/Dungeon/OC/OCActions.cpp:128
bool OccFlyDrakeAction::Execute(Event /*event*/)
{
    Player* master = botAI->GetMaster();
    if (!master) { return false; }
    Unit* masterVehicle = master->GetVehicleBase();
    Unit* vehicleBase = bot->GetVehicleBase();
    if (!vehicleBase || !masterVehicle) { return false; }

    MotionMaster* mm = vehicleBase->GetMotionMaster();
    Unit* boss = AI_VALUE2(Unit*, "find target", "ley-guardian eregos");
    if (boss && !boss->HasAura(SPELL_PLANAR_SHIFT))
    {
        // Handle as boss encounter instead of formation flight
        mm->Clear(false);
        float distance = vehicleBase->GetExactDist(boss);
        float range = 55.0f;    // Drake range is 60yd
        if (distance > range)
        {
            MoveForwardsOf(vehicleBase, boss, range - distance);
            vehicleBase->SendMovementFlagUpdate();
            return true;
        }

        vehicleBase->SetFacingToObject(boss);
        mm->MoveIdle();
        vehicleBase->SendMovementFlagUpdate();
        return false;
    }

    if (vehicleBase->GetExactDist(masterVehicle) > 20.0f)
    {
        // 3/4 of a circle, with frontal cone 90 deg unobstructed
        float angle = botAI->GetGroupSlotIndex(bot) * (2*M_PI - M_PI_2)/5 + M_PI_2;
        vehicleBase->SetCanFly(true);
        mm->MoveFollow(masterVehicle, 15.0f, angle);
        vehicleBase->SendMovementFlagUpdate();
        return true;
    }
    return false;
}

// AC Ai/Dungeon/OC/OCActions.cpp:169
bool OccDrakeAttackAction::Execute(Event /*event*/)
{
    vehicleBase = bot->GetVehicleBase();
    if (!vehicleBase) { return false; }

    Unit* target = AI_VALUE(Unit*, "current target");

    if (!target)
    {
        GuidVector npcs = AI_VALUE(GuidVector, "possible targets");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsInCombat()) { continue; }

            target = unit;
            break;
        }
    }
    // Check this again to see if a target was assigned
    if (!target) { return false; }

    switch (vehicleBase->GetEntry())
    {
        case NPC_AMBER_DRAKE:
            return AmberDrakeAction(target);
        case NPC_EMERALD_DRAKE:
            return EmeraldDrakeAction(target);
        case NPC_RUBY_DRAKE:
            return RubyDrakeAction(target);
        default:
            break;
    }
    return false;
}

// AC Ai/Dungeon/OC/OCActions.cpp:205
bool OccDrakeAttackAction::CastDrakeSpellAction(Unit* target, uint32 spellId, uint32 cooldown)
{
    if (botAI->CanCastVehicleSpell(spellId, target))
        if (botAI->CastVehicleSpell(spellId, target))
        {
            AddDrakeSpellCooldown(vehicleBase, spellId, cooldown);
            return true;
        }
    return false;
}

// AC Ai/Dungeon/OC/OCActions.cpp:216
bool OccDrakeAttackAction::AmberDrakeAction(Unit* target)
{
    Aura* shockCharges = target->GetAura(SPELL_SHOCK_CHARGE, vehicleBase->GetGUID());
    if (shockCharges && shockCharges->GetStackAmount() > 8)
    {
        // At 9 charges, better to detonate and re-channel rather than stacking the last charge due to gcd
        // If stacking Amber drakes, may need to drop this even lower as the charges stack so fast
        return CastDrakeSpellAction(target, SPELL_SHOCK_LANCE, 0);
    }

    // Deal with enrage after shock charges, as Stop Time adds 5 charges and they may get wasted
    if (target->HasAura(SPELL_ENRAGED_ASSAULT) &&
        !target->HasAura(SPELL_STOP_TIME) &&
        !HasDrakeSpellCooldown(vehicleBase, SPELL_STOP_TIME))
    {
        return CastDrakeSpellAction(target, SPELL_STOP_TIME, 60000);
    }

    if (!vehicleBase->FindCurrentSpellBySpellId(SPELL_TEMPORAL_RIFT))
    {
        return CastDrakeSpellAction(target, SPELL_TEMPORAL_RIFT, 0);
    }

    return false;
}

// AC Ai/Dungeon/OC/OCActions.cpp:242
bool OccDrakeAttackAction::EmeraldDrakeAction(Unit* target)
{
    Aura* poisonStacks = target->GetAura(SPELL_LEECHING_POISON, vehicleBase->GetGUID());
    if (!poisonStacks || (poisonStacks->GetStackAmount() < 3 ||
                         poisonStacks->GetDuration() < 4000))
    {
        return CastDrakeSpellAction(target, SPELL_LEECHING_POISON, 0);
    }

    if (!HasDrakeSpellCooldown(vehicleBase, SPELL_TOUCH_THE_NIGHTMARE) &&
        (!target->HasAura(SPELL_TOUCH_THE_NIGHTMARE) || vehicleBase->HealthAbovePct(90)))
    {
        return CastDrakeSpellAction(target, SPELL_TOUCH_THE_NIGHTMARE, 10000);
    }

    Unit* healingTarget = nullptr;
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (!unit || bot->GetGUID() == member)
        {
            continue;
        }

        Unit* drake = unit->GetVehicleBase();
        if (!drake || drake->IsFullHealth()) { continue; }

        if (!healingTarget || drake->GetHealthPct() < healingTarget->GetHealthPct() - 15.0f)
        {
            healingTarget = drake;
        }
    }

    Spell* currentSpell = vehicleBase->FindCurrentSpellBySpellId(SPELL_DREAM_FUNNEL);
    if (healingTarget)
    {
        if (!currentSpell || currentSpell->m_targets.GetUnitTarget() != healingTarget)
        {
            float distance = vehicleBase->GetExactDist(healingTarget);
            float range = 55.0f;
            if (distance > range)
            {
                MotionMaster* mm = vehicleBase->GetMotionMaster();
                mm->Clear(false);
                MoveForwardsOf(vehicleBase, healingTarget, distance - range - 10.0f);
                vehicleBase->SendMovementFlagUpdate();
                return false;
            }
            return CastDrakeSpellAction(healingTarget, SPELL_DREAM_FUNNEL, 0);
        }
    }
    // Fill GCDs with Leeching Poison to refresh timer, rather than idling
    if (!currentSpell)
    {
        return CastDrakeSpellAction(target, SPELL_LEECHING_POISON, 0);
    }

    return false;
}

// AC Ai/Dungeon/OC/OCActions.cpp:303
bool OccDrakeAttackAction::RubyDrakeAction(Unit* target)
{
    Aura* evasiveCharges = vehicleBase->GetAura(SPELL_EVASIVE_CHARGES);
    Aura* evasiveManeuvers = vehicleBase->GetAura(SPELL_EVASIVE_MANEUVERS);

    if (evasiveCharges)
    {
        if (evasiveManeuvers &&
            !HasDrakeSpellCooldown(vehicleBase, SPELL_MARTYR) &&
            evasiveManeuvers->GetDuration() > 10000 &&
            evasiveCharges->GetStackAmount() >= 5)
        {
            return CastDrakeSpellAction(vehicleBase, SPELL_MARTYR, 10000);
        }

        if (!HasDrakeSpellCooldown(vehicleBase, SPELL_EVASIVE_MANEUVERS) &&
            evasiveCharges->GetStackAmount() >= 10)
        {
            return CastDrakeSpellAction(vehicleBase, SPELL_EVASIVE_MANEUVERS, 5000);
        }
    }

    return CastDrakeSpellAction(target, SPELL_SEARING_WRATH, 0);
}

// AC Ai/Dungeon/OC/OCActions.cpp:328
bool AvoidArcaneExplosionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "mage-lord urom");
    if (!boss) { return false; }

    Position const* closestPos = nullptr;

    for (auto& position : uromSafePositions)
    {
        if (!closestPos || bot->GetExactDist(position) < bot->GetExactDist(closestPos))
            {
                closestPos = &position;
            }
    }

    if (!closestPos) { return false; }

    return MoveNear(bot->GetMapId(), closestPos->GetPositionX(), closestPos->GetPositionY(), closestPos->GetPositionZ(), 2.0f, MovementPriority::MOVEMENT_COMBAT);
}

// AC Ai/Dungeon/OC/OCActions.cpp:348
bool TimeBombSpreadAction::Execute(Event /*event*/)
{
    float radius = 10.0f;
    float distanceExtra = 2.0f;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        if (bot->GetGUID() == member)
        {
            continue;
        }

        Unit* unit = botAI->GetUnit(member);
        if (unit && bot->GetExactDist2d(unit) < radius)
        {
            return MoveAway(unit, radius + distanceExtra - bot->GetExactDist2d(unit));
        }
    }
    return false;
}
