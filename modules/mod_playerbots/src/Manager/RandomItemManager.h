#ifndef _PLAYERBOT_RANDOMITEMMGR_H
#define _PLAYERBOT_RANDOMITEMMGR_H

#include <map>
#include <mutex>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AiFactory.h"
#include "Common.h"
#include "Item.h"
#include "Player.h"

enum item_sub_class : uint32
{
    PLATE = 0,
    MAIL,
    LEATHER,
    CLOTH
};

class RandomItemManager
{
public:
    RandomItemManager();
    void Init();
    virtual ~RandomItemManager();
    static RandomItemManager* instance()
    {
        static RandomItemManager instance;
        return &instance;
    }

    // desiredQuality >= 0: AC PlayerbotFactory::InitEquipment (Bot/Factory/PlayerbotFactory.cpp:2413-2454) - only items of exactly that
    // quality, and none above gearScoreLimit (ilvl * quality multiplier) when it is non-zero. -1 keeps the old per-level qualityRules.
    uint32 FindBestItemForLevelAndEquip(Player* bot, InventoryType invType, int32 desiredQuality = -1, uint32 gearScoreLimit = 0);
    bool IsTestItem(uint32 itemId) { return _itemForTest.find(itemId) != _itemForTest.end(); }

    std::vector<uint32> GetCachedEquipments(uint32 requiredLevel, uint32 inventoryType);
    // #626: AC RandomItemMgr::GetEquipmentNew - the cached list for one required level and inventory type, read-only
    // (find(), never operator[]: map threads share the cache, see #620). Empty when there is none.
    std::vector<uint32> const& GetEquipmentNew(uint32 requiredLevel, uint32 inventoryType) const;
    // #626: the per-item checks of FindBestItemForLevelAndEquip (test item, level, class/race, spec armor/weapon/shield,
    // item stats) without its level and quality loop, so BotFactory::InitEquipment can walk every cached item like
    // AC PlayerbotFactory::InitEquipment (Bot/Factory/PlayerbotFactory.cpp:2417-2479).
    bool IsEquipCandidate(Player* bot, ItemTemplate const* proto);

    // #534: AC RandomItemMgr::GetRandomPotion (Mgr/Item/RandomItemMgr.cpp:989-1003) - a random cached healing
    // (SPELL_EFFECT_HEAL) or mana (SPELL_EFFECT_ENERGIZE) potion for a level; 0 when there is none. The cache
    // (AC BuildCachePotion :2632-2705) is built on the first call, not in Init(): it reads SpellInfo, which is not
    // loaded yet when OnConfigLoad runs Init().
    uint32 GetRandomPotion(uint32 level, uint32 effect);

    bool CanEquipArmor(Player* bot, ItemTemplate const* proto);
    bool CanEquipWeapon(Player* bot, ItemTemplate const* proto);
    bool CanEquipSubArmor(Player* bot, ItemTemplate const* proto);
private:
    void AddItemStats(uint32 mod, uint8& sp, uint8& ap, uint8& tank);
    bool CanEquipItem(EquipmentSlots slot, ItemTemplate const* proto);
    
    bool CheckItemStats(Classes clazz, uint8 sp, uint8 ap, uint8 tank);
    bool ShouldEquipArmorForSpec(uint32 level, Classes playerclass, Specializations spec, ItemTemplate const* proto);
    bool ShouldEquipWeaponForSpec(Classes playerclass, Specializations spec, ItemTemplate const* proto);
    void BuildCachePotion();
private:
    std::map<uint32, std::map<uint32, std::vector<uint32>>> _equipCacheNew;
    std::map<EquipmentSlots, std::set<InventoryType>> _viable_slots;
    std::unordered_set<uint32> _itemForTest;
    // #534: AC RandomItemMgr::potionCache, level -> spell effect -> item ids.
    std::unordered_map<uint32, std::unordered_map<uint32, std::vector<uint32>>> _potionCache;
    std::once_flag _potionCacheOnce;
};

#define sRandomItemMgr RandomItemManager::instance()

#endif
