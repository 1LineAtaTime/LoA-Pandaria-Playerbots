/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CheckMountStateAction.h"

#include "DatabaseEnvFwd.h"
#include "DatabaseEnv.h"  // #530: PlayerbotsDatabase / QueryResult / Field (preferred mounts)

#include "Battleground.h"
#include "BattlegroundDG.h"
#include "BattlegroundEY.h"
#include "BattlegroundTOK.h"
#include "BattlegroundWS.h"
// #134 mount/riding (AC parity): AreaTableEntry/sAreaTableStore (BotCanUseFlyingMount) and
// AREA_FLAG_NO_FLY_ZONE.
#include "BotScenario.h"
#include "DBCEnums.h"
#include "MotionMaster.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "IVMapManager.h"  // VMAP_INVALID_HEIGHT_VALUE (AC :152 parachute cleanup)
#include "Map.h"           // INVALID_HEIGHT
#include "NewRpgInfo.h"
#include "NewRpgRoamTransport.h"
#include "PlayerbotSpec.h"  // #742: PlayerBotSpec::IsMelee (AC PlayerbotAI::IsMelee)
#include "Playerbots.h"
#include "Random.h"
#include "SharedDefines.h"
#include "SpellMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <mutex>
#include <unordered_map>
#include <vector>

// #968: defined in MovementActions.cpp - whether "reach melee"/"reach spell" (MovementAction::ReachCombatTo) can move
// the bot towards the target at all (a usable navmesh path, or a straight flight). Forward-declared, no header change.
bool CombatReachHasPath(Player* bot, Unit* target);

// #134 AC parity (CheckMountStateAction::Dismount, AC :345-356): dismount the way the client does, via
// CMSG_CANCEL_MOUNT_AURA, which removes the mount aura. Unit::Dismount() alone only clears the mount
// display on this core; the aura stays, the next mount cast then only refreshes it (HandleAuraMounted
// skips AURA_EFFECT_HANDLE_REAPPLY) and the bot can never mount again.
// AC parity (CheckMountStateAction::ClearStaleFlightFlags, AC :359-370): a flying-mount ride leaves
// DISABLE_GRAVITY/FLYING set after the mount aura goes (SetCanFly(false) does not clear them), and
// IsFlying() then makes PlayerbotAI::CastSpell refuse every mount cast silently.
// #401 (beyond AC): external linkage so PlayerbotAI::HandleTeleportAck can also run it right after a
// teleport (forward-declared there; no header change). No `static` on purpose.
void ClearStaleFlightFlags(Player* bot)
{
    if (bot->HasIncreaseMountedFlightSpeedAura() || bot->HasFlyAura())
        return;

    if (bot->HasUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY))
    {
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_CAN_FLY);
        if (!bot->HasUnitState(UNIT_STATE_ROOT))
            bot->SendMovementFlagUpdate();
    }
}

// #871 (AC CheckMountStateAction::CompleteDismount, AC Ai/Base/Actions/CheckMountStateAction.cpp:372-390): runs
// from the SMSG_DISMOUNT handler. AC's body ported 1:1; MoP equivalents grep-checked: MotionMaster::MoveFall(uint32)
// (MotionMaster.h:183), Player::SetFallInformation(uint32,float) / HandleFall(MovementInfo const&) (Player.h:2819-2820),
// Object::UpdateAllowedPositionZ (Object.h:436), MovementInfo::pos (MovementInfo.h:30).
void CheckMountStateAction::CompleteDismount(Player* bot)
{
    if (!bot || !bot->IsInWorld())
        return;

    // #957: a dismount while a taxi flight is still running (UNIT_STATE_IN_FLIGHT; the flight generator's own landing,
    // FlightPathMovementGenerator::DoFinalize, clears it before it dismounts) has no fall to complete. AC's
    // MotionMaster::MoveFall returns for a UNIT_FLAG_DISABLE_MOVE rider (AC MotionMaster.cpp:700-701); this core's has
    // no such guard (MotionMaster.cpp MoveFall), so its EffectMovementGenerator replaced the same-priority flight
    // generator (MotionMaster::DirectAdd) and the bot dropped out of the flight, and HandleFall below charged fall
    // damage from flight height. Leave the flight to the generator.
    if (bot->IsInFlight())
        return;

    float const x = bot->GetPositionX();
    float const y = bot->GetPositionY();
    float const startZ = bot->GetPositionZ();

    float groundZ = startZ;
    bot->UpdateAllowedPositionZ(x, y, groundZ);

    bot->GetMotionMaster()->MoveFall();
    MovementInfo fallInfo = bot->m_movementInfo;
    // Need to set the start of the fall, otherwise the fall may start from too high of a Z and kill the bot.
    bot->SetFallInformation(0, startZ);
    fallInfo.pos.Relocate(x, y, groundZ);
    bot->HandleFall(fallInfo);
    // Re-anchor at the ground: AC's Player::IsFalling() compares standing Z to this, so startZ reads as a fall.
    bot->SetFallInformation(0, groundZ);
    bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);
}

// #530 item 6: twin of the AC copy's ShouldLogMountDiagnostics (AC CheckMountStateAction.cpp:65-72, harness addition) -
// the [MOUNT] diagnostic tag follows AiPlayerbot.LogInGroupOnly like every other AC per-bot log.
static bool ShouldLogMountDiagnostics(Player* bot)
{
    if (!sPlayerbotAIConfig->logInGroupOnly)
        return true;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    return bot->GetGroup() && botAI && botAI->HasRealPlayerMaster();
}

