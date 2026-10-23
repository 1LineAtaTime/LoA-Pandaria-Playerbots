/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BATTLEGROUNDTACTICS_H
#define PLAYERBOTS_BATTLEGROUNDTACTICS_H

// Order matters here. BattlegroundAV.h reaches BattlegroundQueue.h -> RatedPvp.h, which uses both
// CharacterDatabaseTransaction and ObjectGuid without declaring either. MovementActions.h has to
// come first so the ordinary object headers are in scope, and DatabaseEnv.h after it but still
// before the battleground header. This header is included from ActionContext, so getting it wrong
// breaks every translation unit that registers actions.
#include "MovementActions.h"

#include "DatabaseEnv.h"

class ChatHandler;
class Battleground;
class PlayerbotAI;
struct Position;

#define SPELL_CAPTURE_BANNER 21651

enum WSBotStrategy : uint8
{
    WS_STRATEGY_BALANCED      = 0,
    WS_STRATEGY_OFFENSIVE     = 1,
    WS_STRATEGY_DEFENSIVE     = 2,
    WS_STRATEGY_MAX           = 3,
};

enum ABBotStrategy : uint8
{
    AB_STRATEGY_BALANCED      = 0,
    AB_STRATEGY_OFFENSIVE     = 1,
    AB_STRATEGY_DEFENSIVE     = 2,
    AB_STRATEGY_MAX           = 3,
};

enum AVBotStrategy : uint8
{
    AV_STRATEGY_BALANCED      = 0,
    AV_STRATEGY_OFFENSIVE     = 1,
    AV_STRATEGY_DEFENSIVE     = 2,
    AV_STRATEGY_MAX           = 3,
};

enum EYBotStrategy : uint8
{
    EY_STRATEGY_BALANCED      = 0,
    EY_STRATEGY_FRONT_FOCUS   = 1,
    EY_STRATEGY_BACK_FOCUS    = 2,
    EY_STRATEGY_FLAG_FOCUS    = 3,
    EY_STRATEGY_MAX           = 4
};

// Temple of Kotmogu split (owner 2026-10-08, MoP-only - AC has no Temple of Kotmogu): AC's per-match team strategy
// (GetBotStrategyForTeam, rolled once per match per team like AV/EY) extended to the orb map. Each strategy sets how
// many of the 10 "bg role" ranks take orbs (and how many of those go to the west / east orbs), hold the centre with
// the own carriers, or hunt the enemy carriers - see TOK_PLANS in BgTacticsTOK.cpp. Rolled with the weights in
// BattleGroundTactics.cpp (TOK_STRATEGY_WEIGHTS); the 1:1 harness pins it with strat=<a>,<h> like the others.
enum TOKBotStrategy : uint8
{
    TOK_STRATEGY_BALANCED     = 0,
    TOK_STRATEGY_WEST_FOCUS   = 1,
    TOK_STRATEGY_EAST_FOCUS   = 2,
    TOK_STRATEGY_ORB_FOCUS    = 3,
    TOK_STRATEGY_CENTRE_FOCUS = 4,
    TOK_STRATEGY_HUNT_FOCUS   = 5,
    TOK_STRATEGY_MAX          = 6
};

typedef void (*BattleBotWaypointFunc)();

struct BGStrategyData
{
    uint8 allianceStrategy = 0;
    uint8 hordeStrategy = 0;
};

extern std::unordered_map<uint32, BGStrategyData> bgStrategies;

struct BattleBotWaypoint
{
    BattleBotWaypoint(float x_, float y_, float z_, BattleBotWaypointFunc func) : x(x_), y(y_), z(z_), pFunc(func){};

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    BattleBotWaypointFunc pFunc = nullptr;
};

struct AVNodePositionData
{
    Position pos;
    float maxRadius;
};

typedef std::vector<BattleBotWaypoint> BattleBotPath;

extern std::vector<BattleBotPath*> const vPaths_WS;
extern std::vector<BattleBotPath*> const vPaths_AB;
extern std::vector<BattleBotPath*> const vPaths_AV;
extern std::vector<BattleBotPath*> const vPaths_EY;
extern std::vector<BattleBotPath*> const vPaths_TP;
extern std::vector<BattleBotPath*> const vPaths_IC;

/// Objective play inside a battleground: pick a target (flag, node, tower), walk there along the
/// authored paths, and interact with it.
///
/// Arena play is a separate action (ArenaTactics below, #346). The two must never both be on an
/// engine: BGTactics' per-map path data has no arena case, and it would reset the strategies instead
/// of placing the bot - which is why this fork used to send an arena bot down generic PvE instead.
class BGTactics : public MovementAction
{
public:
    static bool HandleConsoleCommand(ChatHandler* handler, char const* args);
    uint8 static GetBotStrategyForTeam(Battleground* bg, TeamId teamId);

    /// #754 (beyond AC): the unit a bot must fight before it starts a capture cast (an attacker or a PvP combat
    /// opponent within 50 yd; enemies merely nearby do not count), else nullptr. Public for SotaBeyondAc::Capture.
    static Unit* CaptureThreat(Player* bot);

