/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/FishingAction.cpp (issue #412; #375 is the same gap).
 * Logic is 1:1; MoP API adaptations (each marked "MoP:" below):
 *  - Map::GetLiquidData() does not exist on this core; Map::GetLiquidStatus(phaseMask, x, y, z,
 *    MAP_ALL_LIQUIDS, &data, collisionHeight) (Map.h:389) is used, "no liquid" == LIQUID_MAP_NO_WATER and
 *    LiquidData fields are entry/level/depth_level (Map.h:160-166) instead of Entry/Level/DepthLevel.
 *  - Map::isInLineOfSight has no LINEOFSIGHT_ALL_CHECKS argument here (Map.h:559).
 *  - Equip goes through Player::SwapItem (as strategy/actions/EquipAction.cpp does) instead of a faked
 *    AutoEquipItemSlot packet.
 *  - PlayerbotTextMgr is not ported: AC's literal default string is used (same as BankAction.cpp).
 *  - sPlayerbotAIConfig has no fishing keys on this fork (PlayerbotAIConfig.h is not touched by this
 *    port): AC's defaults are file-local constants below (AC PlayerbotAIConfig.cpp:291-294).
 *  - The cast spell: AC casts 7620; the first of kFishingSpells the bot knows is cast (see below).
 *  - WorldPosition has no IsValid() on this fork; IsValidPos() below is AC's IsValid() (operator bool has
 *    different semantics - #412). Position::HasInArc's third argument is a divisor here, not AC's target
 *    radius: AcHasInArc() rebuilds AC's check. FishingAction::Execute logs one "[FISH]" debug line per
 *    cast attempt (water point, bearing, orientation before/after).
 */

#include "FishingAction.h"

#include "Bag.h"
#include "Event.h"
#include "FishValues.h"
#include "Item.h"
#include "LastMovementValue.h"
#include "Map.h"
#include "ModelIgnoreFlags.h"
#include "MovementActions.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Position.h"
#include "RandomPlayerbotMgr.h"
#include "Spell.h"
#include "SpellInfo.h"

#include <cmath>

uint32 const FISHING_POLE = 6256;
uint32 const FISHING_BOBBER = 35591;

// AC FISHING_SPELL = 7620. On 5.4.8 7620 is an unscripted DUMMY (effect 3, test-server Spell/SpellEffect.dbc);
// the castable "Fishing" is 131474, whose script spell_gen_fishing (src/server/scripts/Spells/spell_generic.cpp:1299)
// casts the bobber spell 131476 (no pole) / 131490 (pole), both summoning GO 35591 (effect 50). Verified 2026-10-04.
static uint32 const kFishingSpells[] = {131474};

static float const MIN_DISTANCE_TO_WATER = 10.0f;        // Minimum spell distance
static float const MAX_DISTANCE_TO_WATER = 20.0f;        // Maximum spell distance
static float const HEIGHT_ABOVE_WATER_TOLERANCE = 1.0f;  // Can stand in up to 1 unit of water and still fish.
static float const SEARCH_INCREMENT = 2.5f;
static float const HEIGHT_SEARCH_BUFFER = 10.0f;  // Height buffer to prevent potentially missing the model the bot is standing on.
static float const SEARCH_LAND_BUFFER = 0.5f;
static uint32 const FISHING_LOCATION_TIMEOUT = 180000;  // Three minutes

// AC PlayerbotAIConfig.cpp:291-294 defaults (AiPlayerbot.FishingDistanceFromMaster / EndFishingWithMaster /
// FishingDistance). MoP: not config-backed on this fork.
float const kFishingDistanceFromMaster = 10.0f;  // extern in FishingAction.h (FollowAction::isUseful, AC parity)
static float const kEndFishingWithMaster = 30.0f;
static float const kFishingDistance = 40.0f;

static bool IsFishingPole(Item* const item)
{
    if (!item)
        return false;
    ItemTemplate const* proto = item->GetTemplate();
    return proto && proto->Class == ITEM_CLASS_WEAPON && proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE;
}

// MoP: GetLiquidStatus replaces AC's GetLiquidData; returns false when there is no liquid.
static bool GetLiquid(Map* map, uint32 phaseMask, float x, float y, float z, LiquidData& liq)
{
    liq = LiquidData();
    liq.level = INVALID_HEIGHT;
    liq.depth_level = INVALID_HEIGHT;
    ZLiquidStatus status = map->GetLiquidStatus(phaseMask, x, y, z, MAP_ALL_LIQUIDS, &liq, DEFAULT_COLLISION_HEIGHT);
    return status != LIQUID_MAP_NO_WATER;
}

static float HasFishableWaterOrLand(float x, float y, float z, Map* map, uint32 phaseMask, bool checkForLand = false)
{
    if (!map)
        return INVALID_HEIGHT;

    LiquidData liq;
    bool hasLiquid = GetLiquid(map, phaseMask, x, y, z + HEIGHT_ABOVE_WATER_TOLERANCE, liq);
    float ground = map->GetHeight(phaseMask, x, y, z + HEIGHT_SEARCH_BUFFER, true);
    if (!hasLiquid)
    {
        if (checkForLand)
            return ground;
        return INVALID_HEIGHT;
    }
    if (checkForLand)
    {
        if (ground > liq.level - HEIGHT_ABOVE_WATER_TOLERANCE)
            return ground;
        return INVALID_HEIGHT;
    }

    if (liq.level + HEIGHT_ABOVE_WATER_TOLERANCE > ground)
    {
        if (std::abs(liq.depth_level) < 0.5f)  // too shallow to fish in.
            return INVALID_HEIGHT;
        return liq.level;
    }
    return INVALID_HEIGHT;
}

static bool HasLosToWater(Player* bot, float wx, float wy, float waterZ)
{
    float z = bot->GetCollisionHeight() + bot->GetPositionZ();
    return bot->GetMap()->isInLineOfSight(bot->GetPositionX(), bot->GetPositionY(), z, wx, wy, waterZ,
                                          bot->GetPhaseMask(), VMAP::ModelIgnoreFlags::Nothing);
}

static WorldPosition FindLandFromPosition(PlayerbotAI* botAI, float startDistance, float endDistance, float increment,
                                          float orientation, WorldPosition targetPos, float fishingSearchWindow,
                                          bool checkLOS = true)
{
    Player* bot = botAI->GetBot();
    Map* map = bot->GetMap();
    uint32 phaseMask = bot->GetPhaseMask();
    Player* master = botAI->GetMaster();

    float targetX = targetPos.GetPositionX();
    float targetY = targetPos.GetPositionY();
    float targetZ = targetPos.GetPositionZ();

    for (float dist = startDistance; dist <= endDistance; dist += increment)
    {
        // step backwards from position to bot to find edge of shore.
        float checkX = targetX - dist * cos(orientation);
        float checkY = targetY - dist * sin(orientation);

        float groundZ = map->GetHeight(phaseMask, checkX, checkY, targetZ + HEIGHT_SEARCH_BUFFER, true);

        if (groundZ == INVALID_HEIGHT)
            continue;

        LiquidData liq;
        bool hasLiquid = GetLiquid(map, phaseMask, checkX, checkY, targetZ, liq);
        if (!hasLiquid || groundZ > liq.depth_level + HEIGHT_ABOVE_WATER_TOLERANCE)
        {
            if (checkLOS)
            {
                bool hasLOS = map->isInLineOfSight(checkX, checkY, groundZ, targetX, targetY, targetZ, phaseMask,
                                                   VMAP::ModelIgnoreFlags::Nothing);
                if (!hasLOS)
                    continue;
            }
            // Add a distance check for the position to prevent the bot from moving out of range to the master.
            if (master && botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT) &&
                master->GetDistance(checkX, checkY, groundZ) > fishingSearchWindow - SEARCH_LAND_BUFFER)
                continue;

            return WorldPosition(bot->GetMapId(), checkX, checkY, groundZ);
        }
    }

    return WorldPosition();
}

