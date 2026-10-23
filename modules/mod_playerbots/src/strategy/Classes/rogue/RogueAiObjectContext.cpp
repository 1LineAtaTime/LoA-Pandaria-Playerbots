/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RogueAiObjectContext.h"

#include "AssassinationRogueStrategy.h"
#include "DpsRogueStrategy.h"
#include "GenericRogueNonCombatStrategy.h"
#include "NamedObjectContext.h"
#include "Playerbots.h"
#include "PullStrategy.h"
#include "RogueActions.h"
#include "RogueComboActions.h"
#include "RogueFinishingActions.h"
#include "RogueOpeningActions.h"
#include "RogueTriggers.h"

class RogueStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    RogueStrategyFactoryInternal()
    {
        creators["nc"] = &RogueStrategyFactoryInternal::nc;
        creators["pull"] = &RogueStrategyFactoryInternal::pull;
        creators["aoe"] = &RogueStrategyFactoryInternal::aoe;
        creators["boost"] = &RogueStrategyFactoryInternal::boost;
        creators["stealthed"] = &RogueStrategyFactoryInternal::stealthed;
        creators["stealth"] = &RogueStrategyFactoryInternal::stealth;
        creators["cc"] = &RogueStrategyFactoryInternal::cc;
        // #593 (AC Ai/Base/StrategyContext.h:105,139): "behind" (SetBehindCombatStrategy) was never registered, so
        // AiFactory's addStrategy("behind") was a silent no-op and Backstab/Ambush/Garrote failed NOT_BEHIND.
        creators["behind"] = &RogueStrategyFactoryInternal::behind;
    }

private:
    static Strategy* boost(PlayerbotAI* botAI) { return new RogueBoostStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new RogueAoeStrategy(botAI); }
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericRogueNonCombatStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
    static Strategy* stealthed(PlayerbotAI* botAI) { return new StealthedRogueStrategy(botAI); }
    static Strategy* stealth(PlayerbotAI* botAI) { return new StealthStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new RogueCcStrategy(botAI); }
    static Strategy* behind(PlayerbotAI* botAI) { return new SetBehindCombatStrategy(botAI); }  // #593
};

class RogueCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    RogueCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["dps"] = &RogueCombatStrategyFactoryInternal::dps;
        creators["melee"] = &RogueCombatStrategyFactoryInternal::melee;
        // #386 (AC RogueAiObjectContext.cpp:50-51): AC's spec keys; the lead's AiFactory picks them.
        creators["combat"] = &RogueCombatStrategyFactoryInternal::dps;
        creators["assassin"] = &RogueCombatStrategyFactoryInternal::melee;
    }

private:
    static Strategy* dps(PlayerbotAI* botAI) { return new DpsRogueStrategy(botAI); }
    static Strategy* melee(PlayerbotAI* botAI) { return new AssassinationRogueStrategy(botAI); }
};

class RogueTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    RogueTriggerFactoryInternal()
    {
        // #357 slice 2: this context registered 1 of the 18 names AC registers (AC
        // Ai/Class/Rogue/RogueAiObjectContext.cpp:66-87). All 18 are registered below with AC's exact
        // creator names and factory shapes; AC's numbering is in the trailing comments.
        // #149 (AC :76): DpsRogueStrategy.cpp already references "no stealth"; only the registration was missing.
        creators["no stealth"] = &RogueTriggerFactoryInternal::no_stealth;                        // AC :76
        creators["kick"] = &RogueTriggerFactoryInternal::kick;                                      // AC :66
        creators["riposte"] = &RogueTriggerFactoryInternal::riposte;                                // AC :67
        creators["rupture"] = &RogueTriggerFactoryInternal::rupture;                                // AC :68
        creators["slice and dice"] = &RogueTriggerFactoryInternal::slice_and_dice;                  // AC :69
        creators["hunger for blood"] = &RogueTriggerFactoryInternal::hunger_for_blood;              // AC :70
        creators["expose armor"] = &RogueTriggerFactoryInternal::expose_armor;                      // AC :71
        creators["kick on enemy healer"] = &RogueTriggerFactoryInternal::kick_on_enemy_healer;      // AC :72
        creators["unstealth"] = &RogueTriggerFactoryInternal::unstealth;                            // AC :73
        creators["sap"] = &RogueTriggerFactoryInternal::sap;                                        // AC :74
        creators["in stealth"] = &RogueTriggerFactoryInternal::in_stealth;                          // AC :75
        creators["stealth"] = &RogueTriggerFactoryInternal::stealth;                                // AC :77
        creators["sprint"] = &RogueTriggerFactoryInternal::sprint;                                  // AC :78
        creators["main hand weapon no enchant"] = &RogueTriggerFactoryInternal::main_hand_weapon_no_enchant;  // AC :79
        creators["off hand weapon no enchant"] = &RogueTriggerFactoryInternal::off_hand_weapon_no_enchant;    // AC :81
        creators["tricks of the trade on main tank"] = &RogueTriggerFactoryInternal::tricks_of_the_trade_on_main_tank;  // AC :83
        creators["adrenaline rush"] = &RogueTriggerFactoryInternal::adrenaline_rush;                // AC :85
        creators["blade flurry"] = &RogueTriggerFactoryInternal::blade_flurry;                      // AC :86

        // Rotation port epic #854 / issue #859 (MoP rogue rows; beyond AC, MoP-only spells).
        creators["revealing strike"] = &RogueTriggerFactoryInternal::revealing_strike;
        creators["hemorrhage"] = &RogueTriggerFactoryInternal::hemorrhage;
        creators["shadowstep"] = &RogueTriggerFactoryInternal::shadowstep;
        creators["marked for death"] = &RogueTriggerFactoryInternal::marked_for_death;
        creators["vendetta"] = &RogueTriggerFactoryInternal::vendetta;
        creators["recuperate"] = &RogueTriggerFactoryInternal::recuperate;
        creators["preparation"] = &RogueTriggerFactoryInternal::preparation;
        creators["shadow blades"] = &RogueTriggerFactoryInternal::shadow_blades;

        // BEYOND-AC, NOT AC PARITY. DpsRogueStrategy.cpp:101,217 reference "combo points available", but AC
        // registers only "combo points 3/4/5 available" (AC Ai/Base/TriggerContext.h:120-122) - never the bare
        // name. AC's ComboPointsAvailableTrigger does take this name as its StatAvailable stat
        // (AC GenericTriggers.h:105-114), so the registration is the same shape, just a name AC does not use.
        // The correct home for this one is the base TriggerContext (druid CatDpsDruidStrategy.cpp:154 needs it
        // too); it is filed there rather than here because TriggerContext.h is outside this slice's ownership.
    }

