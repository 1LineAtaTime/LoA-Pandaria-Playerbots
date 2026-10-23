#include "RandomItemManager.h"

#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotSpec.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

#include <bitset>
#include <map>
#include <tuple>

RandomItemManager::RandomItemManager()
{

}
RandomItemManager::~RandomItemManager()
{

}

// Cheat class copy to hack into the loot system
class LootTemplateAccess
{
public:
    class LootGroup;  // A set of loot definitions for items (refs are not allowed inside)
    typedef std::vector<LootGroup> LootGroups;
    LootStoreItemList Entries;  // not grouped only
    LootGroups Groups;          // groups have own (optimized) processing, grouped entries go there
};

std::vector<uint32> RandomItemManager::GetCachedEquipments(uint32 requiredLevel, uint32 inventoryType)
{
    return _equipCacheNew[requiredLevel][inventoryType];
}

std::vector<uint32> const& RandomItemManager::GetEquipmentNew(uint32 requiredLevel, uint32 inventoryType) const
{
    static std::vector<uint32> const empty;
    auto const lvlIt = _equipCacheNew.find(requiredLevel);
    if (lvlIt == _equipCacheNew.end())
        return empty;
    auto const typeIt = lvlIt->second.find(inventoryType);
    if (typeIt == lvlIt->second.end())
        return empty;
    return typeIt->second;
}

void RandomItemManager::Init()
{
    _viable_slots[EQUIPMENT_SLOT_HEAD].insert(INVTYPE_HEAD);
    _viable_slots[EQUIPMENT_SLOT_NECK].insert(INVTYPE_NECK);
    _viable_slots[EQUIPMENT_SLOT_SHOULDERS].insert(INVTYPE_SHOULDERS);
    _viable_slots[EQUIPMENT_SLOT_BODY].insert(INVTYPE_BODY);
    _viable_slots[EQUIPMENT_SLOT_CHEST].insert(INVTYPE_CHEST);
    _viable_slots[EQUIPMENT_SLOT_CHEST].insert(INVTYPE_ROBE);
    _viable_slots[EQUIPMENT_SLOT_WAIST].insert(INVTYPE_WAIST);
    _viable_slots[EQUIPMENT_SLOT_LEGS].insert(INVTYPE_LEGS);
    _viable_slots[EQUIPMENT_SLOT_FEET].insert(INVTYPE_FEET);
    _viable_slots[EQUIPMENT_SLOT_WRISTS].insert(INVTYPE_WRISTS);
    _viable_slots[EQUIPMENT_SLOT_HANDS].insert(INVTYPE_HANDS);
    _viable_slots[EQUIPMENT_SLOT_FINGER1].insert(INVTYPE_FINGER);
    _viable_slots[EQUIPMENT_SLOT_FINGER2].insert(INVTYPE_FINGER);
    _viable_slots[EQUIPMENT_SLOT_TRINKET1].insert(INVTYPE_TRINKET);
    _viable_slots[EQUIPMENT_SLOT_TRINKET2].insert(INVTYPE_TRINKET);
    _viable_slots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_WEAPON);
    _viable_slots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_2HWEAPON);
    _viable_slots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_WEAPONMAINHAND);
    _viable_slots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_RANGED);
    // #621, AC RandomItemMgr.cpp:1645-1647 maps INVTYPE_RANGEDRIGHT next to INVTYPE_RANGED. In 5.4.8 there is no ranged
    // slot: Player::FindEquipSlot (Player.cpp:10849) sends RANGED (15, bows) and RANGEDRIGHT (26, every gun and
    // crossbow) to the main hand, so a hunter that only knew type 15 could never be given a gun or crossbow.
    _viable_slots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_RANGEDRIGHT);
    _viable_slots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_WEAPON);
    _viable_slots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_2HWEAPON);
    _viable_slots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_SHIELD);
    // #622, AC RandomItemMgr.cpp:1641 addEquipmentSlot(INVTYPE_WEAPONOFFHAND, EQUIPMENT_SLOT_OFFHAND): this was
    // INVTYPE_WEAPONMAINHAND (a main-hand-only type that can never be equipped in the off hand), so every off-hand-only
    // weapon (type 22, e.g. the rogue off-hand daggers) failed ShouldEquipWeaponForSpec for both slots.
    _viable_slots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_WEAPONOFFHAND);
    _viable_slots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_HOLDABLE);
    _viable_slots[EQUIPMENT_SLOT_TABARD].insert(INVTYPE_TABARD);
    _viable_slots[EQUIPMENT_SLOT_BACK].insert(INVTYPE_CLOAK);

    TC_LOG_INFO("playerbots", "Loading equipments cache...");

    std::unordered_set<uint32> questItemIds;
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        uint32 questId = i->first;
        Quest const* quest = i->second;

        if (quest->IsRepeatable())
            continue;

        if (quest->GetQuestLevel() <= 0)
            continue;

        if (quest->GetAllowableClasses())
            continue;

        for (int j = 0; j < quest->GetRewChoiceItemsCount(); j++)
            if (uint32 itemId = quest->RewardChoiceItemId[j])
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->Class != ITEM_CLASS_WEAPON && proto->Class != ITEM_CLASS_ARMOR)
                    continue;
                int requiredLevel = std::max((int)proto->RequiredLevel, quest->GetQuestLevel());
                _equipCacheNew[requiredLevel][proto->InventoryType].push_back(itemId);
                questItemIds.insert(itemId);
            }

        for (int j = 0; j < quest->GetRewItemsCount(); j++)
            if (uint32 itemId = quest->RewardItemId[j])
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->Class != ITEM_CLASS_WEAPON && proto->Class != ITEM_CLASS_ARMOR)
                    continue;
                int requiredLevel = std::max((int)proto->RequiredLevel, quest->GetQuestLevel());
                _equipCacheNew[requiredLevel][proto->InventoryType].push_back(itemId);
                questItemIds.insert(itemId);
            }
    }

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    for (auto const& itr : *itemTemplates)
    {
        ItemTemplate const* proto = &itr.second;
        if (!proto)
            continue;
        uint32 itemId = proto->ItemId;

        if (questItemIds.find(itemId) != questItemIds.end())
            continue;

        // skip test items
        if (strstr(proto->Name1.c_str(), "(Test)") || strstr(proto->Name1.c_str(), "(TEST)") ||
            strstr(proto->Name1.c_str(), "(test)") || strstr(proto->Name1.c_str(), "(JEFFTEST)") ||
            strstr(proto->Name1.c_str(), "Test ") || strstr(proto->Name1.c_str(), "Test") ||
            strstr(proto->Name1.c_str(), "TEST") || strstr(proto->Name1.c_str(), "TEST ") ||
            strstr(proto->Name1.c_str(), " TEST") || strstr(proto->Name1.c_str(), "2200 ") ||
            strstr(proto->Name1.c_str(), "Deprecated ") || strstr(proto->Name1.c_str(), "Unused ") ||
            strstr(proto->Name1.c_str(), "Monster ") || strstr(proto->Name1.c_str(), "[PH]") ||
            strstr(proto->Name1.c_str(), "(OLD)") || strstr(proto->Name1.c_str(), "QR") ||
            strstr(proto->Name1.c_str(), "zzOLD"))
        {
            _itemForTest.insert(proto->ItemId);
            continue;
        }

        if (proto->Flags & ITEM_PROTO_FLAG_DEPRECATED)
        {
            _itemForTest.insert(proto->ItemId);
            continue;
        }

        if (itemId == 22784)
        {  // Sunwell Orb
            continue;
        }
        _equipCacheNew[proto->RequiredLevel][proto->InventoryType].push_back(itemId);
    }
}

