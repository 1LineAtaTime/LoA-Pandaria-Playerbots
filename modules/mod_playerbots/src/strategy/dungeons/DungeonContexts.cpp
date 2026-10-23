/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DungeonContexts.h"

#include "Action.h"
#include "Strategy.h"
#include "Trigger.h"
#include "Value.h"

#include "ACActionContext.h"
#include "ACTriggerContext.h"
#include "AKActionContext.h"
#include "AKTriggerContext.h"
#include "ANActionContext.h"
#include "ANTriggerContext.h"
#include "CataBcActionContext.h"
#include "CataBcTriggerContext.h"
#include "CataBcValueContext.h"
#include "CataDmActionContext.h"
#include "CataDmTriggerContext.h"
#include "CataEtActionContext.h"
#include "CataEtTriggerContext.h"
#include "CataGbActionContext.h"
#include "CataGbTriggerContext.h"
#include "CataHooActionContext.h"
#include "CataHooTriggerContext.h"
#include "CataHotActionContext.h"
#include "CataHotTriggerContext.h"
#include "CataLctActionContext.h"
#include "CataLctTriggerContext.h"
#include "CataScActionContext.h"
#include "CataScTriggerContext.h"
#include "CataSfkActionContext.h"
#include "CataSfkTriggerContext.h"
#include "CataTottActionContext.h"
#include "CataTottTriggerContext.h"
#include "CataVpActionContext.h"
#include "CataVpTriggerContext.h"
#include "CataWoeActionContext.h"
#include "CataWoeTriggerContext.h"
#include "CataZaActionContext.h"
#include "CataZaTriggerContext.h"
#include "CataZgActionContext.h"
#include "CataZgTriggerContext.h"
#include "CoSActionContext.h"
#include "CoSTriggerContext.h"
#include "DTKActionContext.h"
#include "DTKTriggerContext.h"
#include "FoSActionContext.h"
#include "FoSTriggerContext.h"
#include "GDActionContext.h"
#include "GDTriggerContext.h"
#include "GDValueContext.h"
#include "HoLActionContext.h"
#include "HoLTriggerContext.h"
#include "DungeonStrategyContext.h"
#include "HoSActionContext.h"
#include "HoSTriggerContext.h"
#include "MechActionContext.h"
#include "MechTriggerContext.h"
#include "MechValueContext.h"
#include "MgTActionContext.h"
#include "MgTTriggerContext.h"
#include "MgTValueContext.h"
#include "MopGssActionContext.h"
#include "MopGssTriggerContext.h"
#include "MopGssValueContext.h"
#include "MopMspActionContext.h"
#include "MopMspTriggerContext.h"
#include "MopMspValueContext.h"
#include "MopScholoActionContext.h"
#include "MopScholoTriggerContext.h"
#include "MopShActionContext.h"
#include "MopShTriggerContext.h"
#include "MopSmActionContext.h"
#include "MopSmTriggerContext.h"
#include "MopSntActionContext.h"
#include "MopSntTriggerContext.h"
#include "MopSntValueContext.h"
#include "MopSpmActionContext.h"
#include "MopSpmTriggerContext.h"
#include "MopSsbActionContext.h"
#include "MopSsbTriggerContext.h"
#include "MopTjsActionContext.h"
#include "MopTjsTriggerContext.h"
#include "NexActionContext.h"
#include "NexTriggerContext.h"
#include "OCActionContext.h"
#include "OCTriggerContext.h"
#include "PoSActionContext.h"
#include "PoSTriggerContext.h"
#include "RampActionContext.h"
#include "RampTriggerContext.h"
#include "SethActionContext.h"
#include "SethTriggerContext.h"
#include "TOCActionContext.h"
#include "TOCTriggerContext.h"
#include "UBActionContext.h"
#include "UBTriggerContext.h"
#include "UBValueContext.h"
#include "UKActionContext.h"
#include "UKTriggerContext.h"
#include "UPActionContext.h"
#include "UPTriggerContext.h"
#include "VHActionContext.h"
#include "VHTriggerContext.h"

