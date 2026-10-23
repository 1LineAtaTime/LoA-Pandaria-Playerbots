/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinAiObjectContext.h"

#include "DpsPaladinStrategy.h"
#include "GenericPaladinNonCombatStrategy.h"
#include "PaladinPullStrategy.h"
#include "HealPaladinStrategy.h"
#include "MeleeCombatStrategy.h"
#include "NamedObjectContext.h"
#include "PaladinActions.h"
#include "PaladinBuffStrategies.h"
#include "PaladinTriggers.h"
#include "Playerbots.h"
#include "TankPaladinStrategy.h"

class PaladinStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinStrategyFactoryInternal()
    {
        creators["nc"] = &PaladinStrategyFactoryInternal::nc;
        // #408 (AC PaladinAiObjectContext.cpp:37)
        creators["pull"] = &PaladinStrategyFactoryInternal::pull;
        creators["cure"] = &PaladinStrategyFactoryInternal::cure;
        creators["boost"] = &PaladinStrategyFactoryInternal::boost;
        creators["cc"] = &PaladinStrategyFactoryInternal::cc;
        creators["bthreat"] = &PaladinStrategyFactoryInternal::bthreat;
        creators["healer dps"] = &PaladinStrategyFactoryInternal::healer_dps;
        // #593 (AC Ai/Base/StrategyContext.h:105,139): "behind" (SetBehindCombatStrategy) was never registered, so
        // AiFactory's addStrategy("behind") for retribution was a silent no-op.
        creators["behind"] = &PaladinStrategyFactoryInternal::behind;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericPaladinNonCombatStrategy(botAI); }
    // #408 (AC PaladinAiObjectContext.cpp:37)
    static Strategy* pull(PlayerbotAI* botAI) { return new PaladinPullStrategy(botAI); }
    static Strategy* cure(PlayerbotAI* botAI) { return new PaladinCureStrategy(botAI); }
    static Strategy* boost(PlayerbotAI* botAI) { return new PaladinBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new PaladinCcStrategy(botAI); }
    static Strategy* bthreat(PlayerbotAI* botAI) { return new PaladinBuffThreatStrategy(botAI); }
    static Strategy* healer_dps(PlayerbotAI* botAI) { return new PaladinHealerDpsStrategy(botAI); }
    static Strategy* behind(PlayerbotAI* botAI) { return new SetBehindCombatStrategy(botAI); }  // #593
};

class PaladinResistanceStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinResistanceStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["baoe"] = &PaladinResistanceStrategyFactoryInternal::baoe;
        creators["barmor"] = &PaladinResistanceStrategyFactoryInternal::barmor;
        creators["bcast"] = &PaladinResistanceStrategyFactoryInternal::bcast;
        // #408 (AC PaladinAiObjectContext.cpp:51-52,60-62) - 1:1
        creators["rshadow"] = &PaladinResistanceStrategyFactoryInternal::rshadow;
        creators["rfrost"] = &PaladinResistanceStrategyFactoryInternal::rfrost;
        creators["rfire"] = &PaladinResistanceStrategyFactoryInternal::rfire;
        creators["bspeed"] = &PaladinResistanceStrategyFactoryInternal::bspeed;
    }

private:
    static Strategy* baoe(PlayerbotAI* botAI) { return new PaladinBuffAoeStrategy(botAI); }
    static Strategy* barmor(PlayerbotAI* botAI) { return new PaladinBuffArmorStrategy(botAI); }
    static Strategy* bcast(PlayerbotAI* botAI) { return new PaladinBuffCastStrategy(botAI); }
    static Strategy* rshadow(PlayerbotAI* botAI) { return new PaladinShadowResistanceStrategy(botAI); }
    static Strategy* rfrost(PlayerbotAI* botAI) { return new PaladinFrostResistanceStrategy(botAI); }
    static Strategy* rfire(PlayerbotAI* botAI) { return new PaladinFireResistanceStrategy(botAI); }
    static Strategy* bspeed(PlayerbotAI* botAI) { return new PaladinBuffSpeedStrategy(botAI); }
};

class PaladinBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bhealth"] = &PaladinBuffStrategyFactoryInternal::bhealth;
        creators["bmana"] = &PaladinBuffStrategyFactoryInternal::bmana;
        creators["bdps"] = &PaladinBuffStrategyFactoryInternal::bdps;
        creators["bstats"] = &PaladinBuffStrategyFactoryInternal::bstats;
        // #383 (AC PaladinAiObjectContext.cpp:74-77,81-84 buff keys): AC key aliases onto the 5.x blessings.
        // bmight -> Blessing of Might (19740), bkings -> Blessing of Kings (20217); both exist in 5.4.8
        // (cls.py 2026-10-04). AC "bsanc" (Blessing of Sanctuary, no 5.4.8 spell: cls.py NO-SUCH-NAME) and
        // "bwisdom" (Blessing of Wisdom, NOT-LEARNABLE) are removed spells and are deliberately NOT aliased.
        creators["bmight"] = &PaladinBuffStrategyFactoryInternal::bmight;
        creators["bkings"] = &PaladinBuffStrategyFactoryInternal::bkings;
    }