// #534: AC RandomItemMgr::GetRandomPotion (Mgr/Item/RandomItemMgr.cpp:989-1003); the level is capped like AC's
// NormalizeLevel (:3040-3045, min(level, DEFAULT_MAX_LEVEL)).
uint32 RandomItemManager::GetRandomPotion(uint32 level, uint32 effect)
{
    std::call_once(_potionCacheOnce, [this] { BuildCachePotion(); });

    level = std::min<uint32>(level, DEFAULT_MAX_LEVEL);

    auto const levelItr = _potionCache.find(level);
    if (levelItr == _potionCache.end())
        return 0;

    auto const effectItr = levelItr->second.find(effect);
    if (effectItr == levelItr->second.end() || effectItr->second.empty())
        return 0;

    return effectItr->second[urand(0, effectItr->second.size() - 1)];
}

// #534: AC RandomItemMgr::BuildCachePotion (Mgr/Item/RandomItemMgr.cpp:2632-2705), with AC's IsValidItem (:1391-1441)
// and IsInternalItem (:1368-1389) inlined. Every consumable potion / flask (subclass 1 / 3) that is not bound, needs
// no profession, and whose use spell has exactly one effect - heal or energize - is cached for the levels from its
// RequiredLevel up to 13 levels above it (always from level 1 to 13). `unobtainableItems` (AC :1437) has no
// counterpart on this fork. 5.4.8 result, e.g. level 10: Minor / Lesser Healing Potion (118, 858) and Minor Mana
// Potion (2455); level 85-90: Master Healing Potion (76097) and Master Mana Potion (76098).
void RandomItemManager::BuildCachePotion()
{
    constexpr uint32 MAX_POTION_LEVEL_DELTA = 13;

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    for (auto const& itr : *itemTemplates)
    {
        ItemTemplate const* proto = &itr.second;

        // skip non-consumable items
        if (proto->Class != ITEM_CLASS_CONSUMABLE)
            continue;

        // skip non-potion and non-flask subclasses
        if (proto->SubClass != ITEM_SUBCLASS_POTION && proto->SubClass != ITEM_SUBCLASS_FLASK)
            continue;

        // skip bound items
        if (proto->Bonding != BIND_NONE)
            continue;

        // IsValidItem: no item level, temporary items, area / map / rank restricted, not for a playable class or race
        if (proto->ItemLevel == 0 || proto->Duration || proto->Area || proto->Map || proto->RequiredCityRank ||
            proto->RequiredHonorRank || (proto->AllowableClass & CLASSMASK_ALL_PLAYABLE) == 0 ||
            (proto->AllowableRace & RACEMASK_ALL_PLAYABLE) == 0)
            continue;

        // IsInternalItem: deprecated flag and the test / placeholder name patterns
        if (proto->Flags & ITEM_PROTO_FLAG_DEPRECATED)
            continue;

        char const* name = proto->Name1.c_str();
        if (strstr(name, "NPC ") || strstr(name, "Monster ") || strstr(name, "Test") || strstr(name, "Deprecated ") ||
            strstr(name, "[PH]") || strstr(name, "TEST") || strstr(name, "Unused ") || strstr(name, "zzOLD") ||
            strstr(name, "(test)") || strstr(name, "(OLD)") || strstr(name, "QR") || strstr(name, "2200 "))
            continue;

        // skip items requiring a profession skill
        if (proto->RequiredSkill)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[0].SpellId);
        if (!spellInfo)
            continue;

        // skip potions whose primary effect is not heal or energize
        if (spellInfo->Effects[EFFECT_0].Effect != SPELL_EFFECT_HEAL &&
            spellInfo->Effects[EFFECT_0].Effect != SPELL_EFFECT_ENERGIZE)
            continue;

        // do not accept potions / flasks with more than one spell effect, only the first one (EFFECT_0)
        bool hasOtherEffects = false;
        for (uint8 i = EFFECT_1; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellInfo->Effects[i].Effect != 0)
            {
                hasOtherEffects = true;
                break;
            }
        }

        if (hasOtherEffects)
            continue;

        uint32 requiredLevel = proto->RequiredLevel;
        uint32 effect = spellInfo->Effects[EFFECT_0].Effect;
        for (uint32 level = std::max(1u, requiredLevel); level <= DEFAULT_MAX_LEVEL; ++level)
        {
            // skip levels outside the potion relevance window
            if (level > MAX_POTION_LEVEL_DELTA && level - requiredLevel > MAX_POTION_LEVEL_DELTA)
                break;

            _potionCache[level][effect].push_back(itr.first);
        }
    }
}