static void DismountBot(Player* bot)
{
    if (bot->isMoving())
        bot->StopMoving();
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    ClearStaleFlightFlags(bot);
}

// #868 (beyond AC, duels only): the duel was accepted (countdown, Player::duel->startTimer) or has started
// (startTime). AC mounts a duelling bot like any idle bot: with no current target (a stealthed opponent, a lost
// line of sight) "noAttackers" + "shouldMount" hold because a duel sets no combat flag until a hit lands. Wave 78
// t613: Kalani mounted 18 s into its duel with Catheri and stayed mounted 57 probe rows; Zolio/Petra/Jennah stayed
// mounted 5-20 rows after the countdown. A duelling bot never mounts and leaves its mount at the countdown.
static bool DuelAccepted(Player* bot)
{
    return bot->duel && bot->duel->opponent && (bot->duel->startTimer != 0 || bot->duel->startTime != 0);
}

// #134 mount/riding (AC parity, Ai/Base/Actions/CheckMountStateAction.cpp BotCanUseFlyingMount):
// whether the bot's own riding skill and current location allow a flying mount right now. 225 is the
// riding-skill tier MountCapability.dbc requires for the first (149%) flying tier on every continent
// (this dbc's rows 15/18/21/24/27/39/41 - Northrend/Outland/EK/Kalimdor/Deepholm/Pandaria all gate
// their slowest flying tier at riding-225). Player::IsKnowHowFlyIn() carries the per-continent licence
// spell check (Flight Master's License 90267 for EK/Kalimdor/Deepholm, Cold Weather Flying 54197 for
// Northrend, Wisdom of the Four Winds 115913 for Pandaria, always true in Outland - see Player.cpp
// ~27738-27775) plus its hardcoded IsNoFlyZone() list; AREA_FLAG_NO_FLY_ZONE below matches the extra
// static-area check the real cast makes (src/server/game/Spells/Spell.cpp CheckCast(), SPELL_AURA_FLY /
// SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED case).
//
// This predicate is this fork's MoP substitute for AC's area->IsFlyable() and is already stricter than
// AC: IsKnowHowFlyIn() only knows the continent maps (Player.cpp ~27781-27794), so it returns false for
// every battleground map. AC's explicit battleground clamp is nevertheless applied at the call site in
// Execute below (#394, AC :648), so the "no flyer in a BG" rule does not rest on that accident.
static bool BotCanUseFlyingMount(Player* bot)
{
    // issue: harness "ground" scenario option (goto/wander) - never fly while it runs. No-op unless
    // that scenario option is set (BotScenario::AnyActive() fast path).
    if (BotScenario::GroundOnly(bot))
        return false;

    if (bot->GetPureSkillValue(SKILL_RIDING) < 225)
        return false;

    if (!bot->IsKnowHowFlyIn(bot->GetMapId(), bot->GetZoneId()))
        return false;

    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(bot->GetAreaId()))
        if (area->Flags & AREA_FLAG_NO_FLY_ZONE)
            return false;

    return true;
}

