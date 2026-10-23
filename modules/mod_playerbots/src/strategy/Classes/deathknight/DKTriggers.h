/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DKTRIGGERS_H
#define _PLAYERBOT_DKTRIGGERS_H

#include "GenericTriggers.h"
#include "HealthTriggers.h"

class PlayerbotAI;

class IcyTouch3sDebuffTrigger : public DebuffTrigger
{
public:
    IcyTouch3sDebuffTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "frost fever", 1, true, .0f, 3000) {}
};

class PlagueStrike3sDebuffTrigger : public DebuffTrigger
{
public:
    PlagueStrike3sDebuffTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "blood plague", 1, true, .0f, 3000) {}
};

// 5.4.8: the WotLK Unholy talent "Desolation" (and its buff, WotLK spell id 66817) does not exist on
// MoP's talent system, so "no desolation" is always true; the "dd cd and no desolation" composite this
// feeds degrades to a plain "death and decay cooldown" check.
class DesolationTrigger : public Trigger
{
public:
    DesolationTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no desolation") {}
    bool IsActive() override { return true; }
};

class DeathAndDecayCooldownTrigger : public SpellCooldownTrigger
{
public:
    DeathAndDecayCooldownTrigger(PlayerbotAI* botAI) : SpellCooldownTrigger(botAI, "death and decay") {}
    bool IsActive() override;
};

// Runes are still a real DK resource in MoP (6 slots: blood/frost/unholy). Fires when every rune is on
// cooldown, i.e. nothing is available to spend right now.
class NoRuneTrigger : public Trigger
{
public:
    NoRuneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no rune") {}
    bool IsActive() override;
};

// #149 (AC DKTriggers.h:148-167): rune slot layout is unchanged from WotLK on this core - slots 0/1
// are Blood, 2/3 Unholy, 4/5 Frost (Player::InitRunes(), Player.h RuneType) - so these check the same
// pairs AC does. AC reads a per-rune millisecond cooldown (GetRuneCooldown); this core's rune API only
// exposes RuneInfo::Readiness (0..RUNE_READINESS_FULL, filling back up to "ready"), so "<= 2000" becomes
// "the readiness gap left to fill is <= 2000" (RUNE_READINESS_FULL is 10000 units over the same ~10 s
// base regen AC's 2000 ms threshold assumes, so the units line up 1:1 at base haste).
class HighBloodRuneTrigger : public Trigger
{
public:
    HighBloodRuneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high blood rune") {}
    bool IsActive() override;
};

class HighFrostRuneTrigger : public Trigger
{
public:
    HighFrostRuneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high frost rune") {}
    bool IsActive() override;
};

class HighUnholyRuneTrigger : public Trigger
{
public:
    HighUnholyRuneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high unholy rune") {}
    bool IsActive() override;
};

// #149 (AC DKTriggers.h:176-180): Frost's Rime talent proc is still named "Freezing Fog" in MoP (see
// the existing comment in PlayerbotAI.cpp:2083 and the dead FrostDKStrategy.cpp:104 reference), so this
// ports unchanged.
class FreezingFogTrigger : public HasAuraTrigger
{
public:
    FreezingFogTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "freezing fog") {}
};

class MindFreezeOnEnemyHealerTrigger : public InterruptEnemyHealerTrigger
{
public:
    MindFreezeOnEnemyHealerTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "mind freeze") {}
};

// #357: AC DKTriggers.h:14-16 - buff-on-self triggers. These names are consumed by the fork's
// BloodDKStrategy / FrostDKStrategy / UnholyDKStrategy / GenericDKStrategy but had no
// registration, so Engine::GetTrigger() returned NULL and the TriggerNode was a silent no-op.
BUFF_TRIGGER(HornOfWinterTrigger, "horn of winter");

BUFF_TRIGGER(BoneShieldTrigger, "bone shield");

// rot-lane-2 (#857): "improved icy talons" and "unbreakable armor" triggers removed - NO-SUCH-NAME on 5.4.8
// (forkonly_verdicts.txt), no remaining reference. Pillar of Frost below is the 5.x successor.