    /// #754/#756 (beyond AC): fight `threat` the way AttackAction::Attack does - select it, make it the "current
    /// target", switch to the combat engine and start the auto attack. False when it is not a valid, visible target in
    /// line of sight. Public for SotaBeyondAc::Capture.
    static bool EngageThreat(Player* bot, Unit* threat);

    BGTactics(PlayerbotAI* botAI, std::string const name = "bg tactics") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

private:
    static std::string const HandleConsoleCommandPrivate(WorldSession* session, char const* args);
    bool moveToStart(bool force = false);
    bool selectObjective(bool reset = false);
    bool moveToObjective(bool ignoreDist);
    bool selectObjectiveWp(std::vector<BattleBotPath*> const& vPaths);
    bool moveToObjectiveWp(BattleBotPath* const& currentPath, uint32 currentPoint, bool reverse = false);
    bool startNewPathBegin(std::vector<BattleBotPath*> const& vPaths);
    bool startNewPathFree(std::vector<BattleBotPath*> const& vPaths);
    bool resetObjective();
    bool wsJumpDown();
    bool eyJumpDown();
    bool tpJumpDown();
    bool atFlag(std::vector<BattleBotPath*> const& vPaths, std::vector<uint32> const& vFlagIds);
    bool flagTaken();
    bool teamFlagTaken();
    bool protectFC();
    bool useBuff();
    uint32 getPlayersInArea(TeamId teamId, Position point, float range, bool combat = true);
    bool IsLockedInsideKeep();

    // Cataclysm/MoP-only battlegrounds (#576-#579, epic #580). AC has no spec for them, so each extends
    // AC's own per-map pattern (one selectObjective case, one atFlag case, one moveToStart branch) and
    // lives in its own file so the four can be written independently:
    //   Battle for Gilneas (map 761)  -> BgTacticsBFG.cpp  AB pattern (3 capturable nodes)
    //   Temple of Kotmogu  (map 998)  -> BgTacticsTOK.cpp  WSG/EotS carrier pattern (4 orbs)
    //   Silvershard Mines  (map 727)  -> BgTacticsSM.cpp   AB proximity-capture pattern on moving carts
    //   Deepwind Gorge     (map 1105) -> BgTacticsDG.cpp   AB nodes + WSG cart steal
    // select*: choose the bot's objective and store it in the "bg objective" position (as AC's
    //          selectObjective cases do); returns true when one was set.
    // at*:     interact with the objective in reach (AC atFlag's role); returns true when it acted.
    // moveToStart*: WAIT_JOIN positioning (AC moveToStart's per-map branch); returns true when it moved.
    // isCarrying*: the bot holds the map's carried objective (orb / cart) and must only move to objective.
    bool selectObjectiveBFG(Battleground* bg);
    bool atObjectiveBFG(Battleground* bg);
    bool moveToStartBFG(Battleground* bg);
    bool selectObjectiveTOK(Battleground* bg);
    bool atObjectiveTOK(Battleground* bg);
    bool moveToStartTOK(Battleground* bg);
    bool isCarryingTOK(Battleground* bg);
    bool selectObjectiveSM(Battleground* bg);
    bool atObjectiveSM(Battleground* bg);
    bool moveToStartSM(Battleground* bg);
    bool selectObjectiveDG(Battleground* bg);
    bool atObjectiveDG(Battleground* bg);
    bool moveToStartDG(Battleground* bg);
    bool isCarryingDG(Battleground* bg);
    /// true while the bot carries a WSG/TP/EotS flag, a Kotmogu orb or a Deepwind cart.
    bool isCarryingObjective(Battleground* bg);
    bool FightPathAttacker(Battleground* bg);  // #756 (beyond AC)

    /// Which side a player is on FOR THIS BATTLEGROUND.
    ///
    /// This core assigns battleground teams independently of race (see the cross-faction handling
    /// in BattlegroundWS::EventPlayerClickedOnFlag), so Player::GetTeamId() - the race's team - is
    /// the wrong answer for anything battleground-relative: which base to start at, which paths to
    /// walk, which flag is "ours". Getting this wrong sends a bot to the enemy's half of the map
    /// thinking it is home, and makes it guard the flag it should be stealing.
    static TeamId BgTeamIdOf(Player* player);
};

/// Ported from AC (Ai/Base/Actions/BattleGroundTactics.h:137-140, .cpp:4238-4418) for #346.
/// Arena positioning only - there is nothing to cap, escort or pick up. Out of combat it walks the
/// bot to the middle of the map (a per-class spot, see moveToCenter), and it steps out of the line
/// of sight of a current victim it cannot see.
class ArenaTactics : public MovementAction
{
public:
    ArenaTactics(PlayerbotAI* botAI, std::string const name = "arena tactics") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

private:
    bool moveToCenter(Battleground* bg);
};

#endif