bool CheckMountStateAction::isUseful()
{
    // Same gates as upstream (Ai/Base/Actions/CheckMountStateAction.cpp::isUseful), so the mount
    // behaviour is comparable with the WotLK reference instead of being driven by a travel-distance
    // heuristic (issue #100: 28 mount-ups against the reference's 2482 in the same window).
    if (botAI->IsInVehicle() || bot->isDead() || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
        !bot->IsOutdoors() || bot->InArena())
        return false;

    // #764 (beyond AC, map 726 only like the other #747 guards): not while a jump arc or a fall is in progress. The
    // mount cast stops movement, which cut the Twin Peaks mid-graveyard jump-down (tpJumpDown) 1.5 yd short and left
    // the bot hanging 3 yd above the ground until the fall caught up (wave 27, Henne at 1798.5,164.1,-2.0).
    if (bot->GetMapId() == 726 &&
        (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE || bot->IsFalling()))
        return false;

    // #179 (AC CheckMountStateAction::isUseful): remember the master for Execute() - mount
    // matching only makes sense while a real master exists.
    master = GetMaster();

    // AC :238-245: capture the shapeshift forms for Execute()/Mount(). AC only refreshes them when a master
    // exists (so they go stale otherwise); here they are reset every call, which is the same for a master and
    // harmless without one.
    botInShapeshiftForm = bot->GetShapeshiftForm();
    masterInShapeshiftForm = master ? master->GetShapeshiftForm() : FORM_NONE;

    // #868 (beyond AC, duels only): useful only to leave a mount / travel form (Execute), never to mount.
    if (DuelAccepted(bot) && !bot->IsMounted() && botInShapeshiftForm != FORM_TRAVEL &&
        botInShapeshiftForm != FORM_FLIGHT && botInShapeshiftForm != FORM_FLIGHT_EPIC)
        return false;

    // AC :247-250: in combat the action stays useful only while mounted or travel/flight formed (dismount /
    // unshift). FORM_FLIGHT/FORM_FLIGHT_EPIC exist in this core's Unit.h; Travel Form is the 5.4.8 form.
    if ((bot->IsInCombat() || botAI->GetState() == BOT_STATE_COMBAT) && !bot->IsMounted() &&
        botInShapeshiftForm != FORM_TRAVEL && botInShapeshiftForm != FORM_FLIGHT &&
        botInShapeshiftForm != FORM_FLIGHT_EPIC)
        return false;

    // AC :252-257: IsOutdoors alone lets a bot clipping slightly below the floor (tunnels) think it is outdoors.
    // 5.4.8 has the same WorldObject::GetMapWaterOrGroundLevel and Unit::HasWaterWalkAura.
    {
        float const posZ = bot->GetPositionZ();
        float const groundLevel = bot->GetMapWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), posZ);
        if (!bot->IsMounted() && !bot->HasWaterWalkAura() && posZ < groundLevel)
            return false;
    }

    if (!botAI->HasStrategy("mount", BOT_STATE_NON_COMBAT) && !bot->IsMounted())
        return false;

    // #134: InitMountSpells only grants a mount from this level (AiPlayerbot.UseGroundMountAtMinLevel).
    if (bot->GetLevel() < sPlayerbotAIConfig->useGroundMountAtMinLevel)
        return false;

    if (bot->HasAuraType(SPELL_AURA_TRANSFORM) && bot->IsInDisallowedMountForm())
        return false;

    // Beyond AC (AC bots do not plan boat / zeppelin trips): no mounting up while a roaming trip waits at a dock, boards,
    // rides or steps off. The board step dismounts a flying mount before it walks onto the deck; mounting again cast
    // over that walk and stopped it a yard or two in, so the bot was attached at the deck's rim instead of inside.
    if (!bot->IsMounted() && RoamTransport::HoldsForTransportLeg(bot))
        return false;

    if (bot->InBattleground())
    {
        // Never mount while carrying a flag.
        if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
            bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
            return false;

        // #705 (MoP-only, extends the AC flag rule above, AC CheckMountStateAction.cpp:277-279): the Temple of
        // Kotmogu Orb of Power auras carry AURA_INTERRUPT_FLAG_MOUNT (SpellInterrupts.dbc 0x1A0000), so mounting
        // removes the orb and spell_orb_of_power::HandleRemove -> BattlegroundTOK::ReleaseOrb puts it back on its
        // stand. In the owner session 3464 of 3504 orb pickups were followed by the carrier mounting.
        if (bot->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_1) || bot->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_2) ||
            bot->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_3) || bot->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_4))
            return false;

        // #579 (MoP-only, same rule as #705): the Deepwind Gorge mine-cart auras (Alliance 140876 / Horde 141210)
        // carry the same AuraInterruptFlags 0x1A0000 (AURA_INTERRUPT_FLAG_MOUNT), so mounting removes the cart and
        // BattlegroundDG::EventPlayerDroppedFlag drops it at the carrier's feet. Wave 41: all 10 cart drops came
        // one "check mount state" cast after a pick-up, and the carrier never left the drop spot.
        if (bot->HasAura(BG_DG_ALLIANCE_MINE_CART) || bot->HasAura(BG_DG_HORDE_MINE_CART))
            return false;

        // Only mount in a battleground if the match starts within 30 seconds (AC). Beyond AC (owner 2026-10-08): every
        // bot used the same 30 s mark, so a whole team mounted in the same second - too robotic. Each bot gets its own
        // mount moment between T-30 s and T-1 s, fixed per bot and match (guid x instance hash, so a 1:1 run repeats).
        if (Battleground* bg = bot->GetBattleground())
            if (bg->GetStatus() == STATUS_WAIT_JOIN)
            {
                uint32 const h = (bot->GetGUID().GetCounter() * 2654435761u) ^ (bg->GetInstanceID() * 40503u);
                int32 const mountAtMs = 1000 + int32(h % 29001u);  // 1000..30000 ms before the start
                if (bg->GetStartDelayTime() > mountAtMs)
                    return false;
            }
    }

    return true;
}

// #742 (AC parity, AC CheckMountStateAction.cpp:565-574): melee bots use the melee distance, ranged bots the spell
// distance. #100 had collapsed both to the spell distance, so melee bots dismounted ~30 yd out and walked the rest.
float CheckMountStateAction::CalculateMountDistance() const
{
    // 21 yd floor: at 21 yd the mount cast (1.5 s) and the speed gain cancel out.
    bool const isMelee = PlayerBotSpec::IsMelee(bot);
    float const baseDistance =
        isMelee ? sPlayerbotAIConfig->meleeDistance + 10.0f : sPlayerbotAIConfig->spellDistance + 10.0f;
    return std::max(21.0f, baseDistance);
}

// #742 (AC parity, AC CheckMountStateAction.cpp:556-563).
float CheckMountStateAction::CalculateDismountDistance() const
{
    bool const isMelee = PlayerBotSpec::IsMelee(bot);
    float const distance = isMelee ? sPlayerbotAIConfig->meleeDistance + 2.0f : sPlayerbotAIConfig->spellDistance + 2.0f;
    // Warriors dismount far enough out to charge (charge cannot be cast while mounted).
    return bot->GetClass() == CLASS_WARRIOR ? std::max(18.0f, distance) : distance;
}