private:
    static Strategy* bhealth(PlayerbotAI* botAI) { return new PaladinBuffHealthStrategy(botAI); }
    static Strategy* bmana(PlayerbotAI* botAI) { return new PaladinBuffManaStrategy(botAI); }
    static Strategy* bdps(PlayerbotAI* botAI) { return new PaladinBuffDpsStrategy(botAI); }
    static Strategy* bstats(PlayerbotAI* botAI) { return new PaladinBuffStatsStrategy(botAI); }
    static Strategy* bmight(PlayerbotAI* botAI) { return new PaladinBuffMightStrategy(botAI); }
    static Strategy* bkings(PlayerbotAI* botAI) { return new PaladinBuffKingsStrategy(botAI); }
};

class PaladinCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["tank"] = &PaladinCombatStrategyFactoryInternal::tank;
        creators["dps"] = &PaladinCombatStrategyFactoryInternal::dps;
        creators["heal"] = &PaladinCombatStrategyFactoryInternal::heal;
        // #383 (AC PaladinAiObjectContext.cpp:95)
        creators["offheal"] = &PaladinCombatStrategyFactoryInternal::offheal;
    }

private:
    static Strategy* tank(PlayerbotAI* botAI) { return new TankPaladinStrategy(botAI); }
    static Strategy* dps(PlayerbotAI* botAI) { return new DpsPaladinStrategy(botAI); }
    static Strategy* heal(PlayerbotAI* botAI) { return new HealPaladinStrategy(botAI); }
    static Strategy* offheal(PlayerbotAI* botAI) { return new OffhealRetPaladinStrategy(botAI); }
};

class PaladinTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    PaladinTriggerFactoryInternal()
    {
        creators["seal"] = &PaladinTriggerFactoryInternal::seal;
        creators["blessing"] = &PaladinTriggerFactoryInternal::blessing;
        creators["blessing of kings on party"] = &PaladinTriggerFactoryInternal::blessing_of_kings_on_party;
        creators["blessing of might on party"] = &PaladinTriggerFactoryInternal::blessing_of_might_on_party;
        creators["hammer of justice interrupt"] = &PaladinTriggerFactoryInternal::hammer_of_justice_interrupt;
        creators["cleanse cure disease"] = &PaladinTriggerFactoryInternal::cleanse_cure_disease;
        creators["cleanse cure poison"] = &PaladinTriggerFactoryInternal::cleanse_cure_poison;
        creators["cleanse cure magic"] = &PaladinTriggerFactoryInternal::cleanse_cure_magic;
        creators["cleanse party member cure disease"] =
            &PaladinTriggerFactoryInternal::cleanse_party_member_cure_disease;
        creators["cleanse party member cure poison"] =
            &PaladinTriggerFactoryInternal::cleanse_party_member_cure_poison;
        creators["cleanse party member cure magic"] =
            &PaladinTriggerFactoryInternal::cleanse_party_member_cure_magic;

        // #163 S8 (AC parity)
        creators["hand of freedom on party"] = &PaladinTriggerFactoryInternal::hand_of_freedom_on_party;

        // #357 slice 6 (AC PaladinAiObjectContext.cpp:113-190): AC's remaining 33 Paladin trigger
        // registrations, verbatim names and factory shapes. Three AC names are deliberately absent -
        // "blessing of wisdom on party" and "blessing of sanctuary on party" (Blessing of Wisdom and
        // Blessing of Sanctuary were removed in Cataclysm and have no action here either) and
        // "greater blessing needed" (needs the ai::gbless value/action subsystem, which does not exist
        // on this fork and is WotLK-only: 5.4.8 has no Greater Blessing spells at all).
        creators["judgement"] = &PaladinTriggerFactoryInternal::judgement;
        creators["judgement of light"] = &PaladinTriggerFactoryInternal::judgement_of_light;
        creators["judgement of wisdom"] = &PaladinTriggerFactoryInternal::judgement_of_wisdom;
        creators["art of war"] = &PaladinTriggerFactoryInternal::art_of_war;
        creators["blessing on party"] = &PaladinTriggerFactoryInternal::blessing_on_party;
        creators["crusader aura"] = &PaladinTriggerFactoryInternal::crusader_aura;
        creators["retribution aura"] = &PaladinTriggerFactoryInternal::retribution_aura;
        creators["devotion aura"] = &PaladinTriggerFactoryInternal::devotion_aura;
        creators["sanctity aura"] = &PaladinTriggerFactoryInternal::sanctity_aura;
        creators["concentration aura"] = &PaladinTriggerFactoryInternal::concentration_aura;
        creators["shadow resistance aura"] = &PaladinTriggerFactoryInternal::shadow_resistance_aura;
        creators["frost resistance aura"] = &PaladinTriggerFactoryInternal::frost_resistance_aura;
        creators["fire resistance aura"] = &PaladinTriggerFactoryInternal::fire_resistance_aura;
        creators["hammer of justice snare"] = &PaladinTriggerFactoryInternal::hammer_of_justice_snare;
        creators["righteous fury"] = &PaladinTriggerFactoryInternal::righteous_fury;
        creators["holy shield"] = &PaladinTriggerFactoryInternal::holy_shield;
        creators["hammer of justice on enemy healer"] =
            &PaladinTriggerFactoryInternal::hammer_of_justice_on_enemy_healer;
        creators["hammer of justice on snare target"] =
            &PaladinTriggerFactoryInternal::hammer_of_justice_on_snare_target;
        creators["not sensing undead"] = &PaladinTriggerFactoryInternal::not_sensing_undead;
        creators["divine favor"] = &PaladinTriggerFactoryInternal::divine_favor;
        creators["divine shield low health"] = &PaladinTriggerFactoryInternal::divine_shield_low_health;
        creators["turn undead"] = &PaladinTriggerFactoryInternal::turn_undead;
        creators["avenger's shield"] = &PaladinTriggerFactoryInternal::avengers_shield;
        creators["consecration"] = &PaladinTriggerFactoryInternal::consecration;
        creators["repentance on enemy healer"] = &PaladinTriggerFactoryInternal::repentance_on_enemy_healer;
        creators["repentance on snare target"] = &PaladinTriggerFactoryInternal::repentance_on_snare_target;
        creators["repentance interrupt"] = &PaladinTriggerFactoryInternal::repentance_interrupt;
        creators["beacon of light on main tank"] = &PaladinTriggerFactoryInternal::beacon_of_light_on_main_tank;
        creators["sacred shield on main tank"] = &PaladinTriggerFactoryInternal::sacred_shield_on_main_tank;
        creators["avenging wrath"] = &PaladinTriggerFactoryInternal::avenging_wrath;
        // #525: Inquisition upkeep (5.x Ret, no AC equivalent)
        creators["inquisition"] = &PaladinTriggerFactoryInternal::inquisition;
        // #735: the Holy Power spender gates, AC's "combo points N available" pattern (AC TriggerContext.h:120-125,
        // 367-375 incl. its TwoTriggers "combo points not full and high energy").
        creators["holy power 3 available"] = &PaladinTriggerFactoryInternal::holy_power_3_available;
        creators["holy power 1 available"] = &PaladinTriggerFactoryInternal::holy_power_1_available;
        creators["divine purpose"] = &PaladinTriggerFactoryInternal::divine_purpose;
        creators["inquisition and holy power 1 available"] =
            &PaladinTriggerFactoryInternal::inquisition_and_holy_power_1_available;
        creators["party member low health and holy power 1 available"] =
            &PaladinTriggerFactoryInternal::party_member_low_health_and_holy_power_1_available;
        creators["party member critical health and holy power 1 available"] =
            &PaladinTriggerFactoryInternal::party_member_critical_health_and_holy_power_1_available;
        // rotation lane 6 (#861): MoP paladin rotations
        creators["rebuke interrupt"] = &PaladinTriggerFactoryInternal::rebuke_interrupt;
        creators["paladin health 20"] = &PaladinTriggerFactoryInternal::paladin_health_20;
        creators["paladin health 25"] = &PaladinTriggerFactoryInternal::paladin_health_25;
        creators["paladin health 50"] = &PaladinTriggerFactoryInternal::paladin_health_50;
        creators["paladin health 70"] = &PaladinTriggerFactoryInternal::paladin_health_70;
        creators["bastion of glory"] = &PaladinTriggerFactoryInternal::bastion_of_glory;
        creators["paladin 3 hurt group"] = &PaladinTriggerFactoryInternal::paladin_3_hurt_group;
        creators["paladin holy radiance group"] = &PaladinTriggerFactoryInternal::paladin_holy_radiance_group;
        creators["holy avenger"] = &PaladinTriggerFactoryInternal::holy_avenger;
        creators["guardian of ancient kings"] = &PaladinTriggerFactoryInternal::guardian_of_ancient_kings;
    }