private:
    static Trigger* kick(PlayerbotAI* botAI) { return new KickInterruptSpellTrigger(botAI); }
    static Trigger* riposte(PlayerbotAI* botAI) { return new RiposteAvailableTrigger(botAI); }
    static Trigger* rupture(PlayerbotAI* botAI) { return new RuptureTrigger(botAI); }
    static Trigger* slice_and_dice(PlayerbotAI* botAI) { return new SliceAndDiceTrigger(botAI); }
    static Trigger* hunger_for_blood(PlayerbotAI* botAI) { return new HungerForBloodTrigger(botAI); }
    static Trigger* expose_armor(PlayerbotAI* botAI) { return new ExposeArmorTrigger(botAI); }
    static Trigger* kick_on_enemy_healer(PlayerbotAI* botAI) { return new KickInterruptEnemyHealerSpellTrigger(botAI); }
    static Trigger* unstealth(PlayerbotAI* botAI) { return new UnstealthTrigger(botAI); }
    static Trigger* sap(PlayerbotAI* botAI) { return new SapTrigger(botAI); }
    static Trigger* in_stealth(PlayerbotAI* botAI) { return new InStealthTrigger(botAI); }
    static Trigger* no_stealth(PlayerbotAI* botAI) { return new NoStealthTrigger(botAI); }
    static Trigger* stealth(PlayerbotAI* botAI) { return new StealthTrigger(botAI); }
    static Trigger* sprint(PlayerbotAI* botAI) { return new SprintTrigger(botAI); }
    static Trigger* main_hand_weapon_no_enchant(PlayerbotAI* botAI) { return new MainHandWeaponNoEnchantTrigger(botAI); }
    static Trigger* off_hand_weapon_no_enchant(PlayerbotAI* botAI) { return new OffHandWeaponNoEnchantTrigger(botAI); }
    static Trigger* tricks_of_the_trade_on_main_tank(PlayerbotAI* botAI) { return new TricksOfTheTradeOnMainTankTrigger(botAI); }
    static Trigger* adrenaline_rush(PlayerbotAI* botAI) { return new AdrenalineRushTrigger(botAI); }
    static Trigger* blade_flurry(PlayerbotAI* botAI) { return new BladeFlurryTrigger(botAI); }
    static Trigger* revealing_strike(PlayerbotAI* botAI) { return new RevealingStrikeTrigger(botAI); }
    static Trigger* hemorrhage(PlayerbotAI* botAI) { return new HemorrhageTrigger(botAI); }
    static Trigger* shadowstep(PlayerbotAI* botAI) { return new ShadowstepTrigger(botAI); }
    static Trigger* marked_for_death(PlayerbotAI* botAI) { return new MarkedForDeathTrigger(botAI); }
    static Trigger* vendetta(PlayerbotAI* botAI) { return new VendettaTrigger(botAI); }
    static Trigger* recuperate(PlayerbotAI* botAI) { return new RecuperateTrigger(botAI); }
    static Trigger* preparation(PlayerbotAI* botAI) { return new PreparationTrigger(botAI); }
    static Trigger* shadow_blades(PlayerbotAI* botAI) { return new ShadowBladesTrigger(botAI); }
};

class RogueAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    RogueAiObjectContextInternal()
    {
        creators["riposte"] = &RogueAiObjectContextInternal::riposte;
        creators["mutilate"] = &RogueAiObjectContextInternal::mutilate;
        creators["sinister strike"] = &RogueAiObjectContextInternal::sinister_strike;
        creators["gouge"] = &RogueAiObjectContextInternal::gouge;
        creators["kidney shot"] = &RogueAiObjectContextInternal::kidney_shot;
        creators["rupture"] = &RogueAiObjectContextInternal::rupture;
        creators["slice and dice"] = &RogueAiObjectContextInternal::slice_and_dice;
        creators["hunger for blood"] = &RogueAiObjectContextInternal::hunger_for_blood;
        creators["eviscerate"] = &RogueAiObjectContextInternal::eviscerate;
        creators["vanish"] = &RogueAiObjectContextInternal::vanish;
        creators["evasion"] = &RogueAiObjectContextInternal::evasion;
        creators["cloak of shadows"] = &RogueAiObjectContextInternal::cloak_of_shadows;
        creators["kick"] = &RogueAiObjectContextInternal::kick;
        creators["feint"] = &RogueAiObjectContextInternal::feint;
        creators["backstab"] = &RogueAiObjectContextInternal::backstab;
        creators["expose armor"] = &RogueAiObjectContextInternal::expose_armor;
        creators["kick on enemy healer"] = &RogueAiObjectContextInternal::kick_on_enemy_healer;
        creators["blade flurry"] = &RogueAiObjectContextInternal::blade_flurry;
        creators["adrenaline rush"] = &RogueAiObjectContextInternal::adrenaline_rush;
        creators["ambush"] = &RogueAiObjectContextInternal::ambush;
        creators["stealth"] = &RogueAiObjectContextInternal::stealth;
        creators["sprint"] = &RogueAiObjectContextInternal::sprint;
        creators["garrote"] = &RogueAiObjectContextInternal::garrote;
        creators["cheap shot"] = &RogueAiObjectContextInternal::cheap_shot;
        creators["blind"] = &RogueAiObjectContextInternal::blind;
        creators["unstealth"] = &RogueAiObjectContextInternal::unstealth;
        creators["sap"] = &RogueAiObjectContextInternal::sap;
        creators["check stealth"] = &RogueAiObjectContextInternal::check_stealth;
        creators["envenom"] = &RogueAiObjectContextInternal::envenom;
        creators["tricks of the trade on main tank"] = &RogueAiObjectContextInternal::tricks_of_the_trade_on_main_tank;
        creators["fan of knives"] = &RogueAiObjectContextInternal::fan_of_knives;
        creators["killing spree"] = &RogueAiObjectContextInternal::killing_spree;
        // 5.4.8: replaces AC's item-based "use instant/deadly poison on main/off hand" (UseItemAction)
        // with a single self-cast poison buff spell -- MoP poisons have no per-hand item mechanic (#149).
        creators["deadly poison"] = &RogueAiObjectContextInternal::deadly_poison;
        // #386 (AC GenericRogueNonCombatStrategy.cpp:29-37 applies a second poison): 5.x form is the non-lethal
        // Crippling Poison self-buff, spell 3408 (cls.py rogue YES 3408/skill/L20; Wowhead mop-classic tooltip).
        creators["crippling poison"] = &RogueAiObjectContextInternal::crippling_poison;
        // Rotation port epic #854 / issue #859 (MoP rogue rows).
        creators["revealing strike"] = &RogueAiObjectContextInternal::revealing_strike;
        creators["hemorrhage"] = &RogueAiObjectContextInternal::hemorrhage;
        creators["dispatch"] = &RogueAiObjectContextInternal::dispatch;
        creators["shadowstep"] = &RogueAiObjectContextInternal::shadowstep;
        creators["smoke bomb"] = &RogueAiObjectContextInternal::smoke_bomb;
        creators["recuperate"] = &RogueAiObjectContextInternal::recuperate;
        creators["shadow blades"] = &RogueAiObjectContextInternal::shadow_blades;
        creators["vendetta"] = &RogueAiObjectContextInternal::vendetta;
        creators["marked for death"] = &RogueAiObjectContextInternal::marked_for_death;
        creators["preparation"] = &RogueAiObjectContextInternal::preparation;
        creators["kick on enemy caster"] = &RogueAiObjectContextInternal::kick_on_enemy_caster;
    }

