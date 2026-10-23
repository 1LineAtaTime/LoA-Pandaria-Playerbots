/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man) triggers, MoP-only lane L7 (#669). Pattern: AC Ai/Dungeon/HoS/HoSTriggers.h.

#ifndef PLAYERBOTS_CATAZATRIGGERS_H
#define PLAYERBOTS_CATAZATRIGGERS_H

#include "CataZaShared.h"
#include "Trigger.h"

// The bot stands inside (or on the edge of) a ground hazard of the given kind.
class CataZaHazardTrigger : public Trigger
{
public:
    CataZaHazardTrigger(PlayerbotAI* botAI, std::string const name, CataZulAman::HazardKind kind)
        : Trigger(botAI, name), _kind(kind)
    {
    }

    bool IsActive() override;

private:
    CataZulAman::HazardKind _kind;
};

// Akil'zon: Electrical Storm is up and the bot is outside the eye.
class CataZaStormTrigger : public Trigger
{
public:
    CataZaStormTrigger(PlayerbotAI* botAI) : Trigger(botAI, "za electrical storm") {}
    bool IsActive() override;
};

// Akil'zon: ranged / healers stay spread out for Static Disruption.
class CataZaSpreadTrigger : public Trigger
{
public:
    CataZaSpreadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "za static spread") {}
    bool IsActive() override;
};

// Nalorakk / Daakara bear: the bot carries Surge and is far from the boss.
class CataZaSurgeTrigger : public Trigger
{
public:
    CataZaSurgeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "za surge debuff") {}
    bool IsActive() override;
};

// Halazzi: the tank has the boss inside a Water Totem's healing area.
class CataZaWaterTotemTrigger : public Trigger
{
public:
    CataZaWaterTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "za water totem") {}
    bool IsActive() override;
};

// A priority add / totem exists for this bot to focus.
class CataZaFocusTrigger : public Trigger
{
public:
    CataZaFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "za focus target") {}
    bool IsActive() override;
};

// Malacrass or a servant is casting a heal / hard cast.
class CataZaInterruptTrigger : public Trigger
{
public:
    CataZaInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, "za malacrass interrupt") {}
    bool IsActive() override;
};

#endif
