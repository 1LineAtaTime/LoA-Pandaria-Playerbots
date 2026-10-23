/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DruidAiObjectContext.h"

#include "BearTankDruidStrategy.h"
#include "CasterDruidStrategy.h"
#include "CatDpsDruidStrategy.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "DruidShapeshiftActions.h"
#include "DruidTriggers.h"
#include "DruidPullStrategy.h"
#include "GenericDruidNonCombatStrategy.h"
#include "GenericDruidStrategy.h"
#include "HealDruidStrategy.h"
#include "MeleeDruidStrategy.h"
#include "MeleeCombatStrategy.h"
#include "Playerbots.h"

// Wave-75 fix J (#868): AC's SetBehindCombatStrategy (MeleeCombatStrategy.cpp) with the druid trigger, which stops the cat
// chasing a duel / battleground player's back (DruidTriggers.h DruidNotBehindTargetTrigger). Same name and priority.
class CatSetBehindCombatStrategy : public CombatStrategy
{
public:
    CatSetBehindCombatStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override
    {
        CombatStrategy::InitTriggers(triggers);
        triggers.push_back(new TriggerNode("druid not behind target",
                                           NextAction::array(0, new NextAction("set behind", ACTION_MOVE + 7), nullptr)));
    }
    std::string const getName() override { return "behind"; }
};

class DruidStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    DruidStrategyFactoryInternal()
    {
        creators["nc"] = &DruidStrategyFactoryInternal::nc;
        // #408 (AC DruidAiObjectContext.cpp:43)
        creators["pull"] = &DruidStrategyFactoryInternal::pull;
        creators["cat aoe"] = &DruidStrategyFactoryInternal::cat_aoe;
        creators["caster aoe"] = &DruidStrategyFactoryInternal::caster_aoe;
        creators["caster debuff"] = &DruidStrategyFactoryInternal::caster_debuff;
        creators["dps debuff"] = &DruidStrategyFactoryInternal::caster_debuff;
        creators["cure"] = &DruidStrategyFactoryInternal::cure;
        creators["melee"] = &DruidStrategyFactoryInternal::melee;
        creators["buff"] = &DruidStrategyFactoryInternal::buff;
        creators["boost"] = &DruidStrategyFactoryInternal::boost;
        creators["cc"] = &DruidStrategyFactoryInternal::cc;
        creators["healer dps"] = &DruidStrategyFactoryInternal::healer_dps;
        // #408 (AC parity, Ai/Class/Druid/DruidAiObjectContext.cpp:29,35,37,38). AC registers four
        // extra druid strategies this fork never had. As in AC, "offheal" is deliberately NOT added
        // by AiFactory - it stays opt-in per bot via `co +offheal`.
        creators["aoe"] = &DruidStrategyFactoryInternal::aoe;
        creators["offheal"] = &DruidStrategyFactoryInternal::offheal;
        creators["tranquility"] = &DruidStrategyFactoryInternal::tranquility;
        creators["feral charge"] = &DruidStrategyFactoryInternal::feral_charge;
        // #363 (AC parity, Ai/Class/Druid/DruidAiObjectContext.cpp:36,51). AC exposes the blanket
        // rotation as its own opt-in "blanketing" strategy; the fork had no equivalent at all.
        creators["blanketing"] = &DruidStrategyFactoryInternal::blanketing;
        // #593 (AC Ai/Base/StrategyContext.h:105,139): AiFactory adds "behind" (SetBehindCombatStrategy) to every
        // melee dps spec, but this module never registered the key, so addStrategy("behind") was a silent no-op.
        // Registered per class here (melee classes only) instead of the shared StrategyContext.h.
        creators["behind"] = &DruidStrategyFactoryInternal::behind;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericDruidNonCombatStrategy(botAI); }
    // #408 (AC DruidAiObjectContext.cpp:43)
    static Strategy* pull(PlayerbotAI* botAI) { return new DruidPullStrategy(botAI); }
    static Strategy* cat_aoe(PlayerbotAI* botAI) { return new CatAoeDruidStrategy(botAI); }
    static Strategy* caster_aoe(PlayerbotAI* botAI) { return new CasterDruidAoeStrategy(botAI); }
    static Strategy* caster_debuff(PlayerbotAI* botAI) { return new CasterDruidDebuffStrategy(botAI); }
    static Strategy* cure(PlayerbotAI* botAI) { return new DruidCureStrategy(botAI); }
    static Strategy* melee(PlayerbotAI* botAI) { return new MeleeDruidStrategy(botAI); }
    static Strategy* buff(PlayerbotAI* botAI) { return new GenericDruidBuffStrategy(botAI); }
    static Strategy* boost(PlayerbotAI* botAI) { return new DruidBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new DruidCcStrategy(botAI); }
    static Strategy* healer_dps(PlayerbotAI* botAI) { return new DruidHealerDpsStrategy(botAI); }
    static Strategy* blanketing(PlayerbotAI* botAI) { return new DruidBlanketStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new DruidAoeStrategy(botAI); }
    static Strategy* offheal(PlayerbotAI* botAI) { return new CatOffhealStrategy(botAI); }
    static Strategy* tranquility(PlayerbotAI* botAI) { return new DruidTranquilityStrategy(botAI); }
    static Strategy* feral_charge(PlayerbotAI* botAI) { return new FeralChargeDruidStrategy(botAI); }
    static Strategy* behind(PlayerbotAI* botAI) { return new CatSetBehindCombatStrategy(botAI); }  // #593, wave-75 fix J
};

class DruidDruidStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    DruidDruidStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bear"] = &DruidDruidStrategyFactoryInternal::bear;
        creators["tank"] = &DruidDruidStrategyFactoryInternal::bear;
        creators["cat"] = &DruidDruidStrategyFactoryInternal::cat;
        creators["caster"] = &DruidDruidStrategyFactoryInternal::caster;
        creators["dps"] = &DruidDruidStrategyFactoryInternal::cat;
        creators["heal"] = &DruidDruidStrategyFactoryInternal::heal;
        // #384 (AC DruidAiObjectContext.cpp:64,66): AC's spec strategy keys. "balance" is the fork's "caster" and
        // "resto" its "heal" (same strategy classes), so config / .botscenario `co +balance|resto` and
        // AiFactory's AC-named requests resolve.
        creators["balance"] = &DruidDruidStrategyFactoryInternal::caster;
        creators["resto"] = &DruidDruidStrategyFactoryInternal::heal;
    }

