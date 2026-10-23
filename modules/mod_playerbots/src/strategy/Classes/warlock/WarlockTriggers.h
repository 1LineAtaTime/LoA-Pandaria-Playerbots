/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WARLOCKTRIGGERS_H
#define _PLAYERBOT_WARLOCKTRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "PlayerbotAI.h"
#include "SharedDefines.h"

#include <set>

class PlayerbotAI;

// -- TALENTS

BUFF_TRIGGER_TIMED(SoulLinkTrigger, "soul link", 5000);

// -- GENERIC

// #864: own Dark Intent 109773 missing. The stock BuffTrigger matched ANY aura named
// "Dark Intent" and never fired for the wave-74 bot (no "T:dark intent", 0 casts in 3 legs); this tests the aura by
// id and caster instead (cast by me), like the MoP condition.
class DarkIntentTrigger : public BuffTrigger
{
public:
    DarkIntentTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "dark intent", 5000) {}
    bool IsActive() override;
};
// #594: Dark Soul (113860/113861/113858, MoP-only cooldowns, level 84) for the "boost" strategy.
BUFF_TRIGGER_TIMED(DarkSoulMiseryTrigger, "dark soul: misery", 5000);
BUFF_TRIGGER_TIMED(DarkSoulKnowledgeTrigger, "dark soul: knowledge", 5000);
BUFF_TRIGGER_TIMED(DarkSoulInstabilityTrigger, "dark soul: instability", 5000);
class CurseOfAgonyTrigger : public DebuffTrigger
{
public:
    // #381 (AC WarlockTriggers.h:231-232): needLifeTime 0.5 and IsActive() = BuffTrigger::IsActive() (AC skips the
    // DebuffTrigger lifetime check). The fork's 20.0 needLifeTime kept Agony 980 from firing on a long-lived target
    // (the dummy: hp / "estimated group dps" is compared against needLifeTime).
    CurseOfAgonyTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "agony", 1, true, 0.5f) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class CurseOfElementsTrigger : public DebuffTrigger
{
public:
    CurseOfElementsTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "curse of the elements", 1, false) {}
    bool IsActive() override;
};

class CurseOfEnfeeblementTrigger : public DebuffTrigger
{
public:
    CurseOfEnfeeblementTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "curse of enfeeblement", 1, false) {}
    bool IsActive() override;
};

class CorruptionTrigger : public DebuffTrigger
{
public:
    CorruptionTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "corruption", 1, true) {}
    bool IsActive() override;
};
DEBUFF_CHECKISOWNER_TRIGGER(UnstableAfflictionTrigger, "unstable affliction");

// Same as AC: fires when the Glyph of Life Tap aura (63320, still that id on this core -- see
// SPELL_WARLOCK_GLYPH_OF_LIFE_TAP in spell_warlock.cpp) is present and the "life tap" aura is not.
class LifeTapGlyphBuffTrigger : public BuffTrigger
{
public:
    LifeTapGlyphBuffTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "life tap") {}
    bool IsActive() override;
};

// Healthstones are still a real warlock item in MoP; ItemCountTrigger fires when the bot has fewer than
// `count` of the named item (see GenericTriggers.h), so "no healthstone" is correct despite the class name.
class WarlockConjuredItemTrigger : public ItemCountTrigger
{
public:
    WarlockConjuredItemTrigger(PlayerbotAI* botAI, std::string const item) : ItemCountTrigger(botAI, item, 1) {}
    bool IsActive() override;
};

class HasHealthstoneTrigger : public WarlockConjuredItemTrigger
{
public:
    HasHealthstoneTrigger(PlayerbotAI* botAI) : WarlockConjuredItemTrigger(botAI, "healthstone") {}
};

// #163 S8: AC's OutOfSoulstoneTrigger/"no soulstone" (WarlockTriggers.h:79-84) is N/A on 5.4.8 -
// there's no "Create Soulstone" spell to react to it with (see the WarlockAiObjectContext.cpp
// comment on "create soulstone"), so it is not ported.

// #163 S8 (AC parity, WarlockTriggers.h:86-104): the bot has a soulstone and its resurrection-
// sickness-preventing aura is off cooldown. 5.4.8: AC's WotLK-era per-rank aura id list (20707,
// 20762-20765, 27239, 47883) mostly no longer exists (verified against Spell.dbc: only 20707
// "Soulstone" is present) - a single item/aura id covers every rank now.
class SoulstoneTrigger : public Trigger
{
public:
    SoulstoneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "soulstone") {}

    // #163 S8 fix per research (B.1): direct-cast spell, so this tests "known and off cooldown"
    // instead of an item count. Body in WarlockTriggers.cpp: AI_VALUE2 needs Playerbots.h and
    // bot->HasSpell()/HasSpellCooldown() need a complete Player type, and this header does not
    // pull either in (same reason every other IsActive() override in this file is declared here
    // and defined in the .cpp).
    bool IsActive() override;
};

// -- DEMONO
class MetamorphosisTrigger : public BuffTrigger
{
public:
    MetamorphosisTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "metamorphosis", true) {}
    bool IsActive() override;
};

