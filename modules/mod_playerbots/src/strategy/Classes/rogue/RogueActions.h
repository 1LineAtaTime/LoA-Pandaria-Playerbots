/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ROGUEACTIONS_H
#define _PLAYERBOT_ROGUEACTIONS_H

#include "GenericSpellActions.h"

class PlayerbotAI;

class CastEvasionAction : public CastBuffSpellAction
{
public:
    CastEvasionAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "evasion") {}
};

class CastCloakOfShadowsAction : public CastBuffSpellAction
{
public:
    CastCloakOfShadowsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cloak of shadows") {}
};

class CastHungerForBloodAction : public CastBuffSpellAction
{
public:
    CastHungerForBloodAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "hunger for blood") {}
    std::string const GetTargetName() override { return "current target"; }
};

class CastSprintAction : public CastBuffSpellAction
{
public:
    CastSprintAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "sprint") {}

    std::string const GetTargetName() override { return "self target"; }
};

class CastStealthAction : public CastBuffSpellAction
{
public:
    CastStealthAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "stealth") {}

    std::string const GetTargetName() override { return "self target"; }

    bool isUseful() override;  // #742 (AC RogueActions.h:49)
    bool isPossible() override;
};

class UnstealthAction : public Action
{
public:
    UnstealthAction(PlayerbotAI* botAI) : Action(botAI, "unstealth") {}

    bool Execute(Event event) override;
};

class CheckStealthAction : public Action
{
public:
    CheckStealthAction(PlayerbotAI* botAI) : Action(botAI, "check stealth") {}

    bool isPossible() override { return true; }
    bool Execute(Event event) override;
};

class CastKickAction : public CastSpellAction
{
public:
    CastKickAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "kick") {}
};

class CastFeintAction : public CastBuffSpellAction
{
public:
    CastFeintAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "feint") {}
};

class CastDismantleAction : public CastSpellAction
{
public:
    CastDismantleAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "dismantle") {}
};

class CastDistractAction : public CastSpellAction
{
public:
    CastDistractAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "distract") {}
};

class CastVanishAction : public CastBuffSpellAction
{
public:
    CastVanishAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "vanish") {}

    bool isUseful() override;
};

class CastBlindAction : public CastDebuffSpellAction
{
public:
    CastBlindAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "blind") {}
};

class CastBladeFlurryAction : public CastBuffSpellAction
{
public:
    CastBladeFlurryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blade flurry") {}
};

class CastAdrenalineRushAction : public CastBuffSpellAction
{
public:
    CastAdrenalineRushAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "adrenaline rush") {}
};

// Shadowstep (36554), spec rows, prio group 0/10: gap closer on the target at 8-25 yd.
class CastShadowstepAction : public CastSpellAction
{
public:
    CastShadowstepAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "shadowstep") {}
};

// Smoke Bomb (76577), prio group 5: self cast at low health; BG/arena only ("in pvp instance" gate).
class CastSmokeBombAction : public CastBuffSpellAction
{
public:
    CastSmokeBombAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "smoke bomb") {}

    bool isUseful() override;
};

// Recuperate (73651), prio group 5: finisher heal at <= 30% hp with 5 combo points.
class CastRecuperateAction : public CastBuffSpellAction
{
public:
    CastRecuperateAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "recuperate") {}
};

// Shadow Blades (121471), prio group 5: self buff.
class CastShadowBladesAction : public CastBuffSpellAction
{
public:
    CastShadowBladesAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "shadow blades") {}
};

// Vendetta (79140), prio group 0/10: debuff on the current target (not while stealthed).
class CastVendettaAction : public CastSpellAction
{
public:
    CastVendettaAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "vendetta") {}
};

// Marked for Death (137619), prio group 0/20: on the target while we have < 2 combo points.
class CastMarkedForDeathAction : public CastSpellAction
{
public:
    CastMarkedForDeathAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "marked for death") {}
};

// Preparation (14185), prio group 5: resets Vanish/Sprint/Shadowstep etc.
class CastPreparationAction : public CastBuffSpellAction
{
public:
    CastPreparationAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "preparation") {}
};

// "Rogue kick victim / random" (Kick 1766, prio group 10): kick any enemy caster in range.
// Uses lane 0's CastSpellOnEnemyCasterAction (shared contract). Open world / dungeon behaviour stays AC: BG/arena only.
class CastKickOnEnemyCasterAction : public CastSpellOnEnemyCasterAction
{
public:
    CastKickOnEnemyCasterAction(PlayerbotAI* botAI) : CastSpellOnEnemyCasterAction(botAI, "kick") {}

    std::string const getName() override { return "kick on enemy caster"; }
    bool isUseful() override;
};

class CastKillingSpreeAction : public CastMeleeSpellAction
{
public:
    CastKillingSpreeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "killing spree") {}
};

class CastKickOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
public:
    CastKickOnEnemyHealerAction(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, "kick") {}
};

class EnvenomAction : public CastMeleeSpellAction
{
public:
    EnvenomAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "envenom") {}
};

class CastTricksOfTheTradeOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastTricksOfTheTradeOnMainTankAction(PlayerbotAI* ai) : BuffOnMainTankAction(ai, "tricks of the trade", true) {}
    virtual bool isUseful() override;
};

class FanOfKnivesAction : public CastMeleeSpellAction
{
public:
    FanOfKnivesAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "fan of knives") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

#endif