static WorldPosition FindLandRadialFromPosition(PlayerbotAI* botAI, WorldPosition targetPos, float startDistance,
                                                float endDistance, float increment, float fishingSearchWindow,
                                                int angles = 16)
{
    Player* bot = botAI->GetBot();
    int const numDirections = angles;
    std::vector<WorldPosition> boundaryPoints;
    Player* master = botAI->GetMaster();
    if (!master)
        return WorldPosition();

    Map* map = bot->GetMap();
    uint32 phaseMask = bot->GetPhaseMask();

    float targetX = targetPos.GetPositionX();
    float targetY = targetPos.GetPositionY();
    float targetZ = targetPos.GetPositionZ();

    for (float dist = startDistance; dist <= endDistance; dist += increment)
    {
        for (int i = 0; i < numDirections; ++i)
        {
            float angle = (2.0f * M_PI * i) / numDirections;
            float checkX = targetX - cos(angle) * dist;
            float checkY = targetY - sin(angle) * dist;

            float groundZ = HasFishableWaterOrLand(checkX, checkY, targetZ, map, phaseMask, true);

            if (groundZ == INVALID_HEIGHT)
                continue;

            if (map->isInLineOfSight(checkX, checkY, groundZ, targetX, targetY, targetZ, phaseMask,
                                     VMAP::ModelIgnoreFlags::Nothing) &&
                master->GetDistance(checkX, checkY, groundZ) > fishingSearchWindow - SEARCH_LAND_BUFFER)
                continue;

            boundaryPoints.emplace_back(WorldPosition(bot->GetMapId(), checkX, checkY, groundZ));
        }

        if (!boundaryPoints.empty())
            break;
    }

    if (boundaryPoints.empty())
        return WorldPosition();

    if (boundaryPoints.size() == 1)
        return boundaryPoints[0];

    float minDistance = FLT_MAX;
    WorldLocation closestPoint = WorldPosition();
    for (auto const& pos : boundaryPoints)
    {
        float distance = bot->GetExactDist2d(&pos);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestPoint = pos;
        }
    }
    return closestPoint;
}

