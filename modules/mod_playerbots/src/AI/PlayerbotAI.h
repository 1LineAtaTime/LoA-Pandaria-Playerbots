#ifndef _PLAYERBOT_PLAYERBOTAI_H
#define _PLAYERBOT_PLAYERBOTAI_H

#include <list>
#include <queue>
#include <stack>

#include "Chat.h"
#include "ChatFilter.h"
#include "ChatHelper.h"
#include "Event.h"
#include "ForceRebuff.h"
#include "Item.h"
#include "PlayerbotAIBase.h"
#include "PlayerbotSecurity.h"
#include "NewRpgStrategy.h"
#include "WorldPacket.h"

enum BotState
{
    BOT_STATE_COMBAT = 0,
    BOT_STATE_NON_COMBAT = 1,
    BOT_STATE_DEAD = 2,

    BOT_STATE_MAX
};

enum BotRoles : uint8
{
    BOT_ROLE_NONE = 0x00,
    BOT_ROLE_TANK = 0x01,
    BOT_ROLE_HEALER = 0x02,
    BOT_ROLE_DPS = 0x04
};

enum ActivityType
{
    GRIND_ACTIVITY = 1,
    RPG_ACTIVITY = 2,
    TRAVEL_ACTIVITY = 3,
    OUT_OF_PARTY_ACTIVITY = 4,
    PACKET_ACTIVITY = 5,
    DETAILED_MOVE_ACTIVITY = 6,
    PARTY_ACTIVITY = 7,
    ALL_ACTIVITY = 8,

    MAX_ACTIVITY_TYPE
};

// #611 item 4: AC Bot/PlayerbotAI.h:230-255 (AC enum class GrouperType / GuilderType).
enum class GrouperType : uint8
{
    SOLO = 0,
    MEMBER = 1,
    LEADER_2 = 2,
    LEADER_3 = 3,
    LEADER_4 = 4,
    LEADER_5 = 5
};

enum class GuilderType : uint8
{
    SOLO = 0,
    TINY = 30,
    SMALL = 50,
    MEDIUM = 70,
    LARGE = 120,
    VERY_LARGE = 250
};

class Aura;
class AiObjectContext;
class Creature;
class Engine;
class Strategy;  // #413
class ExternalEventHelper;
class GameObject;
class ObjectGuid;
class Player;
class Position;
class Unit;
class Item;
class WorldObject;
struct CreatureData;

class PlayerbotChatHandler : protected ChatHandler
{
public:
    explicit PlayerbotChatHandler(Player* pMasterPlayer);
    void sysmessage(std::string const str) { SendSysMessage(str.c_str()); }
    uint32 extractQuestId(std::string const str);
    uint32 extractSpellId(std::string const str)
    {
        char* source = (char*)str.c_str();
        return extractSpellIdFromLink(source);
    }
};

class MinValueCalculator
{
public:
    MinValueCalculator(float def = 0.0f) : param(nullptr), minValue(def) {}

    void probe(float value, void* p)
    {
        if (!param || minValue >= value)
        {
            minValue = value;
            param = p;
        }
    }

    void* param;
    float minValue;
};

class PacketHandlingHelper
{
public:
    void AddHandler(uint16 opcode, std::string const handler);
    void Handle(ExternalEventHelper& helper);
    void AddPacket(WorldPacket const& packet);

private:
    std::map<uint16, std::string> _handlers;
    std::stack<WorldPacket> _queue;
};

// Ported from AC Bot/PlayerbotAI.h:361-384 (#173).
class BotChatCommand
{
public:
    BotChatCommand(std::string const command, Player* owner = nullptr, uint32 type = CHAT_MSG_WHISPER,
                      time_t time = 0)
        : command(command), owner(owner), type(type), time(time)
    {
    }
    BotChatCommand(BotChatCommand const& other)
        : command(other.command), owner(other.owner), type(other.type), time(other.time)
    {
    }

    std::string const& GetCommand() { return command; }
    Player* GetOwner() { return owner; }
    uint32& GetType() { return type; }
    time_t& GetTime() { return time; }

private:
    std::string const command;
    Player* owner;
    uint32 type;
    time_t time;
};

class PlayerbotAI : public PlayerbotAIBase
{
public:
    PlayerbotAI();
    PlayerbotAI(Player* bot);
    virtual ~PlayerbotAI();

    /* static */
    static uint32 GetMixedGearScore(Player* player, bool withBags, bool withBank, uint32 topN = 0);
    static float GetItemScoreMultiplier(ItemQualities quality);
private:
    static void _fillGearScoreData(Player* player, Item* item, std::vector<uint32>* gearScore, uint32& twoHandScore, bool mixed = false);

public:
    void UpdateAI(uint32 elapsed, bool minimal = false) override;
    void UpdateAIInternal(uint32 elapsed, bool minimal = false) override;
    bool AllowActivity(ActivityType activityType = ALL_ACTIVITY, bool checkNow = false);
    bool AllowActive(ActivityType activityType);
    void DoNextAction(bool min);