void RandomItemManager::AddItemStats(uint32 mod, uint8& sp, uint8& ap, uint8& tank)
{
    switch (mod)
    {
        case ITEM_MOD_HEALTH:
        case ITEM_MOD_STAMINA:
        case ITEM_MOD_MANA:
        case ITEM_MOD_INTELLECT:
        case ITEM_MOD_SPIRIT:
            ++sp;
            break;
    }

    switch (mod)
    {
        case ITEM_MOD_AGILITY:
        case ITEM_MOD_STRENGTH:
        case ITEM_MOD_HEALTH:
        case ITEM_MOD_STAMINA:
            ++tank;
            break;
    }

    switch (mod)
    {
        case ITEM_MOD_HEALTH:
        case ITEM_MOD_STAMINA:
        case ITEM_MOD_AGILITY:
        case ITEM_MOD_STRENGTH:
            ++ap;
            break;
    }
}

bool RandomItemManager::CheckItemStats(Classes clazz, uint8 sp, uint8 ap, uint8 tank)
{
    switch (clazz)
    {
        case CLASS_PRIEST:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
            if (!sp || ap > sp || tank > sp)
                return false;
            break;
        case CLASS_PALADIN:
        case CLASS_WARRIOR:
            if ((!ap && !tank) || sp > ap || sp > tank)
                return false;
            break;
        case CLASS_HUNTER:
        case CLASS_ROGUE:
        case CLASS_MONK:
            if (!ap || sp > ap || sp > tank)
                return false;
            break;
    }

    return sp || ap || tank;
}

