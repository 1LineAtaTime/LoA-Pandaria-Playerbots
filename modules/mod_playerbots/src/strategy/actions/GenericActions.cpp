/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericActions.h"

#include "CreatureAI.h"
#include "Playerbots.h"

bool MeleeAction::isUseful()
{
    // do not allow if can't attack from vehicle
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    // AC: do not start autoattack while prowled - let opener spells break stealth intentionally.
    return !botAI->HasAura("prowl", bot);
}

bool TogglePetSpellAutoCastAction::Execute(Event event)
{
    Pet* pet = bot->GetPet();
    if (!pet)
    {
        return false;
    }
    // hack on high level spell after low level initialization
    std::vector<unsigned int> shouldRemove;
    for (unsigned int& m_autospell : pet->m_autospells)
    {
        if (!pet->HasSpell(m_autospell))
        {
            shouldRemove.push_back(m_autospell);
        }
    }
    for (unsigned int spellId : shouldRemove)
    {
        auto autospellItr = std::find(pet->m_autospells.begin(), pet->m_autospells.end(), spellId);
        if (autospellItr != pet->m_autospells.end())
            pet->m_autospells.erase(autospellItr);
    }
    bool toggled = false;
    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        uint32 spellId = itr->first;
        const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo->IsAutocastable())
            continue;

        bool shouldApply = true;
        // spellId == 4511 || spellId == 54424 || spellId == 57564 || spellId == 57565 ||
        // spellId == 57566 || spellId == 57567 ||
        // cat stealth, prowl
        if (spellId == 1742 || spellId == 24450)
        {
            shouldApply = false;
        }
        bool isAutoCast = false;
        for (unsigned int& m_autospell : pet->m_autospells)
        {
            if (m_autospell == spellId)
            {
                isAutoCast = true;
                break;
            }
        }
        if (shouldApply != isAutoCast)
        {
            pet->ToggleAutocast(spellInfo, shouldApply);
            toggled = true;
        }
    }
    return toggled;
}

bool PetAttackAction::Execute(Event event)
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet)
    {
        return false;
    }

    // AC parity (Ai/Base/Actions/PetsAction.cpp:413-415): a passive pet is left alone instead of being
    // forced into passive here (see s8-pet-control-and-class-spells.md Part A.4 step 6 - the previous
    // port instead did `pet->SetReactState(REACT_PASSIVE)` unconditionally, which silently overrode
    // AiPlayerbot.DefaultPetStance on every combat "pet attack").
    if (pet->GetReactState() == REACT_PASSIVE)
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
    {
        return false;
    }

    if (!bot->IsValidAttackTarget(target))
    {
        return false;
    }

    pet->ClearUnitState(UNIT_STATE_FOLLOW);
    pet->AttackStop();
    pet->SetTarget(target->GetGUID());

    pet->GetCharmInfo()->SetIsCommandAttack(true);
    pet->GetCharmInfo()->SetIsAtStay(false);
    pet->GetCharmInfo()->SetIsFollowing(false);
    pet->GetCharmInfo()->SetIsCommandFollow(false);
    pet->GetCharmInfo()->SetIsReturning(false);

    pet->ToCreature()->AI()->AttackStart(target);

    // #603: report success only if the pet really engaged. A guardian whose AI never attacks (wave 10 holy priest
    // Bernadela: "pet attack - OK" 47x, in combat 1 of 30 s) kept this action at 40.0 returning true every tick,
    // which ended the tick before any spell. AC returns true here too (Ai/Base/Actions/PetsAction.cpp:438-439), but its
    // trigger then goes quiet because the victim is set; returning false lets the engine fall through instead.
    //
    // #644: AC commands a pet once per target, because its trigger (GenericTriggers.cpp:53-70, identical here) stays
    // quiet while the pet holds the victim with the attack command. A freshly summoned destruction Imp (PetAI pure
    // caster, PetAI.cpp:57-62) held the victim right after AttackStart, so this returned true. By the next tick it had
    // dropped the victim or the command again, and it never landed a Firebolt (the owner stayed out of combat). So
    // the trigger fired 23 ticks in a row (wave 12, Federlo, 6.6-23.3 s) and the 40.0 action ended every tick: no bot
    // spell for 17 s. The command is still sent every time the trigger fires, as in AC. But a repeated command (the
    // same pet on the same target) returns false, so a pet that keeps dropping its target cannot starve the rotation.
    bool const repeated = lastCommandPet == pet->GetGUID() && lastCommandTarget == target->GetGUID();
    lastCommandPet = pet->GetGUID();
    lastCommandTarget = target->GetGUID();
    return pet->GetVictim() == target && !repeated;
}
