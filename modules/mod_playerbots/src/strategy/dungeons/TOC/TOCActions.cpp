/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TOCActions.h"
#include "Bag.h"
#include "GenericSpellActions.h"
#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SpellHistory.h"
#include "Vehicle.h"
#include "WorldPacket.h"
#include "WorldSession.h"

namespace
{
    // 5.4.8 ADAPTATION: AC calls Unit::AddSpellCooldown(spellId, 0, ms) on the vehicle creature
    // (AC TOCActions.cpp:137,150,163,171). The fork's Unit has no AddSpellCooldown (only Player has one,
    // Player.h:2151, and Creature has HasSpellCooldown / AddCreatureSpellCooldown, Creature.h:326-329).
    // The cooldown PlayerbotAI::CanCastVehicleSpell actually checks is the vehicle base's SpellHistory
    // (AI/PlayerbotAI.cpp:3723, GetSpellHistory()->IsReady), so the faithful equivalent is
    // SpellHistory::AddCooldown(spellId, itemId, cooldownMs, category, categoryCooldownMs)
    // (src/server/game/Spells/SpellHistory.h:79, SpellHistory.cpp:403-419; Unit::GetSpellHistory Unit.h:2587).
    void AddVehicleSpellCooldown(Unit* vehicleBase, uint32 spellId, uint32 cooldownMs)
    {
        if (vehicleBase && vehicleBase->GetSpellHistory())
            vehicleBase->GetSpellHistory()->AddCooldown(spellId, 0, cooldownMs, 0, 0);
    }
}

// AC Ai/Dungeon/TOC/TOCActions.cpp:14
bool ToCLanceAction::Execute(Event /*event*/)
{
    // If already has lance equipped, do nothing
    if (bot->HasItemOrGemWithIdEquipped(ITEM_LANCE, 1))
        return false;

    // Store current mainhand item
    Item* oldWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

    // Search inventory for the lance item
    Item* lanceItem = nullptr;

    // Check main inventory
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (item && item->GetEntry() == ITEM_LANCE)
        {
            lanceItem = item;
            break;
        }
    }

    // Check bags if not found in main inventory
    if (!lanceItem)
    {
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; bag++)
        {
            Bag* pBag = bot->GetBagByPos(bag);
            if (pBag)
            {
                for (uint8 slot = 0; slot < pBag->GetBagSize(); slot++)
                {
                    Item* item = bot->GetItemByPos(bag, slot);
                    if (item && item->GetEntry() == ITEM_LANCE)
                    {
                        lanceItem = item;
                        break;
                    }
                }
            }
            if (lanceItem)
                break;
        }
    }

    // If we found the lance, equip it
    if (lanceItem)
    {
        // First unequip current weapon if it exists
        if (oldWeapon)
            bot->SwapItem(oldWeapon->GetPos(), lanceItem->GetPos());

        else
            bot->EquipItem(EQUIPMENT_SLOT_MAINHAND, lanceItem, true);

        return true;
    }

    if (bot->HasItemCount(ITEM_LANCE, 1))
        return false;

    GameObject* lanceRack = bot->FindNearestGameObject(OBJECT_LANCE_RACK, 100.0f);
    if (!lanceRack)
        return false;

    // 5.4.8 ADAPTATION (#561): AC (TOCActions.cpp:80-81) moves to exactly the radius it then tests. On MoP the bots never
    // passed that test from the stop the move leaves them at: wave 57 stop (695.0,603.8) = 6.90 yd from the rack centre,
    // wave 58 (moved to INTERACTION_DISTANCE - CONTACT_DISTANCE) stop (694.8,604.3) = 6.36 yd, 0 casts of 64682 in either
    // gate (880+ attempts per bot), while every later in-combat cast came from 3.3-6.3 yd. The approach now ends at
    // CONTACT_DISTANCE from the rack, well inside the unchanged AC test radius; the rest is AC 1:1.
    bool const inRange = lanceRack->IsWithinDistInMap(bot, INTERACTION_DISTANCE);
    if (bot->GetExactDist(lanceRack) < 10.0f)
        TC_LOG_DEBUG("playerbots", "[TOCLANCE] %s d=%.2f exact=%.2f reach=%.2f inRange=%u moving=%u cast=%u",
                     bot->GetName().c_str(), bot->GetDistance(lanceRack), bot->GetExactDist(lanceRack),
                     bot->GetCombatReach(), inRange ? 1 : 0, bot->isMoving() ? 1 : 0,
                     bot->IsNonMeleeSpellCasted(false, true, true) ? 1 : 0);
    if (!inRange)
        return MoveTo(lanceRack, CONTACT_DISTANCE);

    botAI->RemoveShapeshift();
    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    lanceRack->Use(bot);
    TC_LOG_DEBUG("playerbots", "[TOCLANCE] %s used rack %u lance=%u", bot->GetName().c_str(),
                 lanceRack->GetGUID().GetCounter(), bot->GetItemCount(ITEM_LANCE));

    return false;
}