namespace
{
// #229 (AC parity: CollectMountData, AC CheckMountStateAction.cpp :28-63): the bot's known mount
// spells bucketed by the speed tier the core would actually give it here and now. On 5.4.8 the
// MountCapability - not the spell's own effects - carries the speed, so a bot's slow and its fast
// flyer resolve to the same tier; the old argmax kept whichever PlayerSpellMap (an unordered_map,
// Player.h) yielded first and the other mount was dead weight. One row per known mount spell: a
// random bot owns one per tier (BotFactory::InitMountsAC), so the fixed capacity below is a safety
// valve, not a heuristic, and it keeps this hot path free of heap allocations.
struct MountCandidate
{
    uint32 spellId;
    int32 speed;
    bool isFlying;
};

constexpr uint32 MAX_MOUNT_CANDIDATES = 32;

// #530 item 4 (AC CheckMountStateAction.cpp :25-26 mountCache, :445-471 LoadPreferredMounts, :473-523
// TryPreferredMount). AC loads the table once on the world thread at startup (Playerbots.cpp:464) through
// PLAYERBOTS_SEL_PREFERRED_MOUNTS. This fork has neither that prepared statement (PlayerbotsDatabase.h) nor the
// startup hook, so the same SELECT runs once, on first use, as a plain query; call_once makes it safe from the
// map-update threads. Like AC, the cache is read-only after the load (no reload).
struct PreferredMountCache
{
    std::vector<uint32> groundMounts;
    std::vector<uint32> flightMounts;
};

std::unordered_map<uint32, PreferredMountCache> s_preferredMounts;
std::once_flag s_preferredMountsOnce;

void LoadPreferredMountsOnce()
{
    std::call_once(s_preferredMountsOnce, []()
    {
        uint32 totalResults = 0;
        if (QueryResult result = PlayerbotsDatabase.Query("SELECT guid, spellid, type FROM playerbots_preferred_mounts"))
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 const guid = fields[0].GetUInt32();
                uint32 const spellId = fields[1].GetUInt32();
                uint32 const mountType = fields[2].GetUInt32();
                if (mountType == 0)
                    s_preferredMounts[guid].groundMounts.push_back(spellId);
                else if (mountType == 1)
                    s_preferredMounts[guid].flightMounts.push_back(spellId);
                ++totalResults;
            } while (result->NextRow());
        }
        TC_LOG_INFO("playerbots", "Preferred mounts initialized | Total records: %u", totalResults);
    });
}

// AC TryPreferredMount. mountType is AC's GetMountType(master): 1 when a flying mount is wanted, else 0.
bool TryPreferredMount(Player* bot, PlayerbotAI* botAI, uint32 mountType, float travelDist, std::string const& status)
{
    LoadPreferredMountsOnce();

    auto itr = s_preferredMounts.find(bot->GetGUID().GetCounter());
    if (itr == s_preferredMounts.end())
        return false;

    PreferredMountCache const& preferred = itr->second;
    uint32 chosenMountId = 0;
    if (mountType == 0 && !preferred.groundMounts.empty())
        chosenMountId = preferred.groundMounts[urand(0, uint32(preferred.groundMounts.size() - 1))];
    else if (mountType == 1 && !preferred.flightMounts.empty())
        chosenMountId = preferred.flightMounts[urand(0, uint32(preferred.flightMounts.size() - 1))];

    if (!chosenMountId)
        return false;

    if (!sSpellMgr->GetSpellInfo(chosenMountId))
    {
        TC_LOG_ERROR("playerbots", "Preferred mount failed: Invalid spell %u | Bot Guid: %u", chosenMountId,
                     bot->GetGUID().GetCounter());
        return false;
    }

    // AC: required, otherwise bots won't mount in BGs because they are constantly moving.
    if (bot->isMoving())
        bot->StopMoving();

    // AC: cast without CanCastSpell ("for now allow all, even if the bot does not have the actual mount").
    botAI->CastSpell(chosenMountId, bot);
    if (ShouldLogMountDiagnostics(bot))
    TC_LOG_DEBUG("playerbots", "[MOUNT] %s mounting spell=%u travelDist=%.0f status=%s", bot->GetName().c_str(),
                 chosenMountId, travelDist, status.c_str());
    return true;
}
}  // namespace

