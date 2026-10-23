/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRUIDTRIGGERS_H
#define _PLAYERBOT_DRUIDTRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "HealthTriggers.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include <set>

class PlayerbotAI;

// AC Ai/Class/Druid/DruidTriggers.h:20 - the Omen of Clarity proc aura id FaerieFireFeralTrigger tests.
constexpr uint32 AURA_OMEN_OF_CLARITY = 16864;

// 1:1 port of AC Ai/Class/Druid/DruidTriggers.h (WotLK mod-playerbots), registered in
// DruidAiObjectContext.cpp with AC's exact creator names and factory shapes.
//
// #357 slice 4: AC registers 53 Druid triggers; the fork registered 9 of them (plus 2 MoP-only "eclipse
// ... cooldown" names AC has no equivalent of, left in place). These are AC's remaining 44.
// Every base class below already exists with AC's signature in strategy/triggers/GenericTriggers.h /
// CureTriggers.h (BuffTrigger, BuffOnPartyTrigger, BuffOnMainTankTrigger, BoostTrigger, HasAuraTrigger,
// DebuffTrigger, DebuffOnAttackerTrigger, InterruptSpellTrigger, InterruptEnemyHealerTrigger,
// SpellNoCooldownTrigger, TwoTriggers, RtiCcTrigger, NeedCureTrigger, PartyMemberNeedCureTrigger).
//
// 5.4.8 divergences applied here (see DruidTriggers.cpp for the .cpp-side ones):
//  - every IsActive() body lives in the .cpp, NOT inline in this header: the fork's
//    PlayerbotAI::HasAura(std::string, Unit*) cannot bind a `bot` (Player*) from a header TU (C2665,
//    "types pointed to are unrelated" - Player is not a complete/convertible type there).
//  - AI_VALUE / AI_VALUE2 are only ever used inside a trigger member, never a free helper
//    (they expand to `context->GetValue<...>()` and `context` is an AiObject member).

// Druid: Mark of the Wild (AC DruidTriggers.h:24-36).
class MarkOfTheWildOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    MarkOfTheWildOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "mark of the wild", 4 * 2000) {}
};

class MarkOfTheWildTrigger : public BuffTrigger
{
public:
    MarkOfTheWildTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "mark of the wild", 4 * 2000) {}

    bool IsActive() override;
};

// Druid: Thorns (AC DruidTriggers.h:38-58).
class ThornsOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    ThornsOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "thorns", 2 * 2000) {}

    bool IsActive() override;
};

class ThornsOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    ThornsOnMainTankTrigger(PlayerbotAI* botAI) : BuffOnMainTankTrigger(botAI, "thorns", false, 2 * 2000) {}
};

class ThornsTrigger : public BuffTrigger
{
public:
    ThornsTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "thorns", 2 * 2000) {}

    bool IsActive() override;
};

// Druid: Clearcasting / Predator's Swiftness / Nature's Swiftness (AC DruidTriggers.h:60-77).
class ClearcastingTrigger : public HasAuraTrigger
{
public:
    ClearcastingTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "clearcasting") {}
};

// #384 (AC DruidTriggers.h:65-69): the 3.3.5 aura "Predator's Swiftness" is "Predatory Swiftness" (69369) in
// 5.4.8 (Spell.dbc name; Wowhead mop-classic spell 69369, 2026-10-04: finishing moves make the next Entangling
// Roots / Healing Touch / Hibernate / Rebirth instant, free and castable in all forms). HasAuraTrigger matches
// the aura by name (getName()), so the old spelling never matched and every "predator's swiftness ..." edge was
// dead. The trigger is still REGISTERED under AC's key "predator's swiftness".
class PredatorsSwiftnessTrigger : public HasAuraTrigger
{
public:
    PredatorsSwiftnessTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "predatory swiftness") {}
};

class NaturesSwiftnessActiveTrigger : public HasAuraTrigger
{
public:
    NaturesSwiftnessActiveTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "nature's swiftness") {}

    bool IsActive() override;
};