// #601: Pillar of Frost 51271 = 5.x successor of Unbreakable Armor (FrostDKStrategy.cpp edge).
BUFF_TRIGGER(PillarOfFrostTrigger, "pillar of frost");

// AC DKTriggers.h:18-22 / 31-35 - the non-timed variants of the two 3s debuff triggers.
class PlagueStrikeDebuffTrigger : public DebuffTrigger
{
public:
    PlagueStrikeDebuffTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "blood plague", 1, true, .0f) {}
};

class IcyTouchDebuffTrigger : public DebuffTrigger
{
public:
    IcyTouchDebuffTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "frost fever", 1, true, .0f) {}
};

// AC DKTriggers.h:44-59 - re-apply the debuff when the melee attacker drops it.
class PlagueStrikeDebuffOnAttackerTrigger : public DebuffOnMeleeAttackerTrigger
{
public:
    PlagueStrikeDebuffOnAttackerTrigger(PlayerbotAI* botAI)
        : DebuffOnMeleeAttackerTrigger(botAI, "blood plague", true, .0f)
    {
    }
};

class IcyTouchDebuffOnAttackerTrigger : public DebuffOnMeleeAttackerTrigger
{
public:
    IcyTouchDebuffOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnMeleeAttackerTrigger(botAI, "frost fever", true, .0f)
    {
    }
};

// AC DKTriggers.h:69-79 - proc / cooldown availability triggers.
class BloodTapTrigger : public BuffTrigger
{
public:
    BloodTapTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "blood tap") {}
};

class RaiseDeadTrigger : public BuffTrigger
{
public:
    RaiseDeadTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "raise dead") {}
};

class DeathCoilTrigger : public SpellCanBeCastTrigger
{
public:
    DeathCoilTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "death coil") {}
};

// 5.4.8: AC (DKTriggers.h:93-98 / .cpp:20-37) requires the WotLK "Glyph of Pestilence" (63334), which
// refreshed Blood Plague / Frost Fever to full duration. That glyph no longer exists in Cataclysm; the
// MoP Glyph of Pestilence (58657) only widens Pestilence's radius (see GenericDKStrategy.cpp:194-196).
// The name is still registered for AC parity, but IsActive() is inert - it must not fire, or the bot
// would re-cast Pestilence on expiring diseases and waste runes (#149 / #357).
class PestilenceGlyphTrigger : public SpellTrigger
{
public:
    PestilenceGlyphTrigger(PlayerbotAI* botAI) : SpellTrigger(botAI, "pestilence") {}
    bool IsActive() override;
};

class BloodStrikeTrigger : public DebuffTrigger
{
public:
    BloodStrikeTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "blood strike", 1, true) {}
};

class MindFreezeInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    MindFreezeInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "mind freeze") {}
};

class StrangulateInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    StrangulateInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "strangulate") {}
};

class StrangulateOnEnemyHealerTrigger : public InterruptEnemyHealerTrigger
{
public:
    StrangulateOnEnemyHealerTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "strangulate") {}
};

class ChainsOfIceSnareTrigger : public SnareTargetTrigger
{
public:
    ChainsOfIceSnareTrigger(PlayerbotAI* botAI) : SnareTargetTrigger(botAI, "chains of ice") {}
};

class ArmyOfTheDeadTrigger : public BoostTrigger
{
public:
    ArmyOfTheDeadTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "army of the dead") {}
};

class HysteriaNoCooldownTrigger : public SpellNoCooldownTrigger
{
public:
    HysteriaNoCooldownTrigger(PlayerbotAI* botAI) : SpellNoCooldownTrigger(botAI, "hysteria") {}
};

// --- AC declares these four in DKTriggers.h but deliberately does NOT register them in
// --- DKAiObjectContext.cpp, so they are ported for class parity and left unregistered, exactly as AC.
class DKPresenceTrigger : public BuffTrigger
{
public:
    DKPresenceTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "blood presence") {}

    bool IsActive() override;
};

class RuneStrikeTrigger : public SpellCanBeCastTrigger
{
public:
    RuneStrikeTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "rune strike") {}
};

class HowlingBlastTrigger : public DebuffTrigger
{
public:
    HowlingBlastTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "howling blast", 1, true) {}
};

