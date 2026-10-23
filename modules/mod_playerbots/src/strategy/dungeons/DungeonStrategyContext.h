/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// AC Ai/Dungeon/DungeonStrategyContext.h:1-91 (#545, epic #569). Only the dungeons already ported to this
// fork are registered; each Phase 6 child adds its include + creator + factory line here, in AC's order.

#ifndef PLAYERBOTS_DUNGEONSTRATEGYCONTEXT_H
#define PLAYERBOTS_DUNGEONSTRATEGYCONTEXT_H

#include "ACStrategy.h"
#include "AKStrategy.h"
#include "ANStrategy.h"
#include "CataBcStrategy.h"
#include "CataDmStrategy.h"
#include "CataEtStrategy.h"
#include "CataGbStrategy.h"
#include "CataHooStrategy.h"
#include "CataHotStrategy.h"
#include "CataLctStrategy.h"
#include "CataScStrategy.h"
#include "CataSfkStrategy.h"
#include "CataTottStrategy.h"
#include "CataVpStrategy.h"
#include "CataWoeStrategy.h"
#include "CataZaStrategy.h"
#include "CataZgStrategy.h"
#include "CoSStrategy.h"
#include "DTKStrategy.h"
#include "FoSStrategy.h"
#include "HoLStrategy.h"
#include "GDStrategy.h"
#include "HoSStrategy.h"
#include "MechStrategy.h"
#include "MgTStrategy.h"
#include "MopGssStrategy.h"
#include "MopMspStrategy.h"
#include "MopScholoStrategy.h"
#include "MopShStrategy.h"
#include "MopSmStrategy.h"
#include "MopSntStrategy.h"
#include "MopSpmStrategy.h"
#include "MopSsbStrategy.h"
#include "MopTjsStrategy.h"
#include "NexStrategy.h"
#include "OCStrategy.h"
#include "PoSStrategy.h"
#include "RampStrategy.h"
#include "SethStrategy.h"
#include "Strategy.h"
#include "TOCStrategy.h"
#include "UBStrategy.h"
#include "UKStrategy.h"
#include "UPStrategy.h"
#include "VHStrategy.h"