// AC Bot/Engine/BuildSharedStrategyContexts.cpp:18
void DungeonContexts::AddStrategyContexts(NamedObjectContextList<Strategy>& strategyContexts)
{
    strategyContexts.Add(new DungeonStrategyContext());
}

// AC Bot/Engine/BuildSharedActionContexts.cpp:58-78 (dungeon entries, AC order; ported ones only)
void DungeonContexts::AddActionContexts(NamedObjectContextList<Action>& actionContexts)
{
    actionContexts.Add(new TbcDungeonHellfireRampartsActionContext());
    actionContexts.Add(new TbcDungeonAuchenaiCryptsActionContext());
    actionContexts.Add(new TbcDungeonSethekkHallsActionContext());
    actionContexts.Add(new TbcDungeonMechanarActionContext());
    actionContexts.Add(new TbcDungeonUnderbogActionContext());
    actionContexts.Add(new TbcDungeonMagistersTerraceActionContext());
    actionContexts.Add(new WotlkDungeonGDActionContext());
    actionContexts.Add(new WotlkDungeonUKActionContext());
    actionContexts.Add(new WotlkDungeonNexActionContext());
    actionContexts.Add(new WotlkDungeonANActionContext());
    actionContexts.Add(new WotlkDungeonOKActionContext());
    actionContexts.Add(new WotlkDungeonDTKActionContext());
    actionContexts.Add(new WotlkDungeonVHActionContext());
    actionContexts.Add(new WotlkDungeonHoSActionContext());
    actionContexts.Add(new WotlkDungeonHoLActionContext());
    actionContexts.Add(new WotlkDungeonOccActionContext());
    actionContexts.Add(new WotlkDungeonUPActionContext());
    actionContexts.Add(new WotlkDungeonCoSActionContext());
    actionContexts.Add(new WotlkDungeonFoSActionContext());
    actionContexts.Add(new WotlkDungeonPoSActionContext());
    actionContexts.Add(new WotlkDungeonToCActionContext());
    // Cataclysm + Mists of Pandaria dungeons (epic #373, MoP-only)
    actionContexts.Add(new CataDungeonBcActionContext());
    actionContexts.Add(new CataDungeonTottActionContext());
    actionContexts.Add(new CataDungeonScActionContext());
    actionContexts.Add(new CataDungeonVpActionContext());
    actionContexts.Add(new CataDungeonGbActionContext());
    actionContexts.Add(new CataDungeonHooActionContext());
    actionContexts.Add(new CataDungeonLctActionContext());
    actionContexts.Add(new CataDungeonDmActionContext());
    actionContexts.Add(new CataDungeonSfkActionContext());
    actionContexts.Add(new CataDungeonZulAmanActionContext());
    actionContexts.Add(new CataDungeonZulGurubActionContext());
    actionContexts.Add(new CataDungeonEndTimeActionContext());
    actionContexts.Add(new CataDungeonWellOfEternityActionContext());
    actionContexts.Add(new CataDungeonHourOfTwilightActionContext());
    actionContexts.Add(new MopTjsActionContext());
    actionContexts.Add(new MopSsbActionContext());
    actionContexts.Add(new MopSpmActionContext());
    actionContexts.Add(new MopMspActionContext());
    actionContexts.Add(new MopGssActionContext());
    actionContexts.Add(new MopSntActionContext());
    actionContexts.Add(new MopDungeonScarletHallsActionContext());
    actionContexts.Add(new MopDungeonScarletMonasteryActionContext());
    actionContexts.Add(new MopDungeonScholomanceActionContext());
}