WorldPosition FindWaterRadial(Player* bot, float x, float y, float z, Map* map, uint32 phaseMask, float minDistance,
                              float maxDistance, float increment, bool checkLOS, int numDirections)
{
    std::vector<WorldPosition> boundaryPoints;

    float dist = minDistance;
    while (dist <= maxDistance)
    {
        for (int i = 0; i < numDirections; ++i)
        {
            float angle = (2.0f * M_PI * i) / numDirections;
            float checkX = x + cos(angle) * dist;
            float checkY = y + sin(angle) * dist;

            float waterZ = HasFishableWaterOrLand(checkX, checkY, z, map, phaseMask);

            if (waterZ == INVALID_HEIGHT)
                continue;

            if (checkLOS && !HasLosToWater(bot, checkX, checkY, waterZ))
                continue;

            boundaryPoints.emplace_back(WorldPosition(bot->GetMapId(), checkX, checkY, waterZ));
        }

        if (!boundaryPoints.empty())
            break;

        dist += increment;
    }

    if (boundaryPoints.empty())
        return WorldPosition();

    if (boundaryPoints.size() == 1)
        return boundaryPoints[0];
    // return the central point in the identified positions in to try to be perpendicular to the shore.
    return boundaryPoints[boundaryPoints.size() / 2];
}

static WorldPosition FindFishingHole(PlayerbotAI* botAI)
{
    Player* player = botAI->GetBot();
    // AC uses PAI_VALUE(GuidVector, ...) with `player`; same value, read through the bot's own context.
    GuidVector gos = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los")->Get();
    GameObject* nearestFishingHole = nullptr;
    float minDist = std::numeric_limits<float>::max();
    for (auto const& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;
        if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGHOLE)
        {
            float dist = player->GetDistance2d(go);
            if (dist < minDist)
            {
                minDist = dist;
                nearestFishingHole = go;
            }
        }
    }
    if (nearestFishingHole)
        return WorldPosition(nearestFishingHole->GetMapId(), nearestFishingHole->GetPositionX(),
                             nearestFishingHole->GetPositionY(), nearestFishingHole->GetPositionZ());

    return WorldPosition();
}

