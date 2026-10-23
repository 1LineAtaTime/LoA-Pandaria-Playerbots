/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarlockTriggers.h"
#include "WarlockActions.h"

// 5.4.8 include-order: DynamicObject.h derives from GridObject<DynamicObject> but only includes
// Object.h, and Map.h does NOT pull GridObject.h either - GridObject.h is reached only via Creature.h
// (Creature.h:28) and GameObject.h. So DynamicObject.h does not compile unless one of those came first
// (C7568/C3770 "argument list missing after assumed function template 'GridObject'"). Matches the core's
// own precedent, e.g. ObjectAccessor.cpp includes Creature.h before DynamicObject.h.
#include "Creature.h"
#include "DynamicObject.h"
#include "Player.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "GenericTriggers.h"

bool CurseOfElementsTrigger::IsActive()
{
    return DebuffTrigger::IsActive() && !botAI->HasAura("curse of the elements", GetTarget()) &&
        !botAI->HasAura("curse of enfeeblement", GetTarget(), false, true);
}

bool CurseOfEnfeeblementTrigger::IsActive()
{
    return DebuffTrigger::IsActive() && !botAI->HasAura("curse of enfeeblement", GetTarget()) &&
        !botAI->HasAura("curse of the elements", GetTarget(), false, true);
}

bool OutOfSoulShardsTrigger::IsActive()
{
    // On 5.4.8 soul shards are a capped power resource, not item 6265 in the bags; the WotLK
    // inventory-count condition has no valid equivalent here and must not manufacture a count.
    return false;
}

bool TooManySoulShardsTrigger::IsActive()
{
    // On 5.4.8 soul shards are a capped power resource, not item 6265 in the bags; the WotLK
    // inventory-count condition has no valid equivalent here and must not manufacture a count.
    return false;
}

bool DemonArmorTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("demon skin", target) && !botAI->HasAura("demon armor", target) &&
        !botAI->HasAura("fel armor", target);
}

bool UnendingBreathTrigger::IsActive()
{
    return BuffTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target");
}

bool UnendingBreathOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target");
}

bool FirestoneTrigger::IsActive()
{
    // Firestone was removed before MoP; there is no weapon-imbue aura or action to maintain.
    return false;
}

bool SpellstoneTrigger::IsActive()
{
    // Spellstone was removed before MoP; there is no weapon-imbue aura or action to maintain.
    return false;
}

bool CorruptionOnAttackerTrigger::IsActive()
{
    return DebuffOnAttackerTrigger::IsActive() && !botAI->HasAura("seed of corruption", GetTarget(), false, true);
}

bool ImmolateOnAttackerTrigger::IsActive()
{
    return DebuffOnAttackerTrigger::IsActive();
}

bool UnstableAfflictionOnAttackerTrigger::IsActive()
{
    return DebuffOnAttackerTrigger::IsActive();
}

bool CurseOfAgonyOnAttackerTrigger::IsActive()
{
    return DebuffOnAttackerTrigger::IsActive();
}

bool CurseOfDoomTrigger::IsActive()
{
    return DebuffTrigger::IsActive();
}

bool CurseOfExhaustionTrigger::IsActive()
{
    return DebuffTrigger::IsActive();
}

bool CurseOfTonguesTrigger::IsActive()
{
    return DebuffTrigger::IsActive();
}

bool CurseOfWeaknessTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    static const uint32 exclusiveDebuffs[] = {
        702, 1108, 6205, 7646, 11707, 11708, 27224, 30909, 50511,
        99, 1735, 9490, 9747, 9898, 26998, 48559, 48560,
        1160, 6190, 11554, 11555, 11556, 25202, 25203, 47437,
        67, 26017
    };
    for (uint32 spellId : exclusiveDebuffs)
        if (target->HasAura(spellId))
            return false;

    return DebuffTrigger::IsActive();
}

bool LifeTapTrigger::IsActive()
{
    if (AI_VALUE2(uint8, "health", "self target") <= sPlayerbotAIConfig->lowHealth ||
        !AI_VALUE2(bool, "has mana", "self target") || AI_VALUE2(uint8, "mana", "self target") >= 85)
        return false;
    return true;
}

bool DemonicEmpowermentTrigger::IsActive()
{
    Pet* pet = bot->GetPet();
    return pet && !botAI->HasAura("demonic empowerment", pet);
}