bool RandomItemManager::ShouldEquipArmorForSpec(uint32 level, Classes playerclass, Specializations spec, ItemTemplate const* proto)
{
    if (proto->InventoryType == INVTYPE_TABARD)
        return true;

    std::unordered_set<uint32> resultArmorSubClass;
    switch (playerclass)
    {
        case CLASS_WARRIOR:
        {
            if (proto->InventoryType == INVTYPE_HOLDABLE)
                return false;

            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_PLATE };
            if (spec != Specializations::SPEC_WARRIOR_PROTECTION)
                resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE };
            if (level < 40)
                resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE };
            break;
        }
        case CLASS_DEATH_KNIGHT:
        {
            if (proto->InventoryType == INVTYPE_HOLDABLE)
                return false;

            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_PLATE };
            break;
        }
        case CLASS_PALADIN:
        {
            if (spec == Specializations::SPEC_PALADIN_HOLY && proto->InventoryType == INVTYPE_HOLDABLE)
                return false;

            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_PLATE };
            if (spec != Specializations::SPEC_PALADIN_PROTECTION)
                resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE };
            if (level < 40)
                resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE };

            break;
        }
        case CLASS_HUNTER:
        {
            if (proto->InventoryType == INVTYPE_HOLDABLE)
                return false;

            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL };
            break;
        }
        case CLASS_ROGUE:
        {
            if (proto->InventoryType == INVTYPE_HOLDABLE)
                return false;
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_LEATHER };
            break;
        }
        case CLASS_SHAMAN:
        {
            if (spec == Specializations::SPEC_SHAMAN_ENHANCEMENT && proto->InventoryType == INVTYPE_HOLDABLE)
                return false;
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL };
            if (spec == Specializations::SPEC_SHAMAN_ENHANCEMENT && level > 40)
                resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_MAIL };

            break;
        }
        case CLASS_PRIEST:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
        {
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH };
            break;
        }
        case CLASS_DRUID:
        {
            if ((spec == Specializations::SPEC_DRUID_FERAL || spec == Specializations::SPEC_DRUID_GUARDIAN) &&
                proto->InventoryType == INVTYPE_HOLDABLE)
                return false;
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER };
            if (spec == Specializations::SPEC_DRUID_FERAL || spec == Specializations::SPEC_DRUID_GUARDIAN)
                resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_LEATHER };

            break;
        }
        case CLASS_MONK:
        {
            if (proto->InventoryType == INVTYPE_HOLDABLE && spec != Specializations::SPEC_MONK_MISTWEAVER)
                return false;
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_LEATHER };
            break;
        }
    }

    return (resultArmorSubClass.find(proto->SubClass) != resultArmorSubClass.end());
}

