/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep, revamped (map 33, "cata-sfk"). Epic #373, issue #668.
// AC WotLK has no Cata dungeon, so there is no AC file to cite: this follows AC's dungeon-strategy PATTERN
// (Strategy / Triggers / Actions / Multipliers / contexts, cf. fork strategy/dungeons/HoS, UB, FoS, MgT) and uses
// the REAL ids of THIS core. Ids: the fork's boss scripts
// (src/server/scripts/EasternKingdoms/ShadowfangKeep/boss_*.cpp, shadowfang_keep.h), the MoP test world DB
// (creature_template ScriptName / entry) and 5.4.8 SpellEffect.dbc (summon entries, radii).
// Intended mechanics (read 2026-10-06): Wowpedia MediaWiki API, pages Baron Ashbury (tactics), Baron Silverlaine,
// Commander Springvale, Lord Walden (tactics), Lord Godfrey (tactics).
// Only mechanics the fork's scripts actually drive have an edge here. Script gaps (no edge written): Springvale's
// Word of Shame and the Tormented Officer's Forsaken Ability (EVENT_FORSAKEN_ABILITY is an empty case,
// boss_commander_springvale.cpp:338; 7054's periodic dummy needs a spell script bound in the world DB, #682).
// #682 (2026-10-14): Walden's Toxic Catalyst now hits only players that move (npc_valden_mystery_toxin; TCPP
// boss_lord_walden.cpp spell_walden_toxic_catalyst), so "stand still when red" below has a mechanic to answer.
//   Ashbury   : 1 edge  - interrupt Pain and Suffering
//   Silverlaine: 2 edges - kill the Worgen spirits first, interrupt Soul Drain / Blinding Shadows
//   Springvale: 3 edges - leave Desecration, interrupt Unholy Empowerment (heroic adds), leave the Shield cone
//   Walden    : 3 edges - spread from a Poisonous Mixture target, keep moving during Ice Shards / Toxic Coagulant,
//               AC Hodir "biting cold" cheat at 2 Coagulant stacks; multiplier: no rooting casts while the Coagulant
//               ticks, stand still while the red Catalyst cauldron is up
//   Godfrey   : 3 edges - interrupt Cursed Bullets, leave the Pistol Barrage cone (heroic, tank included), kill the
//               ghouls; multiplier: no class interrupt spent on the (uninterruptible) barrage

#ifndef PLAYERBOTS_CATASFKSHARED_H
#define PLAYERBOTS_CATASFKSHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include "Common.h"
#include "Position.h"
#include <initializer_list>
#include <string>
#include <vector>

class Creature;
class Player;
class PlayerbotAI;
class Spell;
class Unit;