private:
    static Trigger* seal(PlayerbotAI* botAI) { return new SealTrigger(botAI); }
    static Trigger* blessing(PlayerbotAI* botAI) { return new BlessingTrigger(botAI); }
    static Trigger* blessing_on_party(PlayerbotAI* botAI) { return new BlessingOnPartyTrigger(botAI); }
    static Trigger* judgement(PlayerbotAI* botAI) { return new JudgementTrigger(botAI); }
    static Trigger* judgement_of_light(PlayerbotAI* botAI) { return new JudgementOfLightTrigger(botAI); }
    static Trigger* judgement_of_wisdom(PlayerbotAI* botAI) { return new JudgementOfWisdomTrigger(botAI); }
    static Trigger* art_of_war(PlayerbotAI* botAI) { return new ArtOfWarTrigger(botAI); }
    static Trigger* crusader_aura(PlayerbotAI* botAI) { return new CrusaderAuraTrigger(botAI); }
    static Trigger* retribution_aura(PlayerbotAI* botAI) { return new RetributionAuraTrigger(botAI); }
    static Trigger* devotion_aura(PlayerbotAI* botAI) { return new DevotionAuraTrigger(botAI); }
    static Trigger* sanctity_aura(PlayerbotAI* botAI) { return new SanctityAuraTrigger(botAI); }
    static Trigger* concentration_aura(PlayerbotAI* botAI) { return new ConcentrationAuraTrigger(botAI); }
    static Trigger* shadow_resistance_aura(PlayerbotAI* botAI) { return new ShadowResistanceAuraTrigger(botAI); }
    static Trigger* frost_resistance_aura(PlayerbotAI* botAI) { return new FrostResistanceAuraTrigger(botAI); }
    static Trigger* fire_resistance_aura(PlayerbotAI* botAI) { return new FireResistanceAuraTrigger(botAI); }
    static Trigger* hammer_of_justice_snare(PlayerbotAI* botAI) { return new HammerOfJusticeSnareTrigger(botAI); }
    static Trigger* righteous_fury(PlayerbotAI* botAI) { return new RighteousFuryTrigger(botAI); }
    static Trigger* holy_shield(PlayerbotAI* botAI) { return new HolyShieldTrigger(botAI); }
    static Trigger* hammer_of_justice_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new HammerOfJusticeEnemyHealerTrigger(botAI);
    }
    static Trigger* hammer_of_justice_on_snare_target(PlayerbotAI* botAI)
    {
        return new HammerOfJusticeSnareTrigger(botAI);
    }
    static Trigger* not_sensing_undead(PlayerbotAI* botAI) { return new NotSensingUndeadTrigger(botAI); }
    static Trigger* divine_favor(PlayerbotAI* botAI) { return new DivineFavorTrigger(botAI); }
    static Trigger* divine_shield_low_health(PlayerbotAI* botAI) { return new DivineShieldLowHealthTrigger(botAI); }
    static Trigger* turn_undead(PlayerbotAI* botAI) { return new TurnUndeadTrigger(botAI); }
    static Trigger* avengers_shield(PlayerbotAI* botAI) { return new AvengerShieldTrigger(botAI); }
    static Trigger* consecration(PlayerbotAI* botAI) { return new ConsecrationTrigger(botAI); }
    static Trigger* repentance_on_enemy_healer(PlayerbotAI* botAI) { return new RepentanceOnHealerTrigger(botAI); }
    static Trigger* repentance_on_snare_target(PlayerbotAI* botAI) { return new RepentanceSnareTrigger(botAI); }
    static Trigger* repentance_interrupt(PlayerbotAI* botAI) { return new RepentanceInterruptTrigger(botAI); }
    static Trigger* beacon_of_light_on_main_tank(PlayerbotAI* botAI)
    {
        return new BeaconOfLightOnMainTankTrigger(botAI);
    }
    static Trigger* sacred_shield_on_main_tank(PlayerbotAI* botAI) { return new SacredShieldOnMainTankTrigger(botAI); }
    static Trigger* avenging_wrath(PlayerbotAI* botAI) { return new AvengingWrathTrigger(botAI); }
    static Trigger* inquisition(PlayerbotAI* botAI) { return new InquisitionTrigger(botAI); }
    static Trigger* holy_power_3_available(PlayerbotAI* botAI) { return new HolyPowerAvailableTrigger(botAI, 3); }
    static Trigger* holy_power_1_available(PlayerbotAI* botAI) { return new HolyPowerAvailableTrigger(botAI, 1); }
    static Trigger* divine_purpose(PlayerbotAI* botAI) { return new DivinePurposeTrigger(botAI); }
    static Trigger* inquisition_and_holy_power_1_available(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "inquisition", "holy power 1 available");
    }
    static Trigger* party_member_low_health_and_holy_power_1_available(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "party member low health", "holy power 1 available");
    }
    static Trigger* party_member_critical_health_and_holy_power_1_available(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "party member critical health", "holy power 1 available");
    }
    static Trigger* rebuke_interrupt(PlayerbotAI* botAI) { return new RebukeInterruptTrigger(botAI); }
    static Trigger* paladin_health_20(PlayerbotAI* botAI) { return new PaladinSelfHealthTrigger(botAI, "paladin health 20", 20); }
    static Trigger* paladin_health_25(PlayerbotAI* botAI) { return new PaladinSelfHealthTrigger(botAI, "paladin health 25", 25); }
    static Trigger* paladin_health_50(PlayerbotAI* botAI) { return new PaladinSelfHealthTrigger(botAI, "paladin health 50", 50); }
    static Trigger* paladin_health_70(PlayerbotAI* botAI) { return new PaladinSelfHealthTrigger(botAI, "paladin health 70", 70); }
    static Trigger* bastion_of_glory(PlayerbotAI* botAI) { return new BastionOfGloryTrigger(botAI); }
    static Trigger* paladin_3_hurt_group(PlayerbotAI* botAI)
    {
        return new PaladinHurtGroupTrigger(botAI, "paladin 3 hurt group", 40.0f);
    }
    static Trigger* paladin_holy_radiance_group(PlayerbotAI* botAI)
    {
        return new PaladinHurtGroupTrigger(botAI, "paladin holy radiance group", 10.0f);
    }
    static Trigger* holy_avenger(PlayerbotAI* botAI) { return new HolyAvengerTrigger(botAI); }
    static Trigger* guardian_of_ancient_kings(PlayerbotAI* botAI) { return new GuardianOfAncientKingsTrigger(botAI); }
    static Trigger* blessing_of_kings_on_party(PlayerbotAI* botAI) { return new BlessingOfKingsOnPartyTrigger(botAI); }
    static Trigger* blessing_of_might_on_party(PlayerbotAI* botAI) { return new BlessingOfMightOnPartyTrigger(botAI); }
    static Trigger* hammer_of_justice_interrupt(PlayerbotAI* botAI)
    {
        return new HammerOfJusticeInterruptTrigger(botAI);
    }
    static Trigger* cleanse_cure_disease(PlayerbotAI* botAI) { return new CleanseCureDiseaseTrigger(botAI); }
    static Trigger* cleanse_cure_poison(PlayerbotAI* botAI) { return new CleanseCurePoisonTrigger(botAI); }
    static Trigger* cleanse_cure_magic(PlayerbotAI* botAI) { return new CleanseCureMagicTrigger(botAI); }
    static Trigger* cleanse_party_member_cure_disease(PlayerbotAI* botAI)
    {
        return new CleansePartyMemberCureDiseaseTrigger(botAI);
    }
    static Trigger* cleanse_party_member_cure_poison(PlayerbotAI* botAI)
    {
        return new CleansePartyMemberCurePoisonTrigger(botAI);
    }
    static Trigger* cleanse_party_member_cure_magic(PlayerbotAI* botAI)
    {
        return new CleansePartyMemberCureMagicTrigger(botAI);
    }
    static Trigger* hand_of_freedom_on_party(PlayerbotAI* botAI) { return new HandOfFreedomOnPartyTrigger(botAI); }
};

class PaladinAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    PaladinAiObjectContextInternal()
    {
        creators["seal of command"] = &PaladinAiObjectContextInternal::seal_of_command;
        creators["seal of insight"] = &PaladinAiObjectContextInternal::seal_of_insight;
        creators["seal of righteousness"] = &PaladinAiObjectContextInternal::seal_of_righteousness;

        // AC PaladinAiObjectContext.cpp:258,273-274 + 297-299 - seals/auras missing from this fork.
        creators["seal of light"] = &PaladinAiObjectContextInternal::seal_of_light;
        creators["seal of wisdom"] = &PaladinAiObjectContextInternal::seal_of_wisdom;
        creators["seal of justice"] = &PaladinAiObjectContextInternal::seal_of_justice;
        creators["seal of corruption"] = &PaladinAiObjectContextInternal::seal_of_corruption;
        creators["shadow resistance aura"] = &PaladinAiObjectContextInternal::shadow_resistance_aura;
        creators["frost resistance aura"] = &PaladinAiObjectContextInternal::frost_resistance_aura;
        creators["fire resistance aura"] = &PaladinAiObjectContextInternal::fire_resistance_aura;
        creators["blessing of might"] = &PaladinAiObjectContextInternal::blessing_of_might;
        creators["blessing of kings"] = &PaladinAiObjectContextInternal::blessing_of_kings;
        creators["blessing of kings on party"] = &PaladinAiObjectContextInternal::blessing_of_kings_on_party;
        creators["blessing of might on party"] = &PaladinAiObjectContextInternal::blessing_of_might_on_party;
        // 5.4.8: MoP paladins only have Blessing of Kings/Might; Blessing of Wisdom/Sanctuary and the
        // WotLK "blessing of protection on party" rotation model do not have new classes here, but the
        // already-ported reactive CastBlessingOfProtectionProtectAction (PaladinActions.h) IS the MoP
        // equivalent (protect a party member that needs it, same pattern as e.g. priest's "pain
        // suppression on party"), so it gets wired up below.
        creators["blessing of protection on party"] = &PaladinAiObjectContextInternal::blessing_of_protection_on_party;
        // #163 S8 (AC parity)
        creators["hand of freedom on party"] = &PaladinAiObjectContextInternal::hand_of_freedom_on_party;

        creators["divine storm"] = &PaladinAiObjectContextInternal::divine_storm;
        creators["redemption"] = &PaladinAiObjectContextInternal::redemption;
        creators["crusader strike"] = &PaladinAiObjectContextInternal::crusader_strike;
        creators["crusader aura"] = &PaladinAiObjectContextInternal::crusader_aura;
        creators["devotion aura"] = &PaladinAiObjectContextInternal::devotion_aura;
        creators["concentration aura"] = &PaladinAiObjectContextInternal::concentration_aura;
        creators["holy wrath"] = &PaladinAiObjectContextInternal::holy_wrath;
        creators["consecration"] = &PaladinAiObjectContextInternal::consecration;
        creators["cleanse disease"] = &PaladinAiObjectContextInternal::cleanse_disease;
        creators["cleanse poison"] = &PaladinAiObjectContextInternal::cleanse_poison;
        creators["cleanse magic"] = &PaladinAiObjectContextInternal::cleanse_magic;
        creators["purify disease"] = &PaladinAiObjectContextInternal::purify_disease;
        creators["purify poison"] = &PaladinAiObjectContextInternal::purify_poison;
        creators["cleanse poison on party"] = &PaladinAiObjectContextInternal::cleanse_poison_on_party;
        creators["cleanse disease on party"] = &PaladinAiObjectContextInternal::cleanse_disease_on_party;
        creators["cleanse magic on party"] = &PaladinAiObjectContextInternal::cleanse_magic_on_party;
        creators["purify poison on party"] = &PaladinAiObjectContextInternal::purify_poison_on_party;
        creators["purify disease on party"] = &PaladinAiObjectContextInternal::purify_disease_on_party;
        
        creators["flash of light"] = &PaladinAiObjectContextInternal::flash_of_light;
        creators["hand of reckoning"] = &PaladinAiObjectContextInternal::hand_of_reckoning;
        creators["avenger's shield"] = &PaladinAiObjectContextInternal::avengers_shield;
        creators["exorcism"] = &PaladinAiObjectContextInternal::exorcism;
        creators["judgement"] = &PaladinAiObjectContextInternal::judgement;
        creators["divine shield"] = &PaladinAiObjectContextInternal::divine_shield;
        creators["divine protection"] = &PaladinAiObjectContextInternal::divine_protection;
        creators["divine protection on party"] = &PaladinAiObjectContextInternal::divine_protection_on_party;
        creators["hammer of justice"] = &PaladinAiObjectContextInternal::hammer_of_justice;
        creators["flash of light on party"] = &PaladinAiObjectContextInternal::flash_of_light_on_party;
        creators["word of glory on party"] = &PaladinAiObjectContextInternal::word_of_glory_on_party;  // #521
        creators["holy light"] = &PaladinAiObjectContextInternal::holy_light;
        creators["holy light on party"] = &PaladinAiObjectContextInternal::holy_light_on_party;
        creators["lay on hands"] = &PaladinAiObjectContextInternal::lay_on_hands;
        creators["lay on hands on party"] = &PaladinAiObjectContextInternal::lay_on_hands_on_party;
        creators["hammer of wrath"] = &PaladinAiObjectContextInternal::hammer_of_wrath;
        creators["holy shield"] = &PaladinAiObjectContextInternal::holy_shield;
        creators["hammer of the righteous"] = &PaladinAiObjectContextInternal::hammer_of_the_righteous;
        creators["retribution aura"] = &PaladinAiObjectContextInternal::retribution_aura;
        creators["righteous fury"] = &PaladinAiObjectContextInternal::righteous_fury;
        creators["hammer of justice on enemy healer"] =
            &PaladinAiObjectContextInternal::hammer_of_justice_on_enemy_healer;
        creators["hammer of justice on snare target"] =
            &PaladinAiObjectContextInternal::hammer_of_justice_on_snare_target;
        creators["divine favor"] = &PaladinAiObjectContextInternal::divine_favor;
        creators["turn undead"] = &PaladinAiObjectContextInternal::turn_undead;
        creators["sense undead"] = &PaladinAiObjectContextInternal::sense_undead;
        creators["righteous defense"] = &PaladinAiObjectContextInternal::righteous_defense;
        creators["repentance"] = &PaladinAiObjectContextInternal::repentance;
        creators["repentance on snare target"] = &PaladinAiObjectContextInternal::repentance_on_snare_target;
        creators["repentance on enemy healer"] = &PaladinAiObjectContextInternal::repentance_on_enemy_healer;
        creators["sanctity aura"] = &PaladinAiObjectContextInternal::sanctity_aura;
        creators["holy shock"] = &PaladinAiObjectContextInternal::holy_shock;
        creators["holy shock on party"] = &PaladinAiObjectContextInternal::holy_shock_on_party;
        creators["divine plea"] = &PaladinAiObjectContextInternal::divine_plea;
        creators["shield of righteousness"] = &PaladinAiObjectContextInternal::shield_of_righteousness;
        creators["beacon of light on main tank"] = &PaladinAiObjectContextInternal::beacon_of_light_on_main_tank;
        creators["sacred shield on main tank"] = &PaladinAiObjectContextInternal::sacred_shield_on_main_tank;
        creators["avenging wrath"] = &PaladinAiObjectContextInternal::avenging_wrath;
        creators["divine illumination"] = &PaladinAiObjectContextInternal::divine_illumination;
        creators["divine sacrifice"] = &PaladinAiObjectContextInternal::divine_sacrifice;
        creators["cancel divine sacrifice"] = &PaladinAiObjectContextInternal::cancel_divine_sacrifice;
        // #525: Ret 5.x actions (Seal of Truth replaces AC's Seal of Corruption; Holy Power spenders)
        creators["seal of truth"] = &PaladinAiObjectContextInternal::seal_of_truth;
        creators["templar's verdict"] = &PaladinAiObjectContextInternal::templars_verdict;
        creators["inquisition"] = &PaladinAiObjectContextInternal::inquisition;
        // rotation lane 6 (#861): MoP paladin rotations
        creators["rebuke"] = &PaladinAiObjectContextInternal::rebuke;
        creators["rebuke on enemy caster"] = &PaladinAiObjectContextInternal::rebuke_on_enemy_caster;
        creators["holy avenger"] = &PaladinAiObjectContextInternal::holy_avenger;
        creators["guardian of ancient kings"] = &PaladinAiObjectContextInternal::guardian_of_ancient_kings;
        creators["ardent defender"] = &PaladinAiObjectContextInternal::ardent_defender;
        creators["execution sentence"] = &PaladinAiObjectContextInternal::execution_sentence;
        creators["execution sentence on party"] = &PaladinAiObjectContextInternal::execution_sentence_on_party;
        creators["eternal flame"] = &PaladinAiObjectContextInternal::eternal_flame;
        creators["eternal flame on party"] = &PaladinAiObjectContextInternal::eternal_flame_on_party;
        creators["divine light on party"] = &PaladinAiObjectContextInternal::divine_light_on_party;
        creators["holy radiance on party"] = &PaladinAiObjectContextInternal::holy_radiance_on_party;
    }

