/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (map 645, "cata-bc"). Epic #373, issue #660.
// AC WotLK has no Cata dungeon, so there is no AC file to cite: this follows AC's dungeon-strategy PATTERN
// (Strategy / Triggers / Actions / Multipliers / contexts, cf. fork strategy/dungeons/HoS, UB, FoS, MgT) and uses
// the REAL ids of THIS core. Ids come from the fork's boss scripts
// (src/server/scripts/EasternKingdoms/BlackrockMountain/BlackrockCaverns/*.cpp) and the MoP test world DB.
// Intended mechanics (read 2026-10-06):
//   Wowpedia MediaWiki API: Rom'ogg Bonecrusher, Corla Herald of Twilight, Karsh Steelbender, Beauty,
//   Ascendant Lord Obsidius (Abilities + Strategy sections); Wowhead tooltip API
//   (nether.wowhead.com/mop-classic/tooltip/spell/<id>) for cast times and spell text.
// Only mechanics the fork's scripts / SmartAI rows actually drive have an edge here (see the lane report for the
// script gaps).

#ifndef PLAYERBOTS_CATABCSHARED_H
#define PLAYERBOTS_CATABCSHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include "Common.h"
#include "Position.h"

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace CataBc
{
constexpr uint32 MAP_ID = 645;

// Bosses and adds (fork blackrock_caverns.h:60-64,84-85, boss_*.cpp enums; MoP world DB creature_template)
constexpr uint32 NPC_ROMOGG                 = 39665;
constexpr uint32 NPC_CORLA                  = 39679;
constexpr uint32 NPC_KARSH                  = 39698;
constexpr uint32 NPC_BEAUTY                 = 39700;
constexpr uint32 NPC_OBSIDIUS               = 39705;
constexpr uint32 NPC_CHAINS_OF_WOE          = 40447;  // boss_romogg_bonecrusher.cpp:47
constexpr uint32 NPC_SHADOW_OF_OBSIDIUS     = 40817;  // boss_ascendant_lord_obsidius.cpp:50
constexpr uint32 NPC_TWILIGHT_ZEALOT_CORLA  = 50284;  // blackrock_caverns.h:84
constexpr uint32 NPC_NETHER_ESSENCE_TRIGGER = 49526;  // blackrock_caverns.h:85

// Spells (fork boss scripts; every id verified against the script that casts or applies it)
constexpr uint32 SPELL_QUAKE                = 75272;  // boss_romogg_bonecrusher.cpp:35, periodic damage aura on every player
constexpr uint32 SPELL_THE_SKULLCRACKER     = 75543;  // :34 (cast in the AfterCast of spell_chains_of_woe, :247), 8 s cast, 12 yd
constexpr uint32 SPELL_CHAINS_OF_WOE_ROOT   = 82192;  // :41, root on the chained players
constexpr uint32 SPELL_DARK_COMMAND         = 75823;  // boss_corla_hearld_of_twilight.cpp:39, 2 s cast
constexpr uint32 SPELL_EVOLUTION            = 75697;  // :35, stacking aura on the player standing in a beam
constexpr uint32 SPELL_SUPERHEATED_QUICKSILVER_ARMOR = 75846;  // boss_karsh_steelbender.cpp:36 (aura Karsh gains in the pillar)
constexpr uint32 SPELL_LAVA_DROOL           = 76628;  // Lucky / Buster SmartAI (smart_scripts 40008, 40013), 5 yd persistent area

// Geometry
constexpr float SKULLCRACKER_RADIUS         = 12.0f;  // Wowhead 75543: "all enemies within 12 yds"
constexpr float SKULLCRACKER_SAFE_RADIUS    = 15.5f;
constexpr float QUAKE_SAFE_RADIUS           = 6.5f;   // Wowpedia: the crack ruptures within 4 yd
constexpr uint32 QUAKE_ORIGIN_RESET_MS      = 6000;
constexpr float LAVA_DROOL_RADIUS           = 5.0f;   // SpellEffect.dbc 76628: persistent area aura, 5 yd
constexpr float LAVA_DROOL_SAFE_RADIUS      = 6.5f;
constexpr float LAVA_DROOL_SCAN             = 25.0f;
constexpr uint32 EVOLUTION_LEAVE_STACKS     = 60;     // Wowpedia: 100 stacks transforms the player; ~5.7 stacks/s here
constexpr float BEAM_CLEAR_RADIUS           = 4.5f;
constexpr float KARSH_PILLAR_X              = 237.84f;  // boss_karsh_steelbender.cpp:53 centerkarshPos
constexpr float KARSH_PILLAR_Y              = 784.76f;
constexpr float KARSH_PILLAR_Z              = 95.67f;
constexpr float KARSH_PILLAR_RADIUS         = 6.0f;   // :119 bHeat = distance <= 6
constexpr float KARSH_FAR_SIDE              = 12.0f;  // tank post beyond the pillar, so Karsh stops outside it
constexpr uint32 KARSH_HOLD_MS             = 8000;   // longest the tank waits on its post for Karsh to arrive
constexpr uint32 KARSH_MAX_STACKS           = 3;      // Wowpedia: "more than two stacks - the tank did something wrong"
constexpr float INTERRUPT_SCAN              = 30.0f;

// Cheap lookups ------------------------------------------------------------------------------------------------
Creature* FindCreature(Player* bot, uint32 entry, float range);
bool IsCasting(Unit* unit, uint32 spellId);

// Rom'ogg
Creature* GetRomogg(Player* bot);
Creature* GetChains(Player* bot);
bool IsChained(Player* bot);
bool SkullcrackerDanger(Player* bot);

// Corla
bool EvolutionDanger(Player* bot);
// Closest point of any Nether Essence beam (Nether Essence trigger -> Twilight Zealot segment) to the bot.
bool GetNearestBeamPoint(Player* bot, float& px, float& py);

// Karsh
Creature* GetKarsh(Player* bot);
uint32 KarshSuperheatStacks(Unit* karsh);
// The tank's post on the far side of the pillar, seen from Karsh (Karsh then walks through the pillar to reach it).
void GetKarshPost(Unit* karsh, float& tx, float& ty);

// Beauty
bool GetLavaDroolDanger(Player* bot, float& cx, float& cy);

// Obsidius
GuidVector FindShadowGuids(Player* bot);

// Interrupts: the closest attacker in range that is casting a spell listed in InterruptPriority()
Unit* GetInterruptTarget(Player* bot);
bool CastInterrupt(PlayerbotAI* botAI, Unit* target);

// True while one of this dungeon's "get out of the way now" mechanics applies to the bot; the evade multiplier
// then keeps formation / follow / reach movement from walking the bot back in.
bool InEvadeDanger(Player* bot);
}

#endif