// Druid: Feral opener debuffs (AC DruidTriggers.h:79-151).
class RakeTrigger : public DebuffTrigger
{
public:
    // Wave-74 fix G (#860), MoP rotation row 10303 (spec 103, prio group 1/5): refresh Rake at <= 1500 ms left
    // (beforeDuration 1500; needLifeTime keeps the AC default 8.0).
    RakeTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "rake", 1, true, 8.0f, 1500) {}

    bool IsActive() override;
};

// #384: Insect Swarm (5570) is not learnable in 5.4.8 (cls.py druid: NOT-LEARNABLE, ids 5570/65855/...). Its 5.x
// successor is Sunfire (93402, balance spec102, level 18 - the same level Insect Swarm was learned at; Wowhead
// mop-classic 93402, 2026-10-04: instant 14 sec Nature DoT). The AC trigger keys stay "insect swarm" /
// "insect swarm on attacker"; only the spell they track changes.
class InsectSwarmTrigger : public DebuffTrigger
{
public:
    InsectSwarmTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "sunfire", 1, true) {}
};

class InsectSwarmOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    InsectSwarmOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "sunfire", true) {}

    bool IsActive() override;
};

class MoonfireTrigger : public DebuffTrigger
{
public:
    MoonfireTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "moonfire", 1, true) {}
};

class MoonfireOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    MoonfireOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "moonfire", true) {}

    bool IsActive() override;
};

class FaerieFireTrigger : public DebuffTrigger
{
public:
    FaerieFireTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "faerie fire", 1, false, 25.0f) {}

    bool IsActive() override;
};

// #384: 5.x has ONE Faerie Fire (770, Wowhead mop-classic 770, 2026-10-04: Weakened Armor, usable in any form);
// "Faerie Fire (Feral)" no longer exists (cls.py: NO-SUCH-NAME), so the feral trigger tracks plain "faerie fire".
class FaerieFireFeralTrigger : public DebuffTrigger
{
public:
    FaerieFireFeralTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "faerie fire") {}

    bool IsActive() override;
};

// Druid: interrupts and cooldowns (AC DruidTriggers.h:153-181).
// #384: Bash (5211) is the talent "Mighty Bash" (a 5 sec stun, not an interrupt) in 5.4.8. The 5.x feral/guardian
// interrupt is Skull Bash (106839, spec103/104 level 64, Wowhead mop-classic 106839, 2026-10-04: interrupts and
// locks the school 4 sec, requires cat or bear form), so the "bash" interrupt edges track Skull Bash.
class BashInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    BashInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "skull bash") {}
};

class BerserkTrigger : public BoostTrigger
{
public:
    BerserkTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "berserk") {}
};

class BerserkActiveTrigger : public HasAuraTrigger
{
public:
    BerserkActiveTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "berserk") {}
};

class SavageRoarTrigger : public BuffTrigger
{
public:
    SavageRoarTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "savage roar") {}
};

class NaturesGraspTrigger : public BuffTrigger
{
public:
    NaturesGraspTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "nature's grasp") {}
};

// Druid: crowd control (AC DruidTriggers.h:183-207). Hibernate/Cyclone/EntanglingRoots are the same
// RtiCcTrigger shapes the fork already registers inline in DruidAiObjectContext.cpp (#163 S8).
class EntanglingRootsTrigger : public RtiCcTrigger
{
public:
    EntanglingRootsTrigger(PlayerbotAI* botAI) : RtiCcTrigger(botAI, "entangling roots") {}
};

class HibernateTrigger : public RtiCcTrigger
{
public:
    HibernateTrigger(PlayerbotAI* botAI) : RtiCcTrigger(botAI, "hibernate") {}
};

class CycloneTrigger : public RtiCcTrigger
{
public:
    CycloneTrigger(PlayerbotAI* botAI) : RtiCcTrigger(botAI, "cyclone") {}
};

// Druid: cures. CurePoisonTrigger is the bot-self variant AC registers as "cure poison" (the fork only
// had the party-member shape); the party-member shape already exists below.
class CurePoisonTrigger : public NeedCureTrigger
{
public:
    CurePoisonTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cure poison", DISPEL_POISON) {}
};

// Druid: shapeshift forms (AC DruidTriggers.h:223-253).
class BearFormTrigger : public BuffTrigger
{
public:
    BearFormTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "bear form") {}

    bool IsActive() override;
};