uint32 CheckMountStateAction::FindMountSpell(bool preferFlying) const
{
    // #134/#179 mount/riding (AC parity): with no real master the caller passes
    // BotCanUseFlyingMount(bot) - #394 in a battleground it passes false outright (AC :648), so
    // preferFlying is never true inside a BG. The bot rides the best mount its own riding skill and
    // current location allow, the "no real master" branch of AC's
    // CalculateMasterMountSpeed/GetMountType (Ai/Base/Actions/CheckMountStateAction.cpp :615-627).
    // With a real master, Execute() passes MasterWantsFlying(master) && BotCanUseFlyingMount(bot)
    // instead, so the bot never tries to match a flying master it cannot itself fly here.
    //
    // 5.4.8 data: a mount spell has a single SPELL_AURA_MOUNTED effect whose MiscValueB is a MountType;
    // speed and flight come from the MountCapability the core picks for the rider here and now
    // (Unit::GetMountCapability: riding skill, map, area, licence spell), via its SpeedModSpell. So a
    // mount is ranked by that capability, not by the spell's own effects (which are all zero here).
    bool const canFly = preferFlying;

    MountCandidate candidates[MAX_MOUNT_CANDIDATES];
    uint32 candidateCount = 0;

    auto addCandidate = [&candidates, &candidateCount](uint32 spellId, int32 speed, bool isFlying)
    {
        if (candidateCount < MAX_MOUNT_CANDIDATES)
        {
            candidates[candidateCount++] = MountCandidate{ spellId, speed, isFlying };
            return;
        }

        // Safety valve: keep the fastest candidate of that kind, dropping its slowest one.
        uint32 weakest = MAX_MOUNT_CANDIDATES;
        for (uint32 i = 0; i < MAX_MOUNT_CANDIDATES; ++i)
            if (candidates[i].isFlying == isFlying &&
                (weakest == MAX_MOUNT_CANDIDATES || candidates[i].speed < candidates[weakest].speed))
                weakest = i;

        if (weakest < MAX_MOUNT_CANDIDATES && speed > candidates[weakest].speed)
            candidates[weakest] = MountCandidate{ spellId, speed, isFlying };
    };

    for (auto const& entry : bot->GetSpellMap())
    {
        uint32 const spellId = entry.first;
        if (!entry.second || entry.second->state == PLAYERSPELL_REMOVED || !entry.second->active)
            continue;

        SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
        if (!info || info->IsPassive())
            continue;

        if (info->Effects[0].ApplyAuraName != SPELL_AURA_MOUNTED)
            continue;

        MountCapabilityEntry const* capability = bot->GetMountCapability(uint32(info->Effects[0].MiscValueB));
        if (!capability)
            continue;  // the cast would fail SPELL_FAILED_NOT_HERE

        bool isFlying = false;
        int32 speed = 0;
        if (SpellInfo const* speedInfo = sSpellMgr->GetSpellInfo(capability->SpeedModSpell))
        {
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                uint32 const aura = speedInfo->Effects[i].ApplyAuraName;
                if (aura == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)
                    isFlying = true;
                if (aura == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                    aura == SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED)
                    speed = std::max(speed, speedInfo->Effects[i].BasePoints);
            }
        }

        addCandidate(spellId, speed, isFlying);
    }

    // #229 (AC parity, TryRandomMountFiltered, AC :526-554): walk the speed tiers of the wanted kind
    // from fastest down and urand inside the first non-empty tier, instead of an argmax with a
    // first-seen tie-break that always picked the same one of a bot's two flyers. A pick the bot
    // cannot cast falls through to the next tier down (AC's `continue` in its descending loop) rather
    // than giving up. Flying is only tried first where the bot may actually fly here (AC
    // GetMountType); a bot that cannot use its flyers still gets its ground mount, as before.
    for (int32 kind = canFly ? 1 : 0; kind >= 0; --kind)
    {
        bool const wantFlying = kind == 1;
        bool tierTried = false;
        int32 tierCeiling = 0;  // only speeds below the last tier we gave up on are left

        while (true)
        {
            int32 tierSpeed = -1;
            for (uint32 i = 0; i < candidateCount; ++i)
                if (candidates[i].isFlying == wantFlying && (!tierTried || candidates[i].speed < tierCeiling))
                    tierSpeed = std::max(tierSpeed, candidates[i].speed);

            if (tierSpeed < 0)
                break;

            uint32 tier[MAX_MOUNT_CANDIDATES];
            uint32 tierCount = 0;
            for (uint32 i = 0; i < candidateCount; ++i)
                if (candidates[i].isFlying == wantFlying && candidates[i].speed == tierSpeed)
                    tier[tierCount++] = candidates[i].spellId;

            uint32 const pick = tier[urand(0, tierCount - 1)];

            // AC :540-541: stop before CanCastSpell, which refuses a cast-time spell while moving -
            // a roaming bot is almost always moving, so without this no bot would ever mount.
            if (bot->isMoving())
                bot->StopMoving();

            if (botAI->CanCastSpell(pick, bot))
                return pick;

            tierTried = true;
            tierCeiling = tierSpeed;
        }
    }

    return 0;
}
bool CheckMountStateAction::Execute(Event /*event*/)
{
    // AC :141-155 forced-flight-dismount cleanup: a forced dismount (e.g. Dalaran) leaves the feather-fall
    // parachute on; drop it once the bot is within PARACHUTE_LAND_THRESHOLD of the floor so it does not hover.
    // HasFeatherFallAura / WorldObject::GetMapHeight / INVALID_HEIGHT / VMAP_INVALID_HEIGHT_VALUE exist on 5.4.8.
    constexpr float PARACHUTE_LAND_THRESHOLD = 15.0f;  // AC :23
    if (bot->HasFeatherFallAura())
    {
        float const floorZ = bot->GetMapHeight(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        if (floorZ != INVALID_HEIGHT && floorZ != VMAP_INVALID_HEIGHT_VALUE &&
            bot->GetPositionZ() - floorZ <= PARACHUTE_LAND_THRESHOLD)
            bot->RemoveAurasByType(SPELL_AURA_FEATHER_FALL);
    }
    ClearStaleFlightFlags(bot); // AC calls it at the top of every run (AC :156)

    // #868 (beyond AC, duels only): a duelling bot leaves its mount / travel form and never mounts (DuelAccepted).
    if (DuelAccepted(bot))
    {
        if (botInShapeshiftForm == FORM_TRAVEL || botInShapeshiftForm == FORM_FLIGHT ||
            botInShapeshiftForm == FORM_FLIGHT_EPIC)
            botAI->RemoveShapeshift();

        if (bot->IsMounted())
        {
            if (ShouldLogMountDiagnostics(bot))
            TC_LOG_DEBUG("playerbots", "[MOUNT] %s dismount (duel with %s)", bot->GetName().c_str(),
                         bot->duel->opponent->GetName().c_str());
            DismountBot(bot);
        }
        return true;
    }

    // Upstream's model (issue #100): mount whenever there is nothing to fight - that is the case a
    // player mounts in - and dismount as soon as a target is close enough to engage. The old version
    // only mounted when the RPG layer happened to be travelling more than 30 yd to a grind/camp/flight
    // goal, which is why our bots almost never rode (28 mount-ups vs the reference's 2482).
    NewRpgInfo& info = botAI->GetRpgInfo();
    float travelDist = -1.0f;
    switch (info.GetStatus())
    {
        case RPG_GO_GRIND:
            travelDist = bot->GetExactDist(
                std::get<NewRpgInfo::GoGrind>(info.data).pos.GetPositionX(),
                std::get<NewRpgInfo::GoGrind>(info.data).pos.GetPositionY(),
                std::get<NewRpgInfo::GoGrind>(info.data).pos.GetPositionZ());
            break;
        case RPG_GO_CAMP:
            travelDist = bot->GetExactDist(
                std::get<NewRpgInfo::GoCamp>(info.data).pos.GetPositionX(),
                std::get<NewRpgInfo::GoCamp>(info.data).pos.GetPositionY(),
                std::get<NewRpgInfo::GoCamp>(info.data).pos.GetPositionZ());
            break;
        case RPG_TRAVEL_FLIGHT:
            travelDist = bot->GetExactDist(
                std::get<NewRpgInfo::TravelFlight>(info.data).flightMasterPos.GetPositionX(),
                std::get<NewRpgInfo::TravelFlight>(info.data).flightMasterPos.GetPositionY(),
                std::get<NewRpgInfo::TravelFlight>(info.data).flightMasterPos.GetPositionZ());
            break;
        default:
            break;
    }

    bool const inCombat = bot->IsInCombat() || botAI->GetState() == BOT_STATE_COMBAT;
    // AC :157: no attackers when the bot's own "combat" value is false OR there are zero attackers. The other
    // checks below keep MoP's inCombat (AC uses bot->IsInCombat() there).
    bool const noAttackers = !AI_VALUE2(bool, "combat", "self target") || !AI_VALUE(uint8, "attacker count");

    bool shouldDismount = false;
    bool shouldMount = false;
    // #742 diagnostic: AC's keep-mount line logs the distance to the current target (AC LogKeepMount,
    // CheckMountStateAction.cpp:131-139); logged below as tdist= so both cores can be bucketed by distance.
    float targetDist = -1.0f;

    Unit* const currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget)
    {
        float const dismountDistance = CalculateDismountDistance();
        float const mountDistance = CalculateMountDistance();
        float const combatReach = bot->GetCombatReach() + currentTarget->GetCombatReach();
        float const distanceToTarget = bot->GetExactDist(currentTarget);
        targetDist = distanceToTarget;

        shouldDismount = (distanceToTarget <= dismountDistance + combatReach);
        shouldMount = (distanceToTarget > mountDistance + combatReach);
    }
    else
    {
        // Nothing to fight: travel case, so mount.
        shouldMount = true;
    }

    // AC :174-180: if should dismount, or the master (if any) is no longer in travel/flight form while the bot
    // still is, remove the shapeshift (AC's botAI->RemoveShapeshift(), present on this fork).
    if (shouldDismount ||
        (masterInShapeshiftForm != FORM_TRAVEL && botInShapeshiftForm == FORM_TRAVEL) ||
        (masterInShapeshiftForm != FORM_FLIGHT && botInShapeshiftForm == FORM_FLIGHT && master &&
         !master->IsMounted()) ||
        (masterInShapeshiftForm != FORM_FLIGHT_EPIC && botInShapeshiftForm == FORM_FLIGHT_EPIC && master &&
         !master->IsMounted()))
        botAI->RemoveShapeshift();

    if (shouldDismount && bot->IsMounted())
    {
        // #843 (diagnostic only): combat= is bot->IsInCombat() as in AC LogDismounting (CheckMountStateAction.cpp:121-129),
        // not this file's inCombat (which adds BOT_STATE_COMBAT), so the field run's "dismount combat=0" row compares
        // like for like (w52/w54 MoP logged 0 combat=0 dismounts against AC's 0.3-1.0 per bot-hour). tdist= is the
        // distance to the current target, AC's travelDist.
        if (ShouldLogMountDiagnostics(bot))
        TC_LOG_DEBUG("playerbots",
                     "[MOUNT] %s dismount (combat=%d indoors=%d water=%d vehicle=%d tdist=%.1f travelDist=%.0f status=%s)",
                     bot->GetName().c_str(), bot->IsInCombat() ? 1 : 0, bot->IsOutdoors() ? 0 : 1,
                     bot->IsUnderWater() ? 1 : 0, botAI->IsInVehicle() ? 1 : 0, targetDist, travelDist,
                     info.ToString().c_str());
        DismountBot(bot);
        return true;
    }

    // #968 (beyond AC; AC :163-186 has the same gap): in combat with a target between the dismount and the mount
    // distance a mounted bot keeps its mount and lets "reach melee"/"reach spell" ride it in (AC's model, kept above).
    // When that reach can never move it - the bot stands off the navmesh, PathGenerator answers NOPATH and the bot cannot
    // fly straight in (MovementActions.cpp CombatReachHasPath) - it would sit mounted for good: Unit::Attack refuses a
    // mounted player, every cast fails SPELL_FAILED_NOT_MOUNTED (result=67), and "combat stuck" needs IsInCombat. Soak
    // mop1k-v4-1028: Florenia and Itthul kept their mount for the whole hour at tdist 18.4 / 36.0 (42,503 of 57,171
    // result=67 checks). Such a bot dismounts and fights from where it stands. The path check runs only in this state.
    if (!shouldDismount && inCombat && currentTarget && bot->IsMounted() && !CombatReachHasPath(bot, currentTarget))
    {
        if (ShouldLogMountDiagnostics(bot))
        TC_LOG_DEBUG("playerbots",
                     "[MOUNT] %s dismount (unreachable target, combat=%d tdist=%.1f travelDist=%.0f status=%s)",
                     bot->GetName().c_str(), bot->IsInCombat() ? 1 : 0, targetDist, travelDist,
                     info.ToString().c_str());
        DismountBot(bot);
        return true;
    }

    // MoP-only (#134): a mount aura left behind by another path (taxi, spell actions, which call
    // Unit::Dismount() as on AC) blocks every later mount on this core, see DismountBot above.
    if (!bot->IsMounted() && bot->HasAuraType(SPELL_AURA_MOUNTED))
        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);

    // #179 (AC CheckMountStateAction::Execute): while a real master exists and the bot is not in a
    // battleground, match the master's mount state instead of the plain travel-distance heuristic
    // below (which stays the "no real master / in a battleground" path, unchanged from before #179).
    bool const noRealMaster = (!master || master == bot);
    if (!noRealMaster && !bot->InBattleground())
    {
        if (ShouldFollowMasterMountState(master, noAttackers, shouldMount))
        {
            if (bot->isMoving())
                bot->StopMoving();

            // AC :200-201 `return Mount();`
            return Mount(MasterWantsFlying(master) && BotCanUseFlyingMount(bot), travelDist, info.ToString());
        }

        if (ShouldDismountForMaster(master) && bot->IsMounted())
        {
            if (ShouldLogMountDiagnostics(bot))
            TC_LOG_DEBUG("playerbots", "[MOUNT] %s dismount (master no longer mounted)", bot->GetName().c_str());
            DismountBot(bot);
            return true;
        }

        if (bot->IsMounted())
            if (ShouldLogMountDiagnostics(bot))
            TC_LOG_DEBUG("playerbots", "[MOUNT] %s keep mount (tdist=%.1f travelDist=%.0f status=%s)",
                         bot->GetName().c_str(), targetDist, travelDist, info.ToString().c_str());

        return false;
    }

    if (!bot->IsMounted() && noAttackers && shouldMount && !inCombat)
    {
        // #394 (AC parity, AC :648 `!bot->InBattleground() && BotCanUseFlyingMount(bot)`): the
        // no-real-master mount path must never offer a flyer inside a battleground. preferFlying false
        // starts the tier walk at kind 0 (ground), so the bot mounts a ground mount or nothing at all.
        // This is the clamp AC :648 applies; the master path above is already inside
        // `!bot->InBattleground()` (:369), so this is the one place it was missing.
        // AC :216-218 `return Mount();`
        return Mount(!bot->InBattleground() && BotCanUseFlyingMount(bot), travelDist, info.ToString());
    }

    if (bot->IsMounted())
        if (ShouldLogMountDiagnostics(bot))
        TC_LOG_DEBUG("playerbots", "[MOUNT] %s keep mount (tdist=%.1f travelDist=%.0f status=%s)",
                     bot->GetName().c_str(), targetDist, travelDist, info.ToString().c_str());

    return false;
}

