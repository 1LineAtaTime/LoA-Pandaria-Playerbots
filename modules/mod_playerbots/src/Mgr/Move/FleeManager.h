/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * Ported from AC mod-playerbots Mgr/Move/FleeManager.h (#177). AC has no "Mgr" directory tree on this
 * fork; kept at the same relative path (Mgr/Move/) for 1:1 diffing against AC since it is a brand new
 * file (the module's CMakeLists.txt GLOB_RECURSEs modules/mod_playerbots/src, so the new directory is
 * picked up on the next -Reconfigure without any build-file edits).
 */

#ifndef _PLAYERBOT_FLEEMANAGER_H
#define _PLAYERBOT_FLEEMANAGER_H

#include "Common.h"
#include "TravelMgr.h"
#include <vector>

class Player;
class PlayerbotAI;

class FleePoint
{
public:
    FleePoint(float x, float y, float z) : x(x), y(y), z(z), sumDistance(0.0f), minDistance(0.0f)
    {
    }

    float x;
    float y;
    float z;

    float sumDistance;
    float minDistance;
};

class FleeManager
{
public:
    FleeManager(Player* bot, float maxAllowedDistance, float followAngle, bool forceMaxDistance = false,
                WorldPosition startPosition = WorldPosition());

    bool CalculateDestination(float* rx, float* ry, float* rz);
    bool isUseful();

private:
    void calculatePossibleDestinations(std::vector<FleePoint*>& points);
    void calculateDistanceToCreatures(FleePoint* point);
    void cleanup(std::vector<FleePoint*>& points);
    FleePoint* selectOptimalDestination(std::vector<FleePoint*>& points);
    bool isBetterThan(FleePoint* point, FleePoint* other);

    Player* bot;
    float maxAllowedDistance;
    bool forceMaxDistance;
    WorldPosition startPosition;
};

#endif
