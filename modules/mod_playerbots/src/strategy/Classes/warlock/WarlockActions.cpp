/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarlockActions.h"

#include <mutex>
#include <unordered_map>

#include "Event.h"
#include "Group.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ServerFacade.h"

// #693: AC WarlockActions.cpp:110-116 (mod-playerbots#2855): never while tanking, and not on a player target
// (players have no threat).
bool CastSoulShatterAction::isUseful()
{
    if (botAI->HasStrategy("tank", BOT_STATE_COMBAT))
        return false;
    Unit* target = AI_VALUE(Unit*, "current target");
    return !(target && target->IsPlayer());  // players have no threat
}

bool CastDrainSoulAction::isUseful() { /*return AI_VALUE2(uint32, "item count", "soul shard") < 10;*/ return std::rand() % 5 < 2; }

Value<Unit*>* CastBanishAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", "banish"); }

bool CastBanishAction::Execute(Event event) { return botAI->CastSpell("banish", GetTarget()); }

Value<Unit*>* CastFearOnCcAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", "fear"); }

bool CastFearOnCcAction::Execute(Event event) { return botAI->CastSpell("fear", GetTarget()); }

bool CastFearOnCcAction::isPossible() { return true; }

bool CastFearOnCcAction::isUseful() { return true; }

bool CastLifeTapAction::isUseful() { return AI_VALUE2(uint8, "health", "self target") > sPlayerbotAIConfig->lowHealth; }

// AC parity (WarlockActions.cpp:226-233): the resurrection-sickness-preventing "Soulstone
// Resurrection" aura. 5.4.8: AC's WotLK-era per-rank id list (20707, 20762-20765, 27239, 47883)
// mostly no longer exists (verified against the client Spell.dbc: only 20707 "Soulstone" remains,
// and it is the aura id applied directly by the 5.0.4+ direct-cast spell of the same id).
bool UseSoulstoneAction::HasSoulstoneAura(Unit* unit)
{
    return unit->HasAura(20707);
}

// #163 S8 gap fix per research (s8-pet-control-and-class-spells.md B.1): 5.0.4 made Soulstone a
// direct cast (spell 20707, no item, no "Create Soulstone"). This replaces AC's UseItem() call with
// a plain botAI->CastSpell("soulstone", target); the 30 yd + LoS check, HasSoulstoneAura check and
// 2.5 s cross-call reservation (below) are otherwise unchanged from AC.
// #713: the trigger ("soulstone") only tests known + off cooldown, so the action must reject the ticks on which
// the cast cannot start. 20707 has a cast time, so a mounted/moving/GCD-locked bot failed it on every tick.
bool UseSoulstoneAction::isPossible()
{
    return botAI->CanCastSpell("soulstone", bot);
}

// #713: the soulstone aura (15 min) outlasts the spell cooldown (10 min), so for 5 min the trigger was active
// while Execute() returned false at once: reject it here instead.
bool UseSoulstoneSelfAction::isUseful()
{
    return !HasSoulstoneAura(bot);
}

bool UseSoulstoneMasterAction::isUseful()
{
    Player* master = botAI->GetMaster();
    return master && !HasSoulstoneAura(master);
}

bool UseSoulstoneAction::CastSoulstoneOn(Unit* target)
{
    if (!target)
        return false;

    return botAI->CastSpell("soulstone", target);
}

// AC parity (WarlockActions.cpp:249-299): a reservation map so two warlocks in the same group don't
// both commit to soulstoning the same target on the same tick before either cast lands.
namespace
{
    std::unordered_map<ObjectGuid, uint32> soulstoneReservations;
    // AC WarlockActions.cpp:251 (soulstoneReservationsMutex): warlocks on different maps run on different map-update
    // threads (MapUpdate.Threads > 1) and share this map.
    std::mutex soulstoneReservationsMutex;

    void CleanupSoulstoneReservations()
    {
        uint32 now = getMSTime();
        std::lock_guard<std::mutex> lock(soulstoneReservationsMutex);
        for (auto it = soulstoneReservations.begin(); it != soulstoneReservations.end();)
        {
            if (it->second <= now)
                it = soulstoneReservations.erase(it);
            else
                ++it;
        }
    }

    bool IsReserved(ObjectGuid const& guid, uint32 now)
    {
        std::lock_guard<std::mutex> lock(soulstoneReservationsMutex);
        auto it = soulstoneReservations.find(guid);
        return it != soulstoneReservations.end() && it->second > now;
    }

    void Reserve(ObjectGuid const& guid, uint32 now)
    {
        std::lock_guard<std::mutex> lock(soulstoneReservationsMutex);
        soulstoneReservations[guid] = now + 2500;
    }
}

// AC parity (WarlockActions.cpp:236-247): soulstone the bot itself.
bool UseSoulstoneSelfAction::Execute([[maybe_unused]] Event event)
{
    if (HasSoulstoneAura(bot))
        return false;

    return CastSoulstoneOn(bot);
}

// AC parity (WarlockActions.cpp:249-299).
bool UseSoulstoneMasterAction::Execute([[maybe_unused]] Event event)
{
    CleanupSoulstoneReservations();

    Player* master = botAI->GetMaster();
    if (!master || HasSoulstoneAura(master))
        return false;

    uint32 now = getMSTime();
    if (IsReserved(master->GetGUID(), now))
        return false;

    if (sServerFacade->GetDistance2d(bot, master) >= 30.0f || !bot->IsWithinLOSInMap(master))
        return false;

    Reserve(master->GetGUID(), now);
    return CastSoulstoneOn(master);
}

