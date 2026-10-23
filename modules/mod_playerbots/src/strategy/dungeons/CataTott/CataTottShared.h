/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (map 643, "cata-tott"). Epic #373, issue #661.
// AC WotLK has no Cata dungeon, so there is no AC file to cite: this follows AC's dungeon-strategy PATTERN
// (Strategy / Triggers / Actions / Multipliers / contexts, cf. fork strategy/dungeons/HoS, UB, FoS, MgT) and uses
// the REAL ids of THIS core. Ids come from the fork's boss scripts
// (src/server/scripts/EasternKingdoms/ThroneOfTheTides/*.cpp) and the MoP test world DB.
// Intended mechanics (read 2026-10-06):
//   Wowpedia MediaWiki API: Throne of the Tides, Lady Naz'jar (tactics), Commander Ulthok, Mindbender Ghur'sha,
//   Ozumat (Abilities + Strategy/Tactics sections); Wowhead tooltip API
//   (nether.wowhead.com/mop-classic/tooltip/spell/<id>) for cast times and spell text.
// Ozumat event edges (#700, 2026-10-14): Wowpedia MediaWiki API page "Ozumat" (Abilities + Tactics; there is no
// "Ozumat (tactics)" page), fetched 2026-10-14: the stage 1-2 adds attack Neptulon and "must all be tanked and killed",
// Neptulon dying resets the encounter, stage 3 is a DPS race on Ozumat. Tidal Surge (76133) and the stage 3 blight
// (83608) have SpellRadius 28 = 50000 yd (SpellEffect.dbc), so no "stay near Neptulon" edge is needed.
// Only mechanics the fork's scripts / SmartAI rows actually drive have an edge here (see the lane report for the
// script gaps).

