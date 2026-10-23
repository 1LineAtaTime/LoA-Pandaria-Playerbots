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
private:
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