private:
    static Action* adrenaline_rush(PlayerbotAI* botAI) { return new CastAdrenalineRushAction(botAI); }
    static Action* blade_flurry(PlayerbotAI* botAI) { return new CastBladeFlurryAction(botAI); }
    static Action* riposte(PlayerbotAI* botAI) { return new CastRiposteAction(botAI); }
    static Action* mutilate(PlayerbotAI* botAI) { return new CastMutilateAction(botAI); }
    static Action* sinister_strike(PlayerbotAI* botAI) { return new CastSinisterStrikeAction(botAI); }
    static Action* gouge(PlayerbotAI* botAI) { return new CastGougeAction(botAI); }
    static Action* kidney_shot(PlayerbotAI* botAI) { return new CastKidneyShotAction(botAI); }
    static Action* rupture(PlayerbotAI* botAI) { return new CastRuptureAction(botAI); }
    static Action* slice_and_dice(PlayerbotAI* botAI) { return new CastSliceAndDiceAction(botAI); }
    static Action* hunger_for_blood(PlayerbotAI* botAI) { return new CastHungerForBloodAction(botAI); }
    static Action* eviscerate(PlayerbotAI* botAI) { return new CastEviscerateAction(botAI); }
    static Action* vanish(PlayerbotAI* botAI) { return new CastVanishAction(botAI); }
    static Action* evasion(PlayerbotAI* botAI) { return new CastEvasionAction(botAI); }
    static Action* cloak_of_shadows(PlayerbotAI* botAI) { return new CastCloakOfShadowsAction(botAI); }
    static Action* kick(PlayerbotAI* botAI) { return new CastKickAction(botAI); }
    static Action* feint(PlayerbotAI* botAI) { return new CastFeintAction(botAI); }
    static Action* backstab(PlayerbotAI* botAI) { return new CastBackstabAction(botAI); }
    static Action* expose_armor(PlayerbotAI* botAI) { return new CastExposeArmorAction(botAI); }
    static Action* kick_on_enemy_healer(PlayerbotAI* botAI) { return new CastKickOnEnemyHealerAction(botAI); }
    static Action* ambush(PlayerbotAI* botAI) { return new CastAmbushAction(botAI); }
    static Action* stealth(PlayerbotAI* botAI) { return new CastStealthAction(botAI); }
    static Action* sprint(PlayerbotAI* botAI) { return new CastSprintAction(botAI); }
    static Action* garrote(PlayerbotAI* botAI) { return new CastGarroteAction(botAI); }
    static Action* cheap_shot(PlayerbotAI* botAI) { return new CastCheapShotAction(botAI); }
    static Action* blind(PlayerbotAI* botAI) { return new CastBlindAction(botAI); }
    static Action* check_stealth(PlayerbotAI* botAI) { return new CheckStealthAction(botAI); }
    static Action* sap(PlayerbotAI* botAI) { return new CastSapAction(botAI); }
    static Action* unstealth(PlayerbotAI* botAI) { return new UnstealthAction(botAI); }
    static Action* envenom(PlayerbotAI* ai) { return new EnvenomAction(ai); }
    static Action* tricks_of_the_trade_on_main_tank(PlayerbotAI* ai)
    {
        return new CastTricksOfTheTradeOnMainTankAction(ai);
    }

    static Action* fan_of_knives(PlayerbotAI* ai) { return new FanOfKnivesAction(ai); }
    static Action* killing_spree(PlayerbotAI* ai) { return new CastKillingSpreeAction(ai); }
    static Action* deadly_poison(PlayerbotAI* ai) { return new CastBuffSpellAction(ai, "deadly poison"); }
    static Action* crippling_poison(PlayerbotAI* ai) { return new CastBuffSpellAction(ai, "crippling poison"); }
    static Action* revealing_strike(PlayerbotAI* ai) { return new CastRevealingStrikeAction(ai); }
    static Action* hemorrhage(PlayerbotAI* ai) { return new CastHemorrhageAction(ai); }
    static Action* dispatch(PlayerbotAI* ai) { return new CastDispatchAction(ai); }
    static Action* shadowstep(PlayerbotAI* ai) { return new CastShadowstepAction(ai); }
    static Action* smoke_bomb(PlayerbotAI* ai) { return new CastSmokeBombAction(ai); }
    static Action* recuperate(PlayerbotAI* ai) { return new CastRecuperateAction(ai); }
    static Action* shadow_blades(PlayerbotAI* ai) { return new CastShadowBladesAction(ai); }
    static Action* vendetta(PlayerbotAI* ai) { return new CastVendettaAction(ai); }
    static Action* marked_for_death(PlayerbotAI* ai) { return new CastMarkedForDeathAction(ai); }
    static Action* preparation(PlayerbotAI* ai) { return new CastPreparationAction(ai); }
    static Action* kick_on_enemy_caster(PlayerbotAI* ai) { return new CastKickOnEnemyCasterAction(ai); }
};

RogueAiObjectContext::RogueAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new RogueStrategyFactoryInternal());
    strategyContexts.Add(new RogueCombatStrategyFactoryInternal());
    actionContexts.Add(new RogueAiObjectContextInternal());
    triggerContexts.Add(new RogueTriggerFactoryInternal());
}