// AC Bot/Engine/BuildSharedTriggerContexts.cpp:58-78 (dungeon entries, AC order; ported ones only)
void DungeonContexts::AddTriggerContexts(NamedObjectContextList<Trigger>& triggerContexts)
{
    triggerContexts.Add(new TbcDungeonHellfireRampartsTriggerContext());
    triggerContexts.Add(new TbcDungeonAuchenaiCryptsTriggerContext());
    triggerContexts.Add(new TbcDungeonSethekkHallsTriggerContext());
    triggerContexts.Add(new TbcDungeonMechanarTriggerContext());
    triggerContexts.Add(new TbcDungeonUnderbogTriggerContext());
    triggerContexts.Add(new TbcDungeonMagistersTerraceTriggerContext());
    triggerContexts.Add(new WotlkDungeonGDTriggerContext());
    triggerContexts.Add(new WotlkDungeonUKTriggerContext());
    triggerContexts.Add(new WotlkDungeonNexTriggerContext());
    triggerContexts.Add(new WotlkDungeonANTriggerContext());
    triggerContexts.Add(new WotlkDungeonOKTriggerContext());
    triggerContexts.Add(new WotlkDungeonDTKTriggerContext());
    triggerContexts.Add(new WotlkDungeonVHTriggerContext());
    triggerContexts.Add(new WotlkDungeonHoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonHoLTriggerContext());
    triggerContexts.Add(new WotlkDungeonOccTriggerContext());
    triggerContexts.Add(new WotlkDungeonUPTriggerContext());
    triggerContexts.Add(new WotlkDungeonCoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonFoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonPoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonToCTriggerContext());
    // Cataclysm + Mists of Pandaria dungeons (epic #373, MoP-only)
    triggerContexts.Add(new CataDungeonBcTriggerContext());
    triggerContexts.Add(new CataDungeonTottTriggerContext());
    triggerContexts.Add(new CataDungeonScTriggerContext());
    triggerContexts.Add(new CataDungeonVpTriggerContext());
    triggerContexts.Add(new CataDungeonGbTriggerContext());
    triggerContexts.Add(new CataDungeonHooTriggerContext());
    triggerContexts.Add(new CataDungeonLctTriggerContext());
    triggerContexts.Add(new CataDungeonDmTriggerContext());
    triggerContexts.Add(new CataDungeonSfkTriggerContext());
    triggerContexts.Add(new CataDungeonZulAmanTriggerContext());
    triggerContexts.Add(new CataDungeonZulGurubTriggerContext());
    triggerContexts.Add(new CataDungeonEndTimeTriggerContext());
    triggerContexts.Add(new CataDungeonWellOfEternityTriggerContext());
    triggerContexts.Add(new CataDungeonHourOfTwilightTriggerContext());
    triggerContexts.Add(new MopTjsTriggerContext());
    triggerContexts.Add(new MopSsbTriggerContext());
    triggerContexts.Add(new MopSpmTriggerContext());
    triggerContexts.Add(new MopMspTriggerContext());
    triggerContexts.Add(new MopGssTriggerContext());
    triggerContexts.Add(new MopSntTriggerContext());
    triggerContexts.Add(new MopDungeonScarletHallsTriggerContext());
    triggerContexts.Add(new MopDungeonScarletMonasteryTriggerContext());
    triggerContexts.Add(new MopDungeonScholomanceTriggerContext());
}

// AC Bot/Engine/BuildSharedValueContexts.cpp (dungeon entries: Mech, MgT, UB, GD; ported ones only)
void DungeonContexts::AddValueContexts(NamedObjectContextList<UntypedValue>& valueContexts)
{
    valueContexts.Add(new TbcDungeonMechValueContext());
    valueContexts.Add(new TbcDungeonMgTValueContext());
    valueContexts.Add(new TbcDungeonUnderbogValueContext());
    valueContexts.Add(new WotlkDungeonGDValueContext());
    // Cataclysm + Mists of Pandaria dungeons with values (epic #373, MoP-only)
    valueContexts.Add(new CataDungeonBcValueContext());
    valueContexts.Add(new MopMspValueContext());
    valueContexts.Add(new MopGssValueContext());
    valueContexts.Add(new MopSntValueContext());
}
