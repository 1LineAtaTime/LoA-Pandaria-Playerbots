#include "WarlockAiObjectContext.h"

#include "AffliWarlockStrategy.h"
#include "DemonoWarlockStrategy.h"
#include "DestruWarlockStrategy.h"
#include "GenericTriggers.h"
#include "GenericWarlockNonCombatStrategy.h"
#include "NamedObjectContext.h"
#include "Playerbots.h"
#include "PullStrategy.h"
#include "Strategy.h"
#include "WarlockActions.h"
#include "WarlockTriggers.h"

class WarlockStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockStrategyFactoryInternal()
    {
        creators["nc"] = &WarlockStrategyFactoryInternal::nc;
        creators["pull"] = &WarlockStrategyFactoryInternal::pull;
        creators["aoe"] = &WarlockStrategyFactoryInternal::aoe;
        creators["boost"] = &WarlockStrategyFactoryInternal::boost;
        creators["cc"] = &WarlockStrategyFactoryInternal::cc;
        creators["pet"] = &WarlockStrategyFactoryInternal::pet;
        // #408 (AC parity, AC WarlockAiObjectContext.cpp:32-33, :43-44)
        creators["meta melee"] = &WarlockStrategyFactoryInternal::meta_melee_aoe;
        creators["tank"] = &WarlockStrategyFactoryInternal::tank;
    }

private:
    static Strategy* pet(PlayerbotAI* botAI) { return new WarlockPetStrategy(botAI); }
    static Strategy* meta_melee_aoe(PlayerbotAI* botAI) { return new MetaMeleeAoeStrategy(botAI); }
    static Strategy* tank(PlayerbotAI* botAI) { return new TankWarlockStrategy(botAI); }
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericWarlockNonCombatStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new AoeWarlockStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
    static Strategy* boost(PlayerbotAI* botAI) { return new WarlockBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new WarlockCcStrategy(botAI); }
};

// #408 (AC parity, AC WarlockAiObjectContext.cpp:64-82). AC registers all five pets here; imp,
// felhunter and felguard already live in NonCombatBuffStrategyFactoryInternal above under this
// module's spec-slot names ("bhealth"/"bmana"/"bdps"), so only the two missing pets go here - their
// creator keys match their getName() exactly, which Engine::addStrategy requires.
class WarlockPetStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockPetStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["voidwalker"] = &WarlockPetStrategyFactoryInternal::voidwalker;
        creators["succubus"] = &WarlockPetStrategyFactoryInternal::succubus;
        // #521 (AC WarlockAiObjectContext.cpp:69,72,73): AC's keys for the three pets this module
        // also registers as bhealth/bmana/bdps (both key sets stay).
        creators["imp"] = &WarlockPetStrategyFactoryInternal::imp;
        creators["felhunter"] = &WarlockPetStrategyFactoryInternal::felhunter;
        creators["felguard"] = &WarlockPetStrategyFactoryInternal::felguard;
    }

private:
    static Strategy* imp(PlayerbotAI* ai) { return new SummonImpAcKeyStrategy(ai); }
    static Strategy* felhunter(PlayerbotAI* ai) { return new SummonFelhunterAcKeyStrategy(ai); }
    static Strategy* felguard(PlayerbotAI* ai) { return new SummonFelguardAcKeyStrategy(ai); }
    static Strategy* voidwalker(PlayerbotAI* ai) { return new SummonVoidwalkerStrategy(ai); }
    static Strategy* succubus(PlayerbotAI* ai) { return new SummonSuccubusStrategy(ai); }
};

// #408 (AC parity, AC WarlockAiObjectContext.cpp:102-122)
class WarlockCurseStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockCurseStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["curse of agony"] = &WarlockCurseStrategyFactoryInternal::curse_of_agony;
        creators["curse of elements"] = &WarlockCurseStrategyFactoryInternal::curse_of_elements;
        creators["curse of doom"] = &WarlockCurseStrategyFactoryInternal::curse_of_doom;
        creators["curse of exhaustion"] = &WarlockCurseStrategyFactoryInternal::curse_of_exhaustion;
        creators["curse of tongues"] = &WarlockCurseStrategyFactoryInternal::curse_of_tongues;
        creators["curse of weakness"] = &WarlockCurseStrategyFactoryInternal::curse_of_weakness;
    }

private:
    static Strategy* curse_of_agony(PlayerbotAI* botAI) { return new WarlockCurseOfAgonyStrategy(botAI); }
    static Strategy* curse_of_elements(PlayerbotAI* botAI) { return new WarlockCurseOfTheElementsStrategy(botAI); }
    static Strategy* curse_of_doom(PlayerbotAI* botAI) { return new WarlockCurseOfDoomStrategy(botAI); }
    static Strategy* curse_of_exhaustion(PlayerbotAI* botAI) { return new WarlockCurseOfExhaustionStrategy(botAI); }
    static Strategy* curse_of_tongues(PlayerbotAI* botAI) { return new WarlockCurseOfTonguesStrategy(botAI); }
    static Strategy* curse_of_weakness(PlayerbotAI* botAI) { return new WarlockCurseOfWeaknessStrategy(botAI); }
};

// #408 (AC parity, AC WarlockAiObjectContext.cpp:124-136)
class WarlockWeaponStoneStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockWeaponStoneStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["firestone"] = &WarlockWeaponStoneStrategyFactoryInternal::firestone;
        creators["spellstone"] = &WarlockWeaponStoneStrategyFactoryInternal::spellstone;
    }

private:
    static Strategy* firestone(PlayerbotAI* ai) { return new UseFirestoneStrategy(ai); }
    static Strategy* spellstone(PlayerbotAI* ai) { return new UseSpellstoneStrategy(ai); }
};

class WarlockCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["affli"] = &WarlockCombatStrategyFactoryInternal::affli;
        creators["demono"] = &WarlockCombatStrategyFactoryInternal::demono;
        creators["destru"] = &WarlockCombatStrategyFactoryInternal::destru;
        // #521 (AC WarlockAiObjectContext.cpp:54-55): AC's spec keys as aliases (the lead switches AiFactory to them).
        creators["demo"] = &WarlockCombatStrategyFactoryInternal::demo;
        creators["destro"] = &WarlockCombatStrategyFactoryInternal::destro;
    }

private:
    static Strategy* demo(PlayerbotAI* botAI) { return new DemoAcKeyWarlockStrategy(botAI); }
    static Strategy* destro(PlayerbotAI* botAI) { return new DestroAcKeyWarlockStrategy(botAI); }
    static Strategy* affli(PlayerbotAI* botAI) { return new AffliWarlockStrategy(botAI); }
    static Strategy* demono(PlayerbotAI* botAI) { return new DemonoWarlockStrategy(botAI); }
    static Strategy* destru(PlayerbotAI* botAI) { return new DestruWarlockStrategy(botAI); }
};

class NonCombatBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    NonCombatBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bdps"] = &NonCombatBuffStrategyFactoryInternal::felguard;
        creators["bmana"] = &NonCombatBuffStrategyFactoryInternal::felhunter;
        creators["bhealth"] = &NonCombatBuffStrategyFactoryInternal::imp;
    }

private:
    static Strategy* imp(PlayerbotAI* ai) { return new SummonImpStrategy(ai); }
    static Strategy* felhunter(PlayerbotAI* ai) { return new SummonFelhunterStrategy(ai); }
    static Strategy* felguard(PlayerbotAI* ai) { return new SummonFelguardStrategy(ai); }
};

