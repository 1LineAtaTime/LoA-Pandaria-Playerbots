/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MageAiObjectContext.h"

#include "ArcaneMageStrategy.h"
#include "FireMageStrategy.h"
#include "FrostFireMageStrategy.h"
#include "FrostMageStrategy.h"
#include "GenericMageNonCombatStrategy.h"
#include "MageActions.h"
#include "MageTriggers.h"
#include "NamedObjectContext.h"
#include "Playerbots.h"
#include "PullStrategy.h"

/*
* TODO
* aoe => flamestrike / arcane explo / blizzard etc
*/

class MageStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    MageStrategyFactoryInternal()
    {
        creators["nc"] = &MageStrategyFactoryInternal::nc;
        creators["pull"] = &MageStrategyFactoryInternal::pull;
        creators["fire aoe"] = &MageStrategyFactoryInternal::fire_aoe;
        creators["frost aoe"] = &MageStrategyFactoryInternal::frost_aoe;
        creators["arcane aoe"] = &MageStrategyFactoryInternal::arcane_aoe;
        // #408 (AC parity, MageAiObjectContext.cpp:26-30): the four strategies whose classes were
        // never ported to this fork are now in GenericMageStrategy.h, so the commented creators and
        // methods below are live. "aoe" is AC's spec-agnostic MageAoeStrategy; MoP's per-spec
        // "arcane/fire/frost aoe" strategies stay registered alongside it.
        creators["aoe"] = &MageStrategyFactoryInternal::aoe;
        creators["cure"] = &MageStrategyFactoryInternal::cure;
        creators["buff"] = &MageStrategyFactoryInternal::buff;
        creators["boost"] = &MageStrategyFactoryInternal::boost;
        creators["cc"] = &MageStrategyFactoryInternal::cc;
        // #385 (AC MageAiObjectContext.cpp:31,42): "firestarter", not in any default strategy list (AC AiFactory).
        creators["firestarter"] = &MageStrategyFactoryInternal::firestarter;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericMageNonCombatStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
    static Strategy* fire_aoe(PlayerbotAI* botAI) { return new FireMageAoeStrategy(botAI); }
    static Strategy* frost_aoe(PlayerbotAI* botAI) { return new FrostMageAoeStrategy(botAI); }
    static Strategy* arcane_aoe(PlayerbotAI* botAI) { return new ArcaneMageAoeStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new MageAoeStrategy(botAI); }
    static Strategy* cure(PlayerbotAI* botAI) { return new MageCureStrategy(botAI); }
    static Strategy* buff(PlayerbotAI* botAI) { return new MageBuffStrategy(botAI); }
    static Strategy* boost(PlayerbotAI* botAI) { return new MageBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new MageCcStrategy(botAI); }
    static Strategy* firestarter(PlayerbotAI* botAI) { return new FirestarterStrategy(botAI); }
};

class MageCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    MageCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["frost"] = &MageCombatStrategyFactoryInternal::frost;
        creators["fire"] = &MageCombatStrategyFactoryInternal::fire;
        creators["arcane"] = &MageCombatStrategyFactoryInternal::arcane;
        // #408 (AC parity, MageAiObjectContext.cpp:52). AiFactory never selects it on 5.4.8 - its Ice Shards talent
        // gate has no 5.4.8 talent (#385 lane M: Frostfire Bolt 44614 itself is a level-1 5.4.8 spell).
        creators["frostfire"] = &MageCombatStrategyFactoryInternal::frostfire;
    }

private:
    static Strategy* frost(PlayerbotAI* botAI) { return new FrostMageStrategy(botAI); }
    static Strategy* fire(PlayerbotAI* botAI) { return new FireMageStrategy(botAI); }
    static Strategy* arcane(PlayerbotAI* botAI) { return new ArcaneMageStrategy(botAI); }
    static Strategy* frostfire(PlayerbotAI* botAI) { return new FrostFireMageStrategy(botAI); }
};

class MageBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    MageBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bmana"] = &MageBuffStrategyFactoryInternal::bmana;
        creators["bdps"] = &MageBuffStrategyFactoryInternal::bdps;
    }

private:
    static Strategy* bmana(PlayerbotAI* botAI) { return new MageBuffManaStrategy(botAI); }
    static Strategy* bdps(PlayerbotAI* botAI) { return new MageBuffDpsStrategy(botAI); }
};

class MageTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    MageTriggerFactoryInternal()
    {
        creators["arcane brilliance"] = &MageTriggerFactoryInternal::arcane_brilliance;
        creators["arcane brilliance on party"] = &MageTriggerFactoryInternal::arcane_brilliance_on_party;
        creators["rune of power"] = &MageTriggerFactoryInternal::rune_of_power;
        creators["mage armor"] = &MageTriggerFactoryInternal::mage_armor;
        creators["frost armor"] = &MageTriggerFactoryInternal::frost_armor;
        creators["molten armor"] = &MageTriggerFactoryInternal::molten_armor;
        creators["fire ward"] = &MageTriggerFactoryInternal::fire_ward;
        creators["frost ward"] = &MageTriggerFactoryInternal::frost_ward;
        creators["living bomb"] = &MageTriggerFactoryInternal::living_bomb;
        creators["living bomb on attackers"] = &MageTriggerFactoryInternal::living_bomb_on_attackers;
        creators["missile barrage"] = &MageTriggerFactoryInternal::missile_barrage;
        creators["presence of mind"] = &MageTriggerFactoryInternal::presence_of_mind;
        creators["frost nova on target"] = &MageTriggerFactoryInternal::frost_nova_on_target;
        creators["counterspell"] = &MageTriggerFactoryInternal::counterspell;
        creators["spellsteal"] = &MageTriggerFactoryInternal::spellsteal;
        creators["mirror image"] = &MageTriggerFactoryInternal::mirror_image;

        // -- fire
        creators["heating up"] = &MageTriggerFactoryInternal::heating_up;
        creators["pyroblast!"] = &MageTriggerFactoryInternal::pyroblast;
        creators["combustion"] = &MageTriggerFactoryInternal::combustion;

        // -- forst
        creators["icy veins"] = &MageTriggerFactoryInternal::icy_veins;
        creators["fingers of frost single"] = &MageTriggerFactoryInternal::fingers_of_frost_single;
        creators["fingers of frost double"] = &MageTriggerFactoryInternal::fingers_of_frost_double;
        creators["brain freeze"] = &MageTriggerFactoryInternal::brain_freeze;
        creators["frost bomb"] = &MageTriggerFactoryInternal::frost_bomb;  // #521: key was the typo "forst bomb"

        // -- arcane
        creators["arcane blast"] = &MageTriggerFactoryInternal::arcane_blast;
        creators["arcane blast stack"] = &MageTriggerFactoryInternal::arcane_blast_stack;
        creators["arcane power"] = &MageTriggerFactoryInternal::arcane_power;
        creators["arcane missiles"] = &MageTriggerFactoryInternal::arcane_missiles_aura;
        creators["arcane charge"] = &MageTriggerFactoryInternal::arcane_charge_aura;
        creators["slow not on target"] = &MageTriggerFactoryInternal::slow_not_on_target;
        creators["nether tempest"] = &MageTriggerFactoryInternal::nether_tempest;

        // -- AC WotLK parity (#357 slice 8). Names and factory shapes are AC's verbatim.
        creators["hot streak"] = &MageTriggerFactoryInternal::hot_streak;
        creators["fireball"] = &MageTriggerFactoryInternal::fireball;
        creators["fingers of frost"] = &MageTriggerFactoryInternal::fingers_of_frost;
        creators["cold snap"] = &MageTriggerFactoryInternal::cold_snap;
        creators["ice barrier"] = &MageTriggerFactoryInternal::ice_barrier;
        creators["polymorph"] = &MageTriggerFactoryInternal::polymorph;
        creators["remove curse"] = &MageTriggerFactoryInternal::remove_curse;
        creators["remove curse on party"] = &MageTriggerFactoryInternal::remove_curse_on_party;
        creators["counterspell on enemy healer"] = &MageTriggerFactoryInternal::counterspell_enemy_healer;
        creators["frostbite on target"] = &MageTriggerFactoryInternal::frostbite_on_target;
        creators["frostfire bolt"] = &MageTriggerFactoryInternal::frostfire_bolt;
        creators["no focus magic"] = &MageTriggerFactoryInternal::no_focus_magic;
        creators["flamestrike nearby"] = &MageTriggerFactoryInternal::flamestrike_nearby;
        creators["flamestrike active and medium aoe"] = &MageTriggerFactoryInternal::flamestrike_blizzard;
        creators["arcane blast 4 stacks and missile barrage"] = &MageTriggerFactoryInternal::arcane_blast_4_stacks_and_missile_barrage;
        creators["icy veins on cd"] = &MageTriggerFactoryInternal::icy_veins_on_cd;
        creators["deep freeze on cd"] = &MageTriggerFactoryInternal::deep_freeze_on_cd;
        creators["no mana gem"] = &MageTriggerFactoryInternal::NoManaGem;
        creators["blizzard channel check"] = &MageTriggerFactoryInternal::blizzard_channel_check;
        creators["blast wave off cd"] = &MageTriggerFactoryInternal::blast_wave_off_cd;
        creators["blast wave off cd and medium aoe"] = &MageTriggerFactoryInternal::blast_wave_off_cd_and_medium_aoe;
        creators["no firestarter strategy"] = &MageTriggerFactoryInternal::no_firestarter_strategy;
        // Rotation lane 10 (#865): PvP-gated class-local triggers.
        creators["mage pvp melee enemy player"] = &MageTriggerFactoryInternal::mage_pvp_melee_enemy_player;
        creators["mage pvp enemy caster"] = &MageTriggerFactoryInternal::mage_pvp_enemy_caster;
        creators["mage ice block health"] = &MageTriggerFactoryInternal::mage_ice_block_health;
        creators["mage medium threat"] = &MageTriggerFactoryInternal::mage_medium_threat;
        creators["mage flee close"] = &MageTriggerFactoryInternal::mage_flee_close;
        creators["enemy is close and no firestarter strategy"] = &MageTriggerFactoryInternal::enemy_is_close_and_no_firestarter_strategy;
        creators["enemy too close for spell and no firestarter strategy"] = &MageTriggerFactoryInternal::enemy_too_close_for_spell_and_no_firestarter_strategy;

        // 5.4.8 INERT: AC registers "arcane intellect" / "arcane intellect on party". Arcane Intellect
        // was removed in Cataclysm, so these can never fire; the fork's live equivalents are the
        // already-registered "arcane brilliance" / "arcane brilliance on party".
        creators["arcane intellect"] = &MageTriggerFactoryInternal::arcane_intellect;
        creators["arcane intellect on party"] = &MageTriggerFactoryInternal::arcane_intellect_on_party;

        // 5.4.8 INERT: AC registers "firestarter" (Firestarter talent) and "improved scorch"
        // (Improved Scorch talent). Both talents were removed in Cataclysm.
        creators["firestarter"] = &MageTriggerFactoryInternal::firestarter;
        creators["improved scorch"] = &MageTriggerFactoryInternal::improved_scorch;
    }

