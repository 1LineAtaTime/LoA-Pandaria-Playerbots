/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WARRIORACTIONS_H
#define _PLAYERBOT_WARRIORACTIONS_H

#include "AiObject.h"
#include "GenericSpellActions.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotSpec.h"
#include "ReachTargetActions.h"

// -- Generic
class CastBattleStanceAction : public CastBuffSpellAction
{
public:
    CastBattleStanceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "battle stance", true) {}
    bool isUseful() override;
};
class CastBerserkerStanceAction : public CastBuffSpellAction
{
public:
    CastBerserkerStanceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "berserker stance", true) {}
    bool isUseful() override;
};
class CastDefensiveStanceAction : public CastBuffSpellAction
{
public:
    CastDefensiveStanceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "defensive stance", true) {}
    bool isUseful() override;
};

REACH_ACTION(CastChargeAction, "charge", 8.0f);
MELEE_ACTION(CastHeroicStrikeAction, "heroic strike");
BUFF_ACTION(CastCommandingShoutAction, "commanding shout");
BUFF_ACTION(CastBattleShoutAction, "battle shout");

class CastHeroicLeapAction : public CastSpellAction
{
public:
    CastHeroicLeapAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "heroic leap") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

BUFF_ACTION(CastBloodbathAction, "bloodbath");
BUFF_ACTION(CastBerserkerRageAction, "berserker rage");
BUFF_ACTION(CastRallyingCryAction, "rallying cry");
BUFF_ACTION(CastShieldWallAction, "shield wall");
BUFF_ACTION(CastRecklessnessAction, "recklessness");
BUFF_ACTION(CastDieByTheSwordAction, "die by the sword");

MELEE_ACTION(CastColossusSmashAction, "colossus smash");
MELEE_ACTION(CastVictoryRushAction, "victory rush");
MELEE_ACTION(CastExecuteAction, "execute");

// Issue #357 slice 10: AC registers this action (AC Ai/Class/Warrior/WarriorActions.h:92) and
// GenericWarriorStrategy.cpp:78 fires NextAction("pummel"), but no Action context in this fork
// created the action, so the interrupt could never run.
MELEE_ACTION(CastPummelAction, "pummel");
// #522 (AC WarriorActions.h:93): Fury's interrupt on the enemy healer (Pummel 6552, L24 skill, cls.py 2026-10-04).
ENEMY_HEALER_ACTION(CastPummelOnEnemyHealerAction, "pummel");

// #522: AC actions that were missing here; every spell is learnable in 5.4.8 (cls.py 2026-10-04):
// heroic throw 57755 L22, intervene 3411 L72, intimidating shout 5246 L52, piercing howl 12323 talent,
// hamstring 1715 L36, shattering throw 64382 L74, spell reflection 23920 L66, shockwave 46968 talent,
// thunder clap 6343 L20.
// (AC WarriorActions.h:65-66)
SPELL_ACTION(CastHeroicThrowAction, "heroic throw");
SNARE_ACTION(CastHeroicThrowSnareAction, "heroic throw");
// (AC WarriorActions.h:60)
SNARE_ACTION(CastHamstringAction, "hamstring");
// (AC WarriorActions.h:59)
SNARE_ACTION(CastThunderClapSnareAction, "thunder clap");
// (AC WarriorActions.h:133)
SNARE_ACTION(CastShockwaveSnareAction, "shockwave");
// (AC WarriorActions.h:42, :104)
DEBUFF_ACTION_R(CastIntimidatingShoutAction, "intimidating shout", 8.0f);
DEBUFF_ACTION_R(CastPiercingHowlAction, "piercing howl", 8.0f);
// (AC WarriorActions.h:122-123)
PROTECT_ACTION(CastInterveneAction, "intervene");
BUFF_ACTION(CastSpellReflectionAction, "spell reflection");
// (AC WarriorActions.h:32-39)
class CastDemoralizingShoutWithoutLifeTimeCheckAction : public CastMeleeDebuffSpellAction
{
public:
    CastDemoralizingShoutWithoutLifeTimeCheckAction(PlayerbotAI* botAI)
        : CastMeleeDebuffSpellAction(botAI, "demoralizing shout", false, 0.0f)
    {
    }
};
// (AC WarriorActions.h:162-171)
class CastShatteringThrowAction : public CastSpellAction
{
public:
    CastShatteringThrowAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "shattering throw") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
    Unit* GetTarget() override;
};