bool RandomItemManager::ShouldEquipWeaponForSpec(Classes playerclass, Specializations spec, ItemTemplate const* proto)
{
    EquipmentSlots slot_mh = EQUIPMENT_SLOT_START;
    EquipmentSlots slot_oh = EQUIPMENT_SLOT_START;
    for (auto i = _viable_slots.begin(); i != _viable_slots.end(); ++i)
    {
        std::set<InventoryType> slots = _viable_slots[(EquipmentSlots)i->first];
        if (slots.find((InventoryType)proto->InventoryType) != slots.end())
        {
            if (i->first == EQUIPMENT_SLOT_MAINHAND)
                slot_mh = i->first;
            if (i->first == EQUIPMENT_SLOT_OFFHAND)
                slot_oh = i->first;
        }
    }


    if (slot_mh == EQUIPMENT_SLOT_START && slot_oh == EQUIPMENT_SLOT_START)
        return false;

    std::unordered_set<uint32> mh_weapons;
    std::unordered_set<uint32> oh_weapons;
    // #626: only weapon subclasses belong in these sets - this function is reached only for ITEM_CLASS_WEAPON
    // (CanEquipWeapon). Shields and holdables are armor and are handled by CanEquipSubArmor. The old off-hand entries
    // ITEM_SUBCLASS_ARMOR_SHIELD (6) and ITEM_SUBCLASS_ARMOR_MISCELLANEOUS (0) matched ITEM_SUBCLASS_WEAPON_POLEARM (6)
    // and ITEM_SUBCLASS_WEAPON_AXE (0): a two-hand polearm (INVTYPE_2HWEAPON is viable for the off hand) passed for
    // protection warriors/paladins, went into the main hand and blocked the shield (Shield Slam etc. result=31).
    // AC StatsWeightCalculator::CalculateItemTypePenalty gives a tank's two-hander 0.5*0.1 weight (Mgr/Item/StatsWeightCalculator.cpp:631-643).

    switch (playerclass)
    {
        case CLASS_WARRIOR:
        {
            if (spec == Specializations::SPEC_WARRIOR_PROTECTION)
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE,
                              ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_FIST_WEAPON };
            }
            else
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_MACE2,
                              ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_POLEARM };
            }
            break;
        }
        case CLASS_PALADIN:
        {
            if (spec == Specializations::SPEC_PALADIN_PROTECTION)
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE };
            }
            else if (spec == Specializations::SPEC_PALADIN_HOLY)
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE };
            }
            else
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_MACE2,
                              ITEM_SUBCLASS_WEAPON_POLEARM };
            }
            break;
        }
        case CLASS_DEATH_KNIGHT:
        {
            // #619: this switch had no death knight case, so mh_weapons/oh_weapons stayed empty and every weapon
            // failed CanEquipWeapon - DK bots were left with empty main/off hand (193/200 on the test DB) and every
            // rune strike failed with EQUIPPED_ITEM_CLASS. Spec split per AC StatsWeightCalculator::CalculateItemTypePenalty
            // (Mgr/Item/StatsWeightCalculator.cpp:637,655): frost dual-wields one-handers, blood takes a two-hander;
            // unholy is a two-hand spec on 5.4.8 (AC leaves it neutral), and a bot without a spec gets a two-hander too.
            if (spec == Specializations::SPEC_DEATH_KNIGHT_FROST)
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE };
                oh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE };
            }
            else
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_MACE2,
                              ITEM_SUBCLASS_WEAPON_POLEARM };
            }
            break;
        }
        case CLASS_HUNTER:
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN };
            break;
        }
        case CLASS_ROGUE:
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER };
            oh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER };
            break;
        }
        case CLASS_PRIEST:
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_MACE };
            break;
        }
        case CLASS_SHAMAN:
        {
            if (spec == Specializations::SPEC_SHAMAN_RESTORATION)
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE,
                              ITEM_SUBCLASS_WEAPON_FIST_WEAPON };
            }
            else if (spec == Specializations::SPEC_SHAMAN_ENHANCEMENT)
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_DAGGER,
                              ITEM_SUBCLASS_WEAPON_AXE,   ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST_WEAPON };
            }
            else
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF };
            }
            break;
        }
        case CLASS_MAGE:
        case CLASS_WARLOCK:
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_SWORD };
            break;
        }
        case CLASS_DRUID:
        {
            if (spec == Specializations::SPEC_DRUID_GUARDIAN)
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_MACE2 };
            }
            else if (spec == Specializations::SPEC_DRUID_RESTORATION)
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_FIST_WEAPON,
                              ITEM_SUBCLASS_WEAPON_MACE };
            }
            else if (spec == Specializations::SPEC_DRUID_FERAL)
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_MACE2 };
            }
            else
            {
                mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF };
            }
            break;
        }
        case CLASS_MONK:
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_AXE , ITEM_SUBCLASS_WEAPON_SWORD , ITEM_SUBCLASS_WEAPON_MACE , ITEM_SUBCLASS_WEAPON_FIST_WEAPON };
            oh_weapons = { ITEM_SUBCLASS_WEAPON_AXE , ITEM_SUBCLASS_WEAPON_SWORD , ITEM_SUBCLASS_WEAPON_MACE , ITEM_SUBCLASS_WEAPON_FIST_WEAPON };
            break;
        }
    }

    if (slot_mh == EQUIPMENT_SLOT_MAINHAND && mh_weapons.find(proto->SubClass) != mh_weapons.end())
        return true;

    if (slot_oh == EQUIPMENT_SLOT_OFFHAND && oh_weapons.find(proto->SubClass) != oh_weapons.end())
        return true;

    return false;
}

/*
* stat_comb
* 108 armor / agi / stam / dodge / parry
* 105 armor / stam / stren / dodge / parry
* 104 armor / stam / dodge / expert / parry
* 96 armor / dodge / parry
* 77 armor / stren / agi / stam / parry
* 76 armor / stam / agi / parry
* 73 armor / stren / stam / parry
* 72 armor / stam / parry
* 65 armor / stren / parry
* 64 armor / parry / fire resi / heal.s / dodge / parry
* 58 armor / stam / intell / spirit / dodge
* 56 armor / starm / spirit / dodge
* 48 armor / spirit / dodge
* 46 armor / stam / intell / agi / hit / dodge
* 45 armor / agi / stren / stam / dodge / frost res
* 44 armor / stam / agi / dodge
* 42 armor / stam / intel / dodge / crit
* 41 armor / stren / stam / dodge / mastery
* 40 armor / stam / dodge / exprt
* 36 armor / agi / crit / dodge
* 33 armor / stren / dodge / crit
* 32 armor / dodge / health 
* 31 armor / stren / stam / intell / agi / spirit
* 28 armor / agi / stam / spirit / attack power
* 26 armor / intell / stam / spirit / crit
* 25 armor / stam / spirit / hit / pvp power / pvp resi / rage cost
* 24 armor / stam / spirit / spell power / haste
* 18 armor / spirit / intell / crit
* 16 armor / spirit / pvp power / pvp resi
* 14 armor / agi / intel / stam / hit
* 13 armor / stren / stam / agi / haste
* 12 armor / agi / stam / expert / crit / mastery
* 11 armor / stren / intell / stam 
* 10 armor / intell / stam / haste
* 9 armor / stren / stam / haste / mastery
* 8 armor / stam / crit / pvp resi / attack power / haste
* 6 armor / intell / agi / haste
* 5 armor / stren / agi / crit
* 4 armor / agi / hit / haste
* 3 armor / intell / stren (staff unique)
* 2 armor / intell / haste / 
* 1 armor / stren / mastery / crit / hit
* 0 any
*/

