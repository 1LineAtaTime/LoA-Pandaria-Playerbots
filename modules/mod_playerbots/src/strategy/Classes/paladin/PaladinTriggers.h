/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 */

#ifndef _PLAYERBOT_PALADINTRIGGERS_H
#define _PLAYERBOT_PALADINTRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "SharedDefines.h"
#include "Unit.h"

class PlayerbotAI;

// #357 slice 6 (AC PaladinTriggers.h:17-21,60-61,86-89,96,100-105): plain BUFF_TRIGGERs, ported 1:1.
BUFF_TRIGGER(HolyShieldTrigger, "holy shield");
BUFF_TRIGGER(RighteousFuryTrigger, "righteous fury");
// #525: Inquisition (84963) upkeep, Ret Holy Power buff with no AC equivalent. BuffTrigger fires when the
// aura is missing; the cast itself needs >= 1 Holy Power (spell cost check), so no Holy Power value is needed.
BUFF_TRIGGER(InquisitionTrigger, "inquisition");

// #735 (owner 2026-10-08): the Holy Power spenders are gated the way AC gates its combo-point finishers - by a
// resource trigger in the strategy, not in the action. Same shape as AC ComboPointsAvailableTrigger /
// EnergyAvailable (AC Ai/Base/Trigger/GenericTriggers.h:79-114, .cpp:92-94: StatAvailable + an "amount"), registered
// per amount like AC's "combo points 3/4/5 available" (AC Ai/Base/TriggerContext.h:120-122,367-369). Holy Power has
// no AI value (AC has none either), so the trigger reads the power directly, as AC's StealthTrigger reads the bot.
class HolyPowerAvailableTrigger : public StatAvailable
{
public:
    HolyPowerAvailableTrigger(PlayerbotAI* botAI, int32 amount)
        : StatAvailable(botAI, amount, "holy power available")
    {
    }

    bool IsActive() override;
};

// #735: Divine Purpose proc (90174, from talent 86172): the next Holy Power ability costs nothing and acts as if 3
// Holy Power were spent (Spell.dbc 5.4.8; Wowhead 86172 tooltip). AC's proc-aura trigger shape (AC ArtOfWarTrigger,
// PaladinTriggers.h:79-83: HasAuraTrigger on the proc). By id, not by name: the talent's passive 86172 is also
// called "Divine Purpose", so a name test would be active all the time on a bot with the talent.
class DivinePurposeTrigger : public HasAuraTrigger
{
public:
    DivinePurposeTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "divine purpose") {}

    bool IsActive() override;
};

BUFF_TRIGGER(RetributionAuraTrigger, "retribution aura");
BUFF_TRIGGER(SanctityAuraTrigger, "sanctity aura");

// #357 slice 6 (AC PaladinTriggers.h:107-109): same BuffTrigger shape as AC, but AC's IsActive() needs
// the "mounted" value, so the body lives in the .cpp (AI_VALUE2 cannot bind from a header TU here).
class CrusaderAuraTrigger : public BuffTrigger
{
public:
    CrusaderAuraTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "crusader aura") {}

    bool IsActive() override;
};

// #357 slice 6 (AC PaladinTriggers.h:111-116). AC bases this on BuffTrigger and walks every seal the bot
// is missing. The fork had HasAuraTrigger(botAI, "seal", 5.0f), which asks for an aura literally named
// "seal" - no such aura exists in any client, so the old trigger could never be active. AC's shape is the
// port. Every spell AC tests still exists in 5.4.8 (Seal of Justice/Light/Wisdom on all three specs,
// Seal of Command on Guardian, Seal of Vengeance/Corruption on Ret, Seal of Righteousness on all).
class SealTrigger : public BuffTrigger
{
public:
    SealTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "seal") {}

    bool IsActive() override;
};

// judgements
DEBUFF_TRIGGER(JudgementTrigger, "judgement");
DEBUFF_TRIGGER(JudgementOfLightTrigger, "judgement of light");
// 5.4.8 has no "judgement of wisdom" (Cataclysm renamed it Judgement of the Wise), so this registration
// can never fire. Kept because AC registers it and the typed surface must match; MoP uses "judgement of
// the wise" if the judgement surface is ever ported.
DEBUFF_TRIGGER(JudgementOfWisdomTrigger, "judgement of wisdom");

DEBUFF_TRIGGER(ConsecrationTrigger, "consecration");

// repentance triggers
INTERRUPT_HEALER_TRIGGER(RepentanceOnHealerTrigger, "repentance on enemy healer");
SNARE_TRIGGER(RepentanceSnareTrigger, "repentance on snare target");
INTERRUPT_TRIGGER(RepentanceInterruptTrigger, "repentance");