class MoltenCoreTrigger : public HasAuraTrigger
{
public:
    MoltenCoreTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "molten core") {}
};

// -- Destru
DEBUFF_CHECKISOWNER_TRIGGER(ImmolateTrigger, "immolate");
class BackdraftTrigger : public HasAuraStackTrigger
{
public:
    BackdraftTrigger(PlayerbotAI* ai) : HasAuraStackTrigger(ai, "backdraft", 1) {}
};
class FireAndBrimeStoneTrigger : public BuffTrigger
{
public:
    FireAndBrimeStoneTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "fire and brimstone", true) {}
    bool IsActive() override;
};

// AC parity triggers whose WotLK bag-item semantics do not exist on 5.4.8.
class OutOfSoulShardsTrigger : public Trigger
{
public:
    OutOfSoulShardsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no soul shard") {}
    bool IsActive() override;
};

class TooManySoulShardsTrigger : public Trigger
{
public:
    TooManySoulShardsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "too many soul shards") {}
    bool IsActive() override;
};

class DemonArmorTrigger : public BuffTrigger
{
public:
    DemonArmorTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "demon armor") {}
    bool IsActive() override;
};

class ShadowTranceTrigger : public HasAuraTrigger
{
public:
    ShadowTranceTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "shadow trance") {}
};

class UnendingBreathTrigger : public BuffTrigger
{
public:
    UnendingBreathTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "unending breath", 10000) {}
    bool IsActive() override;
};

class UnendingBreathOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    // AC parity, kept 1:1 (owner rule: keep AC code as is): AC WarlockTriggers.h:47 passes "unending breath on party" as the spell
    // name, which never resolves (w76: [SID] resolved=0 x936), so the edge is dead on AC too.
    UnendingBreathOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "unending breath on party", 4000) {}
    bool IsActive() override;
};

// These wrappers are retained for class parity, but are not registered: both weapon stones were
// removed before MoP, and the fork has no corresponding spell/item behavior.
class FirestoneTrigger : public BuffTrigger
{
public:
    FirestoneTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "firestone") {}
    bool IsActive() override;
};

class SpellstoneTrigger : public BuffTrigger
{
public:
    SpellstoneTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "spellstone") {}
    bool IsActive() override;
};

class HasFirestoneTrigger : public WarlockConjuredItemTrigger
{
public:
    HasFirestoneTrigger(PlayerbotAI* botAI) : WarlockConjuredItemTrigger(botAI, "firestone") {}
};

class HasSpellstoneTrigger : public WarlockConjuredItemTrigger
{
public:
    HasSpellstoneTrigger(PlayerbotAI* botAI) : WarlockConjuredItemTrigger(botAI, "spellstone") {}
};

class BanishTrigger : public RtiCcTrigger
{
public:
    BanishTrigger(PlayerbotAI* botAI) : RtiCcTrigger(botAI, "banish") {}
};

class FearTrigger : public RtiCcTrigger
{
public:
    FearTrigger(PlayerbotAI* botAI) : RtiCcTrigger(botAI, "fear") {}
};

class SpellLockInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    SpellLockInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "spell lock") {}
};

class DevourMagicPurgeTrigger : public TargetAuraDispelTrigger
{
public:
    DevourMagicPurgeTrigger(PlayerbotAI* botAI) : TargetAuraDispelTrigger(botAI, "devour magic", DISPEL_MAGIC) {}
};

class DevourMagicCleanseTrigger : public PartyMemberNeedCureTrigger
{
public:
    DevourMagicCleanseTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "devour magic", DISPEL_MAGIC) {}
};

class CorruptionOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    CorruptionOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "corruption", true) {}
    bool IsActive() override;
};

class ImmolateOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    ImmolateOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "immolate", true) {}
    bool IsActive() override;
};

class UnstableAfflictionOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    UnstableAfflictionOnAttackerTrigger(PlayerbotAI* botAI)
        : DebuffOnAttackerTrigger(botAI, "unstable affliction", true) {}
    bool IsActive() override;
};

class HauntTrigger : public DebuffTrigger
{
public:
    HauntTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "haunt", 1, true, 0.0f) {}
};

class CurseOfAgonyOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    CurseOfAgonyOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "agony", true) {}
    bool IsActive() override;
};

class CurseOfDoomTrigger : public DebuffTrigger
{
public:
    CurseOfDoomTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "doom", 1, true, 0.5f) {}
    bool IsActive() override;
};

class CurseOfExhaustionTrigger : public DebuffTrigger
{
public:
    CurseOfExhaustionTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "curse of exhaustion", 1, true, 0.5f) {}
    bool IsActive() override;
};

class CurseOfTonguesTrigger : public DebuffTrigger
{
public:
    CurseOfTonguesTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "curse of enfeeblement", 1, true, 0.5f) {}  // #521: 5.0.4 successor
    bool IsActive() override;
};

class CurseOfWeaknessTrigger : public DebuffTrigger
{
public:
    CurseOfWeaknessTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "curse of enfeeblement", 1, true, 0.5f) {}  // #521: 5.0.4 successor
    bool IsActive() override;
};