// AC WorldPosition::IsValid() (AC Mgr/Travel/TravelMgr.cpp:275-278): only the default-constructed position
// (MAPID_INVALID at 0,0,0) is "no position". #412: this used to be WorldPosition::operator bool
// (Travel/WorldPosition.cpp:159-162), which treats map 0 at the origin as empty and a default-constructed
// WorldPosition (WorldLocation's default map is MAPID_INVALID, Position.h:187) as VALID, so every "nothing
// found" from FindFishingHole/FindWaterRadial/FindLand* passed: FishingAction::Execute never called
// FindWaterRadial and faced/cast at (0,0,0), and MoveNearWaterAction stored empty fishing spots.
static bool IsValidPos(WorldPosition const& p)
{
    return !(p.GetMapId() == MAPID_INVALID && p.GetPositionX() == 0.0f && p.GetPositionY() == 0.0f &&
             p.GetPositionZ() == 0.0f);
}

// AC Position::HasInArc(arc, obj, targetRadius) (AC Entities/Object/Position.cpp:148-178): +-arc/2, widened
// by the target's angular radius 2*atan(r/2d). On this core HasInArc's third argument is a divisor
// (+-arc/border, Object.cpp:1943-1963), so AC's call is rebuilt on top of it with border 2.
static bool AcHasInArc(Position const* self, Position const* obj, float arc, float targetRadius)
{
    float widen = 0.0f;
    if (targetRadius > 0.0f)
    {
        float const distSq = self->GetExactDist2dSq(obj);
        if (distSq < targetRadius * targetRadius)
            return true;
        widen = 2.0f * std::atan(targetRadius / (2.0f * std::sqrt(distSq)));
    }
    return self->HasInArc(arc + 2.0f * widen, obj, 2.0f);
}

bool MoveNearWaterAction::Execute(Event /*event*/)
{
    WorldPosition landSpot = AI_VALUE(WorldPosition, "fishing spot");
    if (IsValidPos(landSpot))
        return MoveTo(landSpot.GetMapId(), landSpot.GetPositionX(), landSpot.GetPositionY(), landSpot.GetPositionZ());

    return false;
}

bool MoveNearWaterAction::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))
        return false;
    FishingSpotValue* fishingSpotValueObject = (FishingSpotValue*)context->GetValue<WorldPosition>("fishing spot");
    WorldPosition pos = fishingSpotValueObject->Get();
    return !IsValidPos(pos) || fishingSpotValueObject->IsStale(FISHING_LOCATION_TIMEOUT) ||
           bot->GetExactDist(&pos) > 0.1f;
}

bool MoveNearWaterAction::isPossible()
{
    Player* master = botAI->GetMaster();
    float fishingSearchWindow;

    if (master)
        fishingSearchWindow = kFishingDistanceFromMaster;
    else
        fishingSearchWindow = kFishingDistance;

    WorldPosition fishingHole = FindFishingHole(botAI);

    if (IsValidPos(fishingHole))
    {
        float distance = bot->GetExactDist2d(&fishingHole);
        bool hasLOS =
            bot->IsWithinLOS(fishingHole.GetPositionX(), fishingHole.GetPositionY(), fishingHole.GetPositionZ());
        // Water spot is in range, and we have LOS to it. Set bot position to fishing spot and do not move
        if (distance >= MIN_DISTANCE_TO_WATER && distance <= MAX_DISTANCE_TO_WATER && hasLOS)
        {
            SET_AI_VALUE(WorldPosition, "fishing spot",
                         WorldPosition(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()));
            return false;
        }
        // Water spot is out of range, lets look for a spot to move to for the fishing hole.
        if (distance > MAX_DISTANCE_TO_WATER || distance < MIN_DISTANCE_TO_WATER)
        {
            WorldPosition landSpot = FindLandRadialFromPosition(botAI, fishingHole, MIN_DISTANCE_TO_WATER,
                                                                MAX_DISTANCE_TO_WATER, SEARCH_INCREMENT,
                                                                fishingSearchWindow, 32);
            if (IsValidPos(landSpot))
            {
                SET_AI_VALUE(WorldPosition, "fishing spot", landSpot);
                return true;
            }
        }
    }
    // Can the bot fish from current position?
    WorldPosition waterAtCurrentPos =
        FindWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(),
                        bot->GetPhaseMask(), MIN_DISTANCE_TO_WATER, MAX_DISTANCE_TO_WATER, SEARCH_INCREMENT, true);
    if (IsValidPos(waterAtCurrentPos))
    {
        SET_AI_VALUE(WorldPosition, "fishing spot",
                     WorldPosition(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()));
        return false;
    }
    // Lets find some water where we can fish.
    WorldPosition water =
        FindWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(),
                        bot->GetPhaseMask(), MIN_DISTANCE_TO_WATER, fishingSearchWindow + MAX_DISTANCE_TO_WATER,
                        SEARCH_INCREMENT, false);

    if (!IsValidPos(water))
        return false;

    float angle = bot->GetAngle(water.GetPositionX(), water.GetPositionY());
    WorldPosition landSpot =
        FindLandFromPosition(botAI, 0.0f, MAX_DISTANCE_TO_WATER, 1.0f, angle, water, fishingSearchWindow, false);

    if (IsValidPos(landSpot))
    {
        SET_AI_VALUE(WorldPosition, "fishing spot", landSpot);
        return true;
    }
    return false;
}

