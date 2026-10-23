/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 */

#ifndef _PLAYERBOT_PRIESTTRIGGERS_H
#define _PLAYERBOT_PRIESTTRIGGERS_H

#include "CureTriggers.h"
#include "HealthTriggers.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include <set>

class PlayerbotAI;

// 1:1 port of AC Ai/Class/Priest/PriestTriggers.h (WotLK mod-playerbots). Only the divergences this
// core forces were applied: the base trigger classes and every macro used below already exist with AC
// signatures in strategy/AiObject.h + strategy/triggers/{GenericTriggers,CureTriggers,HealthTriggers}.h, and
// spell-name strings are the same lower-case full spell names this fork already uses in PriestActions.h
// ("power word: fortitude", "shadow protection", "divine spirit", ...), so HasAura()/GetSpellId() resolve.
DEBUFF_CHECKISOWNER_TRIGGER(HolyFireTrigger, "holy fire");
DEBUFF_CHECKISOWNER_TRIGGER(ShadowWordPainTrigger, "shadow word: pain");
DEBUFF_ENEMY_TRIGGER(ShadowWordPainOnAttackerTrigger, "shadow word: pain");
DEBUFF_CHECKISOWNER_TRIGGER(VampiricTouchTrigger, "vampiric touch");
DEBUFF_ENEMY_TRIGGER(VampiricTouchOnAttackerTrigger, "vampiric touch on attacker");
BUFF_TRIGGER(VampiricEmbraceTrigger, "vampiric embrace");
CURE_TRIGGER(DispelMagicTrigger, "dispel magic", DISPEL_MAGIC);
CURE_PARTY_TRIGGER(DispelMagicPartyMemberTrigger, "dispel magic", DISPEL_MAGIC);
CURE_TRIGGER(CureDiseaseTrigger, "cure disease", DISPEL_DISEASE);
// Priests can still cure disease (Abolish Disease / dispel) in MoP; the actual party-heal spell cast is
// "abolish disease on party" (see GenericPriestStrategy.cpp / GenericPriestStrategyActionNodeFactory.h),
// this trigger only tracks whether a party member needs a disease cured, same as AC.
CURE_PARTY_TRIGGER(PartyMemberCureDiseaseTrigger, "cure disease", DISPEL_DISEASE);
BUFF_TRIGGER_A(InnerFireTrigger, "inner fire");
BUFF_TRIGGER_A(ShadowformTrigger, "shadowform");
// #812 MoP-only (no AC counterpart; WotLK has no Chakra): Holy's stance, kept up like Shadow keeps Shadowform
// (AC ShadowPriestStrategy.cpp:30-36). It takes the slot of AC's dead "inner focus" mana edge for Holy
// (AC GenericPriestStrategy.cpp:31-34; Inner Focus 89485 is Discipline-only in 5.4.8, SpecializationSpells spec256).
BUFF_TRIGGER_A(ChakraSerenityTrigger, "chakra: serenity");
// Lane 8 / #863 (MoP rotation row 25715/25723/25724, holy): Chakra: Sanctuary while >= 3 group members within
// 30 yd are below 80% HP, Chakra: Serenity otherwise (ChakraSerenityTrigger above stands down for it).
class ChakraSanctuaryTrigger : public Trigger
{
public:
    ChakraSanctuaryTrigger(PlayerbotAI* botAI) : Trigger(botAI, "chakra: sanctuary") {}

    bool IsActive() override;
};

// Lane E (#863, MoP rotation row 25721, holy, prio 5/1): Lightwell 126135 whenever the bot is in combat, the
// spell is known and off cooldown.
class LightwellCanCastTrigger : public Trigger
{
public:
    LightwellCanCastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lightwell can cast") {}

    bool IsActive() override;
};

// Lane E (#863, MoP rotation row 25715, holy, prio 2/0, combat + out of combat): Chakra: Sanctuary whenever the
// bot does not carry it, which is how MoP re-enters a stance out of combat (non-combat strategy only).
class ChakraSanctuaryMissingTrigger : public Trigger
{
public:
    ChakraSanctuaryMissingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "chakra: sanctuary missing") {}

    bool IsActive() override;
};

// Lane E (#863, MoP rotation row 25802, shadow, prio 0/10): Void Tendrils 108920 on a current target within
// 7 yd. MoP 5.4.8 has no other condition; here it is a player (pvp), or a creature that is attacking the bot.
class VoidTendrilsTargetCloseTrigger : public Trigger
{
public:
    VoidTendrilsTargetCloseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void tendrils target close") {}

    bool IsActive() override;
};

// Lane 8 / #863 (MoP rotation row 25603/25703, disc + holy, group 1/10-1/12): the Surge of Light (114255)
// proc makes Flash Heal free; fires while the bot carries the proc.
class SurgeOfLightProcTrigger : public Trigger
{
public:
    SurgeOfLightProcTrigger(PlayerbotAI* botAI) : Trigger(botAI, "surge of light proc") {}

    bool IsActive() override;
};

// Lane 8 / #863: MoP self-defensive health thresholds (shadow rows 25817 PW:S 60%, 25816 Vampiric Embrace 40%,
// 25819 Renew 30%): the bot is in combat and its own health is at or below `pct`. Registered as "priest self health <n>".
class PriestSelfHealthTrigger : public Trigger
{
public:
    PriestSelfHealthTrigger(PlayerbotAI* botAI, std::string const& name, float pct) : Trigger(botAI, name), pct(pct) {}