class TreeFormTrigger : public BuffTrigger
{
public:
    TreeFormTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "tree of life") {}

    bool IsActive() override;
};

class CatFormTrigger : public BuffTrigger
{
public:
    CatFormTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "cat form") {}

    bool IsActive() override;
};

class AquaticFormTrigger : public Trigger
{
public:
    AquaticFormTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aquatic form") {}

    bool IsActive() override;
};

// Eclipse is still a real Balance Druid mechanic in MoP (removed only in WoD); "eclipse (solar)"/
// "eclipse (lunar)" are the actual MoP buff names (AC DruidTriggers.h:255-265).
// #521 w13: in 5.4.8 both Eclipse auras (48517/48518) are permanent (SpellDuration index 21 = -1) until the Balance
// Energy bar crosses 0 again (spell_druid.cpp spell_dru_eclipse_energize). HasAuraTrigger checks with checkDuration=true,
// which skips duration -1 auras (PlayerbotAI.cpp HasAura), so the AC trigger never fired and Wrath was never cast.
// Same name check, without the duration filter. MoP-only (WotLK Eclipse lasted 15 s).
class EclipseSolarTrigger : public HasAuraTrigger
{
public:
    EclipseSolarTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "eclipse (solar)") {}

    bool IsActive() override;
};

class EclipseLunarTrigger : public HasAuraTrigger
{
public:
    EclipseLunarTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "eclipse (lunar)") {}

    bool IsActive() override;
};

// #521 w13, MoP-only (no AC counterpart): the 5.4.8 Balance Energy bar has a direction. While the lunar-direction marker
// 67484 is on the druid (from Solar Eclipse until Lunar Eclipse), Starfire generates no energy
// (spell_druid.cpp spell_dru_eclipse::HandleCast), so Wrath must be the filler or the bar never moves again.
class EclipseLunarDirectionTrigger : public Trigger
{
public:
    EclipseLunarDirectionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "eclipse lunar direction") {}

    bool IsActive() override;
};

// Druid: bash interrupt on the enemy healer + the Swiftness buff (AC DruidTriggers.h:267-286).
class BashInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
{
public:
    BashInterruptEnemyHealerSpellTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "skull bash") {}
};

class NaturesSwiftnessTrigger : public BuffTrigger
{
public:
    NaturesSwiftnessTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "nature's swiftness") {}

    // Wave-75 fix J (#868): the "boost" edge (ACTION_HIGH + 9) is on every druid but 132158 is a restoration talent; a cat or
    // bear without it resolved the spell id to 0 and logged [CAST] spell=0 why=no-spellid on every tick (w75 Steli: 249x).
    bool IsActive() override;
};

// #384/#521: "being attacked" (nature's grasp edge) is the shared BeingAttackedTrigger in
// strategy/triggers/TriggerContext.h (AC TriggerContext.h:127).

class DruidPartyMemberRemoveCurseTrigger : public PartyMemberNeedCureTrigger
{
public:
    DruidPartyMemberRemoveCurseTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "druid remove curse", DISPEL_CURSE)
    {
    }
};

// Druid: caster / tank cooldowns and bear-cat debuffs (AC DruidTriggers.h:288-364).
class StarfallTrigger : public SpellNoCooldownTrigger
{
public:
    StarfallTrigger(PlayerbotAI* botAI) : SpellNoCooldownTrigger(botAI, "starfall") {}
};

class ForceOfNatureTrigger : public BoostTrigger
{
public:
    ForceOfNatureTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "force of nature") {}
};

// #384: 5.x Mangle is ONE spell for both forms (33917, Wowhead mop-classic 33917, 2026-10-04: "Requires Cat Form,
// Bear Form"); "mangle (bear)" / "mangle (cat)" are NO-SUCH-NAME in the 5.4.8 DBC, so these track "mangle".
class MangleBearTrigger : public DebuffTrigger
{
public:
    MangleBearTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "mangle") {}

    bool IsActive() override;
};

class LacerateTrigger : public DebuffTrigger
{
public:
    LacerateTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "lacerate") {}

    bool IsActive() override;
};