namespace CataSfk
{
constexpr uint32 MAP_ID = 33;

// Bosses (fork shadowfang_keep.h:83-88) and the creatures the mechanics spawn (MoP world DB creature_template)
constexpr uint32 NPC_ASHBURY              = 46962;
constexpr uint32 NPC_SILVERLAINE          = 3887;
constexpr uint32 NPC_SPRINGVALE           = 4278;
constexpr uint32 NPC_WALDEN               = 46963;  // creature_template name "Lord Walden", script boss_lord_valden
constexpr uint32 NPC_GODFREY              = 46964;
constexpr uint32 NPC_ODO_BLINDWATCHER     = 50857;  // shadowfang_keep.h:89
constexpr uint32 NPC_WOLF_MASTER_NANDOS   = 50851;
constexpr uint32 NPC_RETHILGORE           = 50834;
constexpr uint32 NPC_RAZORCLAW_BUTCHER    = 50869;
constexpr uint32 NPC_LUPINE_SPECTRE       = 50923;  // shadowfang_keep.h:85
constexpr uint32 NPC_DESECRATION_STALKER  = 50503;  // boss_commander_springvale.cpp:85 (summoned by 93688)
constexpr uint32 NPC_WAILING_GUARDSMAN    = 50613;  // :83
constexpr uint32 NPC_TORMENTED_OFFICER    = 50615;  // :84
constexpr uint32 NPC_MYSTERY_TOXIN_GREEN  = 50522;  // boss_lord_valden.cpp:57, Toxic Coagulant cauldron (93695)
constexpr uint32 NPC_MYSTERY_TOXIN_RED    = 50439;  // boss_lord_valden.cpp:58, Toxic Catalyst cauldron (93563, heroic <= 35%)
constexpr uint32 NPC_BLOODTHIRSTY_GHOUL   = 50561;  // boss_lord_godfrey.cpp:64

// Spells (every id is cast / applied by a fork script)
constexpr uint32 SPELL_PAIN_AND_SUFFERING = 93581;  // boss_baron_ashbury.cpp:43, channeled DoT on a random player
constexpr uint32 SPELL_SOUL_DRAIN         = 93863;  // boss_baron_silverlaine.cpp:190 (Rethilgore), 4 s channel
constexpr uint32 SPELL_BLINDING_SHADOWS   = 93952;  // :183 (Odo the Blindwatcher)
constexpr uint32 SPELL_DESECRATION        = 93687;  // boss_commander_springvale.cpp:37 (the aura 93690 / 93691 sits on the stalker)
constexpr uint32 SPELL_UNHOLY_EMPOWERMENT = 93844;  // :56, cast by the guardsman / officer on Springvale (heal + Unholy Power)
constexpr uint32 SPELL_SHIELD_OF_PERFIDIOUS = 93693;  // :39, self channel; 93722 (15-20 yd cone) is its damage
constexpr uint32 SPELL_ICE_SHARDS         = 93527;  // boss_lord_valden.cpp:30, 5 s self channel
constexpr uint32 SPELL_POISONOUS_MIXTURE  = 93697;  // :31, thrown at a random non-tank player (1.5 s cast, 15 yd here)
constexpr uint32 SPELL_TOXIC_COAGULANT    = 93617;  // :41, green cauldron DoT on players: 11.1k/s a stack, 3 stacks, 6 s
constexpr uint32 SPELL_CURSED_BULLET      = 93629;  // boss_lord_godfrey.cpp:41
constexpr uint32 SPELL_PISTOL_BARRAGE     = 93520;  // :32, channel; the cone damage aura 93566 / 93564 sits on Godfrey
constexpr uint32 SPELL_PISTOL_BARRAGE_DMG_AURA = 93566;  // :34

// Geometry (SpellEffect.dbc radii cross-checked with Wowpedia)
constexpr float DESECRATION_RADIUS        = 8.0f;   // 93691: 8 yd damage / 6 yd slow
// 93697: effect 0 (25k) is 15 yd in SpellEffect.dbc (radius 18) and the fork widens the 5.7k/s DoT + slow (effects 1/2)
// to 15 yd too (src/server/game/Spells/SpellMgr.cpp:4406-4408). Wowpedia's 4 yd is the DBC max radius 26 (unused here).
constexpr float POISON_MIXTURE_RADIUS     = 15.0f;
constexpr float SHIELD_CONE_HALF_ANGLE    = 0.87f;  // 50 deg each side (93722 is a TARGET_UNIT_CONE_ENEMY, 15-20 yd)
constexpr float SHIELD_CONE_RANGE         = 22.0f;
// 93564 has no SpellTargetRestrictions cone, so a creature's TARGET_UNIT_CONE_ENEMY_24 is M_PI/2 wide (Spell.cpp:1148):
// 45 deg each side, + 5 deg margin (Wowpedia: 60 deg cone)
constexpr float BARRAGE_CONE_HALF_ANGLE   = 0.87f;
constexpr float BARRAGE_CONE_RANGE        = 62.0f;  // 93564: 60 yd
constexpr float FLEE_MARGIN               = 1.5f;
constexpr float SCAN_RANGE                = 40.0f;
constexpr float FOCUS_RANGE               = 40.0f;
constexpr float INTERRUPT_SCAN            = 35.0f;
constexpr float KEEP_MOVING_STEP          = 8.0f;   // ~1.1 s of running, so the bot is moving on most 1 s Coagulant ticks
// The cauldron spells reach 100 yd here (93617 / 93689 radius 12, SpellMgr.cpp:4413-4422); Walden's room is ~45 yd across.
constexpr float TOXIN_RANGE               = 60.0f;
constexpr uint8 COAGULANT_JUMP_STACKS     = 2;      // AC UldTriggers.cpp:541 (Hodir Biting Cold: 2 stacks)

// Deflection of a lateral escape point from the source's facing (see LateralPoint)
constexpr float LATERAL_ANGLE = 1.396f;  // 80 deg

// Generic helpers (same code in CataLct / CataDm / CataSfk; each folder stands alone) -----------------------------
// Alive creatures of any of `entries` within `range` of the bot (grid search, same call as UBShared.cpp:94).
std::vector<Creature*> FindCreatures(Player* bot, std::initializer_list<uint32> entries, float range);
// The nearest of those within `maxDist` (2D), or nullptr.
Creature* NearestCreature(Player* bot, std::initializer_list<uint32> entries, float range, float maxDist);
// The current cast (generic or channeled) of `caster` if it is one of `spellIds`, else nullptr.
Spell* FindCast(Unit* caster, std::initializer_list<uint32> spellIds);
// True while `unit` casts one of `spellIds`, or carries the aura of that id (channels live as a self aura).
bool IsCastingOrAura(Unit* unit, std::initializer_list<uint32> spellIds);
// `unit` lies inside the cone of `source`'s facing (rear == true: facing + 180 deg) within `maxDist`.
bool InCone(Unit* source, Unit* unit, float halfAngle, float maxDist, bool rear);
// A point beside `source` (80 deg off its facing, on the side the bot already stands) - outside a front cone of
// half angle < 80 deg and outside a rear cone of half angle < 100 deg.
bool LateralPoint(Player* bot, Unit* source, float& x, float& y, float& z);
// LateralPoint, but the mirrored side too when a wall stops the run on the bot's side; true only for a collision-checked
// point outside `source`'s front cone of `halfAngle`.
bool ConeEscapePoint(Player* bot, Unit* source, float halfAngle, float& x, float& y, float& z);
// The highest-priority alive attackable unit (entries listed in priority order) within maxDist, or nullptr when the
// bot already fights a target of equal or higher priority.
Unit* PickFocus(PlayerbotAI* botAI, Player* bot, std::initializer_list<uint32> priority, float maxDist);
// The nearest creature of `casterEntries` currently casting one of `spellIds`, or nullptr.
Unit* InterruptCaster(Player* bot, std::initializer_list<uint32> casterEntries, std::initializer_list<uint32> spellIds,
                      float range);
// Casts the first ready class interrupt on `target`; returns its name, or an empty string.
std::string CastInterrupt(PlayerbotAI* botAI, Unit* target);
// `spellName` is one of the class interrupts CastInterrupt uses.
bool IsClassInterrupt(std::string const& spellName);

// Edges -------------------------------------------------------------------------------------------------------
// Baron Ashbury: interrupt his Pain and Suffering channel (Wowpedia heroic: "should be interrupted or dispelled").
Unit* AshburyInterruptTarget(PlayerbotAI* botAI, Player* bot);

// Baron Silverlaine: damage dealers kill the summoned Worgen spirits (Rethilgore first: Soul Drain heals twice what it
// drains; then Odo, Razorclaw, Nandos and his Lupine Spectres) before Silverlaine (Wowpedia: "killed before resuming DPS").
Unit* WorgenSpiritFocus(PlayerbotAI* botAI, Player* bot);

// Baron Silverlaine: interrupt Rethilgore's Soul Drain (immobilises, heals him twice the drain) and Odo's Blinding
// Shadows.
Unit* WorgenInterruptTarget(PlayerbotAI* botAI, Player* bot);

// Commander Springvale: leave Desecration (the stalker 50503 under his feet: 8 yd of damage and a 50% slow); the tank
// walking out takes the boss with it (Wowpedia: "the tank will need to move him away from Desecrate").
bool DesecrationDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// Commander Springvale, heroic: interrupt the Wailing Guardsman / Tormented Officer casting Unholy Empowerment (heals
// him 4% and gives him a charge of Unholy Power: Shield of the Perfidious / Word of Shame).
Unit* EmpowermentInterruptTarget(PlayerbotAI* botAI, Player* bot);

// Commander Springvale: while he channels Shield of the Perfidious (stationary, a frontal cone of 15-20 yd) a non-tank
// bot in front of him steps beside him. Returns Springvale.
Unit* ShieldConeBoss(PlayerbotAI* botAI, Player* bot);

// Lord Walden: while he casts Conjure Poisonous Mixture (1.5 s) at a random player, the bottle lands where that player
// stands when the cast ends and splashes 15 yd on this core (heroic 25k + 5.7k/s for 15 s, about a bot's health).
// Everyone else within 16.5 yd of the target leaves it (pos = the target); the target walks away from the nearest group
// member still inside the splash (pos = that member). radius = how far from pos to go.
bool PoisonMixtureDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// A point `dist` yd from `from` on the line from `from` through the bot (or up to 90 deg off it when a wall is in the
// way), collision-checked like LateralPoint.
bool AwayPoint(Player* bot, Position const& from, float dist, float& x, float& y, float& z);

// Lord Walden: while Ice Shards rain down (5 s channel, 2 yd per shard), a green Toxic Coagulant cauldron is up or the
// bot still carries Toxic Coagulant (a stack drops on every 1 s tick the player is moving, boss_lord_valden.cpp:240) a
// standing bot hops 8 yd. Never while the red Toxic Catalyst cauldron is up (CatalystUp).
bool KeepMovingDanger(PlayerbotAI* botAI, Player* bot);

// Lord Walden, heroic: the bot carries Toxic Coagulant (the multiplier holds back casts that root it while it ticks).
bool CoagulantTicking(Player* bot);

// Lord Walden, heroic: a red Toxic Catalyst cauldron is up near the bot - stand still (Wowpedia Lord Walden (tactics):
// "If it appears red, everyone must stand still"; #682: the catalyst hits only players that move).
bool CatalystUp(PlayerbotAI* botAI, Player* bot);

// Lord Walden, heroic: AC's Hodir Biting Cold pattern (UldTriggers.cpp:527-543, UldActions.cpp:1452-1489). At 2+ stacks
// of Toxic Coagulant, with Walden alive and the master alive and not at 2+ stacks itself, the raid cheat drops them.
bool CoagulantJump(PlayerbotAI* botAI, Player* bot);

// AC botAI->HasCheat(BotCheatMask::raid). This fork has no per-bot cheat mask, so the configured AiPlayerbot.BotCheats
// (default "food,taxi,raid", AC PlayerbotAIConfig.cpp:545-563) is the whole check (the HasFoodCheat reader,
// NonCombatActions.cpp:38-56).
bool HasRaidCheat();

// Lord Godfrey: interrupt Cursed Bullets (Wowpedia: "can and should be interrupted"; the fork's SpellHit honours it,
// boss_lord_godfrey.cpp:126-132).
Unit* CursedBulletsInterruptTarget(PlayerbotAI* botAI, Player* bot);

// Lord Godfrey, heroic: while Pistol Barrage runs (6 s, 30k a second in a 60 degree cone, 60 yd, towards a random
// player) any bot inside the cone in front of him steps out of it - the tank too: Godfrey stops attacking for the whole
// barrage. Returns Godfrey.
Unit* BarrageConeBoss(PlayerbotAI* botAI, Player* bot);

// Lord Godfrey, heroic: he casts / channels Pistol Barrage (93520). It cannot be interrupted (no SpellCategories row,
// PreventionType 0; Spell::EffectInterruptCast needs SPELL_PREVENTION_TYPE_SILENCE), so the multiplier keeps the class
// interrupts for the Cursed Bullets that follow it.
bool BarrageCasting(Player* bot);

// Lord Godfrey: damage dealers kill the Bloodthirsty Ghouls he raises (very low health) before Godfrey.
Unit* GhoulFocus(PlayerbotAI* botAI, Player* bot);


// True while one of this dungeon's "get out of the way now" mechanics applies to the bot (the avoid multiplier then
// keeps follow / reach / chase movement from walking the bot straight back in).
bool InDanger(PlayerbotAI* botAI, Player* bot);
}

#endif