// AC parity (WarlockActions.cpp:301-367): the main tank first, then any other tank in range.
bool UseSoulstoneTankAction::Execute([[maybe_unused]] Event event)
{
    CleanupSoulstoneReservations();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    uint32 now = getMSTime();
    Player* chosenTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && PlayerBotSpec::IsTank(member) && PlayerBotSpec::IsMainTank(member) &&
            !HasSoulstoneAura(member) && !IsReserved(member->GetGUID(), now) &&
            sServerFacade->GetDistance2d(bot, member) < 30.0f && bot->IsWithinLOSInMap(member))
        {
            chosenTank = member;
            break;
        }
    }
    if (!chosenTank)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && PlayerBotSpec::IsTank(member) && !HasSoulstoneAura(member) &&
                !IsReserved(member->GetGUID(), now) && sServerFacade->GetDistance2d(bot, member) < 30.0f &&
                bot->IsWithinLOSInMap(member))
            {
                chosenTank = member;
                break;
            }
        }
    }

    if (!chosenTank)
        return false;

    Reserve(chosenTank->GetGUID(), now);
    return CastSoulstoneOn(chosenTank);
}

// AC parity (WarlockActions.cpp:369-410): any healer in range.
bool UseSoulstoneHealerAction::Execute([[maybe_unused]] Event event)
{
    CleanupSoulstoneReservations();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    uint32 now = getMSTime();
    Player* healer = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && PlayerBotSpec::IsHeal(member) && !HasSoulstoneAura(member) &&
            !IsReserved(member->GetGUID(), now) && sServerFacade->GetDistance2d(bot, member) < 30.0f &&
            bot->IsWithinLOSInMap(member))
        {
            healer = member;
            break;
        }
    }

    if (!healer)
        return false;

    Reserve(healer->GetGUID(), now);
    return CastSoulstoneOn(healer);
}

bool CastHandOfGuldanAction::Execute(Event event)
{
	return botAI->CastSpell(105174, GetTarget());
}

bool CastChaosBoltAction::isUseful()
{
	return botAI->HasAura("fire and brimstone", bot) || bot->GetPower(Powers::POWER_BURNING_EMBERS) >= 10;
}
bool CastIncinerateAction::Execute(Event event)
{
	if (!isUseful() || !isPossible())
		return false;

	if (botAI->HasAura("fire and brimstone", bot))
		botAI->CastSpell(114654, GetTarget());
	else
		botAI->CastSpell(spell, GetTarget());

	return true;
}
bool CastImmolateAction::Execute(Event event)
{
	if (!isUseful() || !isPossible())
		return false;

	if (botAI->HasAura("fire and brimstone", bot))
		botAI->CastSpell(108686, GetTarget());
	else
		botAI->CastSpell(spell, GetTarget());

	return true;
}
bool CastConflagrateAction::Execute(Event event)
{
	if (!isUseful() || !isPossible())
		return false;

	if (botAI->HasAura("fire and brimstone", bot))
		botAI->CastSpell(108685, GetTarget());
	else
		botAI->CastSpell(spell, GetTarget());

	return true;
}

// #408 (AC parity, Ai/Base/Actions/UseItemAction.cpp:80): AC gates UseSpellItemAction on the
// "spell cast useful" computed value, which does not exist on 5.4.8. Firestone / Spellstone are
// self-only items whose on-use effect lands as a buff named after the item, so the equivalent
// check is "we carry one and that buff is not up yet".
bool UseSpellItemAction::isUseful()
{
    return AI_VALUE2(uint32, "item count", getName()) > 0 && !botAI->HasAura(getName(), bot, true, true);
}

// ---- #864 / epic #854 (rotation lane 9): MoP warlock rotation port ------------------------------------------
namespace
{
// The interrupt spell of the current pet, 0 when the pet cannot cast it now.
uint32 PetInterruptSpellId(Pet* pet)
{
    if (!pet || !pet->IsAlive())
        return 0;

    uint32 spellId = 0;
    switch (pet->GetEntry())
    {
        case 417:    // Felhunter -> Spell Lock
            spellId = 19647;
            break;
        case 58964:  // Observer  -> Optical Blast
            spellId = 115781;
            break;
        default:
            return 0;
    }

    return pet->HasSpell(spellId) && !pet->HasSpellCooldown(spellId) ? spellId : 0;
}
}  // namespace

bool CastPetInterruptAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "enemy caster target");
    Pet* pet = bot->GetPet();
    if (!target || !PetInterruptSpellId(pet))
        return false;

    return pet->IsWithinDistInMap(target, 40.0f) && pet->IsWithinLOSInMap(target);
}

bool CastPetInterruptAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "enemy caster target");
    Pet* pet = bot->GetPet();
    uint32 spellId = PetInterruptSpellId(pet);
    if (!target || !spellId)
        return false;

    pet->CastSpell(target, spellId, false);
    return true;
}

Unit* WarlockHavocTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* main = context->GetValue<Unit*>("current target")->Get();
    if (!main || !main->IsAlive())
        return nullptr;

    GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
    Unit* best = nullptr;
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        // Havoc is single-target: while one of my Havocs is up there is nothing to do (the MoP rule only
        // checks "not my primary target"; its 25 s cooldown made a retry a no-op, this skips the retry).
        if (botAI->HasAura("havoc", unit, false, true))
            return nullptr;

        if (unit == main || !bot->IsWithinCombatRange(unit, botAI->GetRange("spell")) || !bot->IsWithinLOSInMap(unit))
            continue;

        if (!best || (unit->GetTypeId() == TYPEID_PLAYER && best->GetTypeId() != TYPEID_PLAYER))
            best = unit;
    }

    return best;
}
