/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Gurub (Cataclysm 5-man) trigger context, MoP-only lane L7 (#670). Pattern: AC Ai/Dungeon/HoS/HoSTriggerContext.h.

#ifndef PLAYERBOTS_CATAZGTRIGGERCONTEXT_H
#define PLAYERBOTS_CATAZGTRIGGERCONTEXT_H

#include "CataZgTriggers.h"
#include "NamedObjectContext.h"

class CataDungeonZulGurubTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CataDungeonZulGurubTriggerContext()
    {
        creators["zg toxic link"] = &CataDungeonZulGurubTriggerContext::toxic_link;
        creators["zg acrid pools"] = &CataDungeonZulGurubTriggerContext::acrid_pools;
        creators["zg bloodvenom"] = &CataDungeonZulGurubTriggerContext::bloodvenom;
        creators["zg devastating slam"] = &CataDungeonZulGurubTriggerContext::devastating_slam;
        creators["zg wave of agony"] = &CataDungeonZulGurubTriggerContext::wave_of_agony;
        creators["zg zanzil fire"] = &CataDungeonZulGurubTriggerContext::zanzil_fire;
        creators["zg shadows of hakkar"] = &CataDungeonZulGurubTriggerContext::shadows_of_hakkar;
        creators["zg deadzone"] = &CataDungeonZulGurubTriggerContext::deadzone;
        creators["zg interrupt"] = &CataDungeonZulGurubTriggerContext::interrupt;
        creators["zg focus target"] = &CataDungeonZulGurubTriggerContext::focus_target;
        creators["zg breath of hethiss"] = &CataDungeonZulGurubTriggerContext::breath_of_hethiss;
        creators["zg berserker pursuit"] = &CataDungeonZulGurubTriggerContext::berserker_pursuit;
        creators["zg pull gurubashi spirit"] = &CataDungeonZulGurubTriggerContext::pull_gurubashi_spirit;
        creators["zg hakkar chains"] = &CataDungeonZulGurubTriggerContext::hakkar_chains;
        creators["zg spare gurubashi spirit"] = &CataDungeonZulGurubTriggerContext::spare_gurubashi_spirit;
        creators["zg frostburn cauldron"] = &CataDungeonZulGurubTriggerContext::frostburn_cauldron;
        creators["zg poison cloud"] = &CataDungeonZulGurubTriggerContext::poison_cloud;
        creators["zg venoxis tank spot"] = &CataDungeonZulGurubTriggerContext::venoxis_tank_spot;
        creators["zg venomous effusion"] = &CataDungeonZulGurubTriggerContext::venomous_effusion;
        creators["zg graveyard gas"] = &CataDungeonZulGurubTriggerContext::graveyard_gas;
    }

private:
    static Trigger* toxic_link(PlayerbotAI* ai) { return new CataZgToxicLinkTrigger(ai); }
    static Trigger* acrid_pools(PlayerbotAI* ai)
    {
        return new CataZgHazardTrigger(ai, "zg acrid pools", CataZulGurub::HazardKind::AcridPools);
    }
    static Trigger* bloodvenom(PlayerbotAI* ai)
    {
        return new CataZgHazardTrigger(ai, "zg bloodvenom", CataZulGurub::HazardKind::Bloodvenom);
    }
    static Trigger* devastating_slam(PlayerbotAI* ai)
    {
        return new CataZgHazardTrigger(ai, "zg devastating slam", CataZulGurub::HazardKind::DevastatingSlam);
    }
    static Trigger* wave_of_agony(PlayerbotAI* ai)
    {
        return new CataZgHazardTrigger(ai, "zg wave of agony", CataZulGurub::HazardKind::WaveOfAgony);
    }
    static Trigger* zanzil_fire(PlayerbotAI* ai)
    {
        return new CataZgHazardTrigger(ai, "zg zanzil fire", CataZulGurub::HazardKind::ZanzilFire);
    }
    static Trigger* shadows_of_hakkar(PlayerbotAI* ai) { return new CataZgShadowsTrigger(ai); }
    static Trigger* deadzone(PlayerbotAI* ai)
    {
        return new CataZgHazardTrigger(ai, "zg deadzone", CataZulGurub::HazardKind::Deadzone);
    }
    static Trigger* interrupt(PlayerbotAI* ai) { return new CataZgInterruptTrigger(ai); }
    static Trigger* focus_target(PlayerbotAI* ai) { return new CataZgFocusTrigger(ai); }
    static Trigger* breath_of_hethiss(PlayerbotAI* ai)
    {
        return new CataZgHazardTrigger(ai, "zg breath of hethiss", CataZulGurub::HazardKind::BreathOfHethiss);
    }
    static Trigger* berserker_pursuit(PlayerbotAI* ai)
    {
        return new CataZgHazardTrigger(ai, "zg berserker pursuit", CataZulGurub::HazardKind::BerserkerPursuit);
    }
    static Trigger* pull_gurubashi_spirit(PlayerbotAI* ai) { return new CataZgSpiritPullTrigger(ai); }
    static Trigger* hakkar_chains(PlayerbotAI* ai) { return new CataZgChainStandTrigger(ai); }
    static Trigger* spare_gurubashi_spirit(PlayerbotAI* ai) { return new CataZgSpareSpiritTrigger(ai); }
    static Trigger* frostburn_cauldron(PlayerbotAI* ai) { return new CataZgCauldronTrigger(ai); }
    static Trigger* poison_cloud(PlayerbotAI* ai)
    {
        return new CataZgHazardTrigger(ai, "zg poison cloud", CataZulGurub::HazardKind::PoisonCloud);
    }
    static Trigger* venoxis_tank_spot(PlayerbotAI* ai) { return new CataZgVenoxisTankSpotTrigger(ai); }
    static Trigger* venomous_effusion(PlayerbotAI* ai)
    {
        return new CataZgHazardTrigger(ai, "zg venomous effusion", CataZulGurub::HazardKind::VenomousEffusion);
    }
    static Trigger* graveyard_gas(PlayerbotAI* ai) { return new CataZgGraveyardGasTrigger(ai); }
};

#endif