    // #173: whisper/party/raid/guild chat commands (AC Bot/PlayerbotAI.cpp:577-1136 shape). Called
    // from the mod_playerbots.cpp OnChat hooks; HandleCommands() drains the queue every tick from
    // UpdateAIInternal.
    void HandleCommand(uint32 type, std::string const text, Player* fromPlayer);
    bool IsAllowedCommand(std::string const text);

    void HandleBotOutgoingPacket(WorldPacket const& packet);
    void HandleMasterIncomingPacket(WorldPacket const& packet);
    void HandleMasterOutgoingPacket(WorldPacket const& packet);
    void HandleTeleportAck();
    
    void Reset(bool full = false);
    void ChangeEngine(BotState type);
    void ReInitCurrentEngine();
    virtual bool DoSpecificAction(std::string const name, Event event = Event(), bool silent = false, std::string const qualifier = "");

    void ResetStrategies();
    void ChangeStrategy(std::string const name, BotState type);
    void ClearStrategies(BotState type);
    std::vector<std::string> GetStrategies(BotState type);

    Player* GetBot() { return bot; }
    Player* GetMaster() { return master; }
    Player* GetGroupMaster();
    // Ported from AC Bot/PlayerbotAI.cpp:2145-2169 (#173). This bot's 0-based position in its
    // group's member list (or -1 outside a group); reserved for the taxi stagger (spec section 3.3,
    // plan batch S5), unused until then.
    int32 GetGroupSlotIndex(Player* player);
    // #173 declaration only (AC Bot/PlayerbotAI.cpp:162,816,1636,1923 glue); the per-map instance/raid
    // strategy content itself is a separate project - MoP has no Ai/Dungeon or Ai/Raid strategies yet
    // (spec section 3.2). Not called from anywhere in this pass.
    void ApplyInstanceStrategies(uint32 mapId, bool tellMaster = false);
    Creature* GetCreature(ObjectGuid guid);
    Unit* GetUnit(ObjectGuid guid);
    Player* GetPlayer(ObjectGuid guid);
    static Unit* GetUnit(CreatureData const* creatureData);
    GameObject* GetGameObject(ObjectGuid guid);
    // static GameObject* GetGameObject(GameObjectData const* gameObjectData);
    WorldObject* GetWorldObject(ObjectGuid guid);
    AiObjectContext* GetAiObjectContext() { return _aiObjectContext; }
    ChatHelper* GetChatHelper() { return &chatHelper; }
    PlayerbotSecurity* GetSecurity() { return &security; }
    BotState GetState() { return _currentState; };
    NewRpgInfo& GetRpgInfo() { return _rpgInfo; }
    const NewRpgInfo& GetRpgInfo() const { return _rpgInfo; }
    // Running totals for the "new rpg" quest behaviour (accepted / rewarded / dropped ...).
    NewRpgStatistic& GetRpgStatistic() { return _rpgStatistic; }
    // Quests this bot gave up on (no objective progress after sitting at the POI), so the rpg
    // status machine stops picking them again. Per bot, as upstream has it.
    std::set<uint32>& GetLowPriorityQuests() { return _lowPriorityQuest; }
    NewRpgStatistic const& GetRpgStatistic() const { return _rpgStatistic; }
    // #173 header batch (AC PlayerbotAI.h:609): state only, see AI/ForceRebuff.h. The ready-check
    // refinement that reads/writes it is plan batch S10, not this pass.
    ForceRebuffState forceRebuff;
    float GetRange(std::string const type);

    bool IsOpposing(Player* player);
    static bool IsOpposing(uint8 race1, uint8 race2);
    // Checks if the bot is really a player. Players always have themselves as master.
    bool IsRealPlayer() { return master ? (master == bot) : false; }
    // Bot has a master that is a player.
    bool HasRealPlayerMaster();
    // #611 item 4 (AC PlayerbotAI.h:535-585): HasSkill, HasGameClientMaster, GetFixedBotNumber, GetGrouperType,
    // GetGuilderType, IsInRealGuild.
    bool HasSkill(SkillType skill);
    bool HasGameClientMaster();
    uint32 GetFixedBotNumber(uint32 maxNum = 100);
    GrouperType GetGrouperType();
    GuilderType GetGuilderType();
    bool IsInRealGuild();
    // Bot has a master that is activly playing.
    bool HasActivePlayerMaster();
    bool HasPlayerNearby(WorldPosition* pos, float range = sPlayerbotAIConfig->reactDistance);
    bool HasPlayerNearby(float range = sPlayerbotAIConfig->reactDistance);
    bool HasManyPlayersNearby(uint32 trigerrValue = 20, float range = sPlayerbotAIConfig->sightDistance);

    // Check if player is safe to use.
    bool IsSafe(Player* player);
    bool IsSafe(WorldObject* obj);
    bool HasAggro(Unit* unit);

    // Get the group leader or the master of the bot.
    // Checks if the bot is summoned as alt of a player
    bool IsAlt();
    bool IsInVehicle(bool canControl = false, bool canCast = false, bool canAttack = false, bool canTurn = false, bool fixed = false);

    bool HasStrategy(std::string const name, BotState type);
    // AC PlayerbotAI.h:416 (#413)
    bool HasTargetExclusions() const;
    Strategy* GetStrategy(std::string const name, BotState type);
    bool ContainsStrategy(StrategyType type);

