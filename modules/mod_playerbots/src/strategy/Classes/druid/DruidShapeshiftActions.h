/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRUIDSHAPESHIFTACTIONS_H
#define _PLAYERBOT_DRUIDSHAPESHIFTACTIONS_H

#include "GenericSpellActions.h"

class PlayerbotAI;

class CastBearFormAction : public CastBuffSpellAction
{
public:
    CastBearFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "bear form") {}

    bool isPossible() override;
    bool isUseful() override;
};

class CastDireBearFormAction : public CastBuffSpellAction
{
public:
    CastDireBearFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "dire bear form") {}

    NextAction** getAlternatives() override;
};

class CastCatFormAction : public CastBuffSpellAction
{
public:
    CastCatFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cat form") {}
};

// #384: the 3.3.5 "Tree of Life" form spell (48371/53691) is NOT-LEARNABLE in 5.4.8 (cls.py). Its 5.x form is the
// talent "Incarnation" (106731, shown as "Incarnation: Tree of Life" 33891 for restoration; 3 min cooldown, 30 sec,
// +15% healing, free shapeshifting while it lasts - Wowhead mop-classic 33891 + 106731, 2026-10-04). The bot's
// spell book holds the talent spell, so the cast name is "incarnation"; the aura it applies is 33891, which
// TreeFormTrigger / isUseful() already test by id. getName() keeps the AC name.
class CastTreeFormAction : public CastBuffSpellAction
{
public:
    CastTreeFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "incarnation") {}
    std::string const getName() override { return "tree of life"; }
    bool isUseful() override;
};

class CastMoonkinFormAction : public CastBuffSpellAction
{
public:
    CastMoonkinFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "moonkin form") {}
};

class CastAquaticFormAction : public CastBuffSpellAction
{
public:
    CastAquaticFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "aquatic form") {}
};

class CastTravelFormAction : public CastBuffSpellAction
{
public:
    CastTravelFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "travel form") {}

    bool isUseful() override;
};

class CastCasterFormAction : public CastBuffSpellAction
{
public:
    CastCasterFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "caster form") {}

    bool isUseful() override;
    bool isPossible() override { return true; }
    bool Execute(Event event) override;
};

class CastCancelTreeFormAction : public CastBuffSpellAction
{
public:
    CastCancelTreeFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cancel tree form") {}

    bool isUseful() override;
    bool isPossible() override { return true; }
    bool Execute(Event event) override;
};

// #363 (AC parity, Ai/Class/Druid/Action/DruidShapeshiftActions.h:75-90): shared base for the
// six remaining "cancel <form>" actions AC registers at DruidAiObjectContext.cpp:211-216. The fork
// already had "cancel tree form" (above, derived straight from CastBuffSpellAction), so this base
// deliberately covers only the six NEW children and does not reparent the existing action.
//
// Form-cancel is not a cast: it removes the shapeshift aura by name and gates on the aura being
// present by id. Both bodies live in the .cpp - Unit::HasAura(uint32) and PlayerbotAI::RemoveAura()
// need complete types that the header TU does not have.
class CastCancelDruidAction : public CastBuffSpellAction
{
public:
    CastCancelDruidAction(PlayerbotAI* botAI, std::string const& actionName, std::string const& auraName, uint32 auraId)
        : CastBuffSpellAction(botAI, actionName), auraName(auraName), auraId(auraId)
    {
    }

    bool isUseful() override;
    bool isPossible() override { return true; }
    bool Execute(Event event) override;

private:
    std::string auraName;
    uint32 auraId;
};

// Spell ids are the WotLK-era base shapeshift auras, unchanged in 5.4.8: 783 travel form,
// 5487 bear form, 9634 dire bear form, 768 cat form, 24858 moonkin form, 1066 aquatic form.
// 768/1066/783 are already used at these exact ids elsewhere in the fork
// (AI/PlayerbotSpec.cpp:123, Travel/TravelNode.cpp:165).
class CastCancelTravelFormAction : public CastCancelDruidAction
{
public:
    CastCancelTravelFormAction(PlayerbotAI* botAI)
        : CastCancelDruidAction(botAI, "cancel travel form", "travel form", 783)
    {
    }
};

class CastCancelBearFormAction : public CastCancelDruidAction
{
public:
    CastCancelBearFormAction(PlayerbotAI* botAI) : CastCancelDruidAction(botAI, "cancel bear form", "bear form", 5487) {}
};

class CastCancelDireBearFormAction : public CastCancelDruidAction
{
public:
    CastCancelDireBearFormAction(PlayerbotAI* botAI)
        : CastCancelDruidAction(botAI, "cancel dire bear form", "dire bear form", 9634)
    {
    }
};

class CastCancelCatFormAction : public CastCancelDruidAction
{
public:
    CastCancelCatFormAction(PlayerbotAI* botAI) : CastCancelDruidAction(botAI, "cancel cat form", "cat form", 768) {}
};

class CastCancelMoonkinFormAction : public CastCancelDruidAction
{
public:
    CastCancelMoonkinFormAction(PlayerbotAI* botAI)
        : CastCancelDruidAction(botAI, "cancel moonkin form", "moonkin form", 24858)
    {
    }
};

class CastCancelAquaticFormAction : public CastCancelDruidAction
{
public:
    CastCancelAquaticFormAction(PlayerbotAI* botAI)
        : CastCancelDruidAction(botAI, "cancel aquatic form", "aquatic form", 1066)
    {
    }
};

#endif