// #357 slice 6 (AC PaladinTriggers.h:63-67): AC builds this as a comma-separated spell list on a 4s check
// interval, not BUFF_PARTY_TRIGGER's 1s, and the two registered subclasses only override the spell list
// in their ctor bodies. 5.4.8 dropped Blessing of Wisdom and Blessing of Sanctuary (Cataclysm), so the
// list is the two that remain - same adaptation BlessingTrigger below already documents.
class BlessingOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    BlessingOnPartyTrigger(PlayerbotAI* botAI)
        : BuffOnPartyTrigger(botAI, "blessing of kings,blessing of might", 2 * 2000) {}
};

// AC PaladinTriggers.h:221-237. AC also has BlessingOfWisdomOnPartyTrigger and
// BlessingOfSanctuaryOnPartyTrigger here; both are 5.4.8 N/A spells and the fork has no action for
// either, so they are deliberately not ported (see the slice report).
class BlessingOfKingsOnPartyTrigger : public BlessingOnPartyTrigger
{
public:
    BlessingOfKingsOnPartyTrigger(PlayerbotAI* botAI)
        : BlessingOnPartyTrigger(botAI)
    {
        spell = "blessing of kings";
    }
};

class BlessingOfMightOnPartyTrigger : public BlessingOnPartyTrigger
{
public:
    BlessingOfMightOnPartyTrigger(PlayerbotAI* botAI)
        : BlessingOnPartyTrigger(botAI)
    {
        spell = "blessing of might";
    }
};

// self missing a blessing. MoP paladins only have Blessing of Kings/Might (Wisdom/Sanctuary are
// 5.4.8 N/A, see PaladinActions.h), so this only checks those two instead of AC's four.
class BlessingTrigger : public BuffTrigger
{
public:
    BlessingTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "blessing of kings", 2 * 2000) {}
    bool IsActive() override;
};

INTERRUPT_TRIGGER(HammerOfJusticeInterruptTrigger, "hammer of justice");

// #357 slice 6 (AC PaladinTriggers.h:152-156)
class HammerOfJusticeEnemyHealerTrigger : public InterruptEnemyHealerTrigger
{
public:
    HammerOfJusticeEnemyHealerTrigger(PlayerbotAI* botAI)
        : InterruptEnemyHealerTrigger(botAI, "hammer of justice") {}
};

// #357 slice 6 (AC PaladinTriggers.h:157-160)
class HammerOfJusticeSnareTrigger : public SnareTargetTrigger
{
public:
    HammerOfJusticeSnareTrigger(PlayerbotAI* botAI) : SnareTargetTrigger(botAI, "hammer of justice") {}
};

// #357 slice 6 (AC PaladinTriggers.h:80-84)
class ArtOfWarTrigger : public HasAuraTrigger
{
public:
    ArtOfWarTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "the art of war") {}
};

// #357 slice 6 (AC PaladinTriggers.h:162-174)
class ShadowResistanceAuraTrigger : public BuffTrigger
{
public:
    ShadowResistanceAuraTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "shadow resistance aura") {}
};

class FrostResistanceAuraTrigger : public BuffTrigger
{
public:
    FrostResistanceAuraTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "frost resistance aura") {}
};

class FireResistanceAuraTrigger : public BuffTrigger
{
public:
    FireResistanceAuraTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "fire resistance aura") {}
};

class DevotionAuraTrigger : public BuffTrigger
{
public:
    DevotionAuraTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "devotion aura") {}
};

BUFF_TRIGGER(ConcentrationAuraTrigger, "concentration aura");

// #149 (AC PaladinAiObjectContext.cpp:127,129,131): self-cast cleanse, mirroring the "cleanse party
// member cure X" triggers below but on the generic (self-target) NeedCureTrigger base instead of
// PartyMemberNeedCureTrigger. (Registered names are AC's; the class names are the fork's, from #149.)
class CleanseCureDiseaseTrigger : public NeedCureTrigger
{
public:
    CleanseCureDiseaseTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse", DISPEL_DISEASE) {}
};

class CleanseCurePoisonTrigger : public NeedCureTrigger
{
public:
    CleanseCurePoisonTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse", DISPEL_POISON) {}
};

class CleanseCureMagicTrigger : public NeedCureTrigger
{
public:
    CleanseCureMagicTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse", DISPEL_MAGIC) {}
};

class CleansePartyMemberCureDiseaseTrigger : public PartyMemberNeedCureTrigger
{
public:
    CleansePartyMemberCureDiseaseTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "cleanse", DISPEL_DISEASE) {}
};

class CleansePartyMemberCurePoisonTrigger : public PartyMemberNeedCureTrigger
{
public:
    CleansePartyMemberCurePoisonTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "cleanse", DISPEL_POISON) {}
};

class CleansePartyMemberCureMagicTrigger : public PartyMemberNeedCureTrigger
{
public:
    CleansePartyMemberCureMagicTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "cleanse", DISPEL_MAGIC) {}
};