bool EquipFishingPoleAction::Execute(Event /*event*/)
{
    if (!_pole)
        return false;

    // MoP: SwapItem instead of a faked AutoEquipItemSlot packet (see file header / EquipAction.cpp).
    uint16 dstpos = uint16(EQUIPMENT_SLOT_MAINHAND) | (uint16(INVENTORY_SLOT_BAG_0) << 8);
    if (_pole->GetPos() != dstpos)
        bot->SwapItem(_pole->GetPos(), dstpos);
    return true;
}

bool EquipFishingPoleAction::isUseful()
{
    Item* mainHand = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (IsFishingPole(mainHand))
        return false;

    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (IsFishingPole(item))
            {
                _pole = item;
                return true;
            }
        }
    }

    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag* pBag = bot->GetBagByPos(bag))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* item = pBag->GetItemByPos(j))
                {
                    if (IsFishingPole(item))
                    {
                        _pole = item;
                        return true;
                    }
                }
            }
        }
    }

    if (sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        bot->StoreNewItemInBestSlots(FISHING_POLE, 1);  // Try to get a fishing pole
        return true;
    }

    Player* master = botAI->GetMaster();
    if (!master)
        return false;

    std::string masterName = master->GetName();
    // MoP: PlayerbotTextMgr not ported; AC's default text ("no_fishing_pole_error").
    botAI->Whisper("I don't have a Fishing Pole", masterName);

    return false;
}

bool FishingAction::Execute(Event event)
{
    WorldPosition target = WorldPosition();
    WorldPosition fishingHole = FindFishingHole(botAI);
    if (IsValidPos(fishingHole))
    {
        Position pos = fishingHole;
        float distance = bot->GetExactDist2d(&pos);
        bool hasLOS =
            bot->IsWithinLOS(fishingHole.GetPositionX(), fishingHole.GetPositionY(), fishingHole.GetPositionZ());
        if (distance < MAX_DISTANCE_TO_WATER && distance > MIN_DISTANCE_TO_WATER && hasLOS)
            target = fishingHole;
    }
    if (!IsValidPos(target))
    {
        target = FindWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(),
                                 bot->GetPhaseMask(), MIN_DISTANCE_TO_WATER, MAX_DISTANCE_TO_WATER, SEARCH_INCREMENT,
                                 true, 32);
        if (!IsValidPos(target))
            return false;
    }
    Position pos = target;

    // AC: if (!bot->HasInArc(1.0, &pos, 1.0)) - AC's third argument is a target radius (see AcHasInArc);
    // the core then throws the bobber within +-17.5 deg of this facing (Spell.cpp:1463-1494).
    float const oldOrientation = bot->GetOrientation();
    if (!AcHasInArc(bot, &pos, 1.0f, 1.0f))
    {
        float angle = bot->GetAngle(pos.GetPositionX(), pos.GetPositionY());
        bot->SetOrientation(angle);
        if (!bot->HasUnitState(UNIT_STATE_ROOT))  // AC: bot->IsRooted()
            bot->SendMovementFlagUpdate();
    }
    TC_LOG_DEBUG("playerbots", "[FISH] %s water=(%.1f, %.1f, %.1f) dist=%.1f bearing=%.2f o=%.2f->%.2f",
                 bot->GetName().c_str(), pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(),
                 bot->GetExactDist2d(&pos), bot->GetAngle(pos.GetPositionX(), pos.GetPositionY()), oldOrientation,
                 bot->GetOrientation());

    EquipFishingPoleAction equipAction(botAI);
    if (equipAction.isUseful())
        return equipAction.Execute(event);

    uint32 spellId = kFishingSpells[0];
    for (uint32 id : kFishingSpells)
    {
        if (bot->HasSpell(id))
        {
            spellId = id;
            break;
        }
    }
    botAI->CastSpell(spellId, bot);
    botAI->ChangeStrategy("+use bobber", BOT_STATE_NON_COMBAT);

    return true;
}