class DungeonStrategyContext : public NamedObjectContext<Strategy>
{
public:
    DungeonStrategyContext() : NamedObjectContext<Strategy>(false, true)
    {
        // Vanilla
        // ...

        // Burning Crusade
        creators["tbc-ramp"] = &DungeonStrategyContext::tbc_ramp;       // Hellfire Citadel: Hellfire Ramparts
        creators["tbc-ac"] = &DungeonStrategyContext::tbc_ac;           // Auchindoun: Auchenai Crypts
        creators["tbc-seth"] = &DungeonStrategyContext::tbc_seth;       // Auchindoun: Sethekk Halls
        creators["tbc-mech"] = &DungeonStrategyContext::tbc_mech;       // Tempest Keep: The Mechanar
        creators["tbc-ub"] = &DungeonStrategyContext::tbc_ub;           // Coilfang Reservoir: The Underbog
        creators["tbc-mgt"] = &DungeonStrategyContext::tbc_mgt;         // Magisters' Terrace

        // Wrath of the Lich King
        creators["wotlk-uk"] = &DungeonStrategyContext::wotlk_uk;       // Utgarde Keep
        creators["wotlk-nex"] = &DungeonStrategyContext::wotlk_nex;     // The Nexus
        creators["wotlk-an"] = &DungeonStrategyContext::wotlk_an;       // Azjol-Nerub
        creators["wotlk-ok"] = &DungeonStrategyContext::wotlk_ok;       // Ahn'kahet: The Old Kingdom
        creators["wotlk-gd"] = &DungeonStrategyContext::wotlk_gd;       // Gundrak
        creators["wotlk-dtk"] = &DungeonStrategyContext::wotlk_dtk;     // Drak'Tharon Keep
        creators["wotlk-vh"] = &DungeonStrategyContext::wotlk_vh;       // The Violet Hold
        creators["wotlk-hos"] = &DungeonStrategyContext::wotlk_hos;     // Halls of Stone
        creators["wotlk-hol"] = &DungeonStrategyContext::wotlk_hol;     // Halls of Lightning
        creators["wotlk-occ"] = &DungeonStrategyContext::wotlk_occ;     // The Oculus
        creators["wotlk-up"] = &DungeonStrategyContext::wotlk_up;       // Utgarde Pinnacle
        creators["wotlk-cos"] = &DungeonStrategyContext::wotlk_cos;     // The Culling of Stratholme
        creators["wotlk-toc"] = &DungeonStrategyContext::wotlk_toc;     // Trial of the Champion
        creators["wotlk-pos"] = &DungeonStrategyContext::wotlk_pos;     // Pit of Saron
        creators["wotlk-fos"] = &DungeonStrategyContext::wotlk_fos;     // The Forge of Souls

        // Cataclysm (MoP-only dungeons, no AC counterpart; epic #373)
        creators["cata-bc"] = &DungeonStrategyContext::cata_bc;      // Blackrock Caverns
        creators["cata-tott"] = &DungeonStrategyContext::cata_tott;  // Throne of the Tides
        creators["cata-sc"] = &DungeonStrategyContext::cata_sc;      // The Stonecore
        creators["cata-vp"] = &DungeonStrategyContext::cata_vp;      // The Vortex Pinnacle
        creators["cata-gb"] = &DungeonStrategyContext::cata_gb;      // Grim Batol
        creators["cata-hoo"] = &DungeonStrategyContext::cata_hoo;    // Halls of Origination
        creators["cata-lct"] = &DungeonStrategyContext::cata_lct;    // Lost City of the Tol'vir
        creators["cata-dm"] = &DungeonStrategyContext::cata_dm;      // Deadmines (revamped)
        creators["cata-sfk"] = &DungeonStrategyContext::cata_sfk;    // Shadowfang Keep (revamped)
        creators["cata-za"] = &DungeonStrategyContext::cata_za;      // Zul'Aman (5-man)
        creators["cata-zg"] = &DungeonStrategyContext::cata_zg;      // Zul'Gurub (5-man)
        creators["cata-et"] = &DungeonStrategyContext::cata_et;      // Caverns of Time: End Time
        creators["cata-woe"] = &DungeonStrategyContext::cata_woe;    // Caverns of Time: Well of Eternity
        creators["cata-hot"] = &DungeonStrategyContext::cata_hot;    // Caverns of Time: Hour of Twilight

        // Mists of Pandaria (MoP-only dungeons, no AC counterpart; epic #373)
        creators["mop-tjs"] = &DungeonStrategyContext::mop_tjs;        // Temple of the Jade Serpent
        creators["mop-ssb"] = &DungeonStrategyContext::mop_ssb;        // Stormstout Brewery
        creators["mop-spm"] = &DungeonStrategyContext::mop_spm;        // Shado-Pan Monastery
        creators["mop-msp"] = &DungeonStrategyContext::mop_msp;        // Mogu'shan Palace
        creators["mop-gss"] = &DungeonStrategyContext::mop_gss;        // Gate of the Setting Sun
        creators["mop-snt"] = &DungeonStrategyContext::mop_snt;        // Siege of Niuzao Temple
        creators["mop-sh"] = &DungeonStrategyContext::mop_sh;          // Scarlet Halls
        creators["mop-sm"] = &DungeonStrategyContext::mop_sm;          // Scarlet Monastery
        creators["mop-scholo"] = &DungeonStrategyContext::mop_scholo;  // Scholomance
    }

private:
    // #569 wave C (AC Ai/Dungeon/DungeonStrategyContext.h)
    static Strategy* tbc_ramp(PlayerbotAI* botAI) { return new TbcDungeonHellfireRampartsStrategy(botAI); }
    static Strategy* tbc_ac(PlayerbotAI* botAI) { return new TbcDungeonAuchenaiCryptsStrategy(botAI); }
    static Strategy* tbc_seth(PlayerbotAI* botAI) { return new TbcDungeonSethekkHallsStrategy(botAI); }
    static Strategy* tbc_mech(PlayerbotAI* botAI) { return new TbcDungeonMechanarStrategy(botAI); }
    static Strategy* tbc_ub(PlayerbotAI* botAI) { return new TbcDungeonUnderbogStrategy(botAI); }
    static Strategy* tbc_mgt(PlayerbotAI* botAI) { return new TbcDungeonMagistersTerraceStrategy(botAI); }
    static Strategy* wotlk_gd(PlayerbotAI* botAI) { return new WotlkDungeonGDStrategy(botAI); }
    static Strategy* wotlk_uk(PlayerbotAI* botAI) { return new WotlkDungeonUKStrategy(botAI); }
    static Strategy* wotlk_hos(PlayerbotAI* botAI) { return new WotlkDungeonHoSStrategy(botAI); }
    // #569 wave B (AC Ai/Dungeon/DungeonStrategyContext.h:51-64, 70-86)
    static Strategy* wotlk_nex(PlayerbotAI* botAI) { return new WotlkDungeonNexStrategy(botAI); }
    static Strategy* wotlk_an(PlayerbotAI* botAI) { return new WotlkDungeonANStrategy(botAI); }
    static Strategy* wotlk_ok(PlayerbotAI* botAI) { return new WotlkDungeonOKStrategy(botAI); }
    static Strategy* wotlk_dtk(PlayerbotAI* botAI) { return new WotlkDungeonDTKStrategy(botAI); }
    static Strategy* wotlk_vh(PlayerbotAI* botAI) { return new WotlkDungeonVHStrategy(botAI); }
    static Strategy* wotlk_hol(PlayerbotAI* botAI) { return new WotlkDungeonHoLStrategy(botAI); }
    static Strategy* wotlk_occ(PlayerbotAI* botAI) { return new WotlkDungeonOccStrategy(botAI); }
    static Strategy* wotlk_up(PlayerbotAI* botAI) { return new WotlkDungeonUPStrategy(botAI); }
    static Strategy* wotlk_cos(PlayerbotAI* botAI) { return new WotlkDungeonCoSStrategy(botAI); }
    static Strategy* wotlk_toc(PlayerbotAI* botAI) { return new WotlkDungeonToCStrategy(botAI); }
    static Strategy* wotlk_pos(PlayerbotAI* botAI) { return new WotlkDungeonPoSStrategy(botAI); }
    static Strategy* wotlk_fos(PlayerbotAI* botAI) { return new WotlkDungeonFoSStrategy(botAI); }
    // Cataclysm dungeons (epic #373, MoP-only: no AC counterpart)
    static Strategy* cata_bc(PlayerbotAI* botAI) { return new CataDungeonBcStrategy(botAI); }
    static Strategy* cata_tott(PlayerbotAI* botAI) { return new CataDungeonTottStrategy(botAI); }
    static Strategy* cata_sc(PlayerbotAI* botAI) { return new CataDungeonScStrategy(botAI); }
    static Strategy* cata_vp(PlayerbotAI* botAI) { return new CataDungeonVpStrategy(botAI); }
    static Strategy* cata_gb(PlayerbotAI* botAI) { return new CataDungeonGbStrategy(botAI); }
    static Strategy* cata_hoo(PlayerbotAI* botAI) { return new CataDungeonHooStrategy(botAI); }
    static Strategy* cata_lct(PlayerbotAI* botAI) { return new CataDungeonLctStrategy(botAI); }
    static Strategy* cata_dm(PlayerbotAI* botAI) { return new CataDungeonDmStrategy(botAI); }
    static Strategy* cata_sfk(PlayerbotAI* botAI) { return new CataDungeonSfkStrategy(botAI); }
    static Strategy* cata_za(PlayerbotAI* botAI) { return new CataDungeonZulAmanStrategy(botAI); }
    static Strategy* cata_zg(PlayerbotAI* botAI) { return new CataDungeonZulGurubStrategy(botAI); }
    static Strategy* cata_et(PlayerbotAI* botAI) { return new CataDungeonEndTimeStrategy(botAI); }
    static Strategy* cata_woe(PlayerbotAI* botAI) { return new CataDungeonWellOfEternityStrategy(botAI); }
    static Strategy* cata_hot(PlayerbotAI* botAI) { return new CataDungeonHourOfTwilightStrategy(botAI); }
    // Mists of Pandaria dungeons (epic #373, MoP-only: no AC counterpart)
    static Strategy* mop_tjs(PlayerbotAI* botAI) { return new MopTjsStrategy(botAI); }
    static Strategy* mop_ssb(PlayerbotAI* botAI) { return new MopSsbStrategy(botAI); }
    static Strategy* mop_spm(PlayerbotAI* botAI) { return new MopSpmStrategy(botAI); }
    static Strategy* mop_msp(PlayerbotAI* botAI) { return new MopMspStrategy(botAI); }
    static Strategy* mop_gss(PlayerbotAI* botAI) { return new MopGssStrategy(botAI); }
    static Strategy* mop_snt(PlayerbotAI* botAI) { return new MopSntStrategy(botAI); }
    static Strategy* mop_sh(PlayerbotAI* botAI) { return new MopDungeonScarletHallsStrategy(botAI); }
    static Strategy* mop_sm(PlayerbotAI* botAI) { return new MopDungeonScarletMonasteryStrategy(botAI); }
    static Strategy* mop_scholo(PlayerbotAI* botAI) { return new MopDungeonScholomanceStrategy(botAI); }
};

#endif