// AC CheckMountStateAction::Mount() (AC :290-344), adapted to 5.4.8.
bool CheckMountStateAction::Mount(bool preferFlying, float travelDist, std::string const& status)
{
    // AC :292-299: remove a current shapeshift that is not a travel/flight form, and the tree of life aura.
    // (RemoveShapeshift() and RemoveAura(name) exist on this fork.)
    if (botInShapeshiftForm != FORM_TRAVEL && botInShapeshiftForm != FORM_FLIGHT &&
        botInShapeshiftForm != FORM_FLIGHT_EPIC)
    {
        botAI->RemoveShapeshift();
        botAI->RemoveAura("tree of life");
    }

    // AC :301-302 TryPreferredMount(master) (#530 item 4, see the namespace above). AC's GetMountType(master): with
    // no real master the bot's own flying choice (AC :661-662, what the caller passes as preferFlying), else whether
    // the master rides a flyer (AC :664-672). The playerbots_preferred_mounts table is empty on both test DBs.
    if (TryPreferredMount(bot, botAI, ((!master || master == bot) ? preferFlying : MasterWantsFlying(master)) ? 1u : 0u,
                          travelDist, status))
        return true;

    // AC :309-311 TryForms (master-follow only, as in AC).
    if (TryForms(travelDist, status))
        return true;

    // AC :313-320 TryRandomMountFiltered == FindMountSpell's tier walk (#229); it stops the bot itself before
    // each CanCastSpell (AC :540-541).
    uint32 const mountSpell = FindMountSpell(preferFlying);
    if (mountSpell && botAI->CanCastSpell(mountSpell, bot))
    {
        // #843 (diagnostic only): tdist= is the distance to the current target (-1 = none), AC LogMounting's travelDist
        // (CheckMountStateAction.cpp:111-119), so the field run can bucket mount-ups by target distance on both cores.
        Unit* const mountTarget = AI_VALUE(Unit*, "current target");
        if (ShouldLogMountDiagnostics(bot))
        TC_LOG_DEBUG("playerbots", "[MOUNT] %s mounting spell=%u tdist=%.1f travelDist=%.0f status=%s",
                         bot->GetName().c_str(), mountSpell, mountTarget ? bot->GetExactDist(mountTarget) : -1.0f,
                         travelDist, status.c_str());
        botAI->CastSpell(mountSpell, bot);
        return true;
    }

    // AC :322-341 mount ITEM from the bags. #530: "inventory items"/"mount" now resolves (InventoryAction::Find
    // "mount" branch, AC InventoryAction.cpp:277-282) and UseItem(item) is this fork's self-only UseItemAuto
    // (UseItemAction.cpp:41; AC UseItemAuto(item) = UseItem(item, Empty, nullptr)).
    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", "mount");
    if (!items.empty())
    {
        Item* mountItem = *items.begin();
        uint32 mountSpellId = 0;
        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (mountItem->GetTemplate()->Spells[i].SpellId > 0)
            {
                mountSpellId = mountItem->GetTemplate()->Spells[i].SpellId;
                break;
            }
        }

        if (bot->isMoving())
            bot->StopMoving();

        bool const mounted = UseItem(mountItem);
        if (mounted)
        {
            Unit* const mountTarget = AI_VALUE(Unit*, "current target");
            if (ShouldLogMountDiagnostics(bot))
            TC_LOG_DEBUG("playerbots", "[MOUNT] %s mounting spell=%u tdist=%.1f travelDist=%.0f status=%s (item)",
                         bot->GetName().c_str(), mountSpellId,
                         mountTarget ? bot->GetExactDist(mountTarget) : -1.0f, travelDist, status.c_str());
        }

        return mounted;
    }

    return false;
}