bool FishingAction::isUseful()
{
    if (!AI_VALUE(bool, "can fish"))
        return false;

    // #412 (beyond AC, same race exists there): never re-cast while our own bobber channel is running.
    // "can use fishing bobber" sleeps the AI until bobberActiveTime + 0.5 s (FishValues.cpp), but the
    // GO only turns GO_READY once time() > respawnTime - FISHING_BOBBER_READY_TIME (GameObject.cpp:432,
    // whole seconds, strict), so on wake-up the bobber is often not ready yet and the value re-sleeps 1 s.
    // In that same tick the "very often" trigger fired this action, CastSpell (whose in-progress check
    // skips channels, PlayerbotAI.cpp) started a new 131474 and its 131490 replaced the live channel,
    // deleting the bobber before it could be used (wave 10, Castora: channel 164.6-169.2 s, woke 169.5,
    // "go fishing - OK", channel gone, no "use fishing bobber"). Let the channel run; the bobber is
    // used or escapes, the channel ends, and this action fires again as on AC.
    if (Spell const* channel = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        SpellInfo const* channelInfo = channel->GetSpellInfo();
        for (uint8 i = 0; channelInfo && i < MAX_SPELL_EFFECTS; ++i)
            if (channelInfo->Effects[i].Effect == SPELL_EFFECT_TRANS_DOOR &&
                uint32(channelInfo->Effects[i].MiscValue) == FISHING_BOBBER)
                return false;
    }

    FishingSpotValue* fishingSpotValueObject = (FishingSpotValue*)context->GetValue<WorldPosition>("fishing spot");
    WorldPosition pos = fishingSpotValueObject->Get();

    if (!IsValidPos(pos) || fishingSpotValueObject->IsStale(FISHING_LOCATION_TIMEOUT))
        return false;

    return bot->GetExactDist(&pos) < 0.1f;
}

bool UseBobberAction::isUseful() { return AI_VALUE(bool, "can use fishing bobber"); }

bool UseBobberAction::Execute(Event /*event*/)
{
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects no los");
    for (auto const& guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
        {
            if (go->GetEntry() != FISHING_BOBBER)
                continue;
            if (go->GetOwnerGUID() != bot->GetGUID())
                continue;
            if (go->getLootState() == GO_READY)
            {
                go->Use(bot);
                botAI->ChangeStrategy("-use bobber", BOT_STATE_NON_COMBAT);
                return true;
            }
        }
    }
    return false;
}

bool EndMasterFishingAction::Execute(Event /*event*/)
{
    botAI->ChangeStrategy("-master fishing", BOT_STATE_NON_COMBAT);
    return true;
}

bool EndMasterFishingAction::isUseful()
{
    FishingSpotValue* fishingSpotValueObject = (FishingSpotValue*)context->GetValue<WorldPosition>("fishing spot");
    WorldPosition pos = fishingSpotValueObject->Get();
    // AC: pos == bot->GetPosition() (x/y/z equality); MoP: WorldPosition has no Position overload of ==.
    if (IsValidPos(pos) && !fishingSpotValueObject->IsStale(FISHING_LOCATION_TIMEOUT) &&
        pos.GetPositionX() == bot->GetPositionX() && pos.GetPositionY() == bot->GetPositionY() &&
        pos.GetPositionZ() == bot->GetPositionZ())
        return false;

    WorldPosition nearWater =
        FindWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(),
                        bot->GetPhaseMask(), MIN_DISTANCE_TO_WATER, kEndFishingWithMaster, 10.0f);
    return !IsValidPos(nearWater);
}

bool RemoveBobberStrategyAction::Execute(Event /*event*/)
{
    botAI->ChangeStrategy("-use bobber", BOT_STATE_NON_COMBAT);
    return true;
}
