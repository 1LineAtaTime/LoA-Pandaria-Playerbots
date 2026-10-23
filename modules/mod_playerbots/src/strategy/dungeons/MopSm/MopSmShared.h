/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" - Scarlet Monastery (map 1004, level 90 revamp). Epic #373, issue #658.
//
// AC WotLK mod-playerbots has no strategy for this dungeon, so it is authored on AC's dungeon-strategy PATTERN
// (AC Ai/Dungeon/HoS, UB, MgT: Strategy / Triggers / Actions / Multipliers / ActionContext / TriggerContext).
//
// Sources for the intended mechanics (fetched 2026-10-06):
//   Wowpedia MediaWiki API (wowpedia.fandom.com/api.php?action=parse&page=<title>): Thalnos_the_Soulrender,
//     Brother_Korloff, Commander_Durand, High_Inquisitor_Whitemane_(tactics) - "Spells and abilities" / "Tactics".
//   Wowhead MoP tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>) for the spell texts below.
// Ids, radii and interruptibility are checked against THIS core: the fork's boss scripts
// (src/server/scripts/EasternKingdoms/ScarletMonastery/*.cpp) and the 5.4.8 Spell/SpellEffect/SpellInterrupts DBCs.
//
// Edges (1-4 per boss; only mechanics that the fork's scripts really cast):
//   Thalnos the Soulrender - step out of Spirit Gale (aura 115291), interrupt Spirit Gale, kill Empowering Spirits
//                            (before they become Empowered Zombies) and Evicted Souls first
//   Brother Korloff        - leave Firestorm Kick (7 yd, 6 s channel), leave Scorched Earth fire (phase 2 trail)
//   Whitemane / Durand     - interrupt Mass Resurrection / Heal / Holy Smite; never kick Scarlet Resurrection (9232):
//                            Wowpedia: "2 sec cast. Uninterruptible." Since #684 the script makes her immune to interrupt
//                            effects for the resurrection and retries it until it lands, so a kick there is only a wasted
//                            interrupt - the multiplier still vetoes every interrupt action while she casts it.
//                            #684 edge: dispel her Power Word: Shield (22187, cast at the 50% switch) - Wowpedia tactics
//                            (MediaWiki API, fetched 2026-10-14): "Dispel the Power Word: Shield casts."; "While the
//                            shield holds, spells will not be interrupted by damage." (AC BT "spellsteal rune shield"
//                            shape, BTStrategy.cpp:96-97).
//
// Script gaps (mechanics the fork does NOT implement, so no edge exists for them): see the issue comment.

// Issue #781 additions (2026-10-11):
//   Thalnos - the add wave: Fallen Crusaders (59884, Raise Fallen Crusader 115139 every 1 s for 5 s) and the
//             Empowered Zombies (59930) an Empowering Spirit makes of one (boss_thalnos_the_soulrender.cpp:198-219,
//             :320-336). Wowpedia: "fallen crusaders ... should be quickly gathered into a group and killed away from
//             the boss" - the tank picks them up (AoE is left on); the DPS order is Empowering Spirit, Evicted Soul,
//             Empowered Zombie. Spirit Gale interrupts now take turns (MopDungeon::InterruptGate).
//   Durand -> Whitemane - Durand fakes his death at 0 hp (NOT_SELECTABLE | NON_ATTACKABLE, boss_high_inqusitior_
//             whitemane.cpp:149-180) and Whitemane comes out; at 50% she sleeps the group and revives him, then both
//             fight (:353-383). Wowpedia (High Inquisitor Whitemane (tactics)): Whitemane "should be focused";
//             she heals and mass-resurrects, so the damage dealers stay on her while she is attackable.
//   Korloff - Scorched Earth: while a bot stands in the fire no other movement may hold it there (HoS multiplier shape).

#ifndef PLAYERBOTS_MOPSMSHARED_H
#define PLAYERBOTS_MOPSMSHARED_H