// AC Ai/Dungeon/TOC/TOCActions.cpp:91
bool ToCUELanceAction::Execute(Event event)
{
    if (!bot->HasItemOrGemWithIdEquipped(ITEM_LANCE, 1))
        return false;

    // Call EquipUpgradeAction
    EquipUpgradeAction equipUpgradeAction(botAI);
    if (!equipUpgradeAction.Execute(event))
        return false;

    return false;
}

// AC Ai/Dungeon/TOC/TOCActions.cpp:104
bool ToCMountedAction::Execute(Event /*event*/)
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicleBase || !vehicle)
        return false;

    GuidVector attackers = AI_VALUE(GuidVector, "possible targets no los");

    Unit* target = nullptr;
    for (auto i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;
        for (uint32 entry : availableTargets)
        {
            if (unit->GetEntry() == entry)
            {
                target = unit;
                break;
            }
        }
        if (target)
            break;
    }

    Aura* defendBot = botAI->GetAura("defend", bot, false, true);
    if (!defendBot || defendBot->GetStackAmount() < 3)
    {
        uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "Defend");
        if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target))
        {
            AddVehicleSpellCooldown(vehicleBase, spellId, 1000);
            return true;
        }
    }

    if (!target)
        return false;

    if (target->GetDistance2d(bot) > 5.0f)
    {
        uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "Charge");
        if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target))
        {
            AddVehicleSpellCooldown(vehicleBase, spellId, 1000);
            return true;
        }
    }

    Aura* defendTarget = botAI->GetAura("defend", target, false, false);
    if (!defendTarget)
    {}
    else
    {
        uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "Shield-Breaker");
        if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target))
        {
            AddVehicleSpellCooldown(vehicleBase, spellId, 1000);
            return true;
        }
    }

    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "Thrust");
    if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target))
    {
        AddVehicleSpellCooldown(vehicleBase, spellId, 1000);
        return true;
    }

    return false;
}

// AC Ai/Dungeon/TOC/TOCActions.cpp:178
bool ToCMountAction::Execute(Event /*event*/)
{
    // do not switch vehicles yet
    if (bot->GetVehicle())
        return false;

    Unit* vehicleToEnter = nullptr;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* vehicleBase = botAI->GetUnit(*i);
        if (!vehicleBase)
            continue;

        if (vehicleBase->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        if (!vehicleBase->IsFriendlyTo(bot))
            continue;

        if (!vehicleBase->GetVehicleKit() || !vehicleBase->GetVehicleKit()->GetAvailableSeatCount())
            continue;

        uint32 entry = vehicleBase->GetEntry();
        if (entry != NPC_ARGENT_BATTLEWORG && entry != NPC_ARGENT_WARHORSE)
            continue;

        if (!vehicleToEnter || bot->GetExactDist(vehicleToEnter) > bot->GetExactDist(vehicleBase))
            vehicleToEnter = vehicleBase;
    }

    if (!vehicleToEnter)
        return false;

    if (EnterVehicle(vehicleToEnter, true))
        return true;

    return false;
}

// AC Ai/Dungeon/TOC/TOCActions.cpp:218
bool ToCMountAction::EnterVehicle(Unit* vehicleBase, bool moveIfFar)
{
    float dist = bot->GetDistance(vehicleBase);

    if (dist > INTERACTION_DISTANCE && !moveIfFar)
        return false;

    if (dist > INTERACTION_DISTANCE)
        return MoveTo(vehicleBase);

    botAI->RemoveShapeshift();

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    vehicleBase->HandleSpellClick(bot);

    if (!bot->IsOnVehicle(vehicleBase))
        return false;

    // dismount because bots can enter vehicle on mount
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    return true;
}

// AC Ai/Dungeon/TOC/TOCActions.cpp:243
bool ToCEadricAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "eadric the pure");
    if (!boss)
        return false;

    // If Eadric is casting Radiance, face the opposite direction
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_RADIANCE))
    {
        // Calculate the opposite direction
        float angle = bot->GetAngle(boss);
        float newAngle = Position::NormalizeOrientation(angle + M_PI);  // Add 180 degrees (PI radians)

        // Set the bot's orientation to face away from Eadric
        bot->SetFacingTo(newAngle);
        return true;
    }

    return false;
}
