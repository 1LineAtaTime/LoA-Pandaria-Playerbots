/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ROGUECOMBOACTIONS_H
#define _PLAYERBOT_ROGUECOMBOACTIONS_H

#include "GenericSpellActions.h"

class PlayerbotAI;

class CastComboAction : public CastMeleeSpellAction
{
public:
    CastComboAction(PlayerbotAI* botAI, std::string const name) : CastMeleeSpellAction(botAI, name) {}

    bool isUseful() override;
};

class CastSinisterStrikeAction : public CastSpellAction
{
public:
    CastSinisterStrikeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "sinister strike") {}
};

class CastMutilateAction : public CastSpellAction
{
public:
    CastMutilateAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "mutilate") {}
};

class CastRiposteAction : public CastSpellAction
{
public:
    CastRiposteAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "riposte") {}
};

class CastGougeAction : public CastSpellAction
{
public:
    CastGougeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "gouge") {}
};

// Revealing Strike (84617), Combat, prio 0/4: only while the target lacks our Revealing Strike
// (gated by the "revealing strike" trigger).
class CastRevealingStrikeAction : public CastMeleeSpellAction
{
public:
    CastRevealingStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "revealing strike") {}
};

// Hemorrhage (16511), Subtlety, prio 0/2: refresh the Hemorrhage debuff (89775).
class CastHemorrhageAction : public CastMeleeSpellAction
{
public:
    CastHemorrhageAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "hemorrhage") {}
};

// Dispatch (111240), Assassination, prio 0/4. The game only allows it on a target
// below 35% health (or a Blindside proc), so CanCast gates it.
class CastDispatchAction : public CastMeleeSpellAction
{
public:
    CastDispatchAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "dispatch") {}
};

class CastBackstabAction : public CastSpellAction
{
public:
    CastBackstabAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "backstab") {}
};

#endif