class KillingMachineTrigger : public BoostTrigger
{
public:
    KillingMachineTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "killing machine") {}
};

// ---------------------------------------------------------------------------------------------------------
// rot-lane-2 (#857): MoP rotation triggers. Class-local on purpose (no shared header touched).
// Aura ids (5.4.8): Blood Plague 55078, Frost Fever 55095, Chains of Ice 45524, Killing Machine 51124,
// Sudden Doom 81340, Crimson Scourge 81136 (rotation condition columns).
// ---------------------------------------------------------------------------------------------------------
BUFF_TRIGGER(UnholyFrenzyTrigger, "unholy frenzy");  // MoP rotation row 25219 (self, in combat, always)

// Base for triggers that look at the current target.
class DKCurrentTargetTrigger : public Trigger
{
public:
    DKCurrentTargetTrigger(PlayerbotAI* botAI, std::string const name) : Trigger(botAI, name) {}
    std::string const GetTargetName() override { return "current target"; }

protected:
    // Remaining ms of my own aura on the target; 0 when absent.
    int32 OwnAuraLeft(Unit* target, uint32 spellId) const;
};

// MoP rotation row 25203/25204 (Unholy), 25003/25004 (Blood): Outbreak when the target lacks my Blood Plague OR Frost Fever.
class DKOutbreakTrigger : public DKCurrentTargetTrigger
{
public:
    DKOutbreakTrigger(PlayerbotAI* botAI) : DKCurrentTargetTrigger(botAI, "dk outbreak") {}
    bool IsActive() override;
};

// MoP rotation row 25207/25208 (Festering Strike when a disease has <= 2 s left).
class DKDiseaseExpiringTrigger : public DKCurrentTargetTrigger
{
public:
    DKDiseaseExpiringTrigger(PlayerbotAI* botAI, std::string const name, int32 withinMs)
        : DKCurrentTargetTrigger(botAI, name), withinMs(withinMs)
    {
    }
    bool IsActive() override;

private:
    int32 withinMs;
};

// MoP rotation row 25005/25006 (Blood Boil when a disease has <= 3 s left) and 25007 (Crimson Scourge proc).
class DKBloodBoilTrigger : public DKCurrentTargetTrigger
{
public:
    DKBloodBoilTrigger(PlayerbotAI* botAI) : DKCurrentTargetTrigger(botAI, "dk blood boil") {}
    bool IsActive() override;
};

// MoP rotation row 25101/25102 (Frost): Frost Strike at runic power >= 89 or with Killing Machine up.
class DKFrostStrikeProcTrigger : public DKCurrentTargetTrigger
{
public:
    DKFrostStrikeProcTrigger(PlayerbotAI* botAI) : DKCurrentTargetTrigger(botAI, "dk frost strike proc") {}
    bool IsActive() override;
};

// MoP rotation row 25209/25210 (Unholy): Death Coil at runic power >= 90 or with Sudden Doom up.
class DKDeathCoilProcTrigger : public DKCurrentTargetTrigger
{
public:
    DKDeathCoilProcTrigger(PlayerbotAI* botAI) : DKCurrentTargetTrigger(botAI, "dk death coil proc") {}
    bool IsActive() override;
};

// MoP class row 600 (all DK specs): Chains of Ice on a PLAYER target >= 6 yd away that does not already have
// my Chains of Ice. PvP only (gated on lane 0's "in pvp instance"), so open-world / dungeon behaviour stays AC.
class DKChainsOfIcePlayerTrigger : public DKCurrentTargetTrigger
{
public:
    DKChainsOfIcePlayerTrigger(PlayerbotAI* botAI) : DKCurrentTargetTrigger(botAI, "dk chains of ice player") {}
    bool IsActive() override;
};

// MoP class rows 8-11 (prio group 10): an enemy is casting an interruptible spell within range (any enemy,
// not only the target). Uses lane 0's value "enemy caster target" (same condition as trigger "enemy caster in
// range"), gated on "in pvp instance" so PvE keeps AC's target / enemy-healer interrupts only.
class DKPvpEnemyCasterTrigger : public Trigger
{
public:
    DKPvpEnemyCasterTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dk pvp enemy caster") {}
    bool IsActive() override;
};

#endif