private:
    static Action* seal_of_command(PlayerbotAI* botAI) { return new CastSealOfCommandAction(botAI); }
    static Action* seal_of_righteousness(PlayerbotAI* botAI) { return new CastSealOfRighteousnessAction(botAI); }
    static Action* seal_of_insight(PlayerbotAI* botAI) { return new CastSealOfInsightAction(botAI); }

    // #408 (AC PaladinAiObjectContext.cpp:340,365,380-382,407-409) - 1:1. AC's "seal of vengeance"
    // (AC PaladinAiObjectContext.cpp:244/339) is not registered: renamed to Seal of Corruption in
    // patch 3.1, so no 5.4.8 spell carries the old name.
    static Action* seal_of_corruption(PlayerbotAI* botAI) { return new CastSealOfCorruptionAction(botAI); }
    static Action* seal_of_light(PlayerbotAI* botAI) { return new CastSealOfLightAction(botAI); }
    static Action* seal_of_wisdom(PlayerbotAI* botAI) { return new CastSealOfWisdomAction(botAI); }
    static Action* seal_of_justice(PlayerbotAI* botAI) { return new CastSealOfJusticeAction(botAI); }
    static Action* shadow_resistance_aura(PlayerbotAI* botAI) { return new CastShadowResistanceAuraAction(botAI); }
    static Action* frost_resistance_aura(PlayerbotAI* botAI) { return new CastFrostResistanceAuraAction(botAI); }
    static Action* fire_resistance_aura(PlayerbotAI* botAI) { return new CastFireResistanceAuraAction(botAI); }
    static Action* blessing_of_might(PlayerbotAI* botAI) { return new CastBlessingOfMightAction(botAI); }
    static Action* blessing_of_kings(PlayerbotAI* botAI) { return new CastBlessingOfKingsAction(botAI); }
    static Action* blessing_of_kings_on_party(PlayerbotAI* botAI) { return new CastBlessingOfKingsOnPartyAction(botAI); }
    static Action* blessing_of_might_on_party(PlayerbotAI* botAI) { return new CastBlessingOfMightOnPartyAction(botAI); }
    static Action* blessing_of_protection_on_party(PlayerbotAI* botAI)
    {
        return new CastBlessingOfProtectionProtectAction(botAI);
    }
    static Action* hand_of_freedom_on_party(PlayerbotAI* botAI) { return new CastHandOfFreedomOnPartyAction(botAI); }

    static Action* turn_undead(PlayerbotAI* botAI) { return new CastTurnUndeadAction(botAI); }
    // #357 slice 6 (AC PaladinAiObjectContext.cpp:306, AC PaladinActions.h:352) - 1:1. Reached only via
    // AC's TriggerNode("not sensing undead", { NextAction("sense undead", ...) }) in
    // GenericPaladinNonCombatStrategy.cpp, which the fork has not ported (see the slice report).
    // Sense Undead was removed in patch 4.0.1 (no such spell in 5.4.8), so this action never casts (#424).
    static Action* sense_undead(PlayerbotAI* botAI) { return new CastSenseUndeadAction(botAI); }
    static Action* divine_favor(PlayerbotAI* botAI) { return new CastDivineFavorAction(botAI); }
    static Action* righteous_fury(PlayerbotAI* botAI) { return new CastRighteousFuryAction(botAI); }
    static Action* divine_storm(PlayerbotAI* botAI) { return new CastDivineStormAction(botAI); }
    static Action* redemption(PlayerbotAI* botAI) { return new CastRedemptionAction(botAI); }
    static Action* crusader_strike(PlayerbotAI* botAI) { return new CastCrusaderStrikeAction(botAI); }
    static Action* crusader_aura(PlayerbotAI* botAI) { return new CastCrusaderAuraAction(botAI); }
    static Action* devotion_aura(PlayerbotAI* botAI) { return new CastDevotionAuraAction(botAI); }
    static Action* concentration_aura(PlayerbotAI* botAI) { return new CastConcentrationAuraAction(botAI); }
    static Action* holy_wrath(PlayerbotAI* botAI) { return new CastHolyWrathAction(botAI); }
    static Action* consecration(PlayerbotAI* botAI) { return new CastConsecrationAction(botAI); }
    static Action* cleanse_poison(PlayerbotAI* botAI) { return new CastCleansePoisonAction(botAI); }
    static Action* cleanse_disease(PlayerbotAI* botAI) { return new CastCleanseDiseaseAction(botAI); }
    static Action* cleanse_magic(PlayerbotAI* botAI) { return new CastCleanseMagicAction(botAI); }
    static Action* purify_poison(PlayerbotAI* botAI) { return new CastPurifyPoisonAction(botAI); }
    static Action* purify_disease(PlayerbotAI* botAI) { return new CastPurifyDiseaseAction(botAI); }
    static Action* cleanse_poison_on_party(PlayerbotAI* botAI) { return new CastCleansePoisonOnPartyAction(botAI); }
    static Action* cleanse_disease_on_party(PlayerbotAI* botAI) { return new CastCleanseDiseaseOnPartyAction(botAI); }
    static Action* cleanse_magic_on_party(PlayerbotAI* botAI) { return new CastCleanseMagicOnPartyAction(botAI); }
    static Action* purify_poison_on_party(PlayerbotAI* botAI) { return new CastPurifyPoisonOnPartyAction(botAI); }
    static Action* purify_disease_on_party(PlayerbotAI* botAI) { return new CastPurifyDiseaseOnPartyAction(botAI); }
    
    static Action* flash_of_light(PlayerbotAI* botAI) { return new CastFlashOfLightAction(botAI); }
    static Action* hand_of_reckoning(PlayerbotAI* botAI) { return new CastHandOfReckoningAction(botAI); }
    static Action* avengers_shield(PlayerbotAI* botAI) { return new CastAvengersShieldAction(botAI); }
    static Action* exorcism(PlayerbotAI* botAI) { return new CastExorcismAction(botAI); }
    static Action* judgement(PlayerbotAI* botAI) { return new CastJudgementAction(botAI); }
    static Action* divine_shield(PlayerbotAI* botAI) { return new CastDivineShieldAction(botAI); }
    static Action* divine_protection(PlayerbotAI* botAI) { return new CastDivineProtectionAction(botAI); }
    static Action* divine_protection_on_party(PlayerbotAI* botAI)
    {
        return new CastDivineProtectionOnPartyAction(botAI);
    }
    static Action* hammer_of_justice(PlayerbotAI* botAI) { return new CastHammerOfJusticeAction(botAI); }
    static Action* flash_of_light_on_party(PlayerbotAI* botAI) { return new CastFlashOfLightOnPartyAction(botAI); }
    static Action* word_of_glory_on_party(PlayerbotAI* botAI) { return new CastWordOfGloryOnPartyAction(botAI); }
    static Action* holy_light(PlayerbotAI* botAI) { return new CastHolyLightAction(botAI); }
    static Action* holy_light_on_party(PlayerbotAI* botAI) { return new CastHolyLightOnPartyAction(botAI); }
    static Action* lay_on_hands(PlayerbotAI* botAI) { return new CastLayOnHandsAction(botAI); }
    static Action* lay_on_hands_on_party(PlayerbotAI* botAI) { return new CastLayOnHandsOnPartyAction(botAI); }
    static Action* hammer_of_wrath(PlayerbotAI* botAI) { return new CastHammerOfWrathAction(botAI); }
    static Action* holy_shield(PlayerbotAI* botAI) { return new CastHolyShieldAction(botAI); }
    static Action* hammer_of_the_righteous(PlayerbotAI* botAI) { return new CastHammerOfTheRighteousAction(botAI); }
    static Action* retribution_aura(PlayerbotAI* botAI) { return new CastRetributionAuraAction(botAI); }
    static Action* hammer_of_justice_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new CastHammerOfJusticeOnEnemyHealerAction(botAI);
    }
    static Action* hammer_of_justice_on_snare_target(PlayerbotAI* botAI)
    {
        return new CastHammerOfJusticeSnareAction(botAI);
    }
    static Action* righteous_defense(PlayerbotAI* botAI) { return new CastRighteousDefenseAction(botAI); }
    static Action* repentance(PlayerbotAI* botAI) { return new CastRepentanceAction(botAI); }
    static Action* repentance_on_snare_target(PlayerbotAI* botAI) { return new CastRepentanceSnareAction(botAI); }
    static Action* repentance_on_enemy_healer(PlayerbotAI* botAI) { return new CastRepentanceOnHealerAction(botAI); }
    static Action* sanctity_aura(PlayerbotAI* botAI) { return new CastSanctityAuraAction(botAI); }
    static Action* holy_shock(PlayerbotAI* botAI) { return new CastHolyShockAction(botAI); }
    static Action* holy_shock_on_party(PlayerbotAI* botAI) { return new CastHolyShockOnPartyAction(botAI); }
    static Action* divine_plea(PlayerbotAI* ai) { return new CastDivinePleaAction(ai); }
    static Action* shield_of_righteousness(PlayerbotAI* ai) { return new ShieldOfRighteousnessAction(ai); }
    static Action* beacon_of_light_on_main_tank(PlayerbotAI* ai) { return new CastBeaconOfLightOnMainTankAction(ai); }
    static Action* sacred_shield_on_main_tank(PlayerbotAI* ai) { return new CastSacredShieldOnMainTankAction(ai); }
    static Action* avenging_wrath(PlayerbotAI* ai) { return new CastAvengingWrathAction(ai); }
    static Action* seal_of_truth(PlayerbotAI* botAI) { return new CastSealOfTruthAction(botAI); }
    static Action* templars_verdict(PlayerbotAI* botAI) { return new CastTemplarsVerdictAction(botAI); }
    static Action* inquisition(PlayerbotAI* botAI) { return new CastInquisitionAction(botAI); }
    static Action* rebuke(PlayerbotAI* botAI) { return new CastRebukeAction(botAI); }
    static Action* rebuke_on_enemy_caster(PlayerbotAI* botAI) { return new CastRebukeOnEnemyCasterAction(botAI); }
    static Action* holy_avenger(PlayerbotAI* botAI) { return new CastHolyAvengerAction(botAI); }
    static Action* guardian_of_ancient_kings(PlayerbotAI* botAI) { return new CastGuardianOfAncientKingsAction(botAI); }
    static Action* ardent_defender(PlayerbotAI* botAI) { return new CastArdentDefenderAction(botAI); }
    static Action* execution_sentence(PlayerbotAI* botAI) { return new CastExecutionSentenceAction(botAI); }
    static Action* execution_sentence_on_party(PlayerbotAI* botAI)
    {
        return new CastExecutionSentenceOnPartyAction(botAI);
    }
    static Action* eternal_flame(PlayerbotAI* botAI) { return new CastEternalFlameAction(botAI); }
    static Action* eternal_flame_on_party(PlayerbotAI* botAI) { return new CastEternalFlameOnPartyAction(botAI); }
    static Action* divine_light_on_party(PlayerbotAI* botAI) { return new CastDivineLightOnPartyAction(botAI); }
    static Action* holy_radiance_on_party(PlayerbotAI* botAI) { return new CastHolyRadianceOnPartyAction(botAI); }
    static Action* divine_illumination(PlayerbotAI* ai) { return new CastDivineIlluminationAction(ai); }
    static Action* divine_sacrifice(PlayerbotAI* ai) { return new CastDivineSacrificeAction(ai); }
    static Action* cancel_divine_sacrifice(PlayerbotAI* ai) { return new CastCancelDivineSacrificeAction(ai); }
};

PaladinAiObjectContext::PaladinAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new PaladinStrategyFactoryInternal());
    strategyContexts.Add(new PaladinCombatStrategyFactoryInternal());
    strategyContexts.Add(new PaladinBuffStrategyFactoryInternal());
    strategyContexts.Add(new PaladinResistanceStrategyFactoryInternal());
    actionContexts.Add(new PaladinAiObjectContextInternal());
    triggerContexts.Add(new PaladinTriggerFactoryInternal());
}