private:
    static Strategy* bear(PlayerbotAI* botAI) { return new BearTankDruidStrategy(botAI); }
    static Strategy* cat(PlayerbotAI* botAI) { return new CatDpsDruidStrategy(botAI); }
    static Strategy* caster(PlayerbotAI* botAI) { return new CasterDruidStrategy(botAI); }
    static Strategy* heal(PlayerbotAI* botAI) { return new HealDruidStrategy(botAI); }
};

class DruidTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    DruidTriggerFactoryInternal()
    {
        // AC parity (#357 slice 4): the 44 AC DruidTriggerFactoryInternal names this factory was missing.
        // AC's own names and factory shapes (Ai/Class/Druid/DruidAiObjectContext.cpp:81-134). Base classes
        // used directly by AC's factory - TwoTriggers, BuffOnPartyTrigger - are deliberately NOT given a
        // Druid subclass, because AC registers those two names as bare base-class instances.
        creators["clearcasting"] = &DruidTriggerFactoryInternal::clearcasting;
        creators["thorns"] = &DruidTriggerFactoryInternal::thorns;
        creators["thorns on party"] = &DruidTriggerFactoryInternal::thorns_on_party;
        creators["thorns on main tank"] = &DruidTriggerFactoryInternal::thorns_on_main_tank;
        creators["bash"] = &DruidTriggerFactoryInternal::bash;
        creators["faerie fire (feral)"] = &DruidTriggerFactoryInternal::faerie_fire_feral;
        creators["faerie fire"] = &DruidTriggerFactoryInternal::faerie_fire;
        creators["insect swarm"] = &DruidTriggerFactoryInternal::insect_swarm;
        creators["insect swarm on attacker"] = &DruidTriggerFactoryInternal::insect_swarm_on_attacker;
        creators["moonfire"] = &DruidTriggerFactoryInternal::moonfire;
        creators["moonfire on attacker"] = &DruidTriggerFactoryInternal::moonfire_on_attacker;
        creators["nature's grasp"] = &DruidTriggerFactoryInternal::natures_grasp;
        creators["berserk"] = &DruidTriggerFactoryInternal::berserk;
        creators["berserk active"] = &DruidTriggerFactoryInternal::berserk_active;
        creators["savage roar"] = &DruidTriggerFactoryInternal::savage_roar;
        creators["rake"] = &DruidTriggerFactoryInternal::rake;
        creators["mark of the wild"] = &DruidTriggerFactoryInternal::mark_of_the_wild;
        creators["mark of the wild on party"] = &DruidTriggerFactoryInternal::mark_of_the_wild_on_party;
        creators["cure poison"] = &DruidTriggerFactoryInternal::cure_poison;
        creators["bear form"] = &DruidTriggerFactoryInternal::bear_form;
        creators["cat form"] = &DruidTriggerFactoryInternal::cat_form;
        creators["tree form"] = &DruidTriggerFactoryInternal::tree_form;
        creators["bash on enemy healer"] = &DruidTriggerFactoryInternal::bash_on_enemy_healer;
        creators["nature's swiftness"] = &DruidTriggerFactoryInternal::natures_swiftness;
        creators["nature's swiftness active"] = &DruidTriggerFactoryInternal::natures_swiftness_active;
        creators["mangle (bear)"] = &DruidTriggerFactoryInternal::mangle_bear_trigger;
        creators["lacerate"] = &DruidTriggerFactoryInternal::lacerate_trigger;
        creators["demoralizing roar"] = &DruidTriggerFactoryInternal::demoralize_roar;
        creators["mangle (cat)"] = &DruidTriggerFactoryInternal::mangle_cat;
        creators["ferocious bite execute"] = &DruidTriggerFactoryInternal::ferocious_bite_execute;
        creators["hurricane channel check"] = &DruidTriggerFactoryInternal::hurricane_channel_check;
        creators["no healer dps strategy"] = &DruidTriggerFactoryInternal::no_healer_dps_strategy;
        creators["starfall"] = &DruidTriggerFactoryInternal::starfall;
        creators["force of nature"] = &DruidTriggerFactoryInternal::force_of_nature;
        // Rotation lane 5 (#860): MoP druid ports (DruidTriggers.h tail).
        creators["starsurge"] = &DruidTriggerFactoryInternal::starsurge;
        creators["celestial alignment"] = &DruidTriggerFactoryInternal::celestial_alignment;
        creators["nature's vigil"] = &DruidTriggerFactoryInternal::natures_vigil;
        creators["might of ursoc"] = &DruidTriggerFactoryInternal::might_of_ursoc;
        creators["savage defense"] = &DruidTriggerFactoryInternal::savage_defense;
        creators["druid self rejuvenation"] = &DruidTriggerFactoryInternal::druid_self_rejuvenation;
        creators["druid self healing touch"] = &DruidTriggerFactoryInternal::druid_self_healing_touch;
        creators["predator's swiftness heal emergency"] = &DruidTriggerFactoryInternal::ps_heal_emergency;
        creators["predator's swiftness heal"] = &DruidTriggerFactoryInternal::ps_heal;
        creators["ironbark"] = &DruidTriggerFactoryInternal::ironbark;
        creators["predator's swiftness"] = &DruidTriggerFactoryInternal::predators_swiftness;
        creators["predator's swiftness and cyclone"] = &DruidTriggerFactoryInternal::predators_swiftness_and_cyclone;
        creators["predator's swiftness and hibernate"] =
            &DruidTriggerFactoryInternal::predators_swiftness_and_hibernate;
        creators["predator's swiftness and entangling roots"] =
            &DruidTriggerFactoryInternal::predators_swiftness_and_entangling_roots;
        creators["predator's swiftness and combat party member dead"] =
            &DruidTriggerFactoryInternal::predators_swiftness_and_combat_party_member_dead;
        creators["clearcasting and medium aoe"] = &DruidTriggerFactoryInternal::clearcasting_and_medium_aoe;
        creators["prowl"] = &DruidTriggerFactoryInternal::prowl_trigger;
        creators["rejuvenation blanket"] = &DruidTriggerFactoryInternal::rejuvenation_blanket;
        creators["wild growth blanket"] = &DruidTriggerFactoryInternal::wild_growth_blanket;
        creators["aquatic form"] = &DruidTriggerFactoryInternal::aquatic_form;
        creators["eclipse (solar)"] = &DruidTriggerFactoryInternal::eclipse_solar;
        creators["eclipse (lunar)"] = &DruidTriggerFactoryInternal::eclipse_lunar;
        creators["eclipse lunar direction"] = &DruidTriggerFactoryInternal::eclipse_lunar_direction;  // #521 MoP-only
        // #149: CasterDruidStrategy.cpp:132/134 references "... cooldown"; EclipseSolarTrigger /
        // EclipseLunarTrigger are HasAuraTrigger-based (they fire when the eclipse is *up*), so the
        // "cooldown" variant - which schedules starfire/wrath when the eclipse has been spent - needs the
        // inverse aura check. Eclipse is MoP-only, so there is no AC registration to copy 1:1.
        creators["eclipse (solar) cooldown"] = &DruidTriggerFactoryInternal::eclipse_solar_cd;
        creators["eclipse (lunar) cooldown"] = &DruidTriggerFactoryInternal::eclipse_lunar_cd;
        creators["party member cure poison"] = &DruidTriggerFactoryInternal::party_member_cure_poison;
        creators["party member remove curse"] = &DruidTriggerFactoryInternal::party_member_remove_curse;
        // #149 (AC DruidAiObjectContext.cpp:102,117): GenericDruidStrategy.cpp / CatDpsDruidStrategy.cpp
        // already reference these names; only the registration was missing.
        creators["entangling roots kite"] = &DruidTriggerFactoryInternal::entangling_roots_kite;
        creators["ferocious bite time"] = &DruidTriggerFactoryInternal::ferocious_bite_time;
        // #163 S8 gap fix (b) (AC parity, Ai/Class/Druid/DruidAiObjectContext.cpp:179,
        // GenericTriggers.h + RtiTriggers.h): "hibernate"/"entangling roots"/"cyclone" were referenced
        // by TriggerNodes in GenericDruidStrategy.cpp/CasterDruidStrategy.cpp but had no Trigger
        // registered at all, so the CC actions they schedule ("hibernate on cc" etc.) could never
        // fire. RtiCcTrigger is this fork's port of AC's RtiCcTrigger/HasCcTargetTrigger.
        creators["hibernate"] = &DruidTriggerFactoryInternal::hibernate_cc;
        creators["entangling roots"] = &DruidTriggerFactoryInternal::entangling_roots_cc;
        creators["cyclone"] = &DruidTriggerFactoryInternal::cyclone_cc;
        // #384: Thrash debuff trigger (replaces "demoralizing roar" in the bear strategy, see ThrashTrigger).
        creators["thrash"] = &DruidTriggerFactoryInternal::thrash;
        // Rotation fix lane A (#860): cat Thrash (MoP rotation row 10304) and bear Enrage (MoP rotation row 10419, group 5) triggers.
        creators["thrash (cat)"] = &DruidTriggerFactoryInternal::thrash_cat;
        creators["enrage"] = &DruidTriggerFactoryInternal::enrage_trigger;
        // Wave-74 fix G (#860): "prowl active" (MoP rotation row 10316 pounce) and a plain "tiger's fury" aura-missing buff trigger
        // (MoP rotation row 10308, in combat "always"; the AC "low energy" edge never fired, cat energy stayed >= 37).
        creators["prowl active"] = &DruidTriggerFactoryInternal::prowl_active;
        creators["druid not behind target"] = &DruidTriggerFactoryInternal::druid_not_behind_target;  // wave-75 fix J (#868)
        creators["tiger's fury"] = &DruidTriggerFactoryInternal::tigers_fury_buff;
    }