// AC CheckMountStateAction::TryForms() (AC :395-443), minimal 5.4.8 adaptation. AC's three branches are
// Travel Form (783 here, SPELL_TRAVEL_FORM), Flight Form (33943) and Swift Flight Form (40120); 5.4.8 has a
// single Travel Form that covers ground/flying/aquatic, so only the first branch carries over faithfully.
// The flight-form branches are not ported (their spells/masterMountType speed tiers have no 5.4.8
// equivalent, and a druid bot that cannot fly here would be handed Travel Form instead of the mount AC
// would choose); the flight-capable case falls through to FindMountSpell as before.
bool CheckMountStateAction::TryForms(float travelDist, std::string const& status)
{
    if (!master || master == bot)
        return false;

    constexpr uint32 SPELL_TRAVEL_FORM_MOP = 783;

    // AC :401-405: master and bot already in matching forms - nothing to do.
    if (masterInShapeshiftForm == FORM_TRAVEL && botInShapeshiftForm == FORM_TRAVEL)
        return true;

    // AC :407-414: master is in Travel Form and the bot can do the same.
    if (botAI->CanCastSpell(SPELL_TRAVEL_FORM_MOP, bot, true) && masterInShapeshiftForm == FORM_TRAVEL &&
        botInShapeshiftForm != FORM_TRAVEL)
    {
        botAI->CastSpell(SPELL_TRAVEL_FORM_MOP, bot);
        if (ShouldLogMountDiagnostics(bot))
        TC_LOG_DEBUG("playerbots", "[MOUNT] %s mounting spell=%u travelDist=%.0f status=%s", bot->GetName().c_str(),
                     SPELL_TRAVEL_FORM_MOP, travelDist, status.c_str());
        return true;
    }

    return false;
}

