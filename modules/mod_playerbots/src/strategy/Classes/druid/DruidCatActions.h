/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRUIDCATACTIONS_H
#define _PLAYERBOT_DRUIDCATACTIONS_H

#include "GenericSpellActions.h"
#include "ReachTargetActions.h"

class PlayerbotAI;

// #384: Feral Charge - Cat -> Wild Charge talent (102401; the cat form version leaps behind the target), see the
// caveat on CastFeralChargeBearAction in DruidBearActions.h. getName() keeps the AC name.
class CastFeralChargeCatAction : public CastReachTargetSpellAction
{
public:
    CastFeralChargeCatAction(PlayerbotAI* botAI) : CastReachTargetSpellAction(botAI, "wild charge", 1.5f) {}
    std::string const getName() override { return "feral charge - cat"; }
};

class CastCowerAction : public CastBuffSpellAction
{
public:
    CastCowerAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cower") {}
    bool isUseful() override;
};

class CastBerserkAction : public CastBuffSpellAction
{
public:
    CastBerserkAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "berserk") {}
};

class CastTigersFuryAction : public CastBuffSpellAction
{
public:
    CastTigersFuryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "tiger's fury") {}

    // Wave-75 fix J (#868): spell_dru_tigers_fury (spell_druid.cpp, OnCheckCast) refuses Tiger's Fury with
    // SPELL_FAILED_DONT_REPORT while Berserk (cat, 106951) is up; the 27.2 "tiger's fury" edge then "cast" and failed on
    // every tick of the 15 s Berserk (w75 Steli: 24 failed casts, each ending the tick, one executed action per tick).
    bool isUseful() override;
};

class CastSavageRoarAction : public CastBuffSpellAction
{
public:
    CastSavageRoarAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "savage roar") {}
    std::string const GetTargetName() override { return "current target"; }
};

class CastRakeAction : public CastDebuffSpellAction
{
public:
    CastRakeAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "rake", true, 6.0f) {}
};

class CastRakeOnMeleeAttackersAction : public CastDebuffSpellOnMeleeAttackerAction
{
public:
    CastRakeOnMeleeAttackersAction(PlayerbotAI* botAI) : CastDebuffSpellOnMeleeAttackerAction(botAI, "rake", true, 6.0f) {}
};

class CastClawAction : public CastMeleeSpellAction
{
public:
    CastClawAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "claw") {}
};

// #384: one Mangle / one Swipe in 5.x (see CastMangleBearAction / CastSwipeBearAction); "mangle (cat)" and
// "swipe (cat)" are NO-SUCH-NAME in the 5.4.8 DBC. getName() keeps the AC names.
class CastMangleCatAction : public CastMeleeSpellAction
{
public:
    CastMangleCatAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "mangle") {}
    std::string const getName() override { return "mangle (cat)"; }
};

class CastSwipeCatAction : public CastMeleeSpellAction
{
public:
    CastSwipeCatAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "swipe") {}
    std::string const getName() override { return "swipe (cat)"; }
};

class CastFerociousBiteAction : public CastMeleeSpellAction
{
public:
    CastFerociousBiteAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "ferocious bite") {}
};

// #363 (AC parity, Ai/Class/Druid/Action/DruidCatActions.h:112-125): AC gates Maim on the target
// being a player (its rip/bleed is the duel/PvP use). Body in the .cpp - Unit::ToPlayer() and
// CastMeleeSpellAction::isUseful() need complete types the header TU lacks (hard rule 2).
class CastMaimAction : public CastMeleeSpellAction
{
public:
    CastMaimAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "maim") {}

    bool isUseful() override;
};

class CastRipAction : public CastMeleeDebuffSpellAction
{
public:
    CastRipAction(PlayerbotAI* botAI) : CastMeleeDebuffSpellAction(botAI, "rip", true, 12.0f) {}
};

class CastShredAction : public CastMeleeSpellAction
{
public:
    CastShredAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "shred") {}
};

class CastProwlAction : public CastBuffSpellAction
{
public:
    CastProwlAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "prowl") {}
};

class CastDashAction : public CastBuffSpellAction
{
public:
    CastDashAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "dash") {}
};

class CastRavageAction : public CastMeleeSpellAction
{
public:
    CastRavageAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "ravage") {}
};

class CastPounceAction : public CastMeleeSpellAction
{
public:
    CastPounceAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "pounce") {}
};

#endif
