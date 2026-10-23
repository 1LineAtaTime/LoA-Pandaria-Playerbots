/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity (map 939, Cata 4.3 heroic, level 85) bot strategy "cata-woe" - issue #672, epic #373 (MoP-only dungeon
// strategy lanes, lane L8). AC has no strategy for this dungeon; this follows AC's dungeon-strategy PATTERN
// (Ai/Dungeon/<Code>/<Code>{Strategy,Triggers,Actions,Multipliers}, e.g. HoS/PoS) with the fork's own ids.
//
// Mechanics research (fetched 2026-10-06):
//   Wowpedia MediaWiki API (wowpedia.fandom.com/api.php?action=parse&page=<title>&prop=wikitext):
//     "Peroth'arn", "Queen Azshara (tactics)", "Mannoroth and Varo'then", "Well of Eternity (instance)".
//   Wowhead MoP-classic tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>): 103241 (10 sec cast), 102455, 108141.
// Real ids come from THIS core: boss scripts src/server/scripts/Kalimdor/CavernsOfTime/WellofEternity/*.cpp (file:line next
// to every id) and the MoP test world DB (creature_template).
//
// #695 (W1 lane, 2026-10-23): the gaps below are ported in the boss scripts now - Servant of the Queen + Hand of the Queen and
// Coldflame (boss_queen_azshara.cpp), Fel Firestorm + Felblade (boss_mannoroth.cpp), the hunt-phase Shadowcloak
// (boss_perotharn.cpp EVENT_ILLIDAN_HIDE) - and have edges: "woe kill hand of the queen", "woe leave fel firestorm",
// "woe use portal energy focus". Coldflame's 102466 patches are persistent area auras, which the generic "avoid aoe" covers.
// Script gaps as they were found (#672):
//   Azshara  : "Hand of the Queen" / Puppet String is declared (boss_queen_azshara.cpp:70, :59-65) but never used; Coldflame
//              is commented out (:375-379); the rest of the Enchanted Magus kit is scripted.
//   Mannoroth: Fel Firestorm is commented out (boss_mannoroth.cpp:230, :348-350) and Felblade is never scheduled (:352-355; the
//              reschedules at :382, :403 are commented out), so there are no Fel Flames to dodge in the Mannoroth fight. The
//              Varo'then blade is embedded by the script itself (EVENT_EMBEDDED_BLADE, :371-376), no click needed.
//   Peroth'arn: the Easy Prey / Shadowcloak stealth of the hunt phase is a plain 1 yd proximity check
//              (npc_perotharn_eye_of_perotharn, boss_perotharn.cpp:441-510), no aura gives players cover.
//   Instance : the three Portal Energy Focus game objects (go_npc_well_of_eternity_portal_energy_focus,
//              well_of_eternity.cpp:2443-2503) must be used before Peroth'arn is attackable; no bot edge for that gate yet.

#ifndef PLAYERBOTS_CATAWOESHARED_H
#define PLAYERBOTS_CATAWOESHARED_H

#include "Define.h"
#include <functional>
#include <initializer_list>
#include <string>
#include <vector>

class Creature;
class GameObject;
class Player;
class PlayerbotAI;
class Unit;

