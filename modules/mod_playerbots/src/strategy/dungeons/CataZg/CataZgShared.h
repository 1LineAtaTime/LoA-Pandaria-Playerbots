/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Gurub (Cataclysm 5-man, map 859, level 85) bot strategy "cata-zg" (MoP-only lane L7, issue #670, epic #373).
// AC has no strategy for this dungeon, so this follows AC's dungeon-strategy PATTERN (Ai/Dungeon/HoS, UB, MgT):
// Strategy + Triggers + Actions + Multipliers + ActionContext/TriggerContext. No AC file is ported 1:1.
//
// Intended mechanics, sources (fetched 2026-10-06):
//  - Wowpedia MediaWiki API: High Priest Venoxis (tactics), Bloodlord Mandokir (tactics), High Priestess Kilnara,
//    Zanzil (tactics), Jin'do the Godbreaker, Hazza'rah.
//  - Wowhead tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>): cast times (Devastating Slam 2 s,
//    Shadow Bolt 1.7 s, Voodoo Bolt 2.8 s, Shadows of Hakkar 3 s, Tears of Blood instant).
//  - Spell/NPC ids and radii: the fork's own boss scripts src/server/scripts/EasternKingdoms/ZulGurub/*.cpp and
//    the 5.4.8 SpellEffect.dbc / SpellRadius.dbc (DynamicObject = effect 27 PERSISTENT_AREA_AURA).
// A mechanic the fork's script never casts is deliberately NOT given an edge (see the issue comment for the gaps).
// The optional Edge of Madness bosses (Hazza'rah, Renataki, Wushoolay, Gri'lek) are not covered.

#ifndef PLAYERBOTS_CATAZGSHARED_H
#define PLAYERBOTS_CATAZGSHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include <string>
#include <vector>

class Creature;
class GameObject;
class Player;
class PlayerbotAI;
class Unit;

namespace CataZulGurub
{
constexpr uint32 MAP_ZUL_GURUB = 859;

// Bosses (fork scripts: zulgurub.h:38-44)
constexpr uint32 NPC_VENOXIS  = 52155;
constexpr uint32 NPC_MANDOKIR = 52151;
constexpr uint32 NPC_KILNARA  = 52059;
constexpr uint32 NPC_ZANZIL   = 52053;
constexpr uint32 NPC_JINDO    = 52148;

// Adds and ground-effect creatures
constexpr uint32 NPC_POOL_OF_ACRID_TEARS = 52197; // boss_venoxis.cpp:73
constexpr uint32 NPC_BLOODVENOM          = 52525; // boss_venoxis.cpp:75
constexpr uint32 NPC_OHGAN               = 52157; // zulgurub.h:49
constexpr uint32 NPC_PRIDE_OF_BETHEKK    = 52061; // boss_kilnara.cpp:80
constexpr uint32 NPC_WAVE_OF_AGONY_END   = 52160; // boss_kilnara.cpp:82 (the stalker the wave is aimed at)
constexpr uint32 NPC_ZANZILI_BERSERKER   = 52054; // boss_zanzil.cpp:46
constexpr uint32 NPC_HAKKAR_CHAINS       = 52430; // boss_jindo_the_godbreaker.cpp:75
constexpr uint32 NPC_TWISTED_SPIRIT      = 52624; // boss_jindo_the_godbreaker.cpp:80 (spirit-world adds, low health)
constexpr uint32 NPC_GURUBASHI_SPIRIT    = 52730; // boss_jindo_the_godbreaker.cpp:87 (DB spawns, phase 2, 4 corners)
constexpr uint32 NPC_GENERAL_BUNNY_JMF   = 45979; // Venoxis altar + stairs: 26 DB spawns, 20 of them carry Poison Cloud 96729
                                                  // (creature_addon.auras)

// Spells
constexpr uint32 SPELL_TOXIC_LINK_AURA   = 96477; // Venoxis: on both linked players (boss_venoxis.cpp:28)
constexpr uint32 SPELL_DEVASTATING_SLAM  = 96740; // Mandokir, 2 s cast, 50 missiles in a 35 deg cone (boss_mandokir.cpp:44)
constexpr uint32 SPELL_OHGAN_FEIGN_DEATH = 96733; // Ohgan "dies" through this aura (boss_mandokir.cpp:56)
constexpr uint32 SPELL_SHADOW_BOLT       = 96516; // Kilnara, 1.7 s cast (boss_kilnara.cpp:27)
constexpr uint32 SPELL_TEARS_OF_BLOOD    = 96438; // Kilnara, instant, aura on the boss; any interrupt effect removes it
constexpr uint32 SPELL_DARK_SLUMBER      = 96446; // sleeping Pride of Bethekk
constexpr uint32 SPELL_VOODOO_BOLT       = 96346; // Zanzil, 2.8 s cast (boss_zanzil.cpp:20)
constexpr uint32 SPELL_ZANZIL_FIRE_TRAIL = 96916; // persistent area aura (DynamicObject), 4 yd (boss_zanzil.cpp:22)
constexpr uint32 SPELL_ZANZIL_FIRE_CAST  = 96914; // Zanzil: 1.5 s cast at his victim, then a 6 x 0.25 s periodic aura that drops the
                                                  // trail every 4 yd in front of him (boss_zanzil.cpp:155-157, spell_zanzil_fire)
constexpr uint32 SPELL_DEADZONE          = 97170; // Jin'do: persistent area aura, 5 yd, -90% magic damage / cast speed
constexpr uint32 SPELL_SHADOWS_OF_HAKKAR = 97172; // Jin'do: 3 s cast, then an aura on the boss
constexpr uint32 SPELL_BRITTLE_BARRIER   = 97417; // Hakkar's Chains are immune while this is on them
constexpr uint32 SPELL_BREATH_OF_HETHISS = 96509; // Venoxis phase 2: 1.5 s cast, then a 3 s self aura (self stun) that
                                                  // pulses 96744 (15 yd cone in front, 30000 nature per 0.5 s heroic)
// #670 w68: Venoxis is tanked where he spawns, on his altar, and the altar and its stairs are covered by permanent Poison Clouds.
// 96729 (bunny aura, duration -1) pulses 96730 every 2 s: 18499-21500 nature (heroic, SpellEffect.dbc diff 2) to every unit within
// 7 yd (TARGET_UNIT_SRC_AREA_ENTRY with no `conditions` row, so Spell.cpp WorldObjectSpellTargetCheck takes any unit; the area check
// IsWithinDist3d adds the target's combat reach). Wowpedia "High Priest Venoxis (tactics)" (MediaWiki API, 2026-10-15): "The stairs
// and altar surrounding High Priest Venoxis are covered in a powerful poison gas that inflicts 9250 to 10750 Nature damage every 2
// seconds" - he only goes back up there for Bloodvenom; the fight belongs on the maze floor.
constexpr uint32 SPELL_POISON_CLOUD_AURA = 96729;
constexpr uint32 SPELL_POISON_CLOUD_DMG  = 96730;
// #679 Venoxis, stages one and two: the Venomous Effusion maze. A stalker (52302) walks the maze path (boss_venoxis.cpp mazeleftPos) and
// drops a Venomous Effusion (52288, 90 s) every 0.5 s; each pulses 96685 every second: heroic 18849 nature to enemies within 2 yd
// (SpellEffect.dbc). Wowpedia "High Priest Venoxis (tactics)": "the effect is slightly larger than the graphic" - do not touch the walls.
constexpr uint32 NPC_VENOMOUS_EFFUSION       = 52288;
constexpr uint32 SPELL_VENOMOUS_EFFUSION_DMG = 96685;
constexpr float  EFFUSION_MARGIN             = 1.5f;  // #679 w105: was 0.5 (10 hits from 252 clouds); on top of the 2 yd radius + the bot reach, so the escape starts ~1 yd before a cloud reaches it
constexpr uint32 NPC_VENOMOUS_EFFUSION_STALKER = 52302; // #679 w105: the maze walker; clouds appear every 0.5 s along its path
constexpr float  EFFUSION_STALKER_RADIUS     = 5.0f;  // #679 w105: look-ahead circle round the stalker (the next ~1.5 s of its wall)
// #679 Venoxis: Whispers of Hethiss 96466, an 8 s channel on a random player (96469 every 330 ms, heroic 4624 nature); Wowpedia: interrupt
// every cast.
constexpr uint32 SPELL_WHISPERS_OF_HETHISS   = 96466;
// #679 Zanzil's Graveyard Gas (boss_zanzil.cpp TECHNIQUE_GAS): 96338 is his 2 s cast + 7 s self aura; 9.5 s after it every player gets
// 96335 (heroic 20% of max health per second, until 21 s). Wowpedia "Zanzil (tactics)": every group member drinks from the green
// cauldron, Zanzil's Cauldron of Toxic Torment (GO 208315, GAMEOBJECT_TYPE_SPELLCASTER Data0 96328: 90% less nature damage for 30 s).
constexpr uint32 SPELL_GRAVEYARD_GAS         = 96338;
constexpr uint32 SPELL_GRAVEYARD_GAS_DAMAGE  = 96335;
constexpr uint32 GO_TOXIC_TORMENT_CAULDRON   = 208315;
constexpr uint32 SPELL_TOXIC_TORMENT         = 96328;
constexpr float  TOXIC_TORMENT_SCAN_RANGE    = 80.0f; // the cauldron near Zanzil stands ~49 yd from his spawn

// Zanzil's Cauldron of Frostburn Formula (#822). Wowpedia "Zanzil (tactics)": drinking from it makes the player's next spell hit
// stun the target for 15 s and deal 65% of its maximum health; Zanzil, Jin'do and their most powerful allies ignore it.
// GO 208314 is GAMEOBJECT_TYPE_SPELLCASTER (Data0 96325): the user gets the aura 96325 (PROC_TRIGGER_SPELL, 1 charge, procs on
// damaging spells) which casts 96331 on the victim (Unit.cpp:8211 skips victims above level 85); the script
// spell_frostburn_formula (boss_zanzil.cpp:353) then adds 96332 Hypothermia to the victim.
constexpr uint32 GO_FROSTBURN_CAULDRON       = 208314;
constexpr uint32 SPELL_FROSTBURN_AURA        = 96325; // on the player after drinking
constexpr uint32 SPELL_FROSTBURN_FORMULA     = 96331; // the stun on the victim (zulgurub.h:92)
constexpr uint32 SPELL_FROSTBURN_HYPOTHERMIA = 96332; // zulgurub.h:93

// Geometry (yards / radians)
constexpr float BOSS_SCAN_RANGE   = 120.0f;
constexpr float HAZARD_SCAN_RANGE = 80.0f;
constexpr float POOL_BASE_RADIUS  = 2.0f;  // damage reaches 2 * scale yd (spell_venoxis_pool_of_acid_tears_dmg)
constexpr float BLOODVENOM_RADIUS = 7.0f;
constexpr float SLAM_LENGTH       = 58.0f;
constexpr float SLAM_HALF_ANGLE   = 0.50f; // the script scatters the missiles +-17.5 deg, plus a margin
constexpr float WAVE_LENGTH       = 45.0f;
constexpr float WAVE_HALF_ANGLE   = 0.70f;
constexpr float TOXIC_LINK_BREAK_DISTANCE = 32.0f; // the fork's aura script breaks the link past 30 yd
// Breath of Hethiss: 96744 is TARGET_UNIT_CONE_ENEMY_104 radius 15 (SpellEffect.dbc), a creature's default cone is 90 deg
// (Spell.cpp SelectImplicitConeTargets), so 45 deg each side - plus a margin for the bot's own reach and reaction time.
constexpr float BREATH_LENGTH     = 19.0f;
constexpr float BREATH_HALF_ANGLE = 1.05f;
// Venoxis tank spot on the maze floor NE of the stair foot (0859 navmesh poly at z 32.3, offline check). Venoxis follows the tank down
// the stairs and stops ~3.5 yd short of it, facing NE: he then stands ~15.7 yd from the nearest cloud bunny (the floor one at
// (-12010.5,-1693.5)), a melee bot behind him ~14 yd, the leg stage (-11984.4,-1687.0) is 17.5 yd from the spot and ~86 deg off his
// facing (outside the Breath of Hethiss cone), the Venomtooth snakes are 15+ yd away (they never engaged the group in w62-w68).
// The script's own downPos (-12000.5,-1685.9) was not used: tanked there, Venoxis and his back stand 5-9 yd from that floor bunny.
constexpr float VENOXIS_TANK_X     = -11998.0f;
constexpr float VENOXIS_TANK_Y     = -1676.0f;
constexpr float VENOXIS_TANK_Z     = 32.4f;
constexpr float VENOXIS_TANK_SLACK = 4.0f;  // the tank walks back once it is this far off the spot
// Zanzili Berserker: Pursuit fixates a random player (boss_zanzil.cpp:254-258, 5M threat) and the berserker runs at 0.5
// speed (boss_zanzil.cpp:216); Wowpedia "Zanzil (tactics)": the pursued player kites it.
constexpr float BERSERKER_KITE_RADIUS = 10.0f;
// Zanzili Fire line (#670 w61: the tank went 100 -> 0% in 28 s standing in the trail): 6 patches at 4..24 yd, 4 yd radius.
constexpr float ZANZIL_FIRE_LINE_LENGTH = 28.0f;
constexpr float ZANZIL_FIRE_LINE_HALF_WIDTH = 4.5f;
// Jin'do phase 2: a Gurubashi Spirit's Body Slam strips the Brittle Barrier of a chain within 5 yd of where it lands
// (boss_jindo_the_godbreaker.cpp:368-373), and it lands on the Spirit Warrior's Gaze target (97597, one player in 30 yd).
constexpr float CHAIN_STAND_RANGE = 3.0f;
// #670 w65: the whole group stood on the shielded chain, so every Body Slam (97252: 5 yd radius, ~15k damage + knock back,
// SpellEffect.dbc) hit all four bots; the healer went into Spirit of Redemption 10 s after the second one and the group wiped at 70%.
// Wowpedia "Jin'do the Godbreaker" (MediaWiki API, 2026-10-15): "Allow it to slam one party member and break a brittle barrier".
// The Spirit Warrior's Gaze (97597) is an aura on the player the spirit slams ~3 s later (w65 spellwatch: aura+ on Heranu/Kaellio,
// then 97198 cast), and the slam lands where that player stands when the cast completes (Spell::cast -> SelectSpellTargets).
// So only the Gaze target walks onto the chain; everybody else waits just outside the slam radius and reaches the chain in ~1 s
// if the Gaze picks them.
constexpr uint32 SPELL_SPIRIT_WARRIOR_GAZE = 97597;
constexpr float CHAIN_WAIT_DIST = 9.5f;  // a waiting bot stands this far from the chain (5 yd slam radius + margin)
// #670 w67: the mage and the healer walked in from the same side and stood on the same 9.5 yd spot; two Shadow Spikes (97161,
// 63k in 4 yd at a random player's location, SpellEffect.dbc) hit both, the mage died mid gaze run and the healer fell to 0-6%.
// Each bot gets its own spot on the ring instead: the side of the chain away from Hakkar's spirit (where the group comes from),
// fanned out CHAIN_WAIT_SPREAD radians per group slot (9.5 yd * 0.9 rad = 8.3 yd apart > 2 x the 4 yd spike). Every ring point at
// 9.5 yd around the three chainPos spots is on the 0859 navmesh at the chain's height (offline check, 20 deg steps).
constexpr float CHAIN_WAIT_SPREAD = 0.9f;
constexpr float CHAIN_WAIT_SLACK  = 2.5f;  // a waiting bot farther than this from its spot walks back to it
constexpr float HAKKAR_SPIRIT_X   = -11786.5f;  // boss_jindo_the_godbreaker.cpp:104 hakkarPos
constexpr float HAKKAR_SPIRIT_Y   = -1690.92f;
constexpr float SPIRIT_SCAN_RANGE = 120.0f;

constexpr float CAULDRON_SCAN_RANGE      = 40.0f; // a cauldron this close to the bot counts (they stand ~15 yd from Zanzil)
constexpr float CAULDRON_BERSERKER_RANGE = 70.0f;

constexpr float STAND_MARGIN = 0.5f; // a bot closer than this to a hazard edge counts as inside it
constexpr float CLEAR_MARGIN = 1.5f; // an escape spot is wanted at least this far outside every hazard

enum class HazardKind : uint8
{
    AcridPools,       // Venoxis phase 2: growing Pools of Acrid Tears
    Bloodvenom,       // Venoxis phase 3: the tendrils that follow each player
    DevastatingSlam,  // Mandokir: the cone in front of him while he casts
    WaveOfAgony,      // Kilnara: the cone from the boss toward each marked stalker
    ZanzilFire,       // Zanzil: the Zanzili Fire trail (DynamicObjects)
    Deadzone,         // Jin'do: Deadzone, but only while Shadows of Hakkar is NOT up
    BreathOfHethiss,  // Venoxis phase 2: the cone in front of him while he casts / channels Breath of Hethiss
    BerserkerPursuit, // Zanzil: a Zanzili Berserker whose victim is this (non-tank) bot - kite it
    PoisonCloud,      // Venoxis: the permanent Poison Cloud bunnies on his altar and stairs (#670 w68)
    VenomousEffusion  // Venoxis stages one and two: the Venomous Effusion maze walls (#679)
};

enum class HazardShape : uint8 { Circle, Cone, Line };

struct Hazard
{
    HazardShape shape;
    float x;
    float y;
    float radius;    // circle radius, or cone / line length
    float angle;     // cone / line direction (radians, same frame as Position::GetAngle)
    float halfAngle; // cone half opening (radians), or line half width (yards)
};

// Distance by which (x, y) lies outside the hazard; negative when inside.
float HazardSlack(Hazard const& hazard, float x, float y);
float MinSlack(std::vector<Hazard> const& hazards, float x, float y);

// Max effect radius of a spell (SpellInfo::Effects[].CalcRadius, cf. UBShared.cpp MaxEffectRadius).
float SpellRadius(uint32 spellId, float fallback);

Creature* FindBoss(Player* bot, uint32 entry);

void CollectHazards(Player* bot, HazardKind kind, std::vector<Hazard>& out);

// Ring search for the nearest spot clear of every hazard (pattern: MgTShared.cpp CollectHazardEscapes). clearPath (#679 w106): the
// straight walk there must not enter a hazard the bot is not already in (PathClear).
bool FindEscapeSpot(Player* bot, std::vector<Hazard> const& hazards, float margin, float& outX, float& outY,
                    float& outZ, bool clearPath = false);
bool PathClear(std::vector<Hazard> const& hazards, float x0, float y0, float x1, float y1);

// Venoxis Toxic Link: the other linked player, only while the two are still close enough for the link to hold.
Player* GetToxicPartner(Player* bot);

// Venoxis (#670 w68): the spot the tank holds him on, off the poisoned altar. True (with the spot) when this bot is the tank Venoxis is
// fighting, it is more than VENOXIS_TANK_SLACK off the spot, no Breath of Hethiss is coming and no Pool of Acrid Tears covers the spot.
bool GetVenoxisTankSpot(Player* bot, float& x, float& y, float& z);
// Venoxis stands inside a Poison Cloud (altar / stairs): nobody walks in after him.
bool IsVenoxisInPoisonCloud(Player* bot);

// Jin'do: Shadows of Hakkar is being cast / is up, so everybody has to stand in a Deadzone.
bool IsShadowsOfHakkarActive(Player* bot);
bool GetDeadzoneToEnter(Player* bot, float& centreX, float& centreY, float& radius);

// Jin'do phase 2 (#670 w60: 70% at the cap, "zg focus target" never fired - every chain kept its Brittle Barrier because no
// Gurubashi Spirit was ever brought to a chain). Wowpedia "Jin'do the Godbreaker": pull a Gurubashi Spirit up to the chains;
// the player it Body Slams stands on a chain, the slam shatters that chain's barrier, kill the chain, repeat.
// The shielded chain the whole group works on (the lowest GUID counter, so every bot picks the same one), or nullptr.
Creature* GetShieldedChain(Player* bot);
// A chain whose barrier is already broken is still alive (kill it first, stay where it is).
bool HasExposedChain(Player* bot);
// A Gurubashi Spirit that is already fighting the group / the nearest one that is not.
Creature* GetEngagedSpirit(Player* bot);
Creature* GetIdleSpirit(Player* bot);
// The bot that brings the next spirit: the tank, or the lowest-GUID living DPS bot when the group has no living tank.
bool IsSpiritPuller(Player* bot);
// Pull a spirit now: a shielded chain, no exposed chain, no spirit engaged, an idle one exists and this bot is the puller.
Creature* GetSpiritToPull(Player* bot);
// The chain this bot should stand on (everybody in the phase except the puller while it is still fetching a spirit).
Creature* GetChainToStandAt(Player* bot);
// This bot carries the Spirit Warrior's Gaze: it is the next Body Slam's landing spot.
bool IsGazeTarget(Player* bot);
// The spot a bot that is not the Gaze target waits on next to <chain> (CHAIN_WAIT_*).
void GetChainWaitSpot(Player* bot, Creature* chain, float& x, float& y);
// Jin'do phase 2 is running for this bot: in combat with a living Hakkar's Chains in range.
bool IsChainPhase(Player* bot);
// A non-tank bot must not keep hitting the Gurubashi Spirit while a chain is still shielded.
bool ShouldSpareSpirit(Player* bot, Unit* currentTarget);

// Priority focus target for non-tank DPS (Ohgan, awake panthers in Kilnara's phase 2, Zanzili Berserker, chains).
Unit* GetFocusTarget(Player* bot);

// Zanzil (#822): one elected bot (a bot that already carries the aura, else the lowest-GUID ranged DPS, else the lowest-GUID other
// DPS; never the tank, never the bot a Berserker is fixated on) drinks from the Frostburn Formula cauldron while a Zanzili
// Berserker is up and fighting the group and not yet frozen, then hits the Berserker with its next spell.
struct CauldronPlan
{
    Creature* berserker = nullptr;  // the Zanzili Berserker to freeze
    GameObject* cauldron = nullptr; // the cauldron to drink from (nullptr once the bot carries the aura)
    bool armed = false;             // the bot already carries the Frostburn Formula aura
};
bool GetCauldronPlan(Player* bot, CauldronPlan& plan);
// A group member on this map carries the Frostburn Formula aura 96325 (it is a dispellable Magic buff: w64 the priest's Purify /
// Dispel Magic trigger removed it from the mage within seconds, 5 of 14 drinks logged a purify right before the aura-).
bool GroupHasFrostburnCarrier(Player* bot);
// #679: the same for Toxic Torment 96328 (SpellCategories Dispel 1 = Magic): no dispel may strip it before the Graveyard Gas ends.
bool GroupHasCauldronBuff(Player* bot);

// #679 Zanzil's Graveyard Gas: Zanzil casts / carries 96338, or this bot already takes 96335.
bool IsGraveyardGasActive(Player* bot);
// The Toxic Torment cauldron this bot should drink from now: the gas is on, the bot has no 96328 yet, the nearest spawned GO 208315.
GameObject* GetToxicTormentCauldron(Player* bot);
// The armed bot casts 96331 itself when its own spell has not procced it after this long (w64: 15 drinks, 0 casts of 96331).
// PM 2026-10-15: the direct cast is a bot-only workaround (a real player cannot cast 96331), so it is OFF until the 96325 proc
// failure (#836) is root-caused; set to true only for a diagnostic run.
constexpr bool CAULDRON_DIRECT_CAST = false;
constexpr uint32 CAULDRON_PROC_WAIT_MS = 2000;
constexpr float CAULDRON_CAST_RANGE = 35.0f;

// Kilnara (Tears of Blood, Shadow Bolt), Zanzil (Voodoo Bolt) and Venoxis (Whispers of Hethiss, #679): the boss to interrupt, or nullptr.
Unit* GetInterruptTarget(Player* bot);
// #679 w106: the caster's current spellId can be interrupted right now (CASTING, or PREPARING with a cast time).
bool IsInterruptibleNow(Unit* caster, uint32 spellId);
// #679 w106: Venoxis is engaged (the class interrupt actions stand down; "zg interrupt" keeps every interrupt for Whispers of Hethiss).
bool IsVenoxisWhispersFight(Player* bot);

// Dedicated interrupt (kick, pummel, ...), first that can be cast on the target.
bool CastDedicatedInterrupt(PlayerbotAI* botAI, Unit* target);
// #679 w106: one of the dedicated interrupts can be cast on the target now.
bool CanCastDedicatedInterrupt(PlayerbotAI* botAI, Unit* target);
// #679 w106: a class strategy's interrupt action ("kick", "pummel on enemy caster", "rebuke interrupt", ...).
bool IsClassInterruptAction(std::string const& name);
}

#endif