// #384: Demoralizing Roar is not learnable in 5.4.8; Thrash (bear 77758 / cat 106830) applies Weakened Blows
// (-10% physical damage dealt, 30 sec; Wowhead mop-classic 77758 + 106830, 2026-10-04), which is its 5.x
// replacement. AC's DemoralizeRoarTrigger logic is kept, retargeted: only cast when the target does not already
// carry a Weakened Blows source (implemented in the .cpp).
class ThrashTrigger : public DebuffTrigger
{
public:
    ThrashTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "thrash", 1, true) {}

    bool IsActive() override;
};

// Wave-74 fix G (#860), MoP rotation row 10304 (spec 103, prio group 1/4): cat Thrash, refreshed at <= 1500 ms
// left, never while Prowl is up (a Thrash from stealth would break it before Pounce, MoP rotation row 10316, can open).
class ThrashCatTrigger : public DebuffTrigger
{
public:
    ThrashCatTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "thrash", 1, true, 8.0f, 1500) {}

    bool IsActive() override { return !botAI->HasAura("prowl", bot) && DebuffTrigger::IsActive(); }
};

// Wave-74 fix G (#860), MoP rotation row 10316 (spec 103, prio group 10/0 "prowl pounce"): active while the
// bot is prowling, so Pounce (9005, needs stealth) is tried before every other cat edge.
class ProwlActiveTrigger : public HasAuraTrigger
{
public:
    ProwlActiveTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "prowl") {}

    // Wave-75 fix J (#860): Prowl's aura has an infinite duration (-1) and HasAuraTrigger::IsActive passes checkDuration=true,
    // which skips every aura whose duration is -1: the trigger was never active (w75 cat leg: no "T:prowl" after the Prowl
    // cast, Pounce only ever pushed at the 5.6 default edge). Same fix as the Eclipse triggers (DruidTriggers.cpp, w13).
    bool IsActive() override;
};

// Wave-75 fix J (#868): the cat "behind" edge for melee druids. AC's IsNotBehindTargetTrigger keeps pushing "set behind"
// (ACTION_MOVE + 7, above every spell) after a moving or casting duel opponent; w75 Steli (feral, duel vs casters) spent 33
// ticks on "set behind" and 34 on "set facing" (one executed action per tick) and cast 7.8 times a minute. Against a player
// in a duel / battleground / arena the cat stays on the target and casts (Shred then fails NOT_BEHIND and Mangle, no
// positional requirement, takes its slot).
class DruidNotBehindTargetTrigger : public IsNotBehindTargetTrigger
{
public:
    DruidNotBehindTargetTrigger(PlayerbotAI* botAI) : IsNotBehindTargetTrigger(botAI) {}

    bool IsActive() override;
};

class DemoralizeRoarTrigger : public DebuffTrigger
{
public:
    DemoralizeRoarTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "demoralizing roar") {}

    bool IsActive() override;
};

class MangleCatTrigger : public DebuffTrigger
{
public:
    MangleCatTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "mangle", 1, false, 0.0f) {}

    bool IsActive() override;
};

// #149 (AC DruidTriggers.h:189-195): "entangling roots" is unchanged in MoP, so this reuses the same
// DebuffTrigger + IsActive() (attacker count / target has no mana pool) as AC.
class EntanglingRootsKiteTrigger : public DebuffTrigger
{
public:
    EntanglingRootsKiteTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "entangling roots") {}

    bool IsActive() override;
};

// #149 (AC DruidTriggers.h:366-392): Feral finisher timing check (5 combo points, Savage Roar/Rip
// still have enough duration left) - unchanged mechanic in MoP, ported as-is.
class FerociousBiteTimeTrigger : public Trigger
{
public:
    FerociousBiteTimeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ferocious bite time") {}

    bool IsActive() override;
};

// Druid: Feral execute / Hurricane AoE coverage / healer-dps guard / Prowl stealth
// (AC DruidTriggers.h:394-452).
class FerociousBiteExecuteTrigger : public Trigger
{
public:
    FerociousBiteExecuteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ferocious bite execute") {}

    bool IsActive() override;
};