private:
    static Trigger* presence_of_mind(PlayerbotAI* botAI) { return new PresenceOfMindTrigger(botAI); }
    static Trigger* arcane_brilliance(PlayerbotAI* botAI) { return new ArcaneBrillianceTrigger(botAI); }
    static Trigger* arcane_brilliance_on_party(PlayerbotAI* botAI) { return new ArcaneBrillianceOnPartyTrigger(botAI); }
    static Trigger* rune_of_power(PlayerbotAI* botAI) { return new RuneOfPowerTrigger(botAI); }
    static Trigger* mage_armor(PlayerbotAI* botAI) { return new MageArmorTrigger(botAI); }
    static Trigger* frost_armor(PlayerbotAI* botAI) { return new FrostArmorTrigger(botAI); }
    static Trigger* molten_armor(PlayerbotAI* botAI) { return new MoltenArmorTrigger(botAI); }
    static Trigger* fire_ward(PlayerbotAI* botAI) { return new FireWardTrigger(botAI); }
    static Trigger* frost_ward(PlayerbotAI* botAI) { return new FrostWardTrigger(botAI); }
    static Trigger* living_bomb(PlayerbotAI* botAI) { return new LivingBombTrigger(botAI); }
    static Trigger* living_bomb_on_attackers(PlayerbotAI* botAI) { return new LivingBombOnAttackersTrigger(botAI); }
    static Trigger* frost_nova_on_target(PlayerbotAI* botAI) { return new FrostNovaOnTargetTrigger(botAI); }
    static Trigger* counterspell(PlayerbotAI* botAI) { return new CounterspellInterruptSpellTrigger(botAI); }
    static Trigger* spellsteal(PlayerbotAI* botAI) { return new SpellstealTrigger(botAI); }
    static Trigger* mirror_image(PlayerbotAI* botAI) { return new MirrorImageTrigger(botAI); }

    // -- fire
    static Trigger* heating_up(PlayerbotAI* botAI) { return new HeatingUpTrigger(botAI); }
    static Trigger* pyroblast(PlayerbotAI* botAI) { return new PyroblastTrigger(botAI); }
    static Trigger* combustion(PlayerbotAI* botAI) { return new CombustionTrigger(botAI); }

    // -- frost
    static Trigger* icy_veins(PlayerbotAI* botAI) { return new IcyVeinsTrigger(botAI); }
    static Trigger* fingers_of_frost_single(PlayerbotAI* botAI) { return new FingersOfFrostSingleTrigger(botAI); }
    static Trigger* fingers_of_frost_double(PlayerbotAI* botAI) { return new FingersOfFrostDoubleTrigger(botAI); }
    static Trigger* brain_freeze(PlayerbotAI* botAI) { return new BrainFreezeTrigger(botAI); }
    static Trigger* frost_bomb(PlayerbotAI* botAI) { return new FrostBombTrigger(botAI); }

    // -- arcane
    static Trigger* arcane_power(PlayerbotAI* botAI) { return new ArcanePowerTrigger(botAI); }
    static Trigger* missile_barrage(PlayerbotAI* botAI) { return new MissileBarrageTrigger(botAI); }
    static Trigger* arcane_blast(PlayerbotAI* botAI) { return new ArcaneBlastTrigger(botAI); }
    static Trigger* arcane_blast_stack(PlayerbotAI* botAI) { return new ArcaneBlastStackTrigger(botAI); }
    static Trigger* arcane_missiles_aura(PlayerbotAI* botAI) { return new ArcaneMissilesTrigger(botAI); }
    static Trigger* arcane_charge_aura(PlayerbotAI* botAI) { return new ArcaneChargeTrigger(botAI); }
    static Trigger* slow_not_on_target(PlayerbotAI* botAI) { return new SlowNotOnTargetTrigger(botAI); }
    static Trigger* nether_tempest(PlayerbotAI* botAI) { return new NetherTempestTrigger(botAI); }

    // -- AC WotLK parity (#357 slice 8)
    static Trigger* hot_streak(PlayerbotAI* botAI) { return new HotStreakTrigger(botAI); }
    static Trigger* fireball(PlayerbotAI* botAI) { return new FireballTrigger(botAI); }
    static Trigger* fingers_of_frost(PlayerbotAI* botAI) { return new FingersOfFrostTrigger(botAI); }
    static Trigger* cold_snap(PlayerbotAI* botAI) { return new ColdSnapTrigger(botAI); }
    static Trigger* ice_barrier(PlayerbotAI* botAI) { return new IceBarrierTrigger(botAI); }
    static Trigger* polymorph(PlayerbotAI* botAI) { return new PolymorphTrigger(botAI); }
    static Trigger* remove_curse(PlayerbotAI* botAI) { return new RemoveCurseTrigger(botAI); }
    static Trigger* remove_curse_on_party(PlayerbotAI* botAI) { return new PartyMemberRemoveCurseTrigger(botAI); }
    static Trigger* counterspell_enemy_healer(PlayerbotAI* botAI) { return new CounterspellEnemyHealerTrigger(botAI); }
    static Trigger* frostbite_on_target(PlayerbotAI* botAI) { return new FrostbiteOnTargetTrigger(botAI); }
    static Trigger* frostfire_bolt(PlayerbotAI* botAI) { return new FrostfireBoltTrigger(botAI); }
    static Trigger* no_focus_magic(PlayerbotAI* botAI) { return new NoFocusMagicTrigger(botAI); }
    static Trigger* flamestrike_nearby(PlayerbotAI* botAI) { return new FlamestrikeNearbyTrigger(botAI); }
    static Trigger* flamestrike_blizzard(PlayerbotAI* botAI) { return new FlamestrikeBlizzardTrigger(botAI); }
    static Trigger* arcane_blast_4_stacks_and_missile_barrage(PlayerbotAI* botAI) { return new ArcaneBlast4StacksAndMissileBarrageTrigger(botAI); }
    static Trigger* icy_veins_on_cd(PlayerbotAI* botAI) { return new IcyVeinsCooldownTrigger(botAI); }
    static Trigger* deep_freeze_on_cd(PlayerbotAI* botAI) { return new DeepFreezeCooldownTrigger(botAI); }
    static Trigger* NoManaGem(PlayerbotAI* botAI) { return new NoManaGemTrigger(botAI); }
    static Trigger* blizzard_channel_check(PlayerbotAI* botAI) { return new BlizzardChannelCheckTrigger(botAI); }
    static Trigger* blast_wave_off_cd(PlayerbotAI* botAI) { return new BlastWaveOffCdTrigger(botAI); }
    static Trigger* blast_wave_off_cd_and_medium_aoe(PlayerbotAI* botAI) { return new BlastWaveOffCdTriggerAndMediumAoeTrigger(botAI); }
    static Trigger* no_firestarter_strategy(PlayerbotAI* botAI) { return new NoFirestarterStrategyTrigger(botAI); }
    static Trigger* mage_pvp_melee_enemy_player(PlayerbotAI* botAI) { return new MagePvpEnemyPlayerInMeleeRangeTrigger(botAI); }
    static Trigger* mage_pvp_enemy_caster(PlayerbotAI* botAI) { return new MagePvpEnemyCasterInRangeTrigger(botAI); }
    static Trigger* mage_ice_block_health(PlayerbotAI* botAI) { return new MageIceBlockHealthTrigger(botAI); }
    static Trigger* mage_medium_threat(PlayerbotAI* botAI) { return new MageMediumThreatTrigger(botAI); }
    static Trigger* mage_flee_close(PlayerbotAI* botAI) { return new MageFleeCloseTrigger(botAI); }
    static Trigger* enemy_is_close_and_no_firestarter_strategy(PlayerbotAI* botAI) { return new EnemyIsCloseAndNoFirestarterStrategyTrigger(botAI); }
    static Trigger* enemy_too_close_for_spell_and_no_firestarter_strategy(PlayerbotAI* botAI) { return new EnemyTooCloseForSpellAndNoFirestarterStrategyTrigger(botAI); }
    static Trigger* arcane_intellect(PlayerbotAI* botAI) { return new ArcaneIntellectTrigger(botAI); }
    static Trigger* arcane_intellect_on_party(PlayerbotAI* botAI) { return new ArcaneIntellectOnPartyTrigger(botAI); }
    static Trigger* firestarter(PlayerbotAI* botAI) { return new FirestarterTrigger(botAI); }
    static Trigger* improved_scorch(PlayerbotAI* botAI) { return new ImprovedScorchTrigger(botAI); }
};

class MageAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    MageAiObjectContextInternal()
    {
        // -- generic needs actions
        creators["molten armor"] = &MageAiObjectContextInternal::molten_armor;
        creators["mage armor"] = &MageAiObjectContextInternal::mage_armor;
        creators["frost armor"] = &MageAiObjectContextInternal::frost_armor;
        creators["arcane brilliance"] = &MageAiObjectContextInternal::arcane_brilliance;
        creators["arcane brilliance on party"] = &MageAiObjectContextInternal::arcane_brilliance_on_party;
        creators["rune of power"] = &MageAiObjectContextInternal::rune_of_power;

        creators["frostfire bolt"] = &MageAiObjectContextInternal::frostfirebolt;
        creators["frost nova"] = &MageAiObjectContextInternal::frostnova;
        creators["fire blast"] = &MageAiObjectContextInternal::fireblast;
        creators["counterspell"] = &MageAiObjectContextInternal::counterspell;
        creators["counterspell on enemy healer"] = &MageAiObjectContextInternal::counterspell_on_enemy_healer;
        creators["spellsteal"] = &MageAiObjectContextInternal::spellsteal;
        creators["arcane explosion"] = &MageAiObjectContextInternal::arcaneexplosion;
        creators["blizzard"] = &MageAiObjectContextInternal::blizzard;
        creators["ice lance"] = &MageAiObjectContextInternal::icelance;
        creators["arcane missiles"] = &MageAiObjectContextInternal::arcanemissiles;
        creators["cone of cold"] = &MageAiObjectContextInternal::coneofcold;
        creators["remove curse"] = &MageAiObjectContextInternal::removecurse;
        creators["remove curse on party"] = &MageAiObjectContextInternal::removecurseonparty;
        // 5.4.8: no separate "lesser curse" spell exists (single "Remove Curse" mechanic); alias to it.
        creators["remove lesser curse"] = &MageAiObjectContextInternal::removecurse;
        creators["remove lesser curse on party"] = &MageAiObjectContextInternal::removecurseonparty;
        creators["evocation"] = &MageAiObjectContextInternal::evocation;
        creators["deep freeze"] = &MageAiObjectContextInternal::deepfreeze;
        creators["ice block"] = &MageAiObjectContextInternal::ice_block;
        creators["blink"] = &MageAiObjectContextInternal::blink;
        creators["frostjaw"] = &MageAiObjectContextInternal::frostjaw;
        creators["invisibility"] = &MageAiObjectContextInternal::invisibility;
        creators["mirror image"] = &MageAiObjectContextInternal::mirror_image;
        creators["fire ward"] = &MageAiObjectContextInternal::fire_ward;
        creators["frost ward"] = &MageAiObjectContextInternal::frost_ward;

        // #408 (AC parity, MageAiObjectContext.cpp:220,225,236,243): these four action names are
        // referenced by the newly ported MageCcStrategy / MageAoeStrategy triggers and by
        // GenericMageStrategy's defensive nodes, but had no creator here, so they could never
        // resolve. CastPolymorphAction and CastBlinkAction already existed; the classes themselves
        // did not - see MageActions.h.
        creators["polymorph"] = &MageAiObjectContextInternal::polymorph;
        creators["incanter's ward"] = &MageAiObjectContextInternal::incanters_ward;  // #385 (was "mana shield")
        creators["conjure mana gem"] = &MageAiObjectContextInternal::conjure_mana_gem;  // #385
        creators["use mana gem"] = &MageAiObjectContextInternal::use_mana_gem;  // #385
        creators["blink back"] = &MageAiObjectContextInternal::blink_back;
        creators["temporal shield"] = &MageAiObjectContextInternal::temporal_shield;  // lane 10 (#865)
        creators["blazing speed"] = &MageAiObjectContextInternal::blazing_speed;
        creators["counterspell on enemy caster"] = &MageAiObjectContextInternal::counterspell_on_enemy_caster;
        creators["blast wave"] = &MageAiObjectContextInternal::blast_wave;

        // #163 S8 (AC parity, MageAiObjectContext.h): AC's "conjure food"/"conjure water" classes
        // were declared (MageActions.h) but never had creators, so they could never be cast even
        // manually. 5.4.8: registered as a single "conjure refreshment" instead - see the
        // CastConjureRefreshmentAction class comment (MageActions.h) for why. Not auto-triggered here
        // - NoFoodTrigger/NoDrinkTrigger (GenericTriggers.cpp) are stubbed to `return false` pending
        // "inventory items"/"conjured food"|"conjured water" qualifier support, a pre-existing gap
        // this pass doesn't take on.
        creators["conjure refreshment"] = &MageAiObjectContextInternal::conjure_refreshment;

        // #163 S8: AC's "focus magic on party" (CastFocusMagicOnPartyAction, MageActions.h:59; spell
        // 54646) is N/A on 5.4.8 - Focus Magic was removed in the Mists of Pandaria mage talent
        // revamp (verified: spell id 54646 has no row in this fork's `spell_dbc`, world DB on
        // the test DB). Not ported; no 5.4.8 successor with the same role.

        // --

        // -- fire specific talent or spec
        creators["fireball"] = &MageAiObjectContextInternal::fireball;
        creators["dragon's breath"] = &MageAiObjectContextInternal::dragon_breath;
        creators["inferno blast"] = &MageAiObjectContextInternal::inferno_blast;
        creators["pyroblast"] = &MageAiObjectContextInternal::pyroblast;
        creators["scorch"] = &MageAiObjectContextInternal::scorch;
        creators["living bomb"] = &MageAiObjectContextInternal::living_bomb;
        creators["living bomb on attackers"] = &MageAiObjectContextInternal::living_bomb_on_attackers;
        creators["flamestrike"] = &MageAiObjectContextInternal::flamestrike;
        creators["combustion"] = &MageAiObjectContextInternal::combustion;


        // -- frost specific talent or spec
        creators["frostbolt"] = &MageAiObjectContextInternal::frostbolt;
        creators["summon water elemental"] = &summon_water_elemental;
        creators["fingers of frost"] = &MageAiObjectContextInternal::fingers_of_frost;
        creators["icy veins"] = &MageAiObjectContextInternal::icy_veins;
        creators["frozen orb"] = &MageAiObjectContextInternal::frozen_orb;
        creators["cold snap"] = &MageAiObjectContextInternal::cold_snap;
        creators["ice barrier"] = &MageAiObjectContextInternal::ice_barrier;
        creators["frost bomb"] = &MageAiObjectContextInternal::frost_bomb;  // #521: key was the typo "forst bomb"
        // #521: the frost/arcane aoe strategies name these; mirror "living bomb on attackers" (same 5.x tier-75
        // talent row: Frost Bomb 112948, Nether Tempest 114923, cls.py mage).
        creators["frost bomb on attackers"] = &MageAiObjectContextInternal::frost_bomb_on_attackers;
        creators["nether tempest on attackers"] = &MageAiObjectContextInternal::nether_tempest_on_attackers;

        // -- arcane
        creators["arcane power"] = &MageAiObjectContextInternal::arcane_power;
        creators["arcane barrage"] = &MageAiObjectContextInternal::arcane_barrage;
        creators["arcane blast"] = &MageAiObjectContextInternal::arcane_blast;
        creators["slow"] = &MageAiObjectContextInternal::slow;
        creators["nether tempest"] = &MageAiObjectContextInternal::nether_tempest;
    }