class CastSunderArmorAction : public CastDebuffSpellAction
{
public:
    CastSunderArmorAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "sunder armor")
    {
        range = ATTACK_DISTANCE;
    }

    bool isUseful() override;
};

// -- aoe
MELEE_ACTION(CastThunderClapAction, "thunder clap");
MELEE_ACTION(CastWhirlWindAction, "whirlwind");
MELEE_ACTION(CastCleaveAction, "cleave");
MELEE_ACTION(CastBladestormAction, "bladestorm");
MELEE_ACTION(CastDragonroarAction, "dragon roar");  // #522: was "dragonroar" (never resolved to Dragon Roar 118000)
// Lane 11 (#866, "Shockwave", prio group 6): the MoP row's facing test (0-45 deg AND
// 315-360 deg) can never be true (data bug), so the cone is ported sanely: target within 10 yd (Shockwave 46968 is a
// 10 yd frontal cone, 4 s stun), in front of the bot (+-45 deg) and not already stunned/feared/rooted ("is cc").
class CastShockwaveAction : public CastDebuffSpellAction
{
public:
    CastShockwaveAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "shockwave") { range = 8.0f; }
    bool isUseful() override;
};

// -- arms
MELEE_ACTION(CastMortalStrikeAction, "mortal strike");
MELEE_ACTION(CastSlamAction, "slam");
MELEE_ACTION(CastOverPowerAction, "overpower");
BUFF_ACTION(CastSweepingStrikesAction, "sweeping strikes");
class CastDemoralizingBannerAction : public CastSpellAction
{
public:
    CastDemoralizingBannerAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "demoralizing banner") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

// -- FURY
MELEE_ACTION(CastRagingBlowAction, "raging blow");
MELEE_ACTION(CastBloodThirstAction, "bloodthirst");
MELEE_ACTION(CastWildStrikeAction, "wild strike");
class CastSkullBannerAction : public CastSpellAction
{
public:
    CastSkullBannerAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "skull banner") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

// -- PROT
class CastDemoralizingShoutAction : public CastMeleeDebuffSpellAction
{
public:
    CastDemoralizingShoutAction(PlayerbotAI* botAI)
        : CastMeleeDebuffSpellAction(botAI, "demoralizing shout") {}
};
MELEE_ACTION(CastDevastateAction, "devastate");
// #835 (extends AC #2855 "taunts skip players", AC WarriorActions.h:109 is identical without it): "lose aggro" checks the
// current target when the trigger fires, but "dps assist" (50) runs first in the same tick and can switch the target to an
// enemy player, so "taunt" (41) then landed on a player (w72 DG: Debora Taunt(355) on 2 players). Refuse players here too.
MELEE_ACTION_U(CastTauntAction, "taunt", GetTarget() && !GetTarget()->IsPlayer() && GetTarget()->GetTarget() != bot->GetGUID());
// #163 S8 (AC parity, Ai/Class/Warrior/WarriorActions.h:110): taunt whatever "snare target"
// (ValueContext.h, already ported) currently picks - pulls threat off it the same way "taunt"
// pulls threat off the current target.
// #835 (w74 WSG: warrior "taunt on snare target" landed Taunt on a player): the snare target can be a fleeing
// player; a taunt is never cast on players.
class CastTauntOnSnareTargetAction : public CastSnareSpellAction
{
public:
    CastTauntOnSnareTargetAction(PlayerbotAI* botAI) : CastSnareSpellAction(botAI, "taunt") {}
    bool isUseful() override { Unit* t = GetTarget(); return t && !t->IsPlayer() && CastSnareSpellAction::isUseful(); }
};