bool DecimationTrigger::IsActive()
{
    Aura* aura = botAI->GetAura(getName(), GetTarget(), false, true);
    return aura && aura->GetDuration() > 3000;
}

bool WrongPetTrigger::IsActive()
{
    // #521 (AC WarlockTriggers.cpp:178-): fires when exactly one pet strategy is enabled and the active pet is
    // not that strategy's demon. Changes vs the old MoP port: (1) both key sets are accepted - AC's
    // imp/felhunter/felguard and this module's bhealth/bmana/bdps; (2) voidwalker/succubus are covered
    // (AC has them in its pets[] table); (3) the compared summon is the BASE spell (summon imp 688 etc.,
    // learnable) - the Grimoire of Supremacy override summons (Fel Imp 112866, Voidlord 112867,
    // Shivarra 112868, Observer 112869, Wrathguard 112870 - DBC ids via cls.py 2026-10-04) are
    // NOT-LEARNABLE, so "spell id" never resolves them (SpellIdValue.cpp scans the spellbook only) and the
    // trigger could never fire. Their pet entries are accepted as correct too.
    struct PetChoice
    {
        char const* strategy;
        char const* altStrategy;  // this module's spec-slot key, or nullptr
        char const* summonSpell;
        uint32 overrideSpellId;
    };
    static const PetChoice choices[] = {
        {"imp", "bhealth", "summon imp", 112866},
        {"voidwalker", nullptr, "summon voidwalker", 112867},
        {"succubus", nullptr, "summon succubus", 112868},
        {"felhunter", "bmana", "summon felhunter", 112869},
        {"felguard", "bdps", "summon felguard", 112870}
    };

    Player* player = botAI->GetBot();
    Pet* pet = player->GetPet();
    PetChoice const* selected = nullptr;
    uint32 selectedCount = 0;
    for (PetChoice const& choice : choices)
        if (botAI->HasStrategy(choice.strategy, BOT_STATE_NON_COMBAT) ||
            (choice.altStrategy && botAI->HasStrategy(choice.altStrategy, BOT_STATE_NON_COMBAT)))
        {
            selected = &choice;
            ++selectedCount;
        }

    if (selectedCount != 1 || !pet)
        return false;

    auto summonedEntry = [](SpellInfo const* info) -> uint32
    {
        if (!info)
            return 0;
        for (SpellEffectInfo const& effect : info->Effects)
            if (effect.Effect == SPELL_EFFECT_SUMMON_PET && effect.MiscValue > 0)
                return static_cast<uint32>(effect.MiscValue);
        return 0;
    };

    uint32 summonSpellId = AI_VALUE2(uint32, "spell id", selected->summonSpell);
    uint32 baseEntry = summonedEntry(summonSpellId ? sSpellMgr->GetSpellInfo(summonSpellId) : nullptr);
    if (!baseEntry)
        return false;

    uint32 overrideEntry = summonedEntry(sSpellMgr->GetSpellInfo(selected->overrideSpellId));
    return pet->GetEntry() != baseEntry && (!overrideEntry || pet->GetEntry() != overrideEntry);
}

const std::set<uint32> RainOfFireChannelCheckTrigger::RAIN_OF_FIRE_SPELL_IDS = {
    5740, 6219, 11677, 11678, 27212, 47819, 47820
};