bool RandomItemManager::CanEquipItem(EquipmentSlots slot, ItemTemplate const* proto)
{
    if (proto->Duration & 0x80000000)
        return false;

    if (proto->Bonding == BIND_QUEST || proto->Bonding == BIND_ON_USE)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return true;

    std::set<InventoryType> slots = _viable_slots[slot];
    if (slots.find((InventoryType)proto->InventoryType) == slots.end())
        return false;

    return true;
}

bool RandomItemManager::CanEquipSubArmor(Player* bot, ItemTemplate const* proto)
{
    Classes clazz = (Classes)bot->GetClass();
    Specializations spec = bot->GetSpecialization();

    if (proto->InventoryType != InventoryType::INVTYPE_SHIELD && proto->InventoryType != InventoryType::INVTYPE_HOLDABLE)
        return true;

    switch (clazz)
    {
        case CLASS_HUNTER:
        case CLASS_ROGUE:
        case CLASS_DEATH_KNIGHT:
        {
            return false;
        }
        case CLASS_PRIEST:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
        {
            if (proto->SubClass == ITEM_SUBCLASS_ARMOR_MISCELLANEOUS)
                return true;
            return false;
        }
        case CLASS_WARRIOR:
        {
            if (spec == Specializations::SPEC_WARRIOR_PROTECTION && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
                return true;
            return false;
        }
        case CLASS_PALADIN:
        {
            if (spec == Specializations::SPEC_PALADIN_PROTECTION && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
                return true;
            if (spec == Specializations::SPEC_PALADIN_HOLY && proto->SubClass == ITEM_SUBCLASS_ARMOR_MISCELLANEOUS)
                return true;
            return false;
        }
        case CLASS_SHAMAN:
        {
            if ((spec == Specializations::SPEC_SHAMAN_ELEMENTAL || spec == Specializations::SPEC_SHAMAN_RESTORATION)
                && (proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD || proto->SubClass == ITEM_SUBCLASS_ARMOR_MISCELLANEOUS))
                return true;
            return false;
        }
        case CLASS_DRUID:
        {
            if ((spec == Specializations::SPEC_DRUID_BALANCE || spec == Specializations::SPEC_DRUID_FERAL)
                && proto->SubClass == ITEM_SUBCLASS_ARMOR_MISCELLANEOUS)
                return true;
            return false;
        }
        case CLASS_MONK:
        {
            if (spec == Specializations::SPEC_MONK_MISTWEAVER && proto->SubClass != ITEM_SUBCLASS_ARMOR_MISCELLANEOUS)
                return true;
            return false;
        }
    }

    return false;
}

bool RandomItemManager::CanEquipArmor(Player* bot, ItemTemplate const* proto)
{
    Classes clazz = (Classes)bot->GetClass();
    uint32 level = (uint32)bot->GetLevel();

    if (proto->InventoryType == INVTYPE_TABARD)
        return true;

    if (!ShouldEquipArmorForSpec(level, clazz, bot->GetSpecialization(), proto))
        return false;

    uint8 sp = 0, ap = 0, tank = 0;
    for (uint8 j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
    {
        // for ItemStatValue != 0
        if (!proto->ItemStat[j].ItemStatValue)
            continue;

        AddItemStats(proto->ItemStat[j].ItemStatType, sp, ap, tank);
    }

    return CheckItemStats(clazz, sp, ap, tank);
}

bool RandomItemManager::CanEquipWeapon(Player* bot, ItemTemplate const* proto)
{
    Classes clazz = (Classes)bot->GetClass();
    uint32 level = (uint32)bot->GetLevel();

    switch (clazz)
    {
        case CLASS_PRIEST:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER)
                return false;
            break;
        case CLASS_MAGE:
        case CLASS_WARLOCK:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
                return false;
            break;
        case CLASS_WARRIOR:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST_WEAPON &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
                return false;
            break;
        case CLASS_PALADIN:
        case CLASS_DEATH_KNIGHT:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
                return false;
            break;
        case CLASS_SHAMAN:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST_WEAPON && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
                return false;
            break;
        case CLASS_DRUID:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM)
                return false;
            break;
        case CLASS_HUNTER:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW)
                return false;
            break;
        case CLASS_ROGUE:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST_WEAPON && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE)
                return false;
            break;
        case CLASS_MONK:
        {
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST_WEAPON)
                return false;
            break;
        }
    }

    if (!ShouldEquipWeaponForSpec((Classes)bot->GetClass(), bot->GetSpecialization(), proto))
        return false;

    return true;
}

