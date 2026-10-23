/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Gurub (Cataclysm 5-man) action context, MoP-only lane L7 (#670). Pattern: AC Ai/Dungeon/HoS/HoSActionContext.h.

#ifndef PLAYERBOTS_CATAZGACTIONCONTEXT_H
#define PLAYERBOTS_CATAZGACTIONCONTEXT_H

#include "Action.h"
#include "CataZgActions.h"
#include "NamedObjectContext.h"

class CataDungeonZulGurubActionContext : public NamedObjectContext<Action>
{
public:
    CataDungeonZulGurubActionContext()
    {
        creators["zg break toxic link"] = &CataDungeonZulGurubActionContext::break_toxic_link;
        creators["zg escape acrid pools"] = &CataDungeonZulGurubActionContext::escape_acrid_pools;
        creators["zg escape bloodvenom"] = &CataDungeonZulGurubActionContext::escape_bloodvenom;
        creators["zg escape devastating slam"] = &CataDungeonZulGurubActionContext::escape_devastating_slam;
        creators["zg escape wave of agony"] = &CataDungeonZulGurubActionContext::escape_wave_of_agony;
        creators["zg escape zanzil fire"] = &CataDungeonZulGurubActionContext::escape_zanzil_fire;
        creators["zg enter deadzone"] = &CataDungeonZulGurubActionContext::enter_deadzone;
        creators["zg escape deadzone"] = &CataDungeonZulGurubActionContext::escape_deadzone;
        creators["zg interrupt"] = &CataDungeonZulGurubActionContext::interrupt;
        creators["zg focus target"] = &CataDungeonZulGurubActionContext::focus_target;
        creators["zg escape breath of hethiss"] = &CataDungeonZulGurubActionContext::escape_breath_of_hethiss;
        creators["zg kite berserker"] = &CataDungeonZulGurubActionContext::kite_berserker;
        creators["zg pull gurubashi spirit"] = &CataDungeonZulGurubActionContext::pull_gurubashi_spirit;
        creators["zg stand at hakkar chain"] = &CataDungeonZulGurubActionContext::stand_at_hakkar_chain;
        creators["zg drop gurubashi spirit"] = &CataDungeonZulGurubActionContext::drop_gurubashi_spirit;
        creators["zg frostburn cauldron"] = &CataDungeonZulGurubActionContext::frostburn_cauldron;
        creators["zg escape poison cloud"] = &CataDungeonZulGurubActionContext::escape_poison_cloud;
        creators["zg venoxis tank spot"] = &CataDungeonZulGurubActionContext::venoxis_tank_spot;
        creators["zg escape venomous effusion"] = &CataDungeonZulGurubActionContext::escape_venomous_effusion;
        creators["zg toxic torment cauldron"] = &CataDungeonZulGurubActionContext::toxic_torment_cauldron;
    }

private:
    static Action* break_toxic_link(PlayerbotAI* ai) { return new CataZgBreakToxicLinkAction(ai); }
    static Action* escape_acrid_pools(PlayerbotAI* ai)
    {
        return new CataZgEscapeAction(ai, "zg escape acrid pools", CataZulGurub::HazardKind::AcridPools);
    }
    static Action* escape_bloodvenom(PlayerbotAI* ai)
    {
        return new CataZgEscapeAction(ai, "zg escape bloodvenom", CataZulGurub::HazardKind::Bloodvenom);
    }
    static Action* escape_devastating_slam(PlayerbotAI* ai)
    {
        return new CataZgEscapeAction(ai, "zg escape devastating slam", CataZulGurub::HazardKind::DevastatingSlam);
    }
    static Action* escape_wave_of_agony(PlayerbotAI* ai)
    {
        return new CataZgEscapeAction(ai, "zg escape wave of agony", CataZulGurub::HazardKind::WaveOfAgony);
    }
    static Action* escape_zanzil_fire(PlayerbotAI* ai)
    {
        return new CataZgEscapeAction(ai, "zg escape zanzil fire", CataZulGurub::HazardKind::ZanzilFire);
    }
    static Action* enter_deadzone(PlayerbotAI* ai) { return new CataZgEnterDeadzoneAction(ai); }
    static Action* escape_deadzone(PlayerbotAI* ai)
    {
        return new CataZgEscapeAction(ai, "zg escape deadzone", CataZulGurub::HazardKind::Deadzone);
    }
    static Action* interrupt(PlayerbotAI* ai) { return new CataZgInterruptAction(ai); }
    static Action* focus_target(PlayerbotAI* ai) { return new CataZgFocusAction(ai); }
    static Action* escape_breath_of_hethiss(PlayerbotAI* ai)
    {
        return new CataZgEscapeAction(ai, "zg escape breath of hethiss", CataZulGurub::HazardKind::BreathOfHethiss);
    }
    static Action* kite_berserker(PlayerbotAI* ai)
    {
        return new CataZgEscapeAction(ai, "zg kite berserker", CataZulGurub::HazardKind::BerserkerPursuit);
    }
    static Action* pull_gurubashi_spirit(PlayerbotAI* ai) { return new CataZgPullSpiritAction(ai); }
    static Action* stand_at_hakkar_chain(PlayerbotAI* ai) { return new CataZgStandAtChainAction(ai); }
    static Action* drop_gurubashi_spirit(PlayerbotAI* ai) { return new CataZgDropSpiritAction(ai); }
    static Action* frostburn_cauldron(PlayerbotAI* ai) { return new CataZgCauldronAction(ai); }
    static Action* escape_poison_cloud(PlayerbotAI* ai)
    {
        return new CataZgEscapeAction(ai, "zg escape poison cloud", CataZulGurub::HazardKind::PoisonCloud);
    }
    static Action* venoxis_tank_spot(PlayerbotAI* ai) { return new CataZgVenoxisTankSpotAction(ai); }
    static Action* escape_venomous_effusion(PlayerbotAI* ai)
    {
        return new CataZgEscapeAction(ai, "zg escape venomous effusion", CataZulGurub::HazardKind::VenomousEffusion);
    }
    static Action* toxic_torment_cauldron(PlayerbotAI* ai) { return new CataZgToxicTormentAction(ai); }
};

#endif