class HurricaneChannelCheckTrigger : public Trigger
{
public:
    HurricaneChannelCheckTrigger(PlayerbotAI* botAI, uint32 minEnemies = 3)
        : Trigger(botAI, "hurricane channel check"), minEnemies(minEnemies)
    {
    }

    bool IsActive() override;

protected:
    uint32 minEnemies;
    static const std::set<uint32> HURRICANE_SPELL_IDS;
};

class NoHealerDpsStrategyTrigger : public Trigger
{
public:
    NoHealerDpsStrategyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no healer dps strategy") {}

    bool IsActive() override;
};

class ProwlTrigger : public Trigger
{
public:
    ProwlTrigger(PlayerbotAI* botAI) : Trigger(botAI, "prowl") {}

    bool IsActive() override;
};

// Druids can still cure poison (Abolish Poison) and remove curse in MoP; the "spell" string here is only
// an internal identifier for PartyMemberNeedCureTrigger/NeedCureTrigger (matching AC), the real cast
// spells are "abolish poison on party" / "remove curse on party" (already wired in DruidActions.h).
class PartyMemberCurePoisonTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCurePoisonTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "cure poison", DISPEL_POISON)
    {
    }
};


// ---- Rotation lane 5 (#860, epic #854): MoP druid rotation triggers (IsActive bodies in DruidTriggers.cpp).
// MoP rotation row 10205: Starsurge is off cooldown (instant, no Eclipse condition).
class StarsurgeTrigger : public SpellNoCooldownTrigger
{
public:
    StarsurgeTrigger(PlayerbotAI* botAI) : SpellNoCooldownTrigger(botAI, "starsurge") {}
};

// MoP rotation row 10213/10218 (Celestial Alignment) and 10214/10410 (Nature's Vigil): "always" cooldowns.
class CelestialAlignmentTrigger : public BoostTrigger
{
public:
    CelestialAlignmentTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "celestial alignment") {}
};

class NaturesVigilTrigger : public BoostTrigger
{
public:
    NaturesVigilTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "nature's vigil") {}
};

// MoP rotation row 1100 (class): Might of Ursoc at own health <= 25% (explicit 25, not the configurable
// "critical health", so the threshold stays MoP's).
class MightOfUrsocTrigger : public LowHealthTrigger
{
public:
    MightOfUrsocTrigger(PlayerbotAI* botAI) : LowHealthTrigger(botAI, "might of ursoc", 25.0f, 0) {}
};

// MoP rotation row 10417: Savage Defense at rage >= 80.
class SavageDefenseTrigger : public RageAvailable
{
public:
    SavageDefenseTrigger(PlayerbotAI* botAI) : RageAvailable(botAI, 80) {}
};

// MoP rotation row 10215 (Rejuvenation self, health <= 50%) / 10216 (Healing Touch self, health <= 30%),
// balance in-combat self heals (prio group 5/1).
class DruidSelfRejuvenationTrigger : public LowHealthTrigger
{
public:
    DruidSelfRejuvenationTrigger(PlayerbotAI* botAI) : LowHealthTrigger(botAI, "druid self rejuvenation", 50.0f, 0) {}
};

class DruidSelfHealingTouchTrigger : public LowHealthTrigger
{
public:
    DruidSelfHealingTouchTrigger(PlayerbotAI* botAI) : LowHealthTrigger(botAI, "druid self healing touch", 30.0f, 0) {}
};

// MoP rotation row 10311 (health <= 35% with Predatory Swiftness, prio 6/2) and 10310 (health <= 90%, 6/0):
// the free instant Healing Touch of a cat. 69369 Predatory Swiftness.
class PredatorsSwiftnessHealTrigger : public Trigger
{
public:
    PredatorsSwiftnessHealTrigger(PlayerbotAI* botAI, std::string const name, uint8 maxHealth)
        : Trigger(botAI, name), maxHealth(maxHealth)
    {
    }

    bool IsActive() override;

protected:
    uint8 maxHealth;
};

// MoP rotation row 10505: the main tank is at health <= 40% and has no Ironbark yet.
class IronbarkTrigger : public Trigger
{
public:
    IronbarkTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ironbark") {}

    bool IsActive() override;
};

#endif