class LifeTapTrigger : public Trigger
{
public:
    LifeTapTrigger(PlayerbotAI* botAI) : Trigger(botAI, "life tap") {}
    bool IsActive() override;
};

class DemonicEmpowermentTrigger : public BuffTrigger
{
public:
    DemonicEmpowermentTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "demonic empowerment") {}
    bool IsActive() override;
};

class ImmolationAuraActiveTrigger : public HasAuraTrigger
{
public:
    ImmolationAuraActiveTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "immolation aura") {}
};

class BacklashTrigger : public HasAuraTrigger
{
public:
    BacklashTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "backlash") {}
};

class DecimationTrigger : public HasAuraTrigger
{
public:
    DecimationTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "decimation") {}
    bool IsActive() override;
};

class MetamorphosisNotActiveTrigger : public HasNoAuraTrigger
{
public:
    MetamorphosisNotActiveTrigger(PlayerbotAI* botAI) : HasNoAuraTrigger(botAI, "metamorphosis") {}
};

class MetaMeleeEnemyTooCloseForSpellTrigger : public TwoTriggers
{
public:
    MetaMeleeEnemyTooCloseForSpellTrigger(PlayerbotAI* botAI)
        : TwoTriggers(botAI, "enemy too close for spell", "metamorphosis not active") {}
};

class WrongPetTrigger : public Trigger
{
public:
    WrongPetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "wrong pet") {}
    bool IsActive() override;
};

class RainOfFireChannelCheckTrigger : public Trigger
{
public:
    RainOfFireChannelCheckTrigger(PlayerbotAI* botAI, uint32 minEnemies = 2)
        : Trigger(botAI, "rain of fire channel check"), minEnemies(minEnemies) {}
    bool IsActive() override;

protected:
    uint32 minEnemies;
    static const std::set<uint32> RAIN_OF_FIRE_SPELL_IDS;
};

// -- Pet
BEGIN_TRIGGER(WarlockPetLowHealthTrigger, Trigger)
END_TRIGGER()
BEGIN_TRIGGER(WarlockPetMediumHealthTrigger, Trigger)
END_TRIGGER()

// ---- #864 / epic #854 (rotation lane 9): MoP warlock rotation port ------------------------------------------
// (Destruction Havoc): an attacker other than the primary target exists and no Havoc
// of mine is up yet (see WarlockHavocTarget in WarlockActions.cpp).
class WarlockHavocTrigger : public Trigger
{
public:
    WarlockHavocTrigger(PlayerbotAI* botAI) : Trigger(botAI, "havoc") {}
    bool IsActive() override;
};

// Fear row (every spec, group 5): Fear only in a battleground/arena, only on a player that is on me
// (<= 5 yd), not already crowd controlled ("is cc" guard) and not feared. DR >= 25% is not modelled (no shared DR
// value). Outside pvp instances the AC "cc" strategy stays the only Fear user.
// (all three specs, group 0, "cast Summon Abyssal 112921 [only if learned],
// current target, in combat, always"): the Grimoire of Supremacy greater demon, a cooldown. Active while it is known,
// off cooldown and there is a live current target.
class WarlockSummonAbyssalTrigger : public Trigger
{
public:
    WarlockSummonAbyssalTrigger(PlayerbotAI* botAI) : Trigger(botAI, "summon abyssal ready") {}
    bool IsActive() override;
};

// (affliction, group 0 prio 1 = the same bucket as Malefic Grasp, both "always"):
// the bucket is drawn at random every tick, so Shadow Bolt is the filler on roughly one tick in three.
class WarlockShadowBoltFillerTrigger : public Trigger
{
public:
    WarlockShadowBoltFillerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shadow bolt filler") {}
    bool IsActive() override;
};

class WarlockPvpFearTrigger : public Trigger
{
public:
    WarlockPvpFearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "fear pvp") {}
    bool IsActive() override;
};

// Rotation fix lane K (#868): AC's "enemy too close for spell" / "meta melee flee check" -> flee 39.0 edge (AC
// Destruction/Affliction/DemonologyWarlockStrategy.cpp) outside a pvp instance / started duel only. the MoP rotation never flees; a
// warlock that runs from a duel opponent cannot cast cast-time spells (movement interrupts them) - w76 t613 Helice fled at
// duel second 4. In PvP the Fear / Mortal Coil / Shadowfury rows handle a close enemy.
class WarlockFleeCloseTrigger : public Trigger
{
public:
    WarlockFleeCloseTrigger(PlayerbotAI* botAI, std::string const name, std::string const inner)
        : Trigger(botAI, name, 1), inner(inner) {}
    bool IsActive() override;

private:
    std::string const inner;
};

// Rotation fix lane K (#868, BEYOND AC): a started duel whose opponent is alive but the bot has no current target; drives
// "attack duel opponent" in the combat and non-combat engines (see HunterDuelRetargetTrigger for the root cause).
class WarlockDuelRetargetTrigger : public Trigger
{
public:
    WarlockDuelRetargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "warlock duel retarget", 1) {}
    bool IsActive() override;
};
#endif