private:
    static Trigger* clearcasting(PlayerbotAI* botAI) { return new ClearcastingTrigger(botAI); }
    static Trigger* thorns(PlayerbotAI* botAI) { return new ThornsTrigger(botAI); }
    static Trigger* thorns_on_party(PlayerbotAI* botAI) { return new ThornsOnPartyTrigger(botAI); }
    static Trigger* thorns_on_main_tank(PlayerbotAI* botAI) { return new ThornsOnMainTankTrigger(botAI); }
    static Trigger* bash(PlayerbotAI* botAI) { return new BashInterruptSpellTrigger(botAI); }
    static Trigger* faerie_fire_feral(PlayerbotAI* botAI) { return new FaerieFireFeralTrigger(botAI); }
    static Trigger* faerie_fire(PlayerbotAI* botAI) { return new FaerieFireTrigger(botAI); }
    static Trigger* insect_swarm(PlayerbotAI* botAI) { return new InsectSwarmTrigger(botAI); }
    static Trigger* insect_swarm_on_attacker(PlayerbotAI* botAI) { return new InsectSwarmOnAttackerTrigger(botAI); }
    static Trigger* moonfire(PlayerbotAI* botAI) { return new MoonfireTrigger(botAI); }
    static Trigger* moonfire_on_attacker(PlayerbotAI* botAI) { return new MoonfireOnAttackerTrigger(botAI); }
    static Trigger* natures_grasp(PlayerbotAI* botAI) { return new NaturesGraspTrigger(botAI); }
    static Trigger* berserk(PlayerbotAI* botAI) { return new BerserkTrigger(botAI); }
    static Trigger* berserk_active(PlayerbotAI* botAI) { return new BerserkActiveTrigger(botAI); }
    static Trigger* savage_roar(PlayerbotAI* botAI) { return new SavageRoarTrigger(botAI); }
    static Trigger* rake(PlayerbotAI* botAI) { return new RakeTrigger(botAI); }
    static Trigger* mark_of_the_wild(PlayerbotAI* botAI) { return new MarkOfTheWildTrigger(botAI); }
    static Trigger* mark_of_the_wild_on_party(PlayerbotAI* botAI) { return new MarkOfTheWildOnPartyTrigger(botAI); }
    static Trigger* cure_poison(PlayerbotAI* botAI) { return new CurePoisonTrigger(botAI); }
    static Trigger* bear_form(PlayerbotAI* botAI) { return new BearFormTrigger(botAI); }
    static Trigger* cat_form(PlayerbotAI* botAI) { return new CatFormTrigger(botAI); }
    static Trigger* tree_form(PlayerbotAI* botAI) { return new TreeFormTrigger(botAI); }
    static Trigger* bash_on_enemy_healer(PlayerbotAI* botAI) { return new BashInterruptEnemyHealerSpellTrigger(botAI); }
    static Trigger* natures_swiftness(PlayerbotAI* botAI) { return new NaturesSwiftnessTrigger(botAI); }
    static Trigger* natures_swiftness_active(PlayerbotAI* botAI) { return new NaturesSwiftnessActiveTrigger(botAI); }
    static Trigger* mangle_bear_trigger(PlayerbotAI* botAI) { return new MangleBearTrigger(botAI); }
    static Trigger* lacerate_trigger(PlayerbotAI* botAI) { return new LacerateTrigger(botAI); }
    static Trigger* demoralize_roar(PlayerbotAI* botAI) { return new DemoralizeRoarTrigger(botAI); }
    static Trigger* mangle_cat(PlayerbotAI* botAI) { return new MangleCatTrigger(botAI); }
    static Trigger* ferocious_bite_execute(PlayerbotAI* botAI) { return new FerociousBiteExecuteTrigger(botAI); }
    static Trigger* hurricane_channel_check(PlayerbotAI* botAI) { return new HurricaneChannelCheckTrigger(botAI); }
    static Trigger* no_healer_dps_strategy(PlayerbotAI* botAI) { return new NoHealerDpsStrategyTrigger(botAI); }
    static Trigger* starfall(PlayerbotAI* botAI) { return new StarfallTrigger(botAI); }
    static Trigger* force_of_nature(PlayerbotAI* botAI) { return new ForceOfNatureTrigger(botAI); }
    static Trigger* starsurge(PlayerbotAI* botAI) { return new StarsurgeTrigger(botAI); }
    static Trigger* celestial_alignment(PlayerbotAI* botAI) { return new CelestialAlignmentTrigger(botAI); }
    static Trigger* natures_vigil(PlayerbotAI* botAI) { return new NaturesVigilTrigger(botAI); }
    static Trigger* might_of_ursoc(PlayerbotAI* botAI) { return new MightOfUrsocTrigger(botAI); }
    static Trigger* savage_defense(PlayerbotAI* botAI) { return new SavageDefenseTrigger(botAI); }
    static Trigger* druid_self_rejuvenation(PlayerbotAI* botAI) { return new DruidSelfRejuvenationTrigger(botAI); }
    static Trigger* druid_self_healing_touch(PlayerbotAI* botAI) { return new DruidSelfHealingTouchTrigger(botAI); }
    static Trigger* ps_heal_emergency(PlayerbotAI* botAI)
    {
        return new PredatorsSwiftnessHealTrigger(botAI, "predator's swiftness heal emergency", 35);
    }
    static Trigger* ps_heal(PlayerbotAI* botAI)
    {
        return new PredatorsSwiftnessHealTrigger(botAI, "predator's swiftness heal", 90);
    }
    static Trigger* ironbark(PlayerbotAI* botAI) { return new IronbarkTrigger(botAI); }
    static Trigger* predators_swiftness(PlayerbotAI* botAI) { return new PredatorsSwiftnessTrigger(botAI); }
    static Trigger* predators_swiftness_and_cyclone(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "predator's swiftness", "cyclone");
    }
    static Trigger* predators_swiftness_and_hibernate(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "predator's swiftness", "hibernate");
    }
    static Trigger* predators_swiftness_and_entangling_roots(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "predator's swiftness", "entangling roots");
    }
    static Trigger* predators_swiftness_and_combat_party_member_dead(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "predator's swiftness", "combat party member dead");
    }
    static Trigger* clearcasting_and_medium_aoe(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "clearcasting", "medium aoe");
    }
    static Trigger* prowl_trigger(PlayerbotAI* botAI) { return new ProwlTrigger(botAI); }
    static Trigger* rejuvenation_blanket(PlayerbotAI* botAI) { return new BuffOnPartyTrigger(botAI, "rejuvenation"); }
    static Trigger* wild_growth_blanket(PlayerbotAI* botAI) { return new BuffOnPartyTrigger(botAI, "wild growth"); }
    static Trigger* aquatic_form(PlayerbotAI* botAI) { return new AquaticFormTrigger(botAI); }
    static Trigger* eclipse_solar(PlayerbotAI* botAI) { return new EclipseSolarTrigger(botAI); }
    static Trigger* eclipse_lunar(PlayerbotAI* botAI) { return new EclipseLunarTrigger(botAI); }
    static Trigger* eclipse_lunar_direction(PlayerbotAI* botAI) { return new EclipseLunarDirectionTrigger(botAI); }
    static Trigger* eclipse_solar_cd(PlayerbotAI* botAI) { return new HasNoAuraTrigger(botAI, "eclipse (solar)"); }
    static Trigger* eclipse_lunar_cd(PlayerbotAI* botAI) { return new HasNoAuraTrigger(botAI, "eclipse (lunar)"); }
    static Trigger* party_member_cure_poison(PlayerbotAI* botAI) { return new PartyMemberCurePoisonTrigger(botAI); }
    static Trigger* party_member_remove_curse(PlayerbotAI* botAI) { return new DruidPartyMemberRemoveCurseTrigger(botAI); }
    static Trigger* entangling_roots_kite(PlayerbotAI* botAI) { return new EntanglingRootsKiteTrigger(botAI); }
    static Trigger* ferocious_bite_time(PlayerbotAI* botAI) { return new FerociousBiteTimeTrigger(botAI); }
    static Trigger* hibernate_cc(PlayerbotAI* botAI) { return new HibernateTrigger(botAI); }
    static Trigger* entangling_roots_cc(PlayerbotAI* botAI) { return new EntanglingRootsTrigger(botAI); }
    static Trigger* cyclone_cc(PlayerbotAI* botAI) { return new CycloneTrigger(botAI); }
    static Trigger* thrash(PlayerbotAI* botAI) { return new ThrashTrigger(botAI); }
    static Trigger* thrash_cat(PlayerbotAI* botAI) { return new ThrashCatTrigger(botAI); }
    static Trigger* prowl_active(PlayerbotAI* botAI) { return new ProwlActiveTrigger(botAI); }
    static Trigger* druid_not_behind_target(PlayerbotAI* botAI) { return new DruidNotBehindTargetTrigger(botAI); }
    static Trigger* tigers_fury_buff(PlayerbotAI* botAI) { return new BuffTrigger(botAI, "tiger's fury"); }
    static Trigger* enrage_trigger(PlayerbotAI* botAI) { return new BoostTrigger(botAI, "enrage"); }
};

class DruidAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    DruidAiObjectContextInternal()
    {
        creators["feral charge - bear"] = &DruidAiObjectContextInternal::feral_charge_bear;
        creators["feral charge - cat"] = &DruidAiObjectContextInternal::feral_charge_cat;
        creators["swipe (bear)"] = &DruidAiObjectContextInternal::swipe_bear;
        creators["faerie fire (feral)"] = &DruidAiObjectContextInternal::faerie_fire_feral;
        creators["faerie fire"] = &DruidAiObjectContextInternal::faerie_fire;
        creators["bear form"] = &DruidAiObjectContextInternal::bear_form;
        creators["dire bear form"] = &DruidAiObjectContextInternal::dire_bear_form;
        creators["moonkin form"] = &DruidAiObjectContextInternal::moonkin_form;
        creators["cat form"] = &DruidAiObjectContextInternal::cat_form;
        creators["tree form"] = &DruidAiObjectContextInternal::tree_form;
        creators["travel form"] = &DruidAiObjectContextInternal::travel_form;
        creators["aquatic form"] = &DruidAiObjectContextInternal::aquatic_form;
        creators["caster form"] = &DruidAiObjectContextInternal::caster_form;
        creators["cancel tree form"] = &DruidAiObjectContextInternal::cancel_tree_form;
        // #363 (AC parity, Ai/Class/Druid/DruidAiObjectContext.cpp:211-216): the six remaining
        // form-cancel actions. "cancel tree form" above was already present in the fork.
        creators["cancel travel form"] = &DruidAiObjectContextInternal::cancel_travel_form;
        creators["cancel bear form"] = &DruidAiObjectContextInternal::cancel_bear_form;
        creators["cancel dire bear form"] = &DruidAiObjectContextInternal::cancel_dire_bear_form;
        creators["cancel cat form"] = &DruidAiObjectContextInternal::cancel_cat_form;
        creators["cancel moonkin form"] = &DruidAiObjectContextInternal::cancel_moonkin_form;
        creators["cancel aquatic form"] = &DruidAiObjectContextInternal::cancel_aquatic_form;
        creators["mangle (bear)"] = &DruidAiObjectContextInternal::mangle_bear;
        creators["maul"] = &DruidAiObjectContextInternal::maul;
        creators["bash"] = &DruidAiObjectContextInternal::bash;
        creators["swipe"] = &DruidAiObjectContextInternal::swipe;
        creators["growl"] = &DruidAiObjectContextInternal::growl;
        // #363 (AC parity, Ai/Class/Druid/DruidAiObjectContext.cpp:221).
        creators["challenging roar"] = &DruidAiObjectContextInternal::challenging_roar;
        creators["demoralizing roar"] = &DruidAiObjectContextInternal::demoralizing_roar;
        // #384: Thrash replaces Demoralizing Roar on 5.4.8 (CastThrashAction, DruidBearActions.h).
        creators["thrash"] = &DruidAiObjectContextInternal::thrash;
        creators["hibernate"] = &DruidAiObjectContextInternal::hibernate;
        creators["entangling roots"] = &DruidAiObjectContextInternal::entangling_roots;
        creators["entangling roots on cc"] = &DruidAiObjectContextInternal::entangling_roots_on_cc;
        creators["hibernate"] = &DruidAiObjectContextInternal::hibernate;
        creators["hibernate on cc"] = &DruidAiObjectContextInternal::hibernate_on_cc;
        // #163 S8 (AC parity): Cyclone + cyclone-on-cc, same pattern as hibernate above. The
        // "cyclone" trigger (DruidTriggerFactoryInternal) and the "cc" strategy wiring
        // (Factory/AiFactory.cpp) are what actually make this fire - see gap fix (b).
        creators["cyclone"] = &DruidAiObjectContextInternal::cyclone;
        creators["cyclone on cc"] = &DruidAiObjectContextInternal::cyclone_on_cc;
        creators["wrath"] = &DruidAiObjectContextInternal::wrath;
        creators["starfall"] = &DruidAiObjectContextInternal::starfall;
        creators["insect swarm"] = &DruidAiObjectContextInternal::insect_swarm;
        creators["moonfire"] = &DruidAiObjectContextInternal::moonfire;
        creators["starfire"] = &DruidAiObjectContextInternal::starfire;
        creators["nature's grasp"] = &DruidAiObjectContextInternal::natures_grasp;
        creators["claw"] = &DruidAiObjectContextInternal::claw;
        creators["mangle (cat)"] = &DruidAiObjectContextInternal::mangle_cat;
        creators["swipe (cat)"] = &DruidAiObjectContextInternal::swipe_cat;
        creators["rake"] = &DruidAiObjectContextInternal::rake;
        creators["rake on attacker"] = &DruidAiObjectContextInternal::rake_on_attacker;
        creators["ferocious bite"] = &DruidAiObjectContextInternal::ferocious_bite;
        creators["rip"] = &DruidAiObjectContextInternal::rip;
        // #363 (AC parity, Ai/Class/Druid/DruidAiObjectContext.cpp:240).
        creators["maim"] = &DruidAiObjectContextInternal::maim;
        creators["cower"] = &DruidAiObjectContextInternal::cower;
        creators["survival instincts"] = &DruidAiObjectContextInternal::survival_instincts;
        creators["frenzied regeneration"] = &DruidAiObjectContextInternal::frenzied_regeneration;
        creators["thorns"] = &DruidAiObjectContextInternal::thorns;
        creators["thorns on party"] = &DruidAiObjectContextInternal::thorns_on_party;
        creators["thorns on main tank"] = &DruidAiObjectContextInternal::thorns_on_main_tank;
        // #363 (AC parity, Ai/Class/Druid/DruidAiObjectContext.cpp:247).
        creators["lifebloom on main tank"] = &DruidAiObjectContextInternal::lifebloom_on_main_tank;
        creators["cure poison"] = &DruidAiObjectContextInternal::cure_poison;
        creators["cure poison on party"] = &DruidAiObjectContextInternal::cure_poison_on_party;
        creators["abolish poison"] = &DruidAiObjectContextInternal::abolish_poison;
        creators["abolish poison on party"] = &DruidAiObjectContextInternal::abolish_poison_on_party;
        creators["berserk"] = &DruidAiObjectContextInternal::berserk;
        creators["tiger's fury"] = &DruidAiObjectContextInternal::tigers_fury;
        creators["savage roar"] = &DruidAiObjectContextInternal::savage_roar;
        creators["mark of the wild"] = &DruidAiObjectContextInternal::mark_of_the_wild;
        creators["mark of the wild on party"] = &DruidAiObjectContextInternal::mark_of_the_wild_on_party;
        creators["regrowth"] = &DruidAiObjectContextInternal::regrowth;
        creators["rejuvenation"] = &DruidAiObjectContextInternal::rejuvenation;
        creators["healing touch"] = &DruidAiObjectContextInternal::healing_touch;
        creators["regrowth on party"] = &DruidAiObjectContextInternal::regrowth_on_party;
        creators["rejuvenation on party"] = &DruidAiObjectContextInternal::rejuvenation_on_party;
        creators["rejuvenation on not full"] = &DruidAiObjectContextInternal::rejuvenation_on_not_full;
        creators["healing touch on party"] = &DruidAiObjectContextInternal::healing_touch_on_party;
        creators["rebirth"] = &DruidAiObjectContextInternal::rebirth;
        creators["revive"] = &DruidAiObjectContextInternal::revive;
        creators["barkskin"] = &DruidAiObjectContextInternal::barkskin;
        creators["lacerate"] = &DruidAiObjectContextInternal::lacerate;
        creators["hurricane"] = &DruidAiObjectContextInternal::hurricane;
        creators["innervate"] = &DruidAiObjectContextInternal::innervate;
        // #363 (AC parity, Ai/Class/Druid/DruidAiObjectContext.cpp:270).
        creators["innervate on healer"] = &DruidAiObjectContextInternal::innervate_on_healer;
        creators["tranquility"] = &DruidAiObjectContextInternal::tranquility;
        creators["bash on enemy healer"] = &DruidAiObjectContextInternal::bash_on_enemy_healer;
        creators["omen of clarity"] = &DruidAiObjectContextInternal::omen_of_clarity;
        creators["nature's swiftness"] = &DruidAiObjectContextInternal::natures_swiftness;
        creators["prowl"] = &DruidAiObjectContextInternal::prowl;
        creators["dash"] = &DruidAiObjectContextInternal::dash;
        creators["shred"] = &DruidAiObjectContextInternal::shred;
        creators["ravage"] = &DruidAiObjectContextInternal::ravage;
        creators["pounce"] = &DruidAiObjectContextInternal::pounce;
        creators["wild growth on party"] = &DruidAiObjectContextInternal::wild_growth_on_party;
        creators["swiftmend on party"] = &DruidAiObjectContextInternal::swiftmend_on_party;
        creators["nourish on party"] = &DruidAiObjectContextInternal::nourish_on_party;
        creators["remove curse on party"] = &DruidAiObjectContextInternal::remove_curse_on_party;
        creators["insect swarm on attacker"] = &DruidAiObjectContextInternal::insect_swarm_on_attacker;
        creators["moonfire on attacker"] = &DruidAiObjectContextInternal::moonfire_on_attacker;
        creators["enrage"] = &DruidAiObjectContextInternal::enrage;
        creators["force of nature"] = &DruidAiObjectContextInternal::force_of_nature;
        // #363 (AC parity, Ai/Class/Druid/DruidAiObjectContext.cpp:287-289). These are the ACTION
        // halves; the same two names are already registered as TRIGGERS above at :126-127
        // (#357 slice 4). Different NamedObjectContext, so no clash - and getName() on each
        // blanket action keeps them distinct from the trigger names.
        creators["typhoon"] = &DruidAiObjectContextInternal::typhoon;
        // Rotation lane 5 (#860): MoP druid ports (DruidActions.h tail).
        creators["starsurge"] = &DruidAiObjectContextInternal::starsurge;
        creators["celestial alignment"] = &DruidAiObjectContextInternal::celestial_alignment;
        creators["nature's vigil"] = &DruidAiObjectContextInternal::natures_vigil;
        creators["might of ursoc"] = &DruidAiObjectContextInternal::might_of_ursoc;
        creators["savage defense"] = &DruidAiObjectContextInternal::savage_defense;
        creators["ironbark on main tank"] = &DruidAiObjectContextInternal::ironbark_on_main_tank;
        creators["skull bash on enemy caster"] = &DruidAiObjectContextInternal::skull_bash_on_enemy_caster;
        creators["rejuvenation blanket"] = &DruidAiObjectContextInternal::rejuvenation_blanket;
        creators["wild growth blanket"] = &DruidAiObjectContextInternal::wild_growth_blanket;
    }