#include "Define.h"
#include "MopDungeonShared.h"
#include "ObjectGuid.h"
#include <string>

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace MopScarletMonastery
{
constexpr uint32 MAP_SCARLET_MONASTERY = 1004;

// Creature entries: scarlet_monastery.h:74-89 (fork), identical in the MoP test world DB.
constexpr uint32 NPC_BROTHER_KORLOFF       = 59223;
constexpr uint32 NPC_COMMANDER_DURAND      = 60040;
constexpr uint32 NPC_HIGH_INQUISITOR_WHITEMANE = 3977;
constexpr uint32 NPC_THALNOS_THE_SOULRENDER = 59789;
constexpr uint32 NPC_EVICTED_SOUL          = 59974;
constexpr uint32 NPC_EMPOWERING_SPIRIT     = 59893;
constexpr uint32 NPC_SCORCHED_EARTH        = 59507;   // fire trigger Korloff leaves while moving (phase 2)
constexpr uint32 NPC_FALLEN_CRUSADER       = 59884;   // scarlet_monastery.h:85
constexpr uint32 NPC_EMPOWERED_ZOMBIE      = 59930;   // scarlet_monastery.h:88

// Spell ids: boss_thalnos_the_soulrender.cpp:28-32, boss_brother_korloff.cpp:27-32,
// boss_high_inqusitior_whitemane.cpp:38-42. A heroic cast keeps the same id (SpellMgr.cpp:471-476).
constexpr uint32 SPELL_SPIRIT_GALE           = 115289;  // 2 s cast, interruptible (flags 0x9), forms a 2 yd gale for 20 s
constexpr uint32 SPELL_SPIRIT_GALE_AURA      = 115291;  // on a player standing inside the gale (areatrigger 220)
constexpr uint32 SPELL_FIRESTORM_KICK        = 113764;  // aura on Korloff, 6 s channel, 7 yd (113766)
constexpr uint32 SPELL_SCORCHED_EARTH_FIRE   = 114464;  // aura on a 59507 trigger, damage 114465 within 2 yd
constexpr uint32 SPELL_HOLY_SMITE            = 114848;  // 2.5 s cast, interruptible
constexpr uint32 SPELL_HEAL                  = 36678;   // 2 s cast, interruptible
constexpr uint32 SPELL_MASS_RESURRECTION     = 113134;  // 10 s cast, interruptible, raises every dead Scarlet in 100 yd
constexpr uint32 SPELL_SCARLET_RESURRECTION  = 9232;    // "SPELL_REVIVE" in the script: do NOT interrupt
constexpr uint32 SPELL_WHITEMANE_SHIELD      = 22187;   // Power Word: Shield (Magic) she casts on herself at 50%

constexpr float FIRESTORM_KICK_RADIUS = 7.0f;
constexpr float FIRESTORM_KICK_MARGIN = 2.5f;
constexpr float FIRESTORM_REACT_RANGE = 16.0f;
constexpr float SPIRIT_GALE_STEP      = 4.5f;       // gale radius 2 yd (115290): a tangent step clears it
constexpr float ADD_SCAN_RANGE        = 40.0f;
constexpr float BOSS_SCAN_RANGE       = 100.0f;

// A damaging stalker the bot must stay out of.
struct Hazard
{
    uint32 entry;    // creature entry of the ground-effect carrier
    uint32 aura;     // aura it must carry to be live (0 = any)
    float radius;    // damage radius
    float margin;    // extra clearance the bot aims for
};

inline constexpr Hazard HAZARD_SCORCHED_EARTH = { NPC_SCORCHED_EARTH, SPELL_SCORCHED_EARTH_FIRE, 2.0f, 1.5f };

// Alive NPC by entry within BOSS_SCAN_RANGE ("find target" needs the unit on the bot's threat list; entry lookups do not).
Creature* FindBoss(Player* bot, uint32 entry);

// Nearest live hazard carrier closer than radius + margin; clearDistance = the distance the bot must reach.
Creature* FindNearestHazard(Player* bot, Hazard const& hazard, float& clearDistance);

// Interrupt priority of a cast: 1 = most urgent, 0 = not on the list (Scarlet Resurrection is deliberately absent).
uint8 InterruptPriority(uint32 spellId);
uint8 CurrentCastPriority(Unit* caster);

// Thalnos or Whitemane, whichever is casting the most urgent interruptible spell; nullptr when neither is.
Unit* FindInterruptTarget(Player* bot);

// Cast the bot's interrupt (or hard control) on target; returns the spell name used, empty when none was possible.
std::string CastInterrupt(PlayerbotAI* botAI, Unit* target);

// True for the names of the bot interrupt / hard-control actions (class strategies name them after the spell).
bool IsInterruptActionName(std::string const& name);

// Whitemane is casting Scarlet Resurrection right now.
bool WhitemaneIsResurrecting(Player* bot);

// #684: Whitemane alive, in combat, attackable by the bot and wearing her Power Word: Shield; nullptr otherwise.
Unit* FindShieldedWhitemane(Player* bot);

// The bot knows an offensive magic dispel (spellsteal / purge / dispel magic / tranquilizing shot).
bool HasShieldDispel(PlayerbotAI* botAI);

// Cast the bot's offensive dispel on target; returns the spell name used, empty when none was possible.
std::string CastShieldDispel(PlayerbotAI* botAI, Unit* target);

// Korloff is channelling Firestorm Kick and the bot is within FIRESTORM_REACT_RANGE of him.
bool FirestormKickNear(Player* bot, Creature*& boss);

// Best add for a damage dealer to kill first (Empowering Spirit, Evicted Soul, then #781 Empowered Zombie), nearest of
// its kind; nullptr when none or when the bot is a tank / healer.
Unit* FindFocusAdd(Player* bot);

// #781 add wave: the tank picks up Thalnos' Fallen Crusaders / Empowered Zombies that are not on him.
inline MopDungeon::WaveSpec ThalnosTankWave()
{
    return {NPC_THALNOS_THE_SOULRENDER, {NPC_EMPOWERED_ZOMBIE, NPC_FALLEN_CRUSADER}, MopDungeon::WaveRole::Tank, 50.0f};
}

// #781 phase switch: the damage dealers stay on Whitemane while she is attackable (Durand down, or both up after 50%);
// the tank keeps the generic tank target (in phase 2 that is Whitemane anyway, in phase 3 it holds Durand off them).
inline MopDungeon::WaveSpec WhitemaneSwitch()
{
    return {0, {NPC_HIGH_INQUISITOR_WHITEMANE}, MopDungeon::WaveRole::Dps, 60.0f};
}
}

#endif