// Définition des seuils de qualité avec une valeur de repli
static const std::map<uint32, std::tuple<ItemQualities, ItemQualities, uint32, uint32>> qualityRules =
{
    {1,   std::make_tuple(ItemQualities::ITEM_QUALITY_NORMAL,   ItemQualities::ITEM_QUALITY_POOR,       20, 5)},
    {10,  std::make_tuple(ItemQualities::ITEM_QUALITY_UNCOMMON, ItemQualities::ITEM_QUALITY_NORMAL,     20, 5)},
    {35,  std::make_tuple(ItemQualities::ITEM_QUALITY_RARE,     ItemQualities::ITEM_QUALITY_UNCOMMON,   30, 10)},
    {60,  std::make_tuple(ItemQualities::ITEM_QUALITY_EPIC,     ItemQualities::ITEM_QUALITY_RARE,       40, 15)},
    {61,  std::make_tuple(ItemQualities::ITEM_QUALITY_UNCOMMON, ItemQualities::ITEM_QUALITY_NORMAL,     20, 5)},
    {65,  std::make_tuple(ItemQualities::ITEM_QUALITY_RARE,     ItemQualities::ITEM_QUALITY_UNCOMMON,   30, 10)},
    {70,  std::make_tuple(ItemQualities::ITEM_QUALITY_EPIC,     ItemQualities::ITEM_QUALITY_RARE,       40, 15)},
    {71,  std::make_tuple(ItemQualities::ITEM_QUALITY_UNCOMMON, ItemQualities::ITEM_QUALITY_NORMAL,     20, 5)},
    {75,  std::make_tuple(ItemQualities::ITEM_QUALITY_RARE,     ItemQualities::ITEM_QUALITY_UNCOMMON,   30, 10)},
    {80,  std::make_tuple(ItemQualities::ITEM_QUALITY_EPIC,     ItemQualities::ITEM_QUALITY_RARE,       40, 15)},
    {81,  std::make_tuple(ItemQualities::ITEM_QUALITY_UNCOMMON, ItemQualities::ITEM_QUALITY_NORMAL,     20, 5)},
    {83,  std::make_tuple(ItemQualities::ITEM_QUALITY_RARE,     ItemQualities::ITEM_QUALITY_UNCOMMON,   30, 10)},
    {85,  std::make_tuple(ItemQualities::ITEM_QUALITY_EPIC,     ItemQualities::ITEM_QUALITY_RARE,       40, 15)},
    {86,  std::make_tuple(ItemQualities::ITEM_QUALITY_UNCOMMON, ItemQualities::ITEM_QUALITY_NORMAL,     20, 5)},
    {88,  std::make_tuple(ItemQualities::ITEM_QUALITY_RARE,     ItemQualities::ITEM_QUALITY_UNCOMMON,   30, 10)},
    {90,  std::make_tuple(ItemQualities::ITEM_QUALITY_EPIC,     ItemQualities::ITEM_QUALITY_RARE,       40, 15)}    
};

// #626: the item checks FindBestItemForLevelAndEquip ran inline, moved here unchanged so BotFactory::InitEquipment can
// filter every cached item the same way (AC PlayerbotFactory::InitEquipment walks the whole list, :2417-2479).
bool RandomItemManager::IsEquipCandidate(Player* bot, ItemTemplate const* proto)
{
    if (!proto)
        return false;

    uint32 level = (uint32)bot->GetLevel();
    uint32 bot_class = bot->GetClass();

    if (IsTestItem(proto->ItemId)) return false;

    if (proto->RequiredLevel > level) return false;
    if ((proto->AllowableClass & bot->GetClassMask()) == 0 || (proto->AllowableRace & bot->GetRaceMask()) == 0) return false;
    if (level < 10 && proto->ItemLevel > 100) return false;

    if (proto->Class != ITEM_CLASS_WEAPON && proto->Class != ITEM_CLASS_ARMOR &&
        proto->Class != ITEM_CLASS_CONTAINER && proto->Class != ITEM_CLASS_PROJECTILE)
        return false;

    // #730, AC PlayerbotFactory::InitEquipment (Bot/Factory/PlayerbotFactory.cpp:2457-2466) filters the candidates with
    // CanEquipArmor (body-armor slots) and CanEquipWeapon only. AC's CanEquipArmor is the armor-type check alone (its
    // CheckItemStats call is commented out, :1950-1977) and CheckItemStats is used for enchants only (:2885). The stat
    // gate here (CheckItemStats inside CanEquipArmor and in the final else, which every weapon reached) rejected every
    // item without stats: no level 1-6 two-hander in world.item_template has one, so a randomized level-6 Retribution
    // paladin got no main hand (t537 w16b Omascon: ilvl avg 4.6 = 74/16, Crusader Strike result=31
    // EQUIPPED_ITEM_CLASS, melee FAILED x1661), and a level-1 bot got nothing at all (ilvl 0.0). The best candidate is
    // chosen by StatsWeightCalculator in BotFactory::InitEquipment (AC :2481-2518). CanEquipArmor keeps its stat check
    // for ItemUsageValue.
    if (proto->Class == ITEM_CLASS_ARMOR &&
        (proto->InventoryType == InventoryType::INVTYPE_HEAD || proto->InventoryType == InventoryType::INVTYPE_SHOULDERS ||
            proto->InventoryType == InventoryType::INVTYPE_CHEST || proto->InventoryType == InventoryType::INVTYPE_WAIST ||
            proto->InventoryType == InventoryType::INVTYPE_LEGS || proto->InventoryType == InventoryType::INVTYPE_FEET ||
            proto->InventoryType == InventoryType::INVTYPE_WRISTS || proto->InventoryType == InventoryType::INVTYPE_HANDS ||
            proto->InventoryType == InventoryType::INVTYPE_ROBE) &&
        !ShouldEquipArmorForSpec(level, (Classes)bot_class, bot->GetSpecialization(), proto))
        return false;

    if (proto->Class == ITEM_CLASS_ARMOR &&
        (proto->SubClass == ItemSubclassArmor::ITEM_SUBCLASS_ARMOR_MISCELLANEOUS ||
        proto->SubClass == ItemSubclassArmor::ITEM_SUBCLASS_ARMOR_SHIELD) &&
        !CanEquipSubArmor(bot, proto)) return false;

    if ((proto->Class == ITEM_CLASS_WEAPON) &&
        !CanEquipWeapon(bot, proto))
        return false;

    return true;
}