    bool IsActive() override;

protected:
    float pct;
};

BOOST_TRIGGER(PowerInfusionTrigger, "power infusion");
BUFF_TRIGGER(InnerFocusTrigger, "inner focus");
CC_TRIGGER(ShackleUndeadTrigger, "shackle undead");
INTERRUPT_TRIGGER(SilenceTrigger, "silence");
INTERRUPT_HEALER_TRIGGER(SilenceEnemyHealerTrigger, "silence");
DEBUFF_CHECKISOWNER_TRIGGER(DevouringPlagueTrigger, "devouring plague");

// #735 (owner 2026-10-08): Shadow Orbs (power type 13) gate Devouring Plague the way AC's combo points gate its
// finishers - AC ComboPointsAvailableTrigger (AC Ai/Base/Trigger/GenericTriggers.h:105-114, .cpp:94: StatAvailable
// + an "amount"), registered per amount like "combo points 3 available" (AC Ai/Base/TriggerContext.h:122,369).
// Only Shadow generates orbs, so this is never active for a holy/disc priest with the "shadow debuff" strategy.
class ShadowOrbsAvailableTrigger : public StatAvailable
{
public:
    ShadowOrbsAvailableTrigger(PlayerbotAI* botAI, int32 amount)
        : StatAvailable(botAI, amount, "shadow orbs available")
    {
    }

    bool IsActive() override;
};
BUFF_TRIGGER(TouchOfWeaknessTrigger, "touch of weakness");
DEBUFF_TRIGGER(HexOfWeaknessTrigger, "hex of weakness");
BUFF_TRIGGER(ShadowguardTrigger, "shadowguard");
DEFLECT_TRIGGER(FeedbackTrigger, "feedback");
SNARE_TRIGGER(ChastiseTrigger, "chastise");
BOOST_TRIGGER_A(ShadowfiendTrigger, "shadowfiend");

class ShadowProtectionTrigger : public BuffTrigger
{
public:
    ShadowProtectionTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "shadow protection", 4 * 2000) {}

    bool IsActive() override;
};

class ShadowProtectionOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    ShadowProtectionOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "shadow protection", 4 * 2000) {}
};

class PowerWordFortitudeOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    PowerWordFortitudeOnPartyTrigger(PlayerbotAI* botAI)
        : BuffOnPartyTrigger(botAI, "power word: fortitude", 4 * 2000) {}
};

class PowerWordFortitudeTrigger : public BuffTrigger
{
public:
    PowerWordFortitudeTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "power word: fortitude", 4 * 2000) {}

    bool IsActive() override;
};

class DivineSpiritOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    DivineSpiritOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "divine spirit", 4 * 2000) {}
};

class DivineSpiritTrigger : public BuffTrigger
{
public:
    DivineSpiritTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "divine spirit", 4 * 2000) {}

    bool IsActive() override;
};

// #524/#521: "being attacked" is the shared BeingAttackedTrigger in strategy/triggers/TriggerContext.h
// (AC TriggerContext.h:127).

class FearWardOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    FearWardOnMainTankTrigger(PlayerbotAI* botAI) : BuffOnMainTankTrigger(botAI, "fear ward", false, 2000) {}

    bool IsActive() override;
};

class BindingHealTrigger : public PartyMemberLowHealthTrigger
{
public:
    BindingHealTrigger(PlayerbotAI* botAI);

    bool IsActive() override;
};

class MindSearChannelCheckTrigger : public Trigger
{
public:
    MindSearChannelCheckTrigger(PlayerbotAI* botAI, uint32 minEnemies = 2)
        : Trigger(botAI, "mind sear channel check"), minEnemies(minEnemies) {}

    bool IsActive() override;

protected:
    uint32 minEnemies;
    static const std::set<uint32> MIND_SEAR_SPELL_IDS;
};

// ---- MoP-only, BEYOND AC PARITY (issue #357 slice 1) --------------------------------------------
// GenericPriestStrategyActionNodeFactory.h references TriggerNode("prayer of fortitude on party") and
// TriggerNode("prayer of spirit on party"); AC (WotLK) has no such spells/triggers (it uses divine spirit /
// power word: fortitude instead) and registers neither name. These two are the same AC BuffOnPartyTrigger
// pattern applied to the MoP Prayer of Fortitude / Prayer of Spirit spells, matching the already-ported
// CastPrayerOfFortitudeOnPartyAction / CastPrayerOfSpiritOnPartyAction in PriestActions.h. They are NOT part
// of the 1:1 AC surface - flagged here so a future AC-parity sweep does not count them as AC parity.
BUFF_PARTY_TRIGGER(PrayerOfFortitudeOnPartyTrigger, "prayer of fortitude");
BUFF_PARTY_TRIGGER(PrayerOfSpiritOnPartyTrigger, "prayer of spirit");

// Deliberately NOT defined/registered here:
//  * "boost" - GenericPriestStrategy.cpp:89 has TriggerNode("boost", ...), but AC registers no trigger under
//    that name either (AC TriggerContext.h only has "generic boost"; AC PriestAiObjectContext.cpp:31
//    registers "boost" as a *Strategy*, not a Trigger). AC has the same dangling node, so inventing a
//    BoostTrigger here would not be AC parity.
//  * "fear ward" - the fork only has it commented out (PriestNonCombatStrategy.cpp:83) and AC registers
//    "fear ward on main tank" (FearWardOnMainTankTrigger, ported above), never "fear ward".

#endif
