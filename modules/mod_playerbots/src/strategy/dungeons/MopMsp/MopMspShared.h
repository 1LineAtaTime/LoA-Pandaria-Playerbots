/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace (MoP 5.4.8, map 994) dungeon strategy "mop-msp" - shared ids and helpers (epic #373, issue #654).
//
// AC (WotLK) has no strategy for this dungeon: it is MoP-only content, so this EXTENDS AC's Ai/Dungeon pattern
// (UB / HoS / MgT: Strategy + Triggers + Actions + Multipliers + ValueContext) and does not port an AC file.
// Pattern anchors: fork strategy/dungeons/UB/UBStrategy.cpp (triggers + multipliers + AppendTargetExclusions),
// strategy/dungeons/MgT/MgT{Triggers,Actions,Multipliers,ValueContext}* (value-backed escape + focus + interrupt).
//
// Sources for the intended mechanics (fetched 2026-10-06, evidence only):
//   Wowpedia MediaWiki API: "Trial of the King" (Kuai: ignore Mu'Shiba, dodge Shockwave; Haiyan: be aware of the
//   Conflagrate target and Meteor; Ming: dodge Whirling Dervish, run away from Magnetic Field), "Gekkan" (kill the
//   entourage one by one, interrupt the casters), "Xin the Weaponmaster" (Ground Slam: go to the side; Circle of
//   Flame: leave the spot; whirling axes: knockback).
//   Wowhead MoP tooltip API nether.wowhead.com/mop-classic/tooltip/spell/<id>: 119922 Shockwave (growing cone,
//   60 yd), 120195 Meteor (800000 Fire divided amongst all targets within 10 yd of the impact), 120160/120167
//   Conflagrate (50000 Fire per second for 5 s, spreads at melee range), 119684 Ground Slam (30 yd cone),
//   118940 Cleansing Flame (4 s cast, heals allies for ~1.2M), 118903 Hex of Lethargy, 123654 Lightning Bolt (3 s
//   cast), 119981 Whirling Dervish, 119590 Ring of Fire (explodes, knock up).
// Ids: this fork's scripts src/server/scripts/Pandaria/MogushanPalace/{boss_trial_of_the_king,boss_gekkan,
// boss_xin_the_weaponmaster}.cpp and the MoP test DB creature_template (checked 2026-10-06).
//
// Not covered, on purpose (see the issue comment): Magnetic Field (the pull grows 10/20/40 yd every 0.5 s, so no
// spot is safe - healer check), Whirling-blade traps and Crossbow darts (lines that depend on world-trigger
// placement), Traumatic Blow (tank debuff), Mu'Shiba's Ravage (ignored per Wowpedia).

#ifndef PLAYERBOTS_MOPMSPSHARED_H
#define PLAYERBOTS_MOPMSPSHARED_H

#include "Define.h"
#include "ObjectGuid.h"

class Player;
class PlayerbotAI;
class Unit;

namespace MopMsp
{
constexpr uint32 MSP_MAP_ID = 994;

// Trial of the King (three clan leaders fought one after another)
constexpr uint32 MSP_NPC_KUAI_THE_BRUTE = 61442;
constexpr uint32 MSP_NPC_MING_THE_CUNNING = 61444;
constexpr uint32 MSP_NPC_HAIYAN_THE_UNSTOPPABLE = 61445;
constexpr uint32 MSP_NPC_MU_SHIBA = 61453;
constexpr uint32 MSP_NPC_WHIRLING_DERVISH = 61626;

// Gekkan and his four protectors
constexpr uint32 MSP_NPC_GEKKAN = 61243;
constexpr uint32 MSP_NPC_GLINTROK_IRONHIDE = 61337;
constexpr uint32 MSP_NPC_GLINTROK_SKULKER = 61338;
constexpr uint32 MSP_NPC_GLINTROK_ORACLE = 61339;
constexpr uint32 MSP_NPC_GLINTROK_HEXXER = 61340;

// Xin the Weaponmaster
constexpr uint32 MSP_NPC_XIN_THE_WEAPONMASTER = 61398;
constexpr uint32 MSP_NPC_ANIMATED_AXE = 61451;
constexpr uint32 MSP_NPC_RING_OF_FIRE = 61499;

// spells (ids from the fork's boss scripts)
constexpr uint32 MSP_SPELL_LIGHTNING_BOLT = 123654;        // Ming, 3 s cast on the tank
constexpr uint32 MSP_SPELL_WHIRLING_DERVISH_AURA = 119982; // the dervish trigger's own aura
constexpr uint32 MSP_SPELL_WHIRLING_DERVISH_TARGETS = 119994;
constexpr uint32 MSP_SPELL_SHOCKWAVE = 119922;             // Kuai, cone facing the picked player
constexpr uint32 MSP_SPELL_CONFLAGRATE_AURA = 120160;      // Haiyan, fire DoT that spreads at melee range
constexpr uint32 MSP_SPELL_METEOR = 120195;                // Haiyan, 4 s cast, damage split within 10 yd
constexpr uint32 MSP_SPELL_CLEANSING_FLAME = 118940;       // Glintrok Oracle, 4 s raid heal
constexpr uint32 MSP_SPELL_HEX_OF_LETHARGY = 118903;       // Glintrok Hexxer, -50% cast speed, stacks
constexpr uint32 MSP_SPELL_GROUND_SLAM = 119684;           // Xin, 3 s cast, 30 yd cone on the tank
constexpr uint32 MSP_SPELL_WHIRLWIND = 119373;             // Xin's animated axes
constexpr uint32 MSP_SPELL_EXPLODE_FIRE_RING = 119590;

// tuning
constexpr float MSP_DERVISH_SAFE_DIST = 8.5f;
constexpr float MSP_AXE_FALLBACK_RADIUS = 8.0f;
constexpr float MSP_RING_SAFE_DIST = 10.5f;     // the ring (r=5) is drawn around the creature's start point
constexpr float MSP_CONFLAGRATE_SPREAD = 8.0f;  // Conflagrate spreads at "melee range"
constexpr float MSP_METEOR_STACK_DIST = 5.5f;   // damage is shared within 10 yd of the impact
constexpr float MSP_HAZARD_SCAN = 28.0f;
// #654 w47: Xin keeps his throne dais (z 28.8, the room floor is z ~22.4); a healer on the floor 22-29 yd away has no LOS
// to the tank, so PartyMemberToHeal::Check drops it and no heal trigger fires. The healer walks back into range + LOS.
constexpr float MSP_HEALER_LEASH_MAX = 30.0f;   // tank farther than this (2D) or out of LOS: walk back
constexpr float MSP_HEALER_LEASH_DIST = 12.0f;  // ... to this far from the tank (or half the distance, if closer)

// which edge produced the current hazard; the strategy has one trigger per kind, all use "mop msp escape"
enum class HazardKind : uint8
{
    None = 0,
    FireRing,
    GroundSlam,
    Shockwave,
    Axe,
    Dervish,
    Conflagrate,
    MeteorStack,
    HealerLeash
};

// Where the bot must go to be safe (or to stack), 2D; z is validated by the action.
struct Hazard
{
    HazardKind kind = HazardKind::None;
    float x = 0.0f;
    float y = 0.0f;
};

enum class InterruptGroup : uint8
{
    Ming = 0,
    Gekkan
};

// The most urgent hazard for this bot right now (kind None when there is none).
bool ComputeHazard(Player* bot, Hazard& out);

// Healer bots in Xin's fight: Xin's current victim (a living group member other than the bot), else nullptr.
Unit* HealerLeashTarget(Player* bot);

// DPS bots in Gekkan's room: the protector to kill first (Oracle, Hexxer, Skulker, Ironhide). Empty otherwise.
ObjectGuid ComputeFocusTarget(Player* bot);

// Mu'Shiba (Kuai's pet) is never a target (Wowpedia: "Ignore Mu'Shiba throughout and focus on Kuai").
GuidVector ComputeExclusions(Player* bot);

// A hostile caster of an interrupt-worthy spell of this group within 40 yd of the bot, or nullptr.
Unit* FindInterruptTarget(Player* bot, InterruptGroup group);

// Does the bot know at least one interrupt spell (kick, pummel, wind shear, mind freeze, counterspell, ...).
bool HasInterruptSpell(PlayerbotAI* botAI);

// Casts the first available interrupt on `target`. True when a spell was started.
bool CastInterrupt(PlayerbotAI* botAI, Unit* target);
}

#endif