// #163 S8 (AC parity, WarlockAiObjectContext.cpp:84-93): "ss self" is the AiFactory default for
// every warlock spec (AC AiFactory.cpp:571); "ss master"/"ss tank"/"ss healer" are alternatives a
// bot switches to via ChangeStrategyAction, same as "boost"/"save mana".
class WarlockSoulstoneStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockSoulstoneStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["ss self"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_self;
        creators["ss master"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_master;
        creators["ss tank"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_tank;
        creators["ss healer"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_healer;
    }

private:
    static Strategy* soulstone_self(PlayerbotAI* ai) { return new SoulstoneSelfStrategy(ai); }
    static Strategy* soulstone_master(PlayerbotAI* ai) { return new SoulstoneMasterStrategy(ai); }
    static Strategy* soulstone_tank(PlayerbotAI* ai) { return new SoulstoneTankStrategy(ai); }
    static Strategy* soulstone_healer(PlayerbotAI* ai) { return new SoulstoneHealerStrategy(ai); }
};

class WarlockTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    WarlockTriggerFactoryInternal()
    {
        // AC-registered triggers ported from WotLK. Stone-item triggers and "no soulstone" are
        // intentionally omitted below because those item/creation mechanics were removed before MoP.
        creators["shadow trance"] = &WarlockTriggerFactoryInternal::shadow_trance;
        creators["demon armor"] = &WarlockTriggerFactoryInternal::demon_armor;
        creators["unending breath"] = &WarlockTriggerFactoryInternal::unending_breath;
        creators["unending breath on party"] = &WarlockTriggerFactoryInternal::unending_breath_on_party;
        creators["no soul shard"] = &WarlockTriggerFactoryInternal::no_soul_shard;
        creators["too many soul shards"] = &WarlockTriggerFactoryInternal::too_many_soul_shards;
        creators["banish"] = &WarlockTriggerFactoryInternal::banish;
        creators["fear"] = &WarlockTriggerFactoryInternal::fear;
        creators["spell lock"] = &WarlockTriggerFactoryInternal::spell_lock;
        creators["havoc"] = &WarlockTriggerFactoryInternal::havoc;  // #864
        creators["fear pvp"] = &WarlockTriggerFactoryInternal::fear_pvp;  // #864
        creators["summon abyssal ready"] = &WarlockTriggerFactoryInternal::summon_abyssal_ready;  // #864
        creators["warlock flee close"] = &WarlockTriggerFactoryInternal::warlock_flee_close;  // lane K (#868)
        creators["warlock meta flee close"] = &WarlockTriggerFactoryInternal::warlock_meta_flee_close;  // lane K (#868)
        creators["warlock duel retarget"] = &WarlockTriggerFactoryInternal::warlock_duel_retarget;  // lane K (#868)
        creators["shadow bolt filler"] = &WarlockTriggerFactoryInternal::shadow_bolt_filler;  // #864
        creators["devour magic purge"] = &WarlockTriggerFactoryInternal::devour_magic_purge;
        creators["devour magic cleanse"] = &WarlockTriggerFactoryInternal::devour_magic_cleanse;
        creators["backlash"] = &WarlockTriggerFactoryInternal::backlash;
        creators["corruption on attacker"] = &WarlockTriggerFactoryInternal::corruption_on_attacker;
        creators["immolate on attacker"] = &WarlockTriggerFactoryInternal::immolate_on_attacker;
        creators["unstable affliction on attacker"] = &WarlockTriggerFactoryInternal::unstable_affliction_on_attacker;
        creators["haunt"] = &WarlockTriggerFactoryInternal::haunt;
        creators["decimation"] = &WarlockTriggerFactoryInternal::decimation;
        creators["life tap"] = &WarlockTriggerFactoryInternal::life_tap;
        creators["demonic empowerment"] = &WarlockTriggerFactoryInternal::demonic_empowerment;
        creators["immolation aura active"] = &WarlockTriggerFactoryInternal::immolation_aura_active;
        creators["metamorphosis not active"] = &WarlockTriggerFactoryInternal::metamorphosis_not_active;
        creators["meta melee flee check"] = &WarlockTriggerFactoryInternal::meta_melee_flee_check;
        creators["curse of agony"] = &WarlockTriggerFactoryInternal::curse_of_agony;
        creators["curse of agony on attacker"] = &WarlockTriggerFactoryInternal::curse_of_agony_on_attacker;
        creators["curse of doom"] = &WarlockTriggerFactoryInternal::curse_of_doom;
        creators["curse of exhaustion"] = &WarlockTriggerFactoryInternal::curse_of_exhaustion;
        creators["curse of tongues"] = &WarlockTriggerFactoryInternal::curse_of_tongues;
        creators["curse of weakness"] = &WarlockTriggerFactoryInternal::curse_of_weakness;
        creators["wrong pet"] = &WarlockTriggerFactoryInternal::wrong_pet;
        creators["rain of fire channel check"] = &WarlockTriggerFactoryInternal::rain_of_fire_channel_check;

        // -- talents
        creators["soul link"] = &WarlockTriggerFactoryInternal::soul_link;

        // -- generic
        creators["corruption"] = &WarlockTriggerFactoryInternal::corruption;
        creators["dark intent"] = &WarlockTriggerFactoryInternal::dark_intent;
        creators["curse of enfeeblement"] = &WarlockTriggerFactoryInternal::curse_enfeeblement;
        creators["curse of the elements"] = &WarlockTriggerFactoryInternal::curse_elements;

        // -- Affliction
        creators["agony"] = &WarlockTriggerFactoryInternal::agony;
        creators["unstable affliction"] = &WarlockTriggerFactoryInternal::unstable_affliction;
        creators["life tap glyph buff"] = &WarlockTriggerFactoryInternal::life_tap_glyph_buff;

        // -- Pet / consumables
        creators["no healthstone"] = &WarlockTriggerFactoryInternal::no_healthstone;
        // #408 (AC parity, AC WarlockAiObjectContext.cpp:151-155, :199-203). Firestone / Spellstone
        // are carried items on 5.4.8 (unlike Soulstone, which is a direct cast), so their item
        // triggers do apply; only the "create firestone"/"create spellstone" half stays N/A.
        creators["no firestone"] = &WarlockTriggerFactoryInternal::HasFirestone;
        creators["no spellstone"] = &WarlockTriggerFactoryInternal::HasSpellstone;
        creators["firestone"] = &WarlockTriggerFactoryInternal::firestone;
        creators["spellstone"] = &WarlockTriggerFactoryInternal::spellstone;
        // #163 S8 (AC parity): "no soulstone" -> "create soulstone" is N/A on 5.4.8 (see the
        // create_soulstone comment below) and is not registered. "soulstone" (schedules "soulstone
        // self"/"master"/"tank"/"healer" once the bot has one) is still useful if a bot is ever
        // handed one, so it is kept.
        creators["soulstone"] = &WarlockTriggerFactoryInternal::soulstone;

        // -- Demonologie
        creators["metamorphosis"] = &WarlockTriggerFactoryInternal::metamorphosis;
        creators["molten core"] = &WarlockTriggerFactoryInternal::molten_core;

        // -- Destru
        creators["immolate"] = &WarlockTriggerFactoryInternal::immolate;
        creators["backdraft"] = &WarlockTriggerFactoryInternal::backdraft;
        creators["fire and brimstone"] = &WarlockTriggerFactoryInternal::fire_and_brimstone;
        creators["dark soul: misery"] = &WarlockTriggerFactoryInternal::dark_soul_misery;
        creators["dark soul: knowledge"] = &WarlockTriggerFactoryInternal::dark_soul_knowledge;
        creators["dark soul: instability"] = &WarlockTriggerFactoryInternal::dark_soul_instability;

        // -- Pet
        creators["warlock pet low health"] = &WarlockTriggerFactoryInternal::warlock_pet_low_health;
        creators["warlock pet medium health"] = &WarlockTriggerFactoryInternal::warlock_pet_medium_health;
    }

private:
    static Trigger* shadow_trance(PlayerbotAI* botAI) { return new ShadowTranceTrigger(botAI); }
    static Trigger* demon_armor(PlayerbotAI* botAI) { return new DemonArmorTrigger(botAI); }
    static Trigger* unending_breath(PlayerbotAI* botAI) { return new UnendingBreathTrigger(botAI); }
    static Trigger* unending_breath_on_party(PlayerbotAI* botAI) { return new UnendingBreathOnPartyTrigger(botAI); }
    static Trigger* no_soul_shard(PlayerbotAI* botAI) { return new OutOfSoulShardsTrigger(botAI); }
    static Trigger* too_many_soul_shards(PlayerbotAI* botAI) { return new TooManySoulShardsTrigger(botAI); }
    static Trigger* banish(PlayerbotAI* botAI) { return new BanishTrigger(botAI); }
    static Trigger* fear(PlayerbotAI* botAI) { return new FearTrigger(botAI); }
    static Trigger* spell_lock(PlayerbotAI* botAI) { return new SpellLockInterruptSpellTrigger(botAI); }
    static Trigger* havoc(PlayerbotAI* botAI) { return new WarlockHavocTrigger(botAI); }
    static Trigger* fear_pvp(PlayerbotAI* botAI) { return new WarlockPvpFearTrigger(botAI); }
    static Trigger* summon_abyssal_ready(PlayerbotAI* botAI) { return new WarlockSummonAbyssalTrigger(botAI); }
    static Trigger* warlock_flee_close(PlayerbotAI* botAI)
    {
        return new WarlockFleeCloseTrigger(botAI, "warlock flee close", "enemy too close for spell");
    }
    static Trigger* warlock_meta_flee_close(PlayerbotAI* botAI)
    {
        return new WarlockFleeCloseTrigger(botAI, "warlock meta flee close", "meta melee flee check");
    }
    static Trigger* warlock_duel_retarget(PlayerbotAI* botAI) { return new WarlockDuelRetargetTrigger(botAI); }
    static Trigger* shadow_bolt_filler(PlayerbotAI* botAI) { return new WarlockShadowBoltFillerTrigger(botAI); }
    static Trigger* devour_magic_purge(PlayerbotAI* botAI) { return new DevourMagicPurgeTrigger(botAI); }
    static Trigger* devour_magic_cleanse(PlayerbotAI* botAI) { return new DevourMagicCleanseTrigger(botAI); }
    static Trigger* backlash(PlayerbotAI* botAI) { return new BacklashTrigger(botAI); }
    static Trigger* corruption_on_attacker(PlayerbotAI* botAI) { return new CorruptionOnAttackerTrigger(botAI); }
    static Trigger* immolate_on_attacker(PlayerbotAI* botAI) { return new ImmolateOnAttackerTrigger(botAI); }
    static Trigger* unstable_affliction_on_attacker(PlayerbotAI* botAI)
    {
        return new UnstableAfflictionOnAttackerTrigger(botAI);
    }
    static Trigger* haunt(PlayerbotAI* botAI) { return new HauntTrigger(botAI); }
    static Trigger* decimation(PlayerbotAI* botAI) { return new DecimationTrigger(botAI); }
    static Trigger* life_tap(PlayerbotAI* botAI) { return new LifeTapTrigger(botAI); }
    static Trigger* demonic_empowerment(PlayerbotAI* botAI) { return new DemonicEmpowermentTrigger(botAI); }
    static Trigger* immolation_aura_active(PlayerbotAI* botAI) { return new ImmolationAuraActiveTrigger(botAI); }
    static Trigger* metamorphosis_not_active(PlayerbotAI* botAI) { return new MetamorphosisNotActiveTrigger(botAI); }
    static Trigger* meta_melee_flee_check(PlayerbotAI* botAI)
    {
        return new MetaMeleeEnemyTooCloseForSpellTrigger(botAI);
    }
    static Trigger* curse_of_agony(PlayerbotAI* botAI) { return new CurseOfAgonyTrigger(botAI); }
    static Trigger* curse_of_agony_on_attacker(PlayerbotAI* botAI) { return new CurseOfAgonyOnAttackerTrigger(botAI); }
    static Trigger* curse_of_doom(PlayerbotAI* botAI) { return new CurseOfDoomTrigger(botAI); }
    static Trigger* curse_of_exhaustion(PlayerbotAI* botAI) { return new CurseOfExhaustionTrigger(botAI); }
    static Trigger* curse_of_tongues(PlayerbotAI* botAI) { return new CurseOfTonguesTrigger(botAI); }
    static Trigger* curse_of_weakness(PlayerbotAI* botAI) { return new CurseOfWeaknessTrigger(botAI); }
    static Trigger* wrong_pet(PlayerbotAI* botAI) { return new WrongPetTrigger(botAI); }
    static Trigger* rain_of_fire_channel_check(PlayerbotAI* botAI) { return new RainOfFireChannelCheckTrigger(botAI); }

    // -- talents
    static Trigger* soul_link(PlayerbotAI* botAI) { return new SoulLinkTrigger(botAI); }

    // -- generic
    static Trigger* corruption(PlayerbotAI* botAI) { return new CorruptionTrigger(botAI); }
    static Trigger* dark_intent(PlayerbotAI* botAI) { return new DarkIntentTrigger(botAI); }
    static Trigger* curse_enfeeblement(PlayerbotAI* botAI) { return new CurseOfEnfeeblementTrigger(botAI); }
    static Trigger* curse_elements(PlayerbotAI* botAI) { return new CurseOfElementsTrigger(botAI); }

    // -- Affliction
    static Trigger* agony(PlayerbotAI* botAI) { return new CurseOfAgonyTrigger(botAI); }
    static Trigger* unstable_affliction(PlayerbotAI* botAI) { return new UnstableAfflictionTrigger(botAI); }
    static Trigger* life_tap_glyph_buff(PlayerbotAI* botAI) { return new LifeTapGlyphBuffTrigger(botAI); }

    // -- Pet / consumables
    static Trigger* no_healthstone(PlayerbotAI* botAI) { return new HasHealthstoneTrigger(botAI); }
    static Trigger* HasFirestone(PlayerbotAI* botAI) { return new HasFirestoneTrigger(botAI); }
    static Trigger* HasSpellstone(PlayerbotAI* botAI) { return new HasSpellstoneTrigger(botAI); }
    static Trigger* firestone(PlayerbotAI* botAI) { return new FirestoneTrigger(botAI); }
    static Trigger* spellstone(PlayerbotAI* botAI) { return new SpellstoneTrigger(botAI); }
    static Trigger* soulstone(PlayerbotAI* botAI) { return new SoulstoneTrigger(botAI); }

    // -- Demonologie
    static Trigger* metamorphosis(PlayerbotAI* botAI) { return new MetamorphosisTrigger(botAI); }
    static Trigger* molten_core(PlayerbotAI* botAI) { return new MoltenCoreTrigger(botAI); }

    // -- Destr
    static Trigger* immolate(PlayerbotAI* botAI) { return new ImmolateTrigger(botAI); }
    static Trigger* backdraft(PlayerbotAI* botAI) { return new BackdraftTrigger(botAI); }
    static Trigger* fire_and_brimstone(PlayerbotAI* botAI) { return new FireAndBrimeStoneTrigger(botAI); }
    static Trigger* dark_soul_misery(PlayerbotAI* botAI) { return new DarkSoulMiseryTrigger(botAI); }
    static Trigger* dark_soul_knowledge(PlayerbotAI* botAI) { return new DarkSoulKnowledgeTrigger(botAI); }
    static Trigger* dark_soul_instability(PlayerbotAI* botAI) { return new DarkSoulInstabilityTrigger(botAI); }

    // -- Pet
    static Trigger* warlock_pet_low_health(PlayerbotAI* botAI) { return new WarlockPetLowHealthTrigger(botAI); }
    static Trigger* warlock_pet_medium_health(PlayerbotAI* botAI) { return new WarlockPetMediumHealthTrigger(botAI); }
};

class WarlockAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    WarlockAiObjectContextInternal()
    {
        // -- new
        creators["fel flame"] = &WarlockAiObjectContextInternal::fel_flame;
        creators["dark intent"] = &WarlockAiObjectContextInternal::dark_intent;
        // #521 (AC WarlockAiObjectContext.cpp:243,247-248)
        creators["fel armor"] = &WarlockAiObjectContextInternal::fel_armor;
        creators["unending breath"] = &WarlockAiObjectContextInternal::unending_breath;
        creators["unending breath on party"] = &WarlockAiObjectContextInternal::unending_breath_on_party;
            // low threat
        creators["soulshatter"] = &WarlockAiObjectContextInternal::soulshatter;
            // low life cd
        creators["unending resolve"] = &WarlockAiObjectContextInternal::unending_resolve;
        creators["twilight ward"] = &WarlockAiObjectContextInternal::twilight_ward;
        creators["health funnel"] = &WarlockAiObjectContextInternal::health_funnel;
        // -- talents
        creators["dark regeneration"] = &WarlockAiObjectContextInternal::dark_regeneration;
        creators["demonic breath"] = &WarlockAiObjectContextInternal::demonic_breath;
        creators["mortal coil"] = &WarlockAiObjectContextInternal::mortal_coil;
        creators["mortal coil on enemy caster"] = &WarlockAiObjectContextInternal::mortal_coil_on_enemy_caster;  // #864
        creators["pet interrupt"] = &WarlockAiObjectContextInternal::pet_interrupt;  // #864
        creators["havoc"] = &WarlockAiObjectContextInternal::havoc;  // #864
        creators["shadowfury"] = &WarlockAiObjectContextInternal::shadowfury;
        creators["soul link"] = &WarlockAiObjectContextInternal::soul_link;
        creators["sacrificial pact"] = &WarlockAiObjectContextInternal::sacrificial_pact;
        creators["dark bargain"] = &WarlockAiObjectContextInternal::dark_bargain;
        creators["blood horror"] = &WarlockAiObjectContextInternal::blood_horror;
        creators["grimoire of supremacy"] = &WarlockAiObjectContextInternal::grimoire_of_supremacy;
        creators["grimoire of sacrifice"] = &WarlockAiObjectContextInternal::grimoire_of_sacrifice;
        creators["mannoroth's fury"] = &WarlockAiObjectContextInternal::mannoroths_fury;

        creators["create healthstone"] = &WarlockAiObjectContextInternal::create_healthstone;
        // 5.4.8 (research s8-pet-control-and-class-spells.md B.1): "create soulstone" is N/A and not
        // registered - patch 5.0.4 turned Soulstone into a direct cast (spell 20707) with no item and
        // no creation step. "soulstone self/master/tank/healer" below now cast that spell directly
        // (UseSoulstoneAction::CastSoulstoneOn, WarlockActions.cpp).
        creators["soulstone self"] = &WarlockAiObjectContextInternal::soulstone_self;
        creators["soulstone master"] = &WarlockAiObjectContextInternal::soulstone_master;
        creators["soulstone tank"] = &WarlockAiObjectContextInternal::soulstone_tank;
        creators["soulstone healer"] = &WarlockAiObjectContextInternal::soulstone_healer;
        
        creators["summon voidwalker"] = &WarlockAiObjectContextInternal::summon_voidwalker;
        creators["summon felguard"] = &WarlockAiObjectContextInternal::summon_felguard;
        creators["summon felhunter"] = &WarlockAiObjectContextInternal::summon_felhunter;
        creators["summon succubus"] = &WarlockAiObjectContextInternal::summon_succubus;
        creators["summon imp"] = &WarlockAiObjectContextInternal::summon_imp;
        creators["summon infernal"] = &WarlockAiObjectContextInternal::summon_infernal;
        
        creators["summon wrathguard"] = &WarlockAiObjectContextInternal::summon_wrathguard;
        creators["summon doomguard"] = &WarlockAiObjectContextInternal::summon_doomguard;
        creators["summon fel imp"] = &WarlockAiObjectContextInternal::summon_fel_imp;
        creators["summon voidlord"] = &WarlockAiObjectContextInternal::summon_voidlord;
        creators["summon shivarra"] = &WarlockAiObjectContextInternal::summon_shivarra;
        creators["summon observer"] = &WarlockAiObjectContextInternal::summon_observer;
        creators["summon abyssal"] = &WarlockAiObjectContextInternal::summon_abyssal;
        creators["summon terrorguard"] = &WarlockAiObjectContextInternal::summon_terrorguard;

        // -- generic
        creators["curse of enfeeblement"] = &WarlockAiObjectContextInternal::curse_enfeeblement;
        // #408 (AC parity, AC WarlockAiObjectContext.cpp:304-310): AC names the agony action
        // "curse of agony"/"curse of agony on attacker", this module also keeps the short
        // "agony"/"agony on attacker" keys so existing strategies keep working.
        creators["curse of agony"] = &WarlockAiObjectContextInternal::curse_of_agony;
        creators["curse of agony on attacker"] = &WarlockAiObjectContextInternal::curse_of_agony_on_attacker;
        creators["curse of the elements"] = &WarlockAiObjectContextInternal::curse_elements;
        creators["curse of doom"] = &WarlockAiObjectContextInternal::curse_of_doom;
        creators["curse of exhaustion"] = &WarlockAiObjectContextInternal::curse_of_exhaustion;
        creators["curse of tongues"] = &WarlockAiObjectContextInternal::curse_of_tongues;
        creators["curse of weakness"] = &WarlockAiObjectContextInternal::curse_of_weakness;
        creators["firestone"] = &WarlockAiObjectContextInternal::firestone;
        creators["spellstone"] = &WarlockAiObjectContextInternal::spellstone;
        creators["corruption"] = &WarlockAiObjectContextInternal::corruption;
        creators["corruption on attacker"] = &WarlockAiObjectContextInternal::corruption_on_attacker;
        creators["siphon life"] = &WarlockAiObjectContextInternal::siphon_life;
        creators["siphon life on attacker"] = &WarlockAiObjectContextInternal::siphon_life_on_attacker;
        creators["drain life"] = &WarlockAiObjectContextInternal::drain_life;
        
        creators["shadow bolt"] = &WarlockAiObjectContextInternal::shadow_bolt;
        creators["drain soul"] = &WarlockAiObjectContextInternal::drain_soul;
        creators["banish"] = &WarlockAiObjectContextInternal::banish;
        creators["banish on cc"] = &WarlockAiObjectContextInternal::banish_on_cc;
        creators["seed of corruption"] = &WarlockAiObjectContextInternal::seed_of_corruption;
        creators["seed of corruption on attacker"] = &WarlockAiObjectContextInternal::seed_of_corruption_on_attacker;
        creators["rain of fire"] = &WarlockAiObjectContextInternal::rain_of_fire;
        creators["life tap"] = &WarlockAiObjectContextInternal::life_tap;
        creators["fear"] = &WarlockAiObjectContextInternal::fear;
        creators["fear on cc"] = &WarlockAiObjectContextInternal::fear_on_cc; 
        creators["demonic empowerment"] = &WarlockAiObjectContextInternal::demonic_empowerment;
        
        // -- Affliction
        creators["soul burn"] = &WarlockAiObjectContextInternal::soul_burn;
        creators["dark soul: misery"] = &WarlockAiObjectContextInternal::dark_soul_misery;
        creators["malefic grasp"] = &WarlockAiObjectContextInternal::malefic_grasp;
        creators["agony"] = &WarlockAiObjectContextInternal::curse_of_agony;
        creators["agony on attacker"] = &WarlockAiObjectContextInternal::curse_of_agony_on_attacker;
        creators["unstable affliction"] = &WarlockAiObjectContextInternal::unstable_affliction;
        creators["unstable affliction on attacker"] = &WarlockAiObjectContextInternal::unstable_affliction_on_attacker;
        creators["haunt"] = &WarlockAiObjectContextInternal::haunt;

        // -- Demonologie
        creators["dark soul: knowledge"] = &WarlockAiObjectContextInternal::dark_soul_knowledge;
        creators["hellfire"] = &WarlockAiObjectContextInternal::hellfire;
        creators["hand of guldan"] = &WarlockAiObjectContextInternal::hand_of_guldan;
        creators["soul fire"] = &WarlockAiObjectContextInternal::soul_fire;
        creators["metamorphosis"] = &WarlockAiObjectContextInternal::metamorphosis;
        creators["chaos wave"] = &WarlockAiObjectContextInternal::metamorphosis_chaos_wave;
        creators["doom"] = &WarlockAiObjectContextInternal::metamorphosis_doom;
        creators["immolation aura"] = &WarlockAiObjectContextInternal::metamorphosis_immolation;
        creators["touch of chaos"] = &WarlockAiObjectContextInternal::metamorphosis_touch_chaos;
        creators["void ray"] = &WarlockAiObjectContextInternal::metamorphosis_void_ray;

        // -- Destru
        creators["dark soul: instability"] = &WarlockAiObjectContextInternal::dark_soul_instability;
        creators["chaos bolt"] = &WarlockAiObjectContextInternal::chaos_bolt;
        creators["conflagrate"] = &WarlockAiObjectContextInternal::conflagrate;
        creators["fire and brimstone"] = &WarlockAiObjectContextInternal::fire_and_brimstone;
        creators["flames of xoroth"] = &WarlockAiObjectContextInternal::flames_of_xoroth;
        creators["immolate"] = &WarlockAiObjectContextInternal::immolate;
        creators["immolate on attacker"] = &WarlockAiObjectContextInternal::immolate_on_attacker;
        creators["incinerate"] = &WarlockAiObjectContextInternal::incinerate;
        creators["shadowburn"] = &WarlockAiObjectContextInternal::shadowburn;
        creators["searing pain"] = &WarlockAiObjectContextInternal::searing_pain;
        creators["shadow ward"] = &WarlockAiObjectContextInternal::shadow_ward;
        creators["demon charge"] = &WarlockAiObjectContextInternal::demon_charge;
    }

private:
    // -- generic
    static Action* summon_imp(PlayerbotAI* botAI) { return new CastSummonImpAction(botAI); }
    static Action* summon_succubus(PlayerbotAI* botAI) { return new CastSummonSuccubusAction(botAI); }
    static Action* summon_voidwalker(PlayerbotAI* botAI) { return new CastSummonVoidwalkerAction(botAI); }
    static Action* summon_felguard(PlayerbotAI* botAI) { return new CastSummonFelguardAction(botAI); }
    static Action* summon_felhunter(PlayerbotAI* botAI) { return new CastSummonFelhunterAction(botAI); }
    static Action* summon_infernal(PlayerbotAI* botAI) { return new CastSummonInfernalAction(botAI); }
    static Action* summon_wrathguard(PlayerbotAI* botAI) { return new CastSummonWrathGuardGuardAction(botAI); }
    static Action* summon_doomguard(PlayerbotAI* botAI) { return new CastSummonDoomGuardAction(botAI); }
    static Action* summon_fel_imp(PlayerbotAI* botAI) { return new CastSummonFelImpAction(botAI); }
    static Action* summon_voidlord(PlayerbotAI* botAI) { return new CastSummonVoidLordAction(botAI); }
    static Action* summon_shivarra(PlayerbotAI* botAI) { return new CastSummonShivarraAction(botAI); }
    static Action* summon_observer(PlayerbotAI* botAI) { return new CastSummonObserverAction(botAI); }
    static Action* summon_abyssal(PlayerbotAI* botAI) { return new CastSummonAbyssalAction(botAI); }
    static Action* summon_terrorguard(PlayerbotAI* botAI) { return new CastSummonTerrorGuardAction(botAI); }
    static Action* demonic_empowerment(PlayerbotAI* ai) { return new CastDemonicEmpowermentAction(ai); }

    static Action* soulshatter(PlayerbotAI* botAI) { return new CastSoulShatterAction(botAI); }
    static Action* unending_resolve(PlayerbotAI* botAI) { return new CastUnendingResolveAction(botAI); }
    static Action* fel_flame(PlayerbotAI* botAI) { return new CastFelFlameAction(botAI); }
    static Action* dark_intent(PlayerbotAI* botAI) { return new CastDarkIntentAction(botAI); }
    static Action* fel_armor(PlayerbotAI* botAI) { return new CastFelArmorAction(botAI); }
    static Action* unending_breath(PlayerbotAI* botAI) { return new CastUnendingBreathAction(botAI); }
    static Action* unending_breath_on_party(PlayerbotAI* botAI) { return new CastUnendingBreathOnPartyAction(botAI); }
    static Action* twilight_ward(PlayerbotAI* botAI) { return new CastTwilightWardAction(botAI); }
    static Action* health_funnel(PlayerbotAI* botAI) { return new CastHealthFunnelAction(botAI); }

    static Action* corruption(PlayerbotAI* botAI) { return new CastCorruptionAction(botAI); }
    static Action* curse_enfeeblement(PlayerbotAI* botAI) { return new CastCurseOfEnfeeblementAction(botAI); }
    static Action* curse_elements(PlayerbotAI* botAI) { return new CastCurseOfElementsAction(botAI); }
    static Action* curse_of_doom(PlayerbotAI* ai) { return new CastCurseOfDoomAction(ai); }
    static Action* curse_of_exhaustion(PlayerbotAI* ai) { return new CastCurseOfExhaustionAction(ai); }
    static Action* curse_of_tongues(PlayerbotAI* ai) { return new CastCurseOfTonguesAction(ai); }
    static Action* curse_of_weakness(PlayerbotAI* ai) { return new CastCurseOfWeaknessAction(ai); }
    static Action* firestone(PlayerbotAI* botAI) { return new UseSpellItemAction(botAI, "firestone"); }
    static Action* spellstone(PlayerbotAI* botAI) { return new UseSpellItemAction(botAI, "spellstone"); }
    static Action* searing_pain(PlayerbotAI* botAI) { return new CastSearingPainAction(botAI); }
    static Action* shadow_ward(PlayerbotAI* botAI) { return new CastShadowWardAction(botAI); }
    static Action* demon_charge(PlayerbotAI* ai) { return new DemonChargeAction(ai); }
    static Action* corruption_on_attacker(PlayerbotAI* botAI) { return new CastCorruptionOnAttackerAction(botAI); }
    static Action* siphon_life(PlayerbotAI* botAI) { return new CastSiphonLifeAction(botAI); }
    static Action* siphon_life_on_attacker(PlayerbotAI* botAI) { return new CastSiphonLifeOnAttackerAction(botAI); }
    static Action* fear_on_cc(PlayerbotAI* botAI) { return new CastFearOnCcAction(botAI); }
    static Action* fear(PlayerbotAI* botAI) { return new CastFearAction(botAI); }
    static Action* banish(PlayerbotAI* botAI) { return new CastBanishAction(botAI); }
    static Action* banish_on_cc(PlayerbotAI* botAI) { return new CastBanishAction(botAI); }
    static Action* shadow_bolt(PlayerbotAI* botAI) { return new CastShadowBoltAction(botAI); }
    static Action* drain_life(PlayerbotAI* botAI) { return new CastDrainLifeAction(botAI); }
    static Action* create_healthstone(PlayerbotAI* botAI) { return new CastCreateHealthstoneAction(botAI); }
    static Action* soulstone_self(PlayerbotAI* botAI) { return new UseSoulstoneSelfAction(botAI); }
    static Action* soulstone_master(PlayerbotAI* botAI) { return new UseSoulstoneMasterAction(botAI); }
    static Action* soulstone_tank(PlayerbotAI* botAI) { return new UseSoulstoneTankAction(botAI); }
    static Action* soulstone_healer(PlayerbotAI* botAI) { return new UseSoulstoneHealerAction(botAI); }

    // -- talents
    static Action* dark_regeneration(PlayerbotAI* botAI) { return new CastDarkRegenerationAction(botAI); }
    static Action* demonic_breath(PlayerbotAI* botAI) { return new CastDemonicBreathAction(botAI); }
    static Action* mortal_coil(PlayerbotAI* botAI) { return new CastMortalCoilAction(botAI); }
    static Action* mortal_coil_on_enemy_caster(PlayerbotAI* botAI) { return new CastMortalCoilOnEnemyCasterAction(botAI); }
    static Action* pet_interrupt(PlayerbotAI* botAI) { return new CastPetInterruptAction(botAI); }
    static Action* havoc(PlayerbotAI* botAI) { return new CastHavocAction(botAI); }
    static Action* shadowfury(PlayerbotAI* botAI) { return new CastShadowFuryAction(botAI); }
    static Action* soul_link(PlayerbotAI* botAI) { return new CastSoulLinkAction(botAI); }
    static Action* sacrificial_pact(PlayerbotAI* botAI) { return new CastSacrificialPactAction(botAI); }
    static Action* dark_bargain(PlayerbotAI* botAI) { return new CastDarkBarginAction(botAI); }
    static Action* blood_horror(PlayerbotAI* botAI) { return new CastBloodHorrorAction(botAI); }
    static Action* grimoire_of_supremacy(PlayerbotAI* botAI) { return new CastGrmoireOfSupremacyAction(botAI); }
    static Action* grimoire_of_sacrifice(PlayerbotAI* botAI) { return new CastGrimoireOfSacrificeAction(botAI); }
    static Action* mannoroths_fury(PlayerbotAI* botAI) { return new CastMannorothFuryAction(botAI); }

    // -- affli
    static Action* soul_burn(PlayerbotAI* botAI) { return new CastSoulBurnAction(botAI); }
    static Action* dark_soul_misery(PlayerbotAI* botAI) { return new CastDarkSoulMiseryAction(botAI); }
    static Action* malefic_grasp(PlayerbotAI* botAI) { return new CastMaleficGraspAction(botAI); }
    static Action* curse_of_agony(PlayerbotAI* botAI) { return new CastCurseOfAgonyAction(botAI); }
    static Action* curse_of_agony_on_attacker(PlayerbotAI* botAI) { return new CastCurseOfAgonyOnAttackerAction(botAI); }
    static Action* drain_soul(PlayerbotAI* botAI) { return new CastDrainSoulAction(botAI); }
    static Action* seed_of_corruption(PlayerbotAI* botAI) { return new CastSeedOfCorruptionAction(botAI); }
    static Action* seed_of_corruption_on_attacker(PlayerbotAI* botAI) { return new CastSeedOfCorruptionOnAttackerAction(botAI); }
    static Action* unstable_affliction(PlayerbotAI* ai) { return new CastUnstableAfflictionAction(ai); }
    static Action* unstable_affliction_on_attacker(PlayerbotAI* ai) { return new CastUnstableAfflictionOnAttackerAction(ai); }
    static Action* haunt(PlayerbotAI* ai) { return new CastHauntAction(ai); }

    // -- demono
    static Action* soul_fire(PlayerbotAI* ai) { return new CastSoulFireAction(ai); }
    static Action* dark_soul_knowledge(PlayerbotAI* botAI) { return new CastDarkSoulKnowledgeAction(botAI); }
    static Action* hellfire(PlayerbotAI* botAI) { return new CastHellfireAction(botAI); }
    static Action* hand_of_guldan(PlayerbotAI* botAI) { return new CastHandOfGuldanAction(botAI); }

    static Action* metamorphosis(PlayerbotAI* ai) { return new CastMetamorphosisAction(ai); }
    static Action* metamorphosis_chaos_wave(PlayerbotAI* botAI) { return new CastMetaChaosWaveAction(botAI); }
    static Action* metamorphosis_doom(PlayerbotAI* botAI) { return new CastMetaDoomAction(botAI); }
    static Action* metamorphosis_immolation(PlayerbotAI* botAI) { return new CastMetaImmolationAction(botAI); }
    static Action* metamorphosis_touch_chaos(PlayerbotAI* botAI) { return new CastMetaChaosAction(botAI); }
    static Action* metamorphosis_void_ray(PlayerbotAI* botAI) { return new CastMetaVoidRayAction(botAI); }

    // -- destruc
    static Action* dark_soul_instability(PlayerbotAI* botAI) { return new CastDarkSoulInstabilityAction(botAI); }
    static Action* chaos_bolt(PlayerbotAI* botAI) { return new CastChaosBoltAction(botAI); }
    static Action* fire_and_brimstone(PlayerbotAI* botAI) { return new CastFireAndBrimeStoneAction(botAI); }
    static Action* flames_of_xoroth(PlayerbotAI* botAI) { return new CastFlamesOfXorothAction(botAI); }
    static Action* shadowburn(PlayerbotAI* botAI) { return new CastShadowBurnAction(botAI); }
    static Action* conflagrate(PlayerbotAI* botAI) { return new CastConflagrateAction(botAI); }
    static Action* incinerate(PlayerbotAI* botAI) { return new CastIncinerateAction(botAI); }
    static Action* immolate(PlayerbotAI* botAI) { return new CastImmolateAction(botAI); }
    static Action* rain_of_fire(PlayerbotAI* botAI) { return new CastRainOfFireAction(botAI); }
    static Action* life_tap(PlayerbotAI* botAI) { return new CastLifeTapAction(botAI); }
    static Action* immolate_on_attacker(PlayerbotAI* ai) { return new CastImmolateOnAttackerAction(ai); }
};

WarlockAiObjectContext::WarlockAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new WarlockStrategyFactoryInternal());
    strategyContexts.Add(new WarlockCombatStrategyFactoryInternal());
    strategyContexts.Add(new NonCombatBuffStrategyFactoryInternal());
    strategyContexts.Add(new WarlockSoulstoneStrategyFactoryInternal());
    // #408 (AC parity, AC WarlockAiObjectContext.cpp:404-409)
    strategyContexts.Add(new WarlockPetStrategyFactoryInternal());
    strategyContexts.Add(new WarlockCurseStrategyFactoryInternal());
    strategyContexts.Add(new WarlockWeaponStoneStrategyFactoryInternal());
    actionContexts.Add(new WarlockAiObjectContextInternal());
    triggerContexts.Add(new WarlockTriggerFactoryInternal());
}