#ifndef PLAYERBOTS_CATATOTTSHARED_H
#define PLAYERBOTS_CATATOTTSHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include "Common.h"
#include "Position.h"

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace CataTott
{
constexpr uint32 MAP_ID = 643;

// Bosses and adds (fork throne_of_the_tides.h:24-27, boss_*.cpp enums; MoP world DB creature_template)
constexpr uint32 NPC_LADY_NAZJAR         = 40586;
constexpr uint32 NPC_COMMANDER_ULTHOK    = 40765;
constexpr uint32 NPC_ERUNAK              = 40825;
constexpr uint32 NPC_GHURSHA             = 40788;
constexpr uint32 NPC_TEMPEST_WITCH       = 44404;  // boss_lady_nazjar.cpp:81
constexpr uint32 NPC_HONOR_GUARD         = 40633;  // :82
constexpr uint32 NPC_WATERSPOUT          = 48571;  // :83
constexpr uint32 NPC_WATERSPOUT_H        = 49108;  // :84
constexpr uint32 NPC_GEYSER              = 40597;  // :85
constexpr uint32 NPC_DARK_FISSURE        = 40784;  // boss_commander_ulthok.cpp:50
constexpr uint32 NPC_EARTH_SHARDS        = 45469;  // boss_mindebender_ghursha.cpp:77
constexpr uint32 NPC_MIND_FOG            = 40861;  // :78
constexpr uint32 NPC_FACELESS_SAPPER     = 44752;  // boss_ozumat.cpp:92
constexpr uint32 NPC_BLIGHT_OF_OZUMAT    = 44801;  // :94 (stage 2 pools)
constexpr uint32 NPC_NEPTULON            = 40792;  // throne_of_the_tides.h:32, the NPC the Ozumat event defends
constexpr uint32 NPC_OZUMAT              = 44566;  // throne_of_the_tides.h:28, attackable in stage 3 only
constexpr uint32 NPC_DEEP_MURLOC_INVADER = 44658;  // boss_ozumat.cpp:89 (stage 1 packs)
constexpr uint32 NPC_VICIOUS_MINDLASHER  = 44715;  // :90 (stage 1, three kills end it)
constexpr uint32 NPC_UNYIELDING_BEHEMOTH = 44648;  // :91 (stage 1)
constexpr uint32 NPC_BLIGHT_BEAST        = 44841;  // :93 (stage 2)

// Spells (fork boss scripts; every id verified against the script that casts or applies it)
constexpr uint32 SPELL_SHOCK_BLAST       = 76008;  // boss_lady_nazjar.cpp:35, 2 s cast, 70k nature on the tank
constexpr uint32 SPELL_WATERSPOUT        = 75683;  // :40, Naz'jar's channel at 66% / 33%
constexpr uint32 SPELL_CHAIN_LIGHTNING   = 75813;  // :54, Tempest Witch, 2 s cast, repeated every 2 s
constexpr uint32 SPELL_LIGHTNING_SURGE   = 75992;  // :55, charge on a random player, 5 s later 56k nature to friends within 8 yd
constexpr uint32 SPELL_LAVA_BOLT         = 76171;  // boss_mindebender_ghursha.cpp:44, Erunak, interruptible
constexpr uint32 SPELL_ABSORB_MAGIC      = 76307;  // :50, Ghur'sha absorbs spell damage for 3 s
constexpr uint32 SPELL_SHADOW_BOLT       = 83914;  // boss_ozumat.cpp:58, Vicious Mindlasher
constexpr uint32 SPELL_BRAIN_SPIKE       = 83915;  // :56
constexpr uint32 SPELL_VEIL_OF_SHADOW    = 83926;  // :57 (heroic)

// Geometry (Wowpedia radius; every hazard NPC is a persistent ground effect that lives as long as the effect)
constexpr float HAZARD_MARGIN            = 1.0f;   // a bot counts as inside at radius + margin
constexpr float HAZARD_SAFE_EXTRA        = 1.5f;   // and runs to radius + this
constexpr float SURGE_RADIUS             = 8.0f;   // Lightning Surge: friends within 8 yd take the hit
constexpr float SCAN_RANGE               = 80.0f;
constexpr float INTERRUPT_SCAN           = 30.0f;

// The ground effects a bot steps out of: each is a creature that exists for the lifetime of the effect.
enum class HazardId : uint8
{
    GEYSER,       // Naz'jar: Summon Geyser, erupts after 5 s (23k frost + knockback within 5 yd)
    VORTEX,       // Naz'jar heroic Waterspout vortexes: a player touching one is sucked in and stunned
    FISSURE,      // Ulthok: Dark Fissure (9k shadow per second within 6 yd, grows on heroic)
    MIND_FOG,     // Ghur'sha: Mind Fog, 10 yd, no attacks or casts inside
    EARTH_SHARDS, // Erunak: Earth Shards, 5 yd, 10 s
    BLIGHT_POOL,  // Ozumat: Blight of Ozumat pool, 10 yd, 30 s
    COUNT
};

// What the DPS bots drop everything for
enum class FocusId : uint8
{
    NAZJAR_ADDS,  // Waterspout phase: Tempest Witches first, then the Honor Guard (the boss itself is unreachable)
    SAPPERS,      // Ozumat stage 2: the three Faceless Sappers end the stage
    NEPTULON_ADDS, // Ozumat stages 1-2: the adds beating on Neptulon (he is passive; if he dies the encounter resets)
    OZUMAT,       // Ozumat stage 3: Ozumat himself, once Tidal Surge makes him attackable (DPS race)
    COUNT
};

// Cheap lookups ------------------------------------------------------------------------------------------------
Creature* FindCreature(Player* bot, uint32 entry, float range);

// The nearest live hazard creature of this kind the bot is standing in (or next to); nullptr when clear.
Creature* GetHazardCreature(Player* bot, HazardId id);
float HazardSafeRadius(HazardId id);
bool InAnyHazard(Player* bot);

// Lightning Surge carrier: keep away from the group
bool HasLightningSurge(Player* bot);
Player* GetNearestGroupMember(Player* bot, float maxDist);

// Focus targets
bool IsFocusDps(Player* bot);
// The Ozumat event is running: Neptulon is within reach and fighting (the adds attack HIM, not the group, so the
// group can still be out of combat).
Creature* GetEngagedNeptulon(Player* bot);
Unit* GetFocusTarget(Player* bot, FocusId id);
Unit* GetAnyFocusTarget(Player* bot);

// Ghur'sha is shielded by Absorb Magic: spell damage only heals him
bool GhurshaAbsorbing(Player* bot);

// Interrupts: the closest attacker in range that is casting a spell listed in InterruptPriority()
Unit* GetInterruptTarget(Player* bot);
bool CastInterrupt(PlayerbotAI* botAI, Unit* target);

// True while one of this dungeon's "get out of the way now" mechanics applies to the bot; the evade multiplier
// then keeps formation / follow / reach movement from walking the bot back in.
bool InEvadeDanger(Player* bot);
}

#endif