// #357 slice 6 (AC PaladinTriggers.h:177-181)
class DivineFavorTrigger : public BuffTrigger
{
public:
    DivineFavorTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "divine favor") {}
};

// #357 slice 6 (AC PaladinTriggers.h:198-205). "health" is an AI_VALUE2 lookup, so the body is in the .cpp.
class DivineShieldLowHealthTrigger : public Trigger
{
public:
    DivineShieldLowHealthTrigger(PlayerbotAI* botAI) : Trigger(botAI, "divine shield low health") {}

    bool IsActive() override;
};

// #357 slice 6 (AC PaladinTriggers.h:207-214)
class NotSensingUndeadTrigger : public BuffTrigger
{
public:
    NotSensingUndeadTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "not sensing undead") {}

    bool IsActive() override;
};

// #357 slice 6 (AC PaladinTriggers.h:216-220)
class TurnUndeadTrigger : public HasCcTargetTrigger
{
public:
    // #383: HasCcTargetTrigger passes getName() to the "cc target" value as the spell to cast, so the
    // name must be the 5.4.8 spell. Turn Undead became Turn Evil (10326) in patch 4.0.1 (cls.py
    // 2026-10-04). The registered trigger key stays "turn undead".
    TurnUndeadTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "turn evil") {}
};

// #357 slice 6 (AC PaladinTriggers.h:127)
DEBUFF_TRIGGER(AvengerShieldTrigger, "avenger's shield");

// #357 slice 6 (AC PaladinTriggers.h:129-145)
class BeaconOfLightOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    BeaconOfLightOnMainTankTrigger(PlayerbotAI* botAI)
        : BuffOnMainTankTrigger(botAI, "beacon of light", true) {}
};

class SacredShieldOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    SacredShieldOnMainTankTrigger(PlayerbotAI* botAI)
        : BuffOnMainTankTrigger(botAI, "sacred shield", false) {}
};

// #163 S8 (AC parity, PaladinTriggers.h:241-246): unlike BUFF_PARTY_TRIGGER's "lacks a permanent
// buff" shape, this targets whoever "party member snared target" (ValueContext.h) currently picks -
// the value itself already does the distance/LOS/self-fallback work AC splits between the trigger
// and the action (AC PaladinTriggers.cpp:42-73, which additionally needs the ai::paladin:: caster
// helpers that do not exist on this fork - see the slice report).
class HandOfFreedomOnPartyTrigger : public Trigger
{
public:
    HandOfFreedomOnPartyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hand of freedom on party", 1) {}

    std::string const GetTargetName() override { return "party member snared target"; }
    bool IsActive() override;
};

// #357 slice 6 (AC PaladinTriggers.h:248-252)
class AvengingWrathTrigger : public BoostTrigger
{
public:
    AvengingWrathTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "avenging wrath") {}
};


// ---- rotation lane 6 (#861) ----

// Rebuke on the current target (the "on enemy caster" row uses the lane 0 trigger
// "enemy caster in range").
INTERRUPT_TRIGGER(RebukeInterruptTrigger, "rebuke");

// Self health <= pct (MoP thresholds: Ardent Defender 20, Divine Shield 20, Flash of Light 25, Divine Protection and
// Avenging Wrath 50, Eternal Flame 70). Registered as "paladin health <pct>".
class PaladinSelfHealthTrigger : public Trigger
{
public:
    PaladinSelfHealthTrigger(PlayerbotAI* botAI, std::string const name, uint8 pct) : Trigger(botAI, name), pct(pct) {}

    bool IsActive() override;

private:
    uint8 pct;
};

// Bastion of Glory (114637) at >= 3 stacks (Eternal Flame).
class BastionOfGloryTrigger : public Trigger
{
public:
    BastionOfGloryTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bastion of glory") {}

    bool IsActive() override;
};

// >= 3 group members below 50% health, counted within `radius` yards of a hurt member (Holy Radiance: 10)
// or, for radius >= 40, anywhere within 40 yd of the bot (Divine Favor / GoAK / Avenging Wrath).
class PaladinHurtGroupTrigger : public Trigger
{
public:
    PaladinHurtGroupTrigger(PlayerbotAI* botAI, std::string const name, float radius)
        : Trigger(botAI, name), radius(radius)
    {
    }

    bool IsActive() override;

private:
    float radius;
};

// Cooldowns fired "always in combat" (Ret Holy Avenger, Prot/Ret GoAK). AC pattern: BoostTrigger.
class HolyAvengerTrigger : public BoostTrigger
{
public:
    HolyAvengerTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "holy avenger") {}
};

class GuardianOfAncientKingsTrigger : public BoostTrigger
{
public:
    GuardianOfAncientKingsTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "guardian of ancient kings") {}
};

#endif /*_PLAYERBOT_PALADINTRIGGERS_H*/
