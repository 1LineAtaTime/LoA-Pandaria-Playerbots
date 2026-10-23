/*
 * Ported from AC mod-playerbots Ai/Base/Actions/ImbueAction.h (issue #434).
 *
 * Ported: ImbueWithStoneAction ("apply stone") and ImbueWithOilAction ("apply oil").
 * Not ported:
 *  - ImbueWithPoisonAction ("apply poison", AC ImbueAction.cpp:11-102): rogue poisons are not bag
 *    items in 5.4.8, they are self-cast spells (Deadly Poison etc.); the fork's rogue strategy
 *    already maintains them with the "deadly poison" CastBuffSpellAction edge
 *    (strategy/Classes/rogue/GenericRogueNonCombatStrategy.cpp:67,
 *    RogueAiObjectContext.cpp:154,194). AC's FindPoison (Bot/PlayerbotAI.cpp:5646) has no caller
 *    other than that action, so it is not ported either.
 *  - TryEmergencyAction ("try emergency"): unrelated to this issue, covered by the fork's own
 *    healthstone / healing potion actions (#149, #226).
 */

#ifndef _PLAYERBOT_IMBUEACTION_H
#define _PLAYERBOT_IMBUEACTION_H

#include "Action.h"

class PlayerbotAI;

// Search and apply a sharpening stone / weightstone to the equipped weapons.
class ImbueWithStoneAction : public Action
{
public:
    ImbueWithStoneAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

// Search and apply a wizard / mana oil to the equipped main-hand weapon.
class ImbueWithOilAction : public Action
{
public:
    ImbueWithOilAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

#endif
