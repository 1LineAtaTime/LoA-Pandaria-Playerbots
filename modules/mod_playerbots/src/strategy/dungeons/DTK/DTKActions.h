/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DTKACTIONS_H
#define PLAYERBOTS_DTKACTIONS_H

#include "Action.h"
#include "AttackActions.h"  // 5.4.8: AC "AttackAction.h" -> fork strategy/actions/AttackActions.h
#include "DTKTriggers.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// AC Ai/Dungeon/DTK/DTKActions.h:17
const Position NOVOS_PARTY_POSITION = Position(-378.852f, -760.349f, 28.587f);

// AC Ai/Dungeon/DTK/DTKActions.h:19
class CorpseExplodeSpreadAction : public MovementAction
{
public:
    CorpseExplodeSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "corpse explode spread") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/DTK/DTKActions.h:26
class AvoidArcaneFieldAction : public MovementAction
{
public:
    AvoidArcaneFieldAction(PlayerbotAI* ai) : MovementAction(ai, "avoid arcane field") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/DTK/DTKActions.h:33
class NovosDefaultPositionAction : public MovementAction
{
public:
    NovosDefaultPositionAction(PlayerbotAI* ai) : MovementAction(ai, "novos default position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// AC Ai/Dungeon/DTK/DTKActions.h:41
class NovosTargetPriorityAction : public AttackAction
{
public:
    NovosTargetPriorityAction(PlayerbotAI* ai) : AttackAction(ai, "novos target priority") {}
    bool Execute(Event event) override;
    // bool isUseful() override;
};

// AC Ai/Dungeon/DTK/DTKActions.h:49
class CastSlayingStrikeAction : public CastMeleeSpellAction
{
public:
    CastSlayingStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "slaying strike") {}
};

// AC Ai/Dungeon/DTK/DTKActions.h:55
class CastTauntAction : public CastSpellAction
{
public:
    CastTauntAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "taunt") {}
};

// AC Ai/Dungeon/DTK/DTKActions.h:61
class CastBoneArmorAction : public CastSpellAction
{
public:
    CastBoneArmorAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "bone armor") {}
};

// AC Ai/Dungeon/DTK/DTKActions.h:67
class CastTouchOfLifeAction : public CastSpellAction
{
public:
    CastTouchOfLifeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "touch of life") {}
};

#endif