private:
    static Action* natures_swiftness(PlayerbotAI* botAI) { return new CastNaturesSwiftnessAction(botAI); }
    static Action* omen_of_clarity(PlayerbotAI* botAI) { return new CastOmenOfClarityAction(botAI); }
    static Action* tranquility(PlayerbotAI* botAI) { return new CastTranquilityAction(botAI); }
    static Action* feral_charge_bear(PlayerbotAI* botAI) { return new CastFeralChargeBearAction(botAI); }
    static Action* feral_charge_cat(PlayerbotAI* botAI) { return new CastFeralChargeCatAction(botAI); }
    static Action* swipe_bear(PlayerbotAI* botAI) { return new CastSwipeBearAction(botAI); }
    static Action* faerie_fire_feral(PlayerbotAI* botAI) { return new CastFaerieFireFeralAction(botAI); }
    static Action* faerie_fire(PlayerbotAI* botAI) { return new CastFaerieFireAction(botAI); }
    static Action* bear_form(PlayerbotAI* botAI) { return new CastBearFormAction(botAI); }
    static Action* dire_bear_form(PlayerbotAI* botAI) { return new CastDireBearFormAction(botAI); }
    static Action* cat_form(PlayerbotAI* botAI) { return new CastCatFormAction(botAI); }
    static Action* tree_form(PlayerbotAI* botAI) { return new CastTreeFormAction(botAI); }
    static Action* travel_form(PlayerbotAI* botAI) { return new CastTravelFormAction(botAI); }
    static Action* aquatic_form(PlayerbotAI* botAI) { return new CastAquaticFormAction(botAI); }
    static Action* caster_form(PlayerbotAI* botAI) { return new CastCasterFormAction(botAI); }
    static Action* cancel_tree_form(PlayerbotAI* botAI) { return new CastCancelTreeFormAction(botAI); }
    static Action* cancel_travel_form(PlayerbotAI* botAI) { return new CastCancelTravelFormAction(botAI); }
    static Action* cancel_bear_form(PlayerbotAI* botAI) { return new CastCancelBearFormAction(botAI); }
    static Action* cancel_dire_bear_form(PlayerbotAI* botAI) { return new CastCancelDireBearFormAction(botAI); }
    static Action* cancel_cat_form(PlayerbotAI* botAI) { return new CastCancelCatFormAction(botAI); }
    static Action* cancel_moonkin_form(PlayerbotAI* botAI) { return new CastCancelMoonkinFormAction(botAI); }
    static Action* cancel_aquatic_form(PlayerbotAI* botAI) { return new CastCancelAquaticFormAction(botAI); }
    static Action* mangle_bear(PlayerbotAI* botAI) { return new CastMangleBearAction(botAI); }
    static Action* maul(PlayerbotAI* botAI) { return new CastMaulAction(botAI); }
    static Action* bash(PlayerbotAI* botAI) { return new CastBashAction(botAI); }
    static Action* swipe(PlayerbotAI* botAI) { return new CastSwipeAction(botAI); }
    static Action* growl(PlayerbotAI* botAI) { return new CastGrowlAction(botAI); }
    static Action* challenging_roar(PlayerbotAI* botAI) { return new CastChallengingRoarAction(botAI); }
    static Action* demoralizing_roar(PlayerbotAI* botAI) { return new CastDemoralizingRoarAction(botAI); }
    static Action* thrash(PlayerbotAI* botAI) { return new CastThrashAction(botAI); }
    static Action* moonkin_form(PlayerbotAI* botAI) { return new CastMoonkinFormAction(botAI); }
    static Action* hibernate(PlayerbotAI* botAI) { return new CastHibernateAction(botAI); }
    static Action* entangling_roots(PlayerbotAI* botAI) { return new CastEntanglingRootsAction(botAI); }
    static Action* hibernate_on_cc(PlayerbotAI* botAI) { return new CastHibernateCcAction(botAI); }
    static Action* cyclone(PlayerbotAI* botAI) { return new CastCycloneAction(botAI); }
    static Action* cyclone_on_cc(PlayerbotAI* botAI) { return new CastCycloneCcAction(botAI); }
    static Action* entangling_roots_on_cc(PlayerbotAI* botAI) { return new CastEntanglingRootsCcAction(botAI); }
    static Action* wrath(PlayerbotAI* botAI) { return new CastWrathAction(botAI); }
    static Action* starfall(PlayerbotAI* botAI) { return new CastStarfallAction(botAI); }
    static Action* insect_swarm(PlayerbotAI* botAI) { return new CastInsectSwarmAction(botAI); }
    static Action* moonfire(PlayerbotAI* botAI) { return new CastMoonfireAction(botAI); }
    static Action* starfire(PlayerbotAI* botAI) { return new CastStarfireAction(botAI); }
    static Action* natures_grasp(PlayerbotAI* botAI) { return new CastNaturesGraspAction(botAI); }
    static Action* claw(PlayerbotAI* botAI) { return new CastClawAction(botAI); }
    static Action* mangle_cat(PlayerbotAI* botAI) { return new CastMangleCatAction(botAI); }
    static Action* swipe_cat(PlayerbotAI* botAI) { return new CastSwipeCatAction(botAI); }
    static Action* rake(PlayerbotAI* botAI) { return new CastRakeAction(botAI); }
    static Action* rake_on_attacker(PlayerbotAI* botAI) { return new CastRakeOnMeleeAttackersAction(botAI); }
    static Action* ferocious_bite(PlayerbotAI* botAI) { return new CastFerociousBiteAction(botAI); }
    static Action* rip(PlayerbotAI* botAI) { return new CastRipAction(botAI); }
    static Action* maim(PlayerbotAI* botAI) { return new CastMaimAction(botAI); }
    static Action* cower(PlayerbotAI* botAI) { return new CastCowerAction(botAI); }
    static Action* survival_instincts(PlayerbotAI* botAI) { return new CastSurvivalInstinctsAction(botAI); }
    static Action* frenzied_regeneration(PlayerbotAI* botAI) { return new CastFrenziedRegenerationAction(botAI); }
    static Action* thorns(PlayerbotAI* botAI) { return new CastThornsAction(botAI); }
    static Action* thorns_on_party(PlayerbotAI* botAI) { return new CastThornsOnPartyAction(botAI); }
    static Action* thorns_on_main_tank(PlayerbotAI* botAI) { return new CastThornsOnMainTankAction(botAI); }
    static Action* lifebloom_on_main_tank(PlayerbotAI* botAI) { return new CastLifebloomOnMainTankAction(botAI); }
    static Action* cure_poison(PlayerbotAI* botAI) { return new CastCurePoisonAction(botAI); }
    static Action* cure_poison_on_party(PlayerbotAI* botAI) { return new CastCurePoisonOnPartyAction(botAI); }
    static Action* abolish_poison(PlayerbotAI* botAI) { return new CastAbolishPoisonAction(botAI); }
    static Action* abolish_poison_on_party(PlayerbotAI* botAI) { return new CastAbolishPoisonOnPartyAction(botAI); }
    static Action* berserk(PlayerbotAI* botAI) { return new CastBerserkAction(botAI); }
    static Action* tigers_fury(PlayerbotAI* botAI) { return new CastTigersFuryAction(botAI); }
    static Action* savage_roar(PlayerbotAI* botAI) { return new CastSavageRoarAction(botAI); }
    static Action* mark_of_the_wild(PlayerbotAI* botAI) { return new CastMarkOfTheWildAction(botAI); }
    static Action* mark_of_the_wild_on_party(PlayerbotAI* botAI) { return new CastMarkOfTheWildOnPartyAction(botAI); }
    static Action* regrowth(PlayerbotAI* botAI) { return new CastRegrowthAction(botAI); }
    static Action* rejuvenation(PlayerbotAI* botAI) { return new CastRejuvenationAction(botAI); }
    static Action* healing_touch(PlayerbotAI* botAI) { return new CastHealingTouchAction(botAI); }
    static Action* regrowth_on_party(PlayerbotAI* botAI) { return new CastRegrowthOnPartyAction(botAI); }
    static Action* rejuvenation_on_party(PlayerbotAI* botAI) { return new CastRejuvenationOnPartyAction(botAI); }
    static Action* rejuvenation_on_not_full(PlayerbotAI* botAI) { return new CastRejuvenationOnNotFullAction(botAI); }
    static Action* healing_touch_on_party(PlayerbotAI* botAI) { return new CastHealingTouchOnPartyAction(botAI); }
    static Action* rebirth(PlayerbotAI* botAI) { return new CastRebirthAction(botAI); }
    static Action* revive(PlayerbotAI* botAI) { return new CastReviveAction(botAI); }
    static Action* barkskin(PlayerbotAI* botAI) { return new CastBarkskinAction(botAI); }
    static Action* lacerate(PlayerbotAI* botAI) { return new CastLacerateAction(botAI); }
    static Action* hurricane(PlayerbotAI* botAI) { return new CastHurricaneAction(botAI); }
    static Action* innervate(PlayerbotAI* botAI) { return new CastInnervateAction(botAI); }
    static Action* innervate_on_healer(PlayerbotAI* botAI) { return new CastInnervateOnHealerAction(botAI); }
    static Action* bash_on_enemy_healer(PlayerbotAI* botAI) { return new CastBashOnEnemyHealerAction(botAI); }
    static Action* ravage(PlayerbotAI* botAI) { return new CastRavageAction(botAI); }
    static Action* pounce(PlayerbotAI* botAI) { return new CastPounceAction(botAI); }
    static Action* prowl(PlayerbotAI* botAI) { return new CastProwlAction(botAI); }
    static Action* dash(PlayerbotAI* botAI) { return new CastDashAction(botAI); }
    static Action* shred(PlayerbotAI* botAI) { return new CastShredAction(botAI); }
    static Action* wild_growth_on_party(PlayerbotAI* ai) { return new CastWildGrowthOnPartyAction(ai); }
    static Action* swiftmend_on_party(PlayerbotAI* ai) { return new CastPartySwiftmendAction(ai); }
    static Action* nourish_on_party(PlayerbotAI* ai) { return new CastPartyNourishAction(ai); }
    static Action* remove_curse_on_party(PlayerbotAI* ai) { return new CastDruidRemoveCurseOnPartyAction(ai); }
    static Action* insect_swarm_on_attacker(PlayerbotAI* ai) { return new CastInsectSwarmOnAttackerAction(ai); }
    static Action* moonfire_on_attacker(PlayerbotAI* ai) { return new CastMoonfireOnAttackerAction(ai); }
    static Action* enrage(PlayerbotAI* ai) { return new CastEnrageAction(ai); }
    static Action* force_of_nature(PlayerbotAI* ai) { return new CastForceOfNatureAction(ai); }
    static Action* typhoon(PlayerbotAI* ai) { return new CastTyphoonAction(ai); }
    static Action* starsurge(PlayerbotAI* ai) { return new CastStarsurgeAction(ai); }
    static Action* celestial_alignment(PlayerbotAI* ai) { return new CastCelestialAlignmentAction(ai); }
    static Action* natures_vigil(PlayerbotAI* ai) { return new CastNaturesVigilAction(ai); }
    static Action* might_of_ursoc(PlayerbotAI* ai) { return new CastMightOfUrsocAction(ai); }
    static Action* savage_defense(PlayerbotAI* ai) { return new CastSavageDefenseAction(ai); }
    static Action* ironbark_on_main_tank(PlayerbotAI* ai) { return new CastIronbarkOnMainTankAction(ai); }
    static Action* skull_bash_on_enemy_caster(PlayerbotAI* ai) { return new CastSkullBashOnEnemyCasterAction(ai); }
    static Action* rejuvenation_blanket(PlayerbotAI* ai) { return new CastRejuvenationBlanketAction(ai); }
    static Action* wild_growth_blanket(PlayerbotAI* ai) { return new CastWildGrowthBlanketAction(ai); }
};

DruidAiObjectContext::DruidAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new DruidStrategyFactoryInternal());
    strategyContexts.Add(new DruidDruidStrategyFactoryInternal());
    actionContexts.Add(new DruidAiObjectContextInternal());
    triggerContexts.Add(new DruidTriggerFactoryInternal());
}
