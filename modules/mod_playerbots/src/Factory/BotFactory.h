#ifndef _PLAYERBOT_BOTFACTORY_H
#define _PLAYERBOT_BOTFACTORY_H

#include "Player.h"
#include "PlayerbotAI.h"

class BotFactory
{
public:
    BotFactory(Player* bot, uint32 level, uint32 itemQuality = 0, uint32 gearScoreLimit = 0);

    static ObjectGuid GetRandomBot();
    // AC PlayerbotFactory::CalcMixedGearScore (Bot/Factory/PlayerbotFactory.cpp:3912)
    static uint32 CalcMixedGearScore(uint32 gs, uint32 quality);
    static void Init();
    void Refresh();
    void Randomize(bool incremental);
    void ClearEverything();

    void InitEquipment(bool incremental, bool second_chance = false);
    void InitBags(bool destroyOld = true);
    void InitPet();
    void InitTalentsTree(bool reset);
    /// #734: learn one talent per unlocked 5.4.8 tier (AiPlayerbot.PremadeSpecLink); idempotent.
    void InitTalentTiers();
    /// #734: AC PlayerbotFactory::InitGlyphs - fill the unlocked glyph slots (AiPlayerbot.PremadeSpecGlyph).
    void InitGlyphs(bool increment = false);
    void InitClassSpells();
    /// issue #62: give the bot a ground mount so the mount strategy has something to cast.
    void InitMountSpells();
    /// #745: AC PlayerbotFactory::InitSkills / InitTradeSkills (Bot/Factory/PlayerbotFactory.cpp:3275 / 2933): riding,
    /// profession pair + secondary skills, gathering tools. Public like AC's: the level-up maintenance action calls InitSkills.
    void InitSkills();
    void InitTradeSkills();
    /// #534: AC PlayerbotFactory::InitFood / InitReagents / InitPotions / InitConsumables / CleanupConsumables
    /// (Bot/Factory/PlayerbotFactory.cpp:4164 / 4229 / 4084 / 1091 / 4351). Public like AC's: the level-up maintenance
    /// action (AutoMaintenanceOnLevelupAction::AutoUpgradeEquip, AC :160-177) calls them too.
    void InitFood();
    void InitReagents();
    void InitPotions();
    void InitConsumables();
    void CleanupConsumables();
    /// AC PlayerbotFactory::InitGuild (Bot/Factory/PlayerbotFactory.cpp:4883-4914): join or found a random-bot guild
    /// (PlayerbotGuildMgr) and carry the guild tabard. Public like AC's: RandomPlayerbotMgr::OnBotLoginInternal calls it.
    void InitGuild();
    /// AC PlayerbotFactory::ApplyEnchantAndGemsNew (Bot/Factory/PlayerbotFactory.cpp:5106): best-scoring enchant and gems for
    /// every equipped item. Public like AC's: the random-bot paths and the maintenance / autogear chat commands call it.
    void ApplyEnchantAndGemsNew(bool destroyOld = true);
    /// AiPlayerbot.MinEnchantingBotLevel (AC default 60): callers gate ApplyEnchantAndGemsNew on it, as AC does.
    static uint32 GetMinEnchantingBotLevel();
    /// AC PlayerbotFactory::DestroyEquippedGear / AutoGear (Bot/Factory/PlayerbotFactory.cpp:3917-3942): the gear steps
    /// the "autogear" and "autogear bis" chat commands drive. AutoGear's finishers (enchants and gems, repair) are on by default.
    static void DestroyEquippedGear(Player* bot);
    static void AutoGear(Player* bot, uint32 itemQuality, uint32 ilvl, bool incremental, bool secondChance = false,
                         bool applyFinishers = true);
private:
    std::vector<uint32> GetCurrentGemsCount();
    /// Builds enchantSpellIdCache / enchantGemIdCache once (called by Init() and lazily by ApplyEnchantAndGemsNew).
    static void EnsureEnchantCaches();
    static std::vector<uint32> enchantSpellIdCache;
    static std::vector<uint32> enchantGemIdCache;

    void Prepare();
    void CancelAuras();

    // -- GEARING
    bool CanEquipItem(ItemTemplate const* proto);
    bool CanEquipUnseenItem(uint8 slot, uint16& dest, uint32 item);
    std::vector<InventoryType> GetPossibleInventoryTypeListBySlot(EquipmentSlots slot);

    uint32 level;
    uint32 itemQuality;
    uint32 gearScoreLimit;
protected:
    Player* bot;
    PlayerbotAI* botAI;
};

#endif