private:
    static Action* molten_armor(PlayerbotAI* botAI) { return new CastMoltenArmorAction(botAI); }
    static Action* mage_armor(PlayerbotAI* botAI) { return new CastMageArmorAction(botAI); }
    static Action* frost_armor(PlayerbotAI* botAI) { return new CastFrostArmorAction(botAI); }
    static Action* arcane_brilliance(PlayerbotAI* botAI) { return new CastArcaneBrillanceAction(botAI); }
    static Action* arcane_brilliance_on_party(PlayerbotAI* botAI) { return new CastArcaneBrillianceOnPartyAction(botAI); }
    static Action* rune_of_power(PlayerbotAI* botAI) { return new CastRuneOfPowerAction(botAI); }
    static Action* invisibility(PlayerbotAI* botAI) { return new CastInvisibilityAction(botAI); }
    static Action* mirror_image(PlayerbotAI* botAI) { return new CastMirrorImageAction(botAI); }
    static Action* fire_ward(PlayerbotAI* botAI) { return new CastFireWardAction(botAI); }
    static Action* frost_ward(PlayerbotAI* botAI) { return new CastFrostWardAction(botAI); }
    static Action* conjure_refreshment(PlayerbotAI* botAI) { return new CastConjureRefreshmentAction(botAI); }
    
    static Action* frostfirebolt(PlayerbotAI* botAI) { return new CastFrostfireBoltAction(botAI); }
    static Action* frostbolt(PlayerbotAI* botAI) { return new CastFrostboltAction(botAI); }
    static Action* frostnova(PlayerbotAI* botAI) { return new CastFrostNovaAction(botAI); }
    static Action* fireblast(PlayerbotAI* botAI) { return new CastFireBlastAction(botAI); }
    static Action* counterspell(PlayerbotAI* botAI) { return new CastCounterspellAction(botAI); }
    static Action* counterspell_on_enemy_healer(PlayerbotAI* botAI) { return new CastCounterspellOnEnemyHealerAction(botAI); }
    static Action* spellsteal(PlayerbotAI* botAI) { return new CastSpellstealAction(botAI); }
    static Action* arcaneexplosion(PlayerbotAI* botAI) { return new CastArcaneExplosionAction(botAI); }
    static Action* blizzard(PlayerbotAI* botAI) { return new CastBlizzardAction(botAI); }
    static Action* icelance(PlayerbotAI* botAI) { return new CastIceLanceAction(botAI); }
    static Action* arcanemissiles(PlayerbotAI* botAI) { return new CastArcaneMissilesAction(botAI); }
    static Action* coneofcold(PlayerbotAI* botAI) { return new CastConeOfColdAction(botAI); }
    static Action* removecurse(PlayerbotAI* botAI) { return new CastRemoveCurseAction(botAI); }
    static Action* removecurseonparty(PlayerbotAI* botAI) { return new CastRemoveCurseOnPartyAction(botAI); }
    static Action* evocation(PlayerbotAI* botAI) { return new CastEvocationAction(botAI); }
    static Action* deepfreeze(PlayerbotAI* botAI) { return new CastDeepFreezeAction(botAI); }
    static Action* ice_block(PlayerbotAI* botAI) { return new CastIceBlockAction(botAI); }
    static Action* blink(PlayerbotAI* botAI) { return new CastBlinkAction(botAI); }
    static Action* blink_back(PlayerbotAI* botAI) { return new CastBlinkBackAction(botAI); }
    static Action* temporal_shield(PlayerbotAI* botAI) { return new CastTemporalShieldAction(botAI); }
    static Action* blazing_speed(PlayerbotAI* botAI) { return new CastBlazingSpeedAction(botAI); }
    static Action* counterspell_on_enemy_caster(PlayerbotAI* botAI) { return new CastCounterspellOnEnemyCasterAction(botAI); }
    static Action* incanters_ward(PlayerbotAI* botAI) { return new CastIncantersWardAction(botAI); }
    static Action* conjure_mana_gem(PlayerbotAI* botAI) { return new CastConjureManaGemAction(botAI); }
    static Action* use_mana_gem(PlayerbotAI* botAI) { return new UseManaGemAction(botAI); }
    static Action* polymorph(PlayerbotAI* botAI) { return new CastPolymorphAction(botAI); }
    static Action* fingers_of_frost(PlayerbotAI* botAI) { return new CastFingersOfFrostAction(botAI); }
    static Action* icy_veins(PlayerbotAI* botAI) { return new CastIcyVeinsAction(botAI); }
    static Action* frozen_orb(PlayerbotAI* botAI) { return new CastFrozenOrbAction(botAI); }
    static Action* summon_water_elemental(PlayerbotAI* botAI) { return new CastSummonWaterElementalAction(botAI); }
    static Action* cold_snap(PlayerbotAI* botAI) { return new CastColdSnapAction(botAI); }
    static Action* ice_barrier(PlayerbotAI* botAI) { return new CastIceBarrierAction(botAI); }
    static Action* frostjaw(PlayerbotAI* botAI) { return new CastFrostjawAction(botAI); }

    // -- fire
    static Action* fireball(PlayerbotAI* botAI) { return new CastFireballAction(botAI); }
    static Action* dragon_breath(PlayerbotAI* botAI) { return new CastDragonsBreathAction(botAI); }
    static Action* blast_wave(PlayerbotAI* botAI) { return new CastBlastWaveAction(botAI); }
    static Action* inferno_blast(PlayerbotAI* botAI) { return new CastInfernoBlastAction(botAI); }
    static Action* pyroblast(PlayerbotAI* botAI) { return new CastPyroblastAction(botAI); }
    static Action* scorch(PlayerbotAI* botAI) { return new CastScorchAction(botAI); }
    static Action* living_bomb(PlayerbotAI* botAI) { return new CastLivingBombAction(botAI); }
    static Action* living_bomb_on_attackers(PlayerbotAI* botAI) { return new CastLivingBombOnAttackersAction(botAI); }
    static Action* flamestrike(PlayerbotAI* botAI) { return new CastFlamestrikeAction(botAI); }
    static Action* combustion(PlayerbotAI* botAI) { return new CastCombustionAction(botAI); }


    // -- forst
    static Action* frost_bomb(PlayerbotAI* botAI) { return new CastFrostBombAction(botAI); }

    // -- arcane
    static Action* arcane_power(PlayerbotAI* botAI) { return new CastArcanePowerAction(botAI); }
    static Action* arcane_blast(PlayerbotAI* botAI) { return new CastArcaneBlastAction(botAI); }
    static Action* arcane_barrage(PlayerbotAI* botAI) { return new CastArcaneBarrageAction(botAI); }
    static Action* slow(PlayerbotAI* botAI) { return new CastSlowAction(botAI); }
    static Action* nether_tempest(PlayerbotAI* botAI) { return new CastNetherTempestAction(botAI); }
    static Action* frost_bomb_on_attackers(PlayerbotAI* botAI) { return new CastFrostBombOnAttackersAction(botAI); }
    static Action* nether_tempest_on_attackers(PlayerbotAI* botAI) { return new CastNetherTempestOnAttackersAction(botAI); }

    static Action* focus_magic_on_party(PlayerbotAI* botAI) { return new CastFocusMagicOnPartyAction(botAI); }
};

MageAiObjectContext::MageAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new MageStrategyFactoryInternal());
    strategyContexts.Add(new MageCombatStrategyFactoryInternal());
    strategyContexts.Add(new MageBuffStrategyFactoryInternal());
    actionContexts.Add(new MageAiObjectContextInternal());
    triggerContexts.Add(new MageTriggerFactoryInternal());
}