// #163 S8: AC's "challenging shout" (WarriorActions.h:41, on "high aoe") is N/A - Challenging Shout
// 1161 was removed in 5.0.4 with no skill line left in Spell.dbc. Extending AC's pattern per
// research (s8-pet-control-and-class-spells.md B.5): the MoP-only ground-targeted AoE taunt Mocking
// Banner (114192, 3 min cooldown, 30 yd range, 15 yd taunt radius) at the bot's own position, since
// this fork's target-position spell cast has no "aoe centre" value to plant it at instead.
class CastMockingBannerAction : public Action
{
public:
    CastMockingBannerAction(PlayerbotAI* botAI) : Action(botAI, "mocking banner") {}

    bool isPossible() override { return botAI->CanCastSpell(114192, bot); }
    bool isUseful() override { return true; }
    bool Execute(Event event) override
    {
        return botAI->CastSpell(114192, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    }
};
MELEE_ACTION(CastShieldBashAction, "shield bash");
MELEE_ACTION(CastRevengeAction, "revenge");
BUFF_ACTION(CastShieldBlockAction, "shield block");
BUFF_ACTION(CastShieldBarrierAction, "shield barrier");
DEBUFF_ACTION_U(CastDisarmAction, "disarm",
    GetTarget() && GetTarget()->IsPlayer() ? !PlayerBotSpec::IsRanged((Player*)GetTarget())
    : CastDebuffSpellAction::isUseful());
BUFF_ACTION(CastLastStandAction, "last stand");
BUFF_ACTION(CastEnragedRegenerationAction, "enraged regeneration");
MELEE_ACTION(CastShieldSlamAction, "shield slam");
class CastVigilanceAction : public BuffOnPartyAction
{
public:
    CastVigilanceAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "vigilance") {}

    Unit* GetTarget() override;
    bool Execute(Event event) override;
};
// Lane 11 (#866, "Warrior Storm Bolt", prio group 5): stun only a target that is not
// already CC'd (shared value "is cc", lane 0).
class CastStormBoltAction : public CastDebuffSpellAction
{
public:
    CastStormBoltAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "storm bolt") { range = 8.0f; }
    bool isUseful() override;
};

// Lane 11 (#866): Avatar 107574 (prio 5/1, all three specs, "always").
BUFF_ACTION(CastAvatarAction, "avatar");

// Lane 11 (#866): interrupts on ANY casting enemy near the bot (not just the current target / healer), through
// lane 0's shared "enemy caster target" value. Gated on "in pvp instance" so open-world / dungeon behaviour stays AC.
//  - Pummel 6552 (prio group 10): melee range
//  - Disrupting Shout 102060 (109; talent, 10 yd PBAoE interrupt)
//  - Intimidating Shout 5246 (7; caster within 8 yd)
class CastPummelOnEnemyCasterAction : public CastSpellOnEnemyCasterAction
{
public:
    CastPummelOnEnemyCasterAction(PlayerbotAI* botAI) : CastSpellOnEnemyCasterAction(botAI, "pummel") {}
    bool isUseful() override;
};
class CastDisruptingShoutOnEnemyCasterAction : public CastSpellOnEnemyCasterAction
{
public:
    CastDisruptingShoutOnEnemyCasterAction(PlayerbotAI* botAI)
        : CastSpellOnEnemyCasterAction(botAI, "disrupting shout")
    {
    }
    bool isUseful() override;
};
class CastIntimidatingShoutOnEnemyCasterAction : public CastSpellOnEnemyCasterAction
{
public:
    CastIntimidatingShoutOnEnemyCasterAction(PlayerbotAI* botAI)
        : CastSpellOnEnemyCasterAction(botAI, "intimidating shout")
    {
    }
    bool isUseful() override;
};

#endif