    void SetMaster(Player* newMaster) { master = newMaster; }

    bool CanMove();

    uint32 GetReactDelay();
    void InterruptSpell();
    virtual bool IsInterruptableSpellCasting(Unit* player, std::string const spell);
    bool canDispel(SpellInfo const* spellInfo, uint32 dispelType);
    virtual bool HasAuraToDispel(Unit* player, uint32 dispelType);
    /// Hold off the bot's next decision until a cast it just started has finished, so it does not
    /// immediately pick a new action and interrupt itself. Channelled spells wait for the channel
    /// as well as the cast.
    void WaitForSpellCast(Spell* spell);
    void RemoveAura(std::string const name);
    void RemoveShapeshift();
    void SpellInterrupted(uint32 spellid);
    virtual bool CanCastSpell(std::string const name, Unit* target, Item* itemTarget = nullptr);
    virtual bool CastSpell(std::string const name, Unit* target, Item* itemTarget = nullptr);
    bool CastSpell(uint32 spellId, Unit* target, Item* itemTarget = nullptr);
    bool CastSpell(uint32 spellId, float x, float y, float z, Item* itemTarget = nullptr);
    // #581 (AC PlayerbotAI.h:526-527): cast from a vehicle seat (IoC siege engines, demolishers, cannons).
    bool CanCastVehicleSpell(uint32 spellId, Unit* target);
    bool CastVehicleSpell(uint32 spellId, Unit* target);
    virtual bool HasAura(std::string const spellName, Unit* player, bool maxStack = false, bool checkIsOwner = false,
        int maxAmount = -1, bool checkDuration = false);
    Aura* GetAura(std::string const spellName, Unit* unit, bool checkIsOwner = false, bool checkDuration = false,
        int checkStack = -1);
    virtual bool HasAnyAuraOf(Unit* player, ...);
    bool CanCastSpell(uint32 spellid, Unit* target, bool checkHasSpell = true, Item* itemTarget = nullptr,
        Item* castItem = nullptr);
    bool CanCastSpell(uint32 spellid, GameObject* goTarget, uint8 effectMask, bool checkHasSpell = true);
    bool CanCastSpell(uint32 spellid, float x, float y, float z, uint8 effectMask, bool checkHasSpell = true,
        Item* itemTarget = nullptr);
    bool HasAura(uint32 spellId, Unit const* player);

    int32 GetNearGroupMemberCount(float dis = sPlayerbotAIConfig->sightDistance);

    InventoryResult CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap, bool not_loading = true) const;
    uint8 FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const;

    // #173 (AC Bot/PlayerbotAI.h:465-471): per-call security level, defaulted so every existing caller is unchanged.
    bool TellMaster(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    bool TellMaster(std::string const text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    bool TellMasterNoFacing(std::string const text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    bool TellMasterNoFacing(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    bool Whisper(const std::string& msg, const std::string& receiverName);
    bool Say(const std::string& msg);
    bool Yell(const std::string& msg);
    bool SayToRaid(const std::string& msg);
    bool SayToParty(const std::string& msg);
    bool TellError(std::string const text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
private:
    bool _isBotInitializing;
    // #173: drains chatCommands once per tick (called from UpdateAIInternal).
    void HandleCommands();
    // #639 (AC Bot/PlayerbotAI.h:618): group-master upkeep, called from UpdateAI before UpdateAIInternal.
    void UpdateAIGroupMaster();
    // #173: the security gate for TellMaster/TellMasterNoFacing/TellError (AC Bot/PlayerbotAI.h:617, private as in AC).
    bool IsTellAllowed(PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);

protected:
    Player* bot;
    Player* master;
    uint32 accountId;
    AiObjectContext* _aiObjectContext;
    Engine* _currentEngine;
    Engine* _engines[BOT_STATE_MAX];
    BotState _currentState;
    // issue #61: when the bot entered the combat engine while not actually in combat, so a bot
    // stranded there (attacking a non-attackable target) can be recovered back to non-combat.
    uint32 _combatEngineNoCombatSince{0};
    NewRpgInfo _rpgInfo;
    NewRpgStatistic _rpgStatistic;
    std::set<uint32> _lowPriorityQuest;
    ChatHelper chatHelper;
    std::list<BotChatCommand> chatCommands;
    PacketHandlingHelper botOutgoingPacketHandlers;
    PacketHandlingHelper masterIncomingPacketHandlers;
    PacketHandlingHelper masterOutgoingPacketHandlers;
    CompositeChatFilter chatFilter;
    PlayerbotSecurity security;
    std::map<std::string, time_t> whispers;
    std::pair<ChatMsg, time_t> currentChat;
    static std::set<std::string> unsecuredCommands;
    bool _allowActive[MAX_ACTIVITY_TYPE];
    time_t _allowActiveCheckTimer[MAX_ACTIVITY_TYPE];
    //bool inCombat = false;
    //BotCheatMask cheatMask = BotCheatMask::none;
    //Position jumpDestination = Position();
    //uint32 nextTransportCheck = 0;
};

#endif