/*
* Know issues:
* Not always wear wearable (plate / mail etc)
* Hunt have incorrect weapon
* Random items stat not supposed to be assigned (ex shaman enhance with intellect spell power etc)
*/

uint32 RandomItemManager::FindBestItemForLevelAndEquip(Player* bot, InventoryType invType, int32 desiredQuality, uint32 gearScoreLimit)
{
    uint32 level = (uint32)bot->GetLevel();
    int32 delta = std::min(level, 10u);

    for (uint32 requiredLevel = bot->GetLevel(); requiredLevel > std::max((int32)bot->GetLevel() - delta, 0); requiredLevel--)
    {
        std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
        // #620: never mutate the shared cache from map threads - look it up read-only (no operator[] insert)
        // and shuffle a local copy.
        auto const lvlIt = _equipCacheNew.find(requiredLevel);
        if (lvlIt == _equipCacheNew.end())
            continue;
        auto const typeIt = lvlIt->second.find(invType);
        if (typeIt == lvlIt->second.end())
            continue;
        std::vector<uint32> items = typeIt->second;
        std::shuffle(items.begin(), items.end(), gen);

        for (auto it = items.end(); it != items.begin(); )
        {
            --it;

            uint32 itemID = *it;

            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemID);
            // #626: test item, level, class/race and the spec armor/weapon/shield/stat checks, shared with
            // BotFactory::InitEquipment.
            if (!IsEquipCandidate(bot, proto)) continue;

            // #629, AC PlayerbotFactory.cpp:2446-2454: gear-score cap (ilvl * quality multiplier) and the exact requested quality.
            if (desiredQuality >= 0)
            {
                if (gearScoreLimit != 0 &&
                    uint32(proto->ItemLevel * PlayerbotAI::GetItemScoreMultiplier(ItemQualities(proto->Quality))) > gearScoreLimit)
                    continue;
                if (proto->Quality != uint32(desiredQuality))
                    continue;
            }
            // Item Quality (legacy per-level table, only when no quality was requested)
            else
            {
                auto quality_iterator = qualityRules.upper_bound(bot->GetLevel());
                if (quality_iterator != qualityRules.begin()) --quality_iterator; // Récupérer la tranche correcte
                // Extraire les valeurs correspondantes
                ItemQualities minQuality = std::get<0>(quality_iterator->second);
                ItemQualities fallbackQuality = std::get<1>(quality_iterator->second); // Qualité inférieure en cas d'absence
                uint32 rareChance = std::get<2>(quality_iterator->second);
                uint32 uncommonChance = std::get<3>(quality_iterator->second);
                // Vérification de la qualité de l'objet
                if (proto->Quality >= minQuality) {
                    // Accepter immédiatement l'objet
                }
                else if (proto->Quality >= fallbackQuality) {
                    // Accepter un objet de qualité inférieure en dernier recours
                }
                else if (proto->Quality == ItemQualities::ITEM_QUALITY_RARE && urand(1, 100) <= rareChance) {
                    // Accepter avec un % pour Rare
                }
                else if (proto->Quality == ItemQualities::ITEM_QUALITY_UNCOMMON && urand(1, 100) <= uncommonChance) {
                    // Accepter avec un % pour Uncommon
                }
                else {
                    continue; // Sinon, ignorer l'objet
                }
            }

            // ok ?
            return proto->ItemId;
        }
    }
    return 0;
}