bool RainOfFireChannelCheckTrigger::IsActive()
{
    if (Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        if (spell->m_spellInfo && RAIN_OF_FIRE_SPELL_IDS.count(spell->m_spellInfo->Id))
            return AI_VALUE(uint8, "attacker count") < minEnemies;
    return false;
}

bool LifeTapGlyphBuffTrigger::IsActive()
{
    constexpr uint32 SPELL_LIFE_TAP_GLYPH = 63320;
    return bot->HasAura(SPELL_LIFE_TAP_GLYPH) && BuffTrigger::IsActive();
}

bool WarlockConjuredItemTrigger::IsActive()
{
    // 5.4.8: AC also required a spendable "soul shard" inventory item here, but Cataclysm turned Soul
    // Shards into a capped resource (not a bag item) and Create Healthstone/Firestone/Spellstone no
    // longer cost one on this core (see the commented-out "soul shard" item-count check this fork already
    // left in WarlockActions.cpp CastDrainSoulAction::isUseful) -- so only the item-count check applies.
    return ItemCountTrigger::IsActive();
}

bool DarkIntentTrigger::IsActive()
{
    constexpr uint32 SPELL_DARK_INTENT = 109773;
    if (!bot->HasSpell(SPELL_DARK_INTENT))
        return false;
    return !bot->HasAura(SPELL_DARK_INTENT, bot->GetGUID());
}

bool MetamorphosisTrigger::IsActive()
{
    if (botAI->HasAura("metamorphosis", bot) && bot->GetPower(Powers::POWER_DEMONIC_FURY) <= 50)
    {
        botAI->RemoveAura("metamorphosis");
    }
    // #864 ("cast Metamorphosis 103958 ... in combat, demonic fury >= 500"): the
    // fork needed 800 and a 40 s dummy fight never got there (wave 73/74: 0 casts).
    return BuffTrigger::IsActive() && bot->GetPower(Powers::POWER_DEMONIC_FURY) >= 500;
}

bool CorruptionTrigger::IsActive()
{
    if (botAI->HasAura("metamorphosis", bot))
    {
        spell = "doom";
        return DebuffTrigger::IsActive();
    }

    spell = "corruption";
    return DebuffTrigger::IsActive() && !botAI->HasAura("doom", GetTarget(), false, true);
}

bool FireAndBrimeStoneTrigger::IsActive()
{
    if (botAI->HasAura("fire and brimstone", bot) && bot->GetPower(Powers::POWER_BURNING_EMBERS) <= 0)
    {
        botAI->RemoveAura("fire and brimstone");
    }
    return BuffTrigger::IsActive() && bot->GetPower(Powers::POWER_BURNING_EMBERS) >= 40;
}

bool SoulstoneTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", "soulstone");
    return spellId && bot->HasSpell(spellId) && !bot->HasSpellCooldown(spellId);
}

bool WarlockPetLowHealthTrigger::IsActive()
{
    Unit* pet = AI_VALUE(Unit*, "pet target");
    return pet && AI_VALUE2(uint8, "health", "pet target") < 40 && !AI_VALUE2(bool, "dead", "pet target") /*&& !AI_VALUE2(bool, "mounted", "self target")*/;
}
bool WarlockPetMediumHealthTrigger::IsActive()
{
    Unit* pet = AI_VALUE(Unit*, "pet target");
    return pet && AI_VALUE2(uint8, "health", "pet target") < sPlayerbotAIConfig->mediumHealth /* && !AI_VALUE2(bool, "dead", "pet target") && !AI_VALUE2(bool, "mounted", "self target")*/;
}

// ---- #864 / epic #854 (rotation lane 9) ----
bool WarlockHavocTrigger::IsActive()
{
    return AI_VALUE2(uint32, "spell id", "havoc") && WarlockHavocTarget(botAI);
}

bool WarlockSummonAbyssalTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", "summon abyssal");
    if (!spellId || !bot->HasSpell(spellId) || bot->HasSpellCooldown(spellId))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    return target && target->IsAlive();
}

bool WarlockShadowBoltFillerTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && target->IsAlive() && (rand() % 3) == 0;
}

bool WarlockPvpFearTrigger::IsActive()
{
    if (!AI_VALUE(bool, "in pvp instance"))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || target->GetTypeId() != TYPEID_PLAYER || !target->IsAlive())
        return false;

    return target->GetVictim() == bot && bot->IsWithinDistInMap(target, 5.0f) &&
           !AI_VALUE2(bool, "is cc", "current target") && !target->HasAuraType(SPELL_AURA_MOD_FEAR);
}

// Rotation fix lane K (#868): see WarlockTriggers.h.
bool WarlockFleeCloseTrigger::IsActive()
{
    if (AI_VALUE(bool, "in pvp instance"))
        return false;

    Trigger* t = botAI->GetAiObjectContext()->GetTrigger(inner);
    return t && t->IsActive();
}

// Rotation fix lane K (#868): see WarlockTriggers.h.
bool WarlockDuelRetargetTrigger::IsActive()
{
    if (!bot->duel || !bot->duel->opponent || bot->duel->startTime == 0)
        return false;

    return bot->duel->opponent->IsAlive() && !AI_VALUE(Unit*, "current target");
}