// #179 (AC CheckMountStateAction::ShouldFollowMasterMountState, trimmed to mounts only - see the
// class comment in the header for what AC's shapeshift-form matching is left out: MoP's Travel Form
// unifies ground/flying/aquatic in one spell, so AC's separate WotLK Flight Form (33943) / Swift
// Flight Form (40120) model does not carry over as-is and needs its own MoP-content design).
bool CheckMountStateAction::ShouldFollowMasterMountState(Player* master, bool noAttackers, bool shouldMount) const
{
    bool const isMasterMounted = master->IsMounted() || master->GetShapeshiftForm() == FORM_TRAVEL;
    bool const isBotMountedOrForm = bot->IsMounted() || bot->GetShapeshiftForm() == FORM_TRAVEL;
    return isMasterMounted && !isBotMountedOrForm && noAttackers && shouldMount && !bot->IsInCombat() &&
           botAI->GetState() != BOT_STATE_COMBAT;
}

bool CheckMountStateAction::ShouldDismountForMaster(Player* master) const
{
    bool const isMasterMounted = master->IsMounted() || master->GetShapeshiftForm() == FORM_TRAVEL;
    return !isMasterMounted && bot->IsMounted();
}

bool CheckMountStateAction::MasterWantsFlying(Player* master) const
{
    auto auraEffects = master->GetAuraEffectsByType(SPELL_AURA_MOUNTED);
    if (auraEffects.empty())
        return false;

    SpellInfo const* masterSpell = auraEffects.front()->GetSpellInfo();
    MountCapabilityEntry const* capability = master->GetMountCapability(uint32(masterSpell->Effects[0].MiscValueB));
    if (!capability)
        return false;

    SpellInfo const* speedInfo = sSpellMgr->GetSpellInfo(capability->SpeedModSpell);
    if (!speedInfo)
        return false;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (speedInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)
            return true;

    return false;
}
