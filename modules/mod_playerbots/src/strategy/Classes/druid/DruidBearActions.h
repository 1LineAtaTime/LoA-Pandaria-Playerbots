/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRUIDBEARACTIONS_H
#define _PLAYERBOT_DRUIDBEARACTIONS_H

#include "GenericSpellActions.h"
#include "ReachTargetActions.h"

class PlayerbotAI;

// #384: "Feral Charge - Bear/Cat" (3.3.5) are NO-SUCH-NAME / NOT-LEARNABLE in 5.4.8 (cls.py: "feral charge - cat" ->
// id 79830 only). The 5.x successor is the Wild Charge talent (102401, Wowhead mop-classic 102401, 2026-10-04: bear
// form version charges the target, cat form version leaps to it). CAVEAT for the 1:1 test: the bear/cat versions are
// form overrides of 102401; if the bot's spell book resolves "wild charge" to the base 102401 (ally teleport) the cast
// fails and the ActionNode alternative "reach melee" runs, i.e. no regression. getName() keeps the AC name.
class CastFeralChargeBearAction : public CastReachTargetSpellAction
{
public:
    CastFeralChargeBearAction(PlayerbotAI* botAI) : CastReachTargetSpellAction(botAI, "wild charge", 1.5f) {}
    std::string const getName() override { return "feral charge - bear"; }
};

class CastGrowlAction : public CastSpellAction
{
public:
    CastGrowlAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "growl") {}
    // #835: never taunt a player (BG lose-aggro edge can see the old target, then dps assist switches to a player).
    bool isUseful() override { Unit* t = GetTarget(); return t && !t->IsPlayer() && CastSpellAction::isUseful(); }
};

// #363 (AC parity, Ai/Class/Druid/Action/DruidBearActions.h:27-31): Challenging Roar is a bear
// self-buff that forces the target onto the tank, so AC models it as a melee debuff spell.
// No body needed - it is a pure CastMeleeDebuffSpellAction.
class CastChallengingRoarAction : public CastMeleeDebuffSpellAction
{
public:
    CastChallengingRoarAction(PlayerbotAI* botAI) : CastMeleeDebuffSpellAction(botAI, "challenging roar") {}
};

class CastMaulAction : public CastMeleeSpellAction
{
public:
    CastMaulAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "maul") {}

    bool isUseful() override;
};

// #384: Bash (5211) is the "Mighty Bash" talent in 5.4.8 (a plain stun); the bear interrupt edge ("bash" /
// "bash on enemy healer", AC BearDruidStrategy.cpp:158-175) is carried by Skull Bash (106839, spec103/104 level 64,
// Wowhead mop-classic 106839, 2026-10-04). getName() keeps the AC registry/log name.
class CastBashAction : public CastMeleeSpellAction
{
public:
    CastBashAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "skull bash") {}
    std::string const getName() override { return "bash"; }
};

class CastSwipeAction : public CastMeleeSpellAction
{
public:
    CastSwipeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "swipe") {}
};

// #384: Demoralizing Roar is NOT-LEARNABLE in 5.4.8 (cls.py); the bear strategy no longer references it - Thrash
// (below) carries the Weakened Blows debuff that replaced it. The class stays registered, unused.
class CastDemoralizingRoarAction : public CastMeleeDebuffSpellAction
{
public:
    CastDemoralizingRoarAction(PlayerbotAI* botAI) : CastMeleeDebuffSpellAction(botAI, "demoralizing roar") {}
};

// #384: Thrash (bear 77758, level 28; Wowhead mop-classic 77758, 2026-10-04: AoE bleed within 8 yd + Weakened Blows
// -10% physical damage dealt) - the documented 5.x replacement for Demoralizing Roar.
class CastThrashAction : public CastMeleeDebuffSpellAction
{
public:
    CastThrashAction(PlayerbotAI* botAI) : CastMeleeDebuffSpellAction(botAI, "thrash", true) {}
};

// #384: 5.x Mangle is one spell for bear and cat (33917, Wowhead mop-classic 33917, 2026-10-04); "mangle (bear)" is
// NO-SUCH-NAME in the DBC and spell lookup is by exact name. getName() keeps the AC registry/log name.
class CastMangleBearAction : public CastMeleeSpellAction
{
public:
    CastMangleBearAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "mangle") {}
    std::string const getName() override { return "mangle (bear)"; }
};

// #384: 5.x Swipe is "Swipe" for both forms (779 bear / 62078 cat / 106785 unified, Wowhead mop-classic, 2026-10-04);
// "swipe (bear)" is NO-SUCH-NAME. getName() keeps the AC name.
class CastSwipeBearAction : public CastMeleeSpellAction
{
public:
    CastSwipeBearAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "swipe") {}
    std::string const getName() override { return "swipe (bear)"; }
};

class CastLacerateAction : public CastMeleeSpellAction
{
public:
    CastLacerateAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "lacerate") {}
};

// #384: Skull Bash replaces the bash interrupt (see CastBashAction above).
class CastBashOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
public:
    CastBashOnEnemyHealerAction(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, "skull bash") {}
    std::string const getName() override { return "bash on enemy healer"; }
};

#endif