namespace WellOfEternity
{
constexpr uint32 WOE_MAP_WELL_OF_ETERNITY = 939;

// ---- creatures (fork well_of_eternity.h:43-52 and the boss scripts) -----------------------------------------------
constexpr uint32 WOE_NPC_PEROTHARN              = 55085;   // well_of_eternity.h:43
constexpr uint32 WOE_NPC_AZSHARA                = 54853;   // :45
constexpr uint32 WOE_NPC_VAROTHEN               = 55419;   // :49
constexpr uint32 WOE_NPC_MANNOROTH              = 54969;   // :52

constexpr uint32 WOE_NPC_FEL_FLAMES             = 57329;   // boss_perotharn.cpp:81 (Fel Flames Stalker, carries the pulse aura)
constexpr uint32 WOE_NPC_EYE_OF_PEROTHARN_1     = 55868;   // :85
constexpr uint32 WOE_NPC_EYE_OF_PEROTHARN_2     = 55879;   // :86

constexpr uint32 WOE_NPC_HAMMER_OF_DIVINITY_2   = 54865;   // boss_queen_azshara.cpp:75 (ground marker of an Arcane Bomb)
constexpr uint32 WOE_NPC_DOOMGUARD_DEBILITATOR  = 55762;   // boss_mannoroth.cpp:123
constexpr uint32 WOE_NPC_FEL_FLAMES_FIRESTORM   = 55502;   // boss_mannoroth.cpp:118 (Fel Firestorm flame, aura 103892)
constexpr uint32 WOE_NPC_HAND_OF_THE_QUEEN      = 54728;   // boss_queen_azshara.cpp:70 (rides the puppet; kill it to free them)

// ---- game objects: the three Portal Energy Focus crystals of the Illidan escort (well_of_eternity.cpp:431-433) ----------
constexpr uint32 WOE_GO_PORTAL_ENERGY_FOCUS_1   = 209366;
constexpr uint32 WOE_GO_PORTAL_ENERGY_FOCUS_2   = 209447;
constexpr uint32 WOE_GO_PORTAL_ENERGY_FOCUS_3   = 209448;

// ---- spells ------------------------------------------------------------------------------------------------------
constexpr uint32 WOE_SPELL_FEL_DECAY            = 105544;  // boss_perotharn.cpp:50
constexpr uint32 WOE_SPELL_TOTAL_OBEDIENCE      = 103241;  // boss_queen_azshara.cpp:40 (10 sec cast, "must be interrupted")
constexpr uint32 WOE_SPELL_SERVANT_OF_THE_QUEEN = 102334;  // boss_queen_azshara.cpp:38 (the puppet's charm + damage immunity)

// A circular ground hazard: a bot inside 'stand' is hurt, a bot that reaches 'clear' is safe.
struct Hazard
{
    float x;
    float y;
    float stand;
    float clear;
};

enum class HazardKind : uint8
{
    FelFlames,        // Peroth'arn ground flames (57329)
    EyeOfPerotharn,   // Peroth'arn hunt phase eyes (55868, 55879)
    ArcaneBomb,       // Enchanted Magus bomb marker (54865)
    FelFirestorm,     // Mannoroth Fel Firestorm flames (55502)
};

// ---- generic helpers (same code in every dungeon of this lane) -----------------------------------------------------
void CollectCreatures(Player* bot, std::initializer_list<uint32> entries, float radius, std::vector<Creature*>& out);
bool InsideAnyHazard(std::vector<Hazard> const& hazards, float x, float y);
bool FindClearSpot(Player* bot, std::function<float(float, float)> const& margin, float& outX, float& outY, float& outZ);
bool FindHazardEscape(Player* bot, std::vector<Hazard> const& hazards, float& outX, float& outY, float& outZ);
std::string CastInterrupt(PlayerbotAI* botAI, Unit* target);
void CollectBotMembers(Player* bot, std::vector<Player*>& out);   // alive playerbot members on the bot's map, bot included
Player* GetDesignatedBot(Player* bot);                            // tank first, then lowest guid

// ---- hazards -------------------------------------------------------------------------------------------------------
void CollectHazards(Player* bot, HazardKind kind, std::vector<Hazard>& out);
bool InHazard(Player* bot, HazardKind kind);

// ---- bosses --------------------------------------------------------------------------------------------------------
Creature* GetPerotharn(Player* bot);
Creature* GetAzshara(Player* bot);
Creature* GetMannoroth(Player* bot);
Creature* GetVarothen(Player* bot);

// Azshara: Total Obedience (103241) - the cast has to be interrupted or the group is turned into puppets.
Unit* GetObedienceTarget(Player* bot);
bool ObedienceNeedsInterrupt(Player* bot);

// Mannoroth: the Dreadlord Debilitators keep Tyrande disabled; both have to die before Mannoroth can be hurt below 88%.
Creature* GetNearestDebilitator(Player* bot);
bool NeedsDebilitatorKill(Player* bot);

// Azshara: "All DPS should switch to the hand-shaped mob above the victim's head and kill it. This frees the victim."
// (Wowpedia "Queen Azshara (tactics)"). The puppet itself is immune and is never a target (strategy exclusions).
Creature* GetHandOfTheQueen(Player* bot);
bool NeedsHandKill(Player* bot);
bool IsPuppet(Unit* unit);

// Illidan escort: the crystal's Portal Energy Focus, unlocked once its three guards are dead (fel crystal stalker
// ACTION_FEL_CRYSTAL_STALKER_UNLOCK clears GO_FLAG_LOCKED, well_of_eternity.cpp); the designated bot uses it.
GameObject* GetUsablePortalFocus(Player* bot);
bool NeedsPortalFocusUse(Player* bot);
}

#endif
