#include "BotFactory.h"

#include <algorithm>
#include <map>
#include <mutex>
#include <random>
#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>
 
#include "AccountMgr.h"
#include "AiFactory.h"
#include "Bag.h"
#include "Config.h"
#include "ArenaTeam.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "GuildMgr.h"
#include "Helper.h"
#include "Item.h"
#include "ItemVisitors.h"
#include "Log.h"
#include "LogCommon.h"
#include "LootMgr.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "PetDefines.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotRepository.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "RandomPlayerbotFactory.h"
#include "RandomPlayerbotMgr.h"
#include "RandomItemManager.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "StatsWeightCalculator.h"
#include "World.h"
  
// #629: AC PlayerbotFactory::CalcMixedGearScore (Bot/Factory/PlayerbotFactory.cpp:3912-3915).
uint32 BotFactory::CalcMixedGearScore(uint32 gs, uint32 quality)
{
    return gs * PlayerbotAI::GetItemScoreMultiplier(ItemQualities(quality));
}

// #629: AC PlayerbotFactory::PlayerbotFactory (Bot/Factory/PlayerbotFactory.cpp:546-558) keeps the requested quality and
// gear-score limit; with no quality given it takes AiPlayerbot.RandomGearQualityLimit / RandomGearScoreLimit.
// The fork's PlayerbotAIConfig has no randomGear* members yet, so the keys are read straight from the config (same names
// and defaults as AC PlayerbotAIConfig.cpp:173-175); switch to the members once Utils/PlayerbotAIConfig.* declares them.
BotFactory::BotFactory(Player* bot, uint32 level, uint32 itemQuality, uint32 gearScoreLimit)
    : level(level), itemQuality(itemQuality), gearScoreLimit(gearScoreLimit), bot(bot)
{
    botAI = GET_PLAYERBOT_AI(bot);
    if (!this->itemQuality)
    {
        uint32 const qualityLimit = uint32(sConfigMgr->GetIntDefault("AiPlayerbot.RandomGearQualityLimit", 3));
        uint32 const scoreLimit = uint32(sConfigMgr->GetIntDefault("AiPlayerbot.RandomGearScoreLimit", 0));
        this->itemQuality = qualityLimit;
        this->gearScoreLimit = scoreLimit == 0 ? 0 : CalcMixedGearScore(scoreLimit, qualityLimit);
    }
}

void BotFactory::CancelAuras() { bot->RemoveAllAuras(); }
 
void BotFactory::Init()
{
    /*for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotQuestIds.begin();
        i != sPlayerbotAIConfig->randomBotQuestIds.end(); ++i)
    {
        uint32 questId = *i;
        AddPrevQuests(questId, specialQuestIds);
        specialQuestIds.remove(questId);
        specialQuestIds.push_back(questId);
    }
    uint32 maxStoreSize = sSpellMgr->GetSpellInfoStoreSize();
    for (uint32 id = 1; id < maxStoreSize; ++id)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(id);
        if (!spellInfo)
            continue;
 
        if (id == 47181 || id == 50358 || id == 47242 || id == 52639 || id == 47147 || id == 7218)  // Test Enchant
            continue;
 
        uint32 requiredLevel = spellInfo->BaseLevel;
 
        for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            if (spellInfo->Effects[j].Effect != SPELL_EFFECT_ENCHANT_ITEM)
                continue;
 
            uint32 enchant_id = spellInfo->Effects[j].MiscValue;
            if (!enchant_id)
                continue;
 
            SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if (!enchant || (enchant->Flags != PERM_ENCHANTMENT_SLOT && enchant->slot != TEMP_ENCHANTMENT_SLOT))
                continue;
 
            // SpellInfo const* enchantSpell = sSpellMgr->GetSpellInfo(enchant->spellid[0]);
            // if (!enchantSpell)
            //     continue;
            if (strstr(spellInfo->SpellName[0], "Test"))
                break;
 
            enchantSpellIdCache.push_back(id);
            break;
            // TC_LOG_INFO("playerbots", "Add {} to enchantment spells", id);
        }
    }
    TC_LOG_INFO("playerbots", "Loading {} enchantment spells", enchantSpellIdCache.size());
    for (auto iter = sSpellItemEnchantmentStore.begin(); iter != sSpellItemEnchantmentStore.end(); iter++)
    {
        uint32 gemId = iter->GemID;
        if (gemId == 0)
        {
            continue;
        }
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(gemId);
 
        if (proto->ItemLevel < 60)
            continue;
 
        if (proto->Flags & ITEM_FLAG_UNIQUE_EQUIPPABLE)
        {
            continue;
        }
        if (sRandomItemMgr->IsTestItem(gemId))
            continue;
 
        if (!proto || !sGemPropertiesStore.LookupEntry(proto->GemProperties))
        {
            continue;
        }
        // TC_LOG_INFO("playerbots", "Add {} to enchantment gems", gemId);
        enchantGemIdCache.push_back(gemId);
    }
    TC_LOG_INFO("playerbots", "Loading {} enchantment gems", enchantGemIdCache.size());*/
}
 
// #521: AC PlayerbotFactory::Randomize (Bot/Factory/PlayerbotFactory.cpp:808-809 ClearSkills/ClearSpells, :824
// CancelAuras, :853-859 LearnDefaultSkills/InitClassSpells/InitAvailableSpells) strips every aura and then
// re-learns the whole spellbook, and each re-learn re-casts the spell's passive (AC Player::_addSpell,
// Player.cpp:3326-3330). This fork keeps the spellbook (Prepare has no ClearSpells), so InitClassSpells and
// InitTalentsTree re-learn known spells as no-ops (Player::AddSpell returns early for a known spell in the same
// state, Player.cpp:3624) and every passive CancelAuras removed stayed off until the next login.
// MoP-only effect: Sinister Strike 1752 awards 0 combo points in 5.4.8 (SpellEffect 536, ADD_COMBO_POINTS base 0);
// the +1 comes from the hidden rogue passive 79327 "Sinister Strike Enabler" (aura 107 ADD_FLAT_MODIFIER EFFECT2 +1,
// skill 921), so a randomized combat rogue never reached 5 combo points and never cast Rupture/Eviscerate
// (t521 w15: hasspell 79327 spell=1 aura=0). Re-cast the known passives with the gate the core uses at learn time,
// which is what AC's re-learn does.
// #813: AC PlayerbotFactory::ResetQuests (Bot/Factory/PlayerbotFactory.cpp:1648-1666), 1:1 - a full randomize clears the
// quest log and the rewarded set, so a re-levelled bot does not keep its old (e.g. level-90 Pandaria) log and drop it in bursts.
// #745: AC PlayerbotFactory::ClearSkills (trade-skill half), defined with the trade-skill port below InitMountsAC.
namespace
{
void ClearTradeSkills(Player* bot);
}

static void ResetQuests(Player* bot)
{
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        bot->SetQuestSlot(slot, 0);
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        uint32 const entry = i->first;
        if (bot->GetQuestStatus(entry) == QUEST_STATUS_NONE)
            continue;
        bot->RemoveRewardedQuest(entry);
        bot->RemoveActiveQuest(entry, false);
    }
}

static void ReapplyKnownPassives(Player* bot)
{
    std::vector<uint32> passives;
    for (auto const& entry : bot->GetSpellMap())
    {
        PlayerSpell const* known = entry.second;
        if (!known || known->state == PLAYERSPELL_REMOVED || known->state == PLAYERSPELL_TEMPORARY ||
            !known->active || known->disabled)
            continue;
        SpellInfo const* info = sSpellMgr->GetSpellInfo(entry.first);
        if (info && info->IsPassive() && bot->IsNeedCastPassiveSpellAtLearn(info))
            passives.push_back(entry.first);
    }

    for (uint32 spellId : passives)
        if (!bot->HasAura(spellId))
            bot->CastSpell(bot, spellId, true);
}

// #707: AiPlayerbot.RandomBotQuestItems (AC PlayerbotAIConfig.cpp:207-210, AC IsInRandomQuestItemList :1102-1105) is not
// on this fork's PlayerbotAIConfig. Read once, locally, with the identical AC key and default (same pattern as
// LootAction.cpp's FreeMethodLoot reader) so no widely included header changes.
static std::set<uint32> const& RandomBotQuestItems()
{
    static std::set<uint32> const items = []
    {
        std::set<uint32> ids;
        std::stringstream ss(sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotQuestItems",
                                                          "5175,5176,5177,5178,6948,11000,12382,13704,16309"));
        std::string token;
        while (std::getline(ss, token, ','))
            if (uint32 id = uint32(atoi(token.c_str())))
                ids.insert(id);
        return ids;
    }();
    return items;
}

// #707: AC PlayerbotFactory::ClearInventory (Bot/Factory/PlayerbotFactory.cpp:3856-3860) = DestroyItemsVisitor
// (:1894-1924) over IterateItemsInBags (:5392-5410): destroy every item in the backpack and the equipped bags, keeping
// one stack of each AiPlayerbot.RandomBotQuestItems id. Equipped gear and the bags themselves are not visited.
// 5.4.8: no keyring (removed in Cataclysm), so AC's KEYRING_SLOT loop has nothing to walk (as InventoryAction.cpp).
// Without it a MoP random bot never emptied its bags: 389 of 2198 herbalists (108 of 373 online) sat at 0 free slots
// on 2026-10-08, AC 0 of 645, and a full bag leaves every new herb unstorable (StoreLootAction -> INV_FULL).
static void ClearInventory(Player* bot)
{
    std::set<uint32> keep;
    auto visit = [&](Item* item)
    {
        uint32 const id = item->GetTemplate()->ItemId;
        if (keep.find(id) == keep.end() && RandomBotQuestItems().count(id))
        {
            keep.insert(id);
            return;
        }

        bot->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
    };

    for (uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            visit(pItem);

    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* pBag = bot->GetBagByPos(i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    visit(pItem);
}

// #707: AC PlayerbotFactory::StoreItem (Bot/Factory/PlayerbotFactory.cpp:4789-4797).
static Item* StoreItem(Player* bot, uint32 itemId, uint32 count)
{
    ItemPosCountVec sDest;
    InventoryResult msg = bot->CanStoreNewItem(INVENTORY_SLOT_BAG_0, NULL_SLOT, sDest, itemId, count);
    if (msg != EQUIP_ERR_OK)
        return nullptr;

    return bot->StoreNewItem(sDest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
}

// #707: AC PlayerbotFactory::InitInventorySkill (Bot/Factory/PlayerbotFactory.cpp:4770-4787), reached through AC
// InitSkills (:3407-3408) on every Randomize (:853) and Refresh (:1068). The tool ids are AC's and all exist in the
// 5.4.8 item_template. LootObject::IsLootPossible (LootObjectStack.cpp, AC :334-346) refuses a mining node without one
// of AC's picks and a skinning corpse without one of its knives; 2193 of 2198 MoP miners had none on 2026-10-08.
static void InitInventorySkill(Player* bot)
{
    if (bot->HasSkill(SKILL_MINING) && !bot->HasItemCount(2901, 1, true))
        StoreItem(bot, 2901, 1);  // Mining Pick

    if ((bot->HasSkill(SKILL_BLACKSMITHING) || bot->HasSkill(SKILL_ENGINEERING)) && !bot->HasItemCount(5956, 1, true))
        StoreItem(bot, 5956, 1);  // Blacksmith Hammer

    if (bot->HasSkill(SKILL_ENGINEERING) && !bot->HasItemCount(6219, 1, true))
        StoreItem(bot, 6219, 1);  // Arclight Spanner

    if (bot->HasSkill(SKILL_ENCHANTING) && !bot->HasItemCount(16207, 1, true))
        StoreItem(bot, 16207, 1);  // Runed Arcanite Rod

    if (bot->HasSkill(SKILL_SKINNING) && !bot->HasItemCount(7005, 1, true))
        StoreItem(bot, 7005, 1);  // Skinning Knife
}

namespace
{
// #534: AC Bot/PlayerbotAI.h:169-230 (RoguePoisonId, SharpeningStoneId, WeightstoneId, WizardOilId, ManaOilId). AC keeps
// them in PlayerbotAI.h; they are declared here, in this TU only, so no widely included header changes (ImbueAction.cpp
// holds its own copy of the stone/oil ids, for the same reason). Every id exists in the 5.4.8 item_template; the poison
// ids are there as "Empty Poison Vial" rows (rogue poisons are self-cast spells in 5.4.8).
enum BotConsumableId : uint32
{
    INSTANT_POISON = 6947,
    INSTANT_POISON_II = 6949,
    INSTANT_POISON_III = 6950,
    INSTANT_POISON_IV = 8926,
    INSTANT_POISON_V = 8927,
    INSTANT_POISON_VI = 8928,
    INSTANT_POISON_VII = 21927,
    INSTANT_POISON_VIII = 43230,
    INSTANT_POISON_IX = 43231,
    DEADLY_POISON = 2892,
    DEADLY_POISON_II = 2893,
    DEADLY_POISON_III = 8984,
    DEADLY_POISON_IV = 8985,
    DEADLY_POISON_V = 20844,
    DEADLY_POISON_VI = 22053,
    DEADLY_POISON_VII = 22054,
    DEADLY_POISON_VIII = 43232,
    DEADLY_POISON_IX = 43233,

    ROUGH_SHARPENING_STONE = 2862,
    COARSE_SHARPENING_STONE = 2863,
    HEAVY_SHARPENING_STONE = 2871,
    SOLID_SHARPENING_STONE = 7964,
    DENSE_SHARPENING_STONE = 12404,
    FEL_SHARPENING_STONE = 23528,
    ADAMANTITE_SHARPENING_STONE = 23529,

    ROUGH_WEIGHTSTONE = 3239,
    COARSE_WEIGHTSTONE = 3240,
    HEAVY_WEIGHTSTONE = 3241,
    SOLID_WEIGHTSTONE = 7965,
    DENSE_WEIGHTSTONE = 12643,
    FEL_WEIGHTSTONE = 28420,
    ADAMANTITE_WEIGHTSTONE = 28421,

    MINOR_WIZARD_OIL = 20744,
    LESSER_WIZARD_OIL = 20746,
    WIZARD_OIL = 20750,
    BRILLIANT_WIZARD_OIL = 20749,
    SUPERIOR_WIZARD_OIL = 22522,

    MINOR_MANA_OIL = 20745,
    LESSER_MANA_OIL = 20747,
    BRILLIANT_MANA_OIL = 20748,
    SUPERIOR_MANA_OIL = 22521
};

// #534: AC botAI->HasCheat(BotCheatMask::food) (AC Bot/PlayerbotAI.h:565-569) = the bot's own cheat mask OR the configured
// one (AiPlayerbot.BotCheats, AC PlayerbotAIConfig.cpp:545-563, default "food,taxi,raid"). This fork has no per-bot cheat
// command, so the configured half is the whole check; same reader as NonCombatActions.cpp / UseFoodStrategy.cpp.
bool HasFoodCheat()
{
    static bool const value = []
    {
        std::stringstream ss(sConfigMgr->GetStringDefault("AiPlayerbot.BotCheats", "food,taxi,raid"));
        std::string token;
        while (std::getline(ss, token, ','))
        {
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            if (token == "food")
                return true;
        }
        return false;
    }();
    return value;
}

// #534: AC PlayerbotFactory::IterateItems (Bot/Factory/PlayerbotFactory.cpp:5380-5390), bags + equipment. AC's keyring
// loop (:5398-5401) has nothing to walk on 5.4.8 (keyring removed in Cataclysm), as in InventoryAction.cpp.
void IterateBotItems(Player* bot, IterateItemsVisitor* visitor, uint32 mask = ITERATE_ITEMS_IN_BAGS)
{
    if (mask & ITERATE_ITEMS_IN_BAGS)
    {
        for (uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
            if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                if (!visitor->Visit(pItem))
                    return;

        for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
            if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                    if (Item* pItem = pBag->GetItemByPos(j))
                        if (!visitor->Visit(pItem))
                            return;
    }

    if (mask & ITERATE_ITEMS_IN_EQUIP)
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
            if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                if (!visitor->Visit(pItem))
                    return;
}

// #534: AC StoreNewItemInInventorySlot (Bot/Factory/PlayerbotFactory.cpp:2677-2688).
Item* StoreNewItemInInventorySlot(Player* player, uint32 newItemId, uint32 count)
{
    ItemPosCountVec vDest;
    InventoryResult msg = player->CanStoreNewItem(INVENTORY_SLOT_BAG_0, NULL_SLOT, vDest, newItemId, count);
    if (msg == EQUIP_ERR_OK)
        if (Item* newItem = player->StoreNewItem(vDest, newItemId, true, Item::GenerateItemRandomPropertyId(newItemId)))
            return newItem;

    return nullptr;
}

// #534: AC InitConsumables' per-class "newest item the bot's level allows" pick, written once instead of AC's eight
// copies of the same loop (AC PlayerbotFactory.cpp:1091-1303). `maxLevel` is AC's `level > 75` cut-off. The first id of
// the list whose RequiredLevel fits is the one stored.
void AddBestConsumable(std::vector<std::pair<uint32, uint32>>& items, std::vector<uint32> const& ids, uint32 level,
                       uint32 count, uint32 maxLevel = 75)
{
    for (uint32 itemId : ids)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto || proto->RequiredLevel > level || (maxLevel && level > maxLevel))
            continue;

        items.push_back({itemId, count});
        break;
    }
}

// #534: AC InitFood candidate filter (Bot/Factory/PlayerbotFactory.cpp:4172-4192), minus the two per-bot conditions
// (level window and RequiredSkill), which are applied when the cache is read. Built once: AC walks the whole item
// template store for every bot on every refresh, and the result does not depend on the bot.
struct FoodCandidate
{
    uint32 itemId;
    uint32 category;  // AC SpellCategory 11 = food, 59 = drink
    uint32 requiredLevel;
    uint32 requiredSkill;
};

std::vector<FoodCandidate> const& FoodCandidates()
{
    static std::vector<FoodCandidate> const candidates = []
    {
        std::vector<FoodCandidate> list;
        ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
        for (auto const& itr : *itemTemplates)
        {
            ItemTemplate const* proto = &itr.second;
            // AC :4176-4180 tests ITEM_SUBCLASS_FOOD; 5.4.8 calls the same subclass (5) ITEM_SUBCLASS_FOOD_DRINK.
            if (proto->Class != ITEM_CLASS_CONSUMABLE || proto->SubClass != ITEM_SUBCLASS_FOOD_DRINK ||
                (proto->Spells[0].SpellCategory != 11 && proto->Spells[0].SpellCategory != 59) ||
                proto->Bonding != BIND_NONE)
                continue;

            if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
                continue;

            // MoP-only (not in AC): drop the deprecated / test template rows ("Deprecated Creeper Cakes", ...) that the
            // 5.4.8 item_template keeps in the same pool; RandomItemManager's list is built from AC's name patterns.
            if (sRandomItemMgr->IsTestItem(proto->ItemId))
                continue;

            // MoP-only (not in AC): the 5.4.8 item_template puts the placeable banquets / feasts (use spell =
            // SPELL_EFFECT_TRANS_DOOR + script) and the Noodle Cart kits (SPELL_EFFECT_DUMMY) in the same
            // category-11 pool as plain food (29 of the 706 food/drink templates). A bot that "eats" one only deploys an
            // object and never heals. AC's WotLK feasts (Fish Feast, Great Feast, ...) are the same trap.
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[0].SpellId);
            if (!spellInfo)
                continue;

            bool placeable = false;
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                switch (spellInfo->Effects[i].Effect)
                {
                    case SPELL_EFFECT_TRANS_DOOR:
                    case SPELL_EFFECT_SUMMON:
                    case SPELL_EFFECT_SUMMON_OBJECT_WILD:
                    case SPELL_EFFECT_SUMMON_OBJECT_SLOT1:
                    case SPELL_EFFECT_SUMMON_OBJECT_SLOT2:
                    case SPELL_EFFECT_SUMMON_OBJECT_SLOT3:
                    case SPELL_EFFECT_SUMMON_OBJECT_SLOT4:
                    case SPELL_EFFECT_SCRIPT_EFFECT:
                    case SPELL_EFFECT_DUMMY:
                        placeable = true;
                        break;
                    default:
                        break;
                }
            }
            if (placeable)
                continue;

            list.push_back({proto->ItemId, proto->Spells[0].SpellCategory, proto->RequiredLevel, proto->RequiredSkill});
        }
        return list;
    }();
    return candidates;
}
}  // namespace

// #534: AC PlayerbotFactory::InitFood (Bot/Factory/PlayerbotFactory.cpp:4164-4227). Two random stacks of food (spell
// category 11) and two of drink (59) from the items whose RequiredLevel is within [level - 9, level]; skipped with the
// food cheat (the default), where Eat/Drink apply the refill aura instead. Level 10: Smoked Sagefish (21072) / Refreshing
// Spring Water (159); level 40: Tender Wolf Steak (12216) etc.; level 85: Highland Sheep Cheese (58259) / Highland Spring
// Water (58257); level 90: Black Pepper Ribs and Shrimp (74646) etc. (drink: the level 85 waters and brews, 81-90 window).
void BotFactory::InitFood()
{
    if (HasFoodCheat())
        return;

    std::unordered_map<uint32, std::vector<uint32>> items;
    for (FoodCandidate const& candidate : FoodCandidates())
    {
        if (candidate.requiredLevel > bot->GetLevel() ||
            static_cast<int32>(candidate.requiredLevel) < static_cast<int32>(bot->GetLevel()) - 9)
            continue;

        if (candidate.requiredSkill && !bot->HasSkill(candidate.requiredSkill))
            continue;

        items[candidate.category].push_back(candidate.itemId);
    }

    uint32 categories[] = {11, 59};
    for (size_t i = 0; i < sizeof(categories) / sizeof(uint32); ++i)
    {
        uint32 category = categories[i];
        std::vector<uint32>& ids = items[category];
        int tries = 0;
        for (int j = 0; j < 2; j++)
        {
            uint32 index = urand(0, ids.size() - 1);
            if (index >= ids.size())
                continue;

            uint32 itemId = ids[index];
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            // beer / wine ...
            if (proto->Spells[0].SpellId == 11007 || proto->Spells[0].SpellId == 11008 ||
                proto->Spells[0].SpellId == 11009 || proto->Spells[0].SpellId == 11629 ||
                proto->Spells[0].SpellId == 50986)
            {
                tries++;
                if (tries > 5)
                {
                    continue;
                }
                j--;
                continue;
            }
            StoreItem(bot, itemId, proto->GetMaxStackSize());
        }
    }
}

// #534: AC PlayerbotFactory::InitReagents (Bot/Factory/PlayerbotFactory.cpp:4229-4349). AC's table is: death knight
// Corpse Dust (37201) from 56; druid seeds / Wild Berries / Wild Thornroot / Quillvine / Spineleaf (17034-17038, 17021,
// 17026, 22147, 22148, 44614, 44605) from 20; mage Rune of Teleportation / Portals / Arcane Powder (17031, 17032,
// 17020); paladin Symbol of Kings (21177) from 52; priest Holy / Sacred / Devout Candle (17028, 17029, 44615); shaman
// Earth / Flame / Water / Air Totem (5175-5178) and Ankh (17030); warlock Soul Shard (6265).
// 5.4.8 has none of it: Wowpedia "Reagent" (MediaWiki API, fetched 2026-10-08), "Patch 5.0.4 (2012-08-28): Remaining
// spell reagents have been removed" (4.0.1 had already dropped most of the above). Confirmed against this server's
// client data: no spell in a class skill line (SkillLine category 7) has a SpellReagents.db2 row in the 5.4.8
// set, so no class spell consumes any of those items and handing them out would only fill bags. The structure (per-class
// list, top-up to the target count) is AC's, kept so an item can be added back per class; every list is empty on 5.4.8.
// Ammo (AC InitAmmo :3868) is gone for the same reason (5.0.4), so it is not ported.
void BotFactory::InitReagents()
{
    std::vector<std::pair<uint32, uint32>> items;
    switch (bot->GetClass())
    {
        case CLASS_DEATH_KNIGHT:
        case CLASS_DRUID:
        case CLASS_MAGE:
        case CLASS_PALADIN:
        case CLASS_PRIEST:
        case CLASS_SHAMAN:
        case CLASS_WARLOCK:
        default:
            break;
    }

    for (std::pair<uint32, uint32> item : items)
    {
        int count = (int)item.second - (int)bot->GetItemCount(item.first);
        if (count > 0)
            StoreItem(bot, item.first, count);
    }
}

// #534: AC PlayerbotFactory::InitPotions (Bot/Factory/PlayerbotFactory.cpp:4084-4115). One stack (half to full) of a
// level-appropriate healing potion (SPELL_EFFECT_HEAL) and, for mana users, a mana potion (SPELL_EFFECT_ENERGIZE), only
// when the bot carries none. The pick comes from RandomItemManager::GetRandomPotion (AC RandomItemMgr::GetRandomPotion
// :989-1003). Level 10: Minor Healing Potion (118) / Minor Mana Potion (2455); level 40: Superior Healing Potion (3928)
// / Greater Mana Potion (6149); level 85-90: Master Healing Potion (76097) / Master Mana Potion (76098).
void BotFactory::InitPotions()
{
    uint32 effects[] = {SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE};
    for (uint8 i = 0; i < 2; ++i)
    {
        uint32 effect = effects[i];

        if (effect == SPELL_EFFECT_ENERGIZE && !bot->GetPower(POWER_MANA))
            continue;

        FindPotionVisitor visitor(bot, effect);
        IterateBotItems(bot, &visitor);
        if (!visitor.GetResult().empty())
            continue;

        uint32 itemId = sRandomItemMgr->GetRandomPotion(level, effect);
        if (!itemId)
            continue;

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        uint32 maxCount = proto->GetMaxStackSize();
        if (Item* newItem = StoreNewItemInInventorySlot(bot, itemId, urand(maxCount / 2, maxCount)))
            newItem->AddToUpdateQueueOf(bot);
    }
}

// #534: AC PlayerbotFactory::InitConsumables (Bot/Factory/PlayerbotFactory.cpp:1091-1303): the temporary weapon
// enchants (oils, sharpening stones / weightstones) AC's ImbueAction applies. Same item ids and per-class rules; the
// spec tabs are 5.4.8 specializations (AC tab -> spec as in ImbueAction.cpp). The ids are 5.4.8 item_template rows with a
// working on-use weapon-enchant spell (checked in Spell.dbc: Sharpen Blade 2828, Minor Wizard Oil 25117, ...).
// `level > 75` cuts every list off, as in AC. Level 10: Rough Sharpening Stone (2862) / Rough Weightstone (3239) for the
// melee classes (Minor Mana Oil needs level 20); level 40: Dense stones (12404 / 12643), Wizard Oil (20750) / Lesser
// Mana Oil (20747); level 70: Adamantite stones (23529 / 28421), Superior Mana / Wizard Oil (22521 / 22522); level 76+:
// nothing. Rogue: AC stocks Deadly / Instant Poison (:1259-1285); 5.4.8 poisons are self-cast spells (ImbueAction.h),
// so the rogue list is empty.
void BotFactory::InitConsumables()
{
    Specializations const specTab = AiFactory::GetPlayerSpecTab(bot);
    std::vector<std::pair<uint32, uint32>> items;

    std::vector<uint32> const wizardOils = {BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL, LESSER_WIZARD_OIL,
                                            MINOR_WIZARD_OIL};
    std::vector<uint32> const manaOils = {BRILLIANT_MANA_OIL, SUPERIOR_MANA_OIL, LESSER_MANA_OIL, MINOR_MANA_OIL};
    std::vector<uint32> const sharpeningStones = {ADAMANTITE_SHARPENING_STONE, FEL_SHARPENING_STONE,
                                                  DENSE_SHARPENING_STONE,      SOLID_SHARPENING_STONE,
                                                  HEAVY_SHARPENING_STONE,      COARSE_SHARPENING_STONE,
                                                  ROUGH_SHARPENING_STONE};
    std::vector<uint32> const weightstones = {ADAMANTITE_WEIGHTSTONE, FEL_WEIGHTSTONE,   DENSE_WEIGHTSTONE,
                                              SOLID_WEIGHTSTONE,      HEAVY_WEIGHTSTONE, COARSE_WEIGHTSTONE,
                                              ROUGH_WEIGHTSTONE};

    auto addStones = [&]
    {
        AddBestConsumable(items, sharpeningStones, level, 20);
        AddBestConsumable(items, weightstones, level, 20);
    };

    switch (bot->GetClass())
    {
        case CLASS_PRIEST:
            // AC :1099 specTab == PRIEST_TAB_SHADOW
            AddBestConsumable(items, specTab == SPEC_PRIEST_SHADOW ? wizardOils : manaOils, level, 2);
            break;
        case CLASS_MAGE:
            AddBestConsumable(items, wizardOils, level, 2);
            break;
        case CLASS_DRUID:
            if (specTab == SPEC_DRUID_BALANCE)  // AC DRUID_TAB_BALANCE
                AddBestConsumable(items, wizardOils, level, 2);
            else if (specTab == SPEC_DRUID_FERAL || specTab == SPEC_DRUID_GUARDIAN)  // AC DRUID_TAB_FERAL (bear + cat)
                addStones();
            else
                AddBestConsumable(items, manaOils, level, 2);
            break;
        case CLASS_PALADIN:
            if (specTab == SPEC_PALADIN_HOLY)  // AC PALADIN_TAB_HOLY
                AddBestConsumable(items, manaOils, level, 2);
            else
                addStones();
            break;
        case CLASS_WARRIOR:
        case CLASS_HUNTER:
        case CLASS_DEATH_KNIGHT:
            addStones();
            break;
        default:
            break;
    }

    for (std::pair<uint32, uint32> item : items)
    {
        int count = (int)item.second - (int)bot->GetItemCount(item.first);
        if (count > 0)
            StoreItem(bot, item.first, count);
    }
}

// #534: AC PlayerbotFactory::CleanupConsumables (Bot/Factory/PlayerbotFactory.cpp:4351-4412): the level-up maintenance
// pass (AC AutoMaintenanceOnLevelupAction.cpp:168) that removes the old, now outlevelled consumables before the
// initialisers restock. Removes ammo, food and drink, potions and flasks, reagents, and the oil / stone / poison ids
// InitConsumables stocks. AC's ITEM_SUBCLASS_FOOD is 5.4.8's ITEM_SUBCLASS_FOOD_DRINK.
void BotFactory::CleanupConsumables()
{
    std::vector<Item*> itemsToDelete;

    std::vector<Item*> items;
    for (uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            items.push_back(item);

    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* bag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < bag->GetBagSize(); ++j)
                if (Item* item = bag->GetItemByPos(j))
                    items.push_back(item);

    for (Item* item : items)
    {
        ItemTemplate const* proto = item->GetTemplate();
        if (!proto)
            continue;

        // Remove ammo
        if (proto->Class == ITEM_CLASS_PROJECTILE)
            itemsToDelete.push_back(item);

        // Remove food/drink
        if (proto->Class == ITEM_CLASS_CONSUMABLE && proto->SubClass == ITEM_SUBCLASS_FOOD_DRINK)
            itemsToDelete.push_back(item);

        // Remove potions
        if (proto->Class == ITEM_CLASS_CONSUMABLE && proto->SubClass == ITEM_SUBCLASS_POTION)
            itemsToDelete.push_back(item);

        // Remove reagents
        if (proto->Class == ITEM_CLASS_REAGENT ||
            (proto->Class == ITEM_CLASS_MISCELLANEOUS && proto->SubClass == ITEM_SUBCLASS_JUNK_REAGENT))
            itemsToDelete.push_back(item);
    }

    std::set<uint32> idsToDelete = {
        BRILLIANT_MANA_OIL, SUPERIOR_MANA_OIL, LESSER_MANA_OIL, MINOR_MANA_OIL,
        BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL, LESSER_WIZARD_OIL, MINOR_WIZARD_OIL,
        ADAMANTITE_SHARPENING_STONE, FEL_SHARPENING_STONE, DENSE_SHARPENING_STONE, SOLID_SHARPENING_STONE,
        HEAVY_SHARPENING_STONE, COARSE_SHARPENING_STONE, ROUGH_SHARPENING_STONE,
        ADAMANTITE_WEIGHTSTONE, FEL_WEIGHTSTONE, DENSE_WEIGHTSTONE, SOLID_WEIGHTSTONE,
        HEAVY_WEIGHTSTONE, COARSE_WEIGHTSTONE, ROUGH_WEIGHTSTONE,
        INSTANT_POISON_IX, INSTANT_POISON_VIII, INSTANT_POISON_VII, INSTANT_POISON_VI, INSTANT_POISON_V,
        INSTANT_POISON_IV, INSTANT_POISON_III, INSTANT_POISON_II, INSTANT_POISON,
        DEADLY_POISON_IX, DEADLY_POISON_VIII, DEADLY_POISON_VII, DEADLY_POISON_VI, DEADLY_POISON_V,
        DEADLY_POISON_IV, DEADLY_POISON_III, DEADLY_POISON_II, DEADLY_POISON
    };

    for (Item* item : items)
    {
        ItemTemplate const* proto = item->GetTemplate();
        if (!proto)
            continue;

        if (idsToDelete.find(proto->ItemId) != idsToDelete.end())
            itemsToDelete.push_back(item);
    }

    // AC pushes an item once per matching rule, so a flask or a poison can be listed twice, and destroying the second
    // pointer is a use after free. De-duplicated here.
    std::sort(itemsToDelete.begin(), itemsToDelete.end());
    itemsToDelete.erase(std::unique(itemsToDelete.begin(), itemsToDelete.end()), itemsToDelete.end());

    for (Item* item : itemsToDelete)
        bot->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
}

void BotFactory::Prepare()
{
    if (bot->isDead())
        bot->ResurrectPlayer(1.0f, false);
 
    bot->CombatStop(true);

    int32 newlevel = level;
    if (newlevel < 1)
        newlevel = 1;
    if (newlevel > 90)
        newlevel = 90;

    bot->GiveLevel(newlevel);
    bot->InitTalentForLevel();
    bot->SetUInt32Value(PLAYER_FIELD_XP, 0);
    bot->RemoveAllSpellCooldown();
    bot->InitStatsForLevel();
    // #893: no CancelAuras here any more - AC's Prepare (PlayerbotFactory.cpp:753-775) strips no auras; Randomize does it
    // after the talent/spell/item reset (AC :823-824).
}
 
void BotFactory::Randomize(bool incremental)
{
    TC_LOG_INFO("playerbots", "%s randomizing %s (level %u class = %s)...", (incremental ? "Incremental" : "Full"),
            bot->GetName().c_str(), level, ClassToString((Classes)bot->GetClass()).c_str());

    uint32 const oldLevel = bot->GetLevel();

    Prepare();
    if (!incremental)
    {
        // #745: AC ClearSkills (PlayerbotFactory.cpp:808) - the trade skills, so InitSkills below re-rolls the profession
        // pair from the stored firstSkill/secondSkill (AC :2976-2981) instead of keeping a contaminated set.
        ClearTradeSkills(bot);
        ResetQuests(bot);   // #813: AC order ClearSkills, ClearSpells, ResetQuests (PlayerbotFactory.cpp:808-810)

        // -- Unlearn talents and spec
        bot->ResetTalents(true, true, true);

        // -- release pet
        bot->RemovePet(PetRemoveMode::PET_REMOVE_ABANDON, PetRemoveFlag::PET_REMOVE_FLAG_NONE);

        // Destroy equipped items.
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            {
                std::string itemName = item->GetTemplate()->Name1;
                bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
            }
        }
    }

    // #707: AC PlayerbotFactory::Randomize (Bot/Factory/PlayerbotFactory.cpp:818) clears the bags on every randomize,
    // full and incremental, after the reset block.
    ClearInventory(bot);

    // #893: AC Randomize (PlayerbotFactory.cpp:823-824) strips the auras only here, after the talent/spec/glyph reset, so no
    // spec or glyph aura script (Shadowform + Glyph of Shadow) can re-apply an aura inside RemoveAllAuras. Prepare() used to
    // do it first, and a down-levelled shadow priest hung the world thread there (1 core, +30 MB/s).
    bot->InitStatsForLevel(true);
    CancelAuras();
    // #521: full randomize only - the counterpart of AC's ClearSpells + re-learn, which re-casts every passive
    // (AC's incremental path keeps the spellbook and does not either). After CancelAuras, which would strip them again.
    if (!incremental)
        ReapplyKnownPassives(bot);

    // Phase 1.1 (issue #59): the full-randomize path clears talents and the specialization
    // (ResetTalents above) but never re-picked one, so those bots ended up with no spec and an
    // empty spellbook - every class action then fell back to auto-attack. Re-pick a spec and learn
    // its spells here (InitTalentsTree also learns the spec spells, before its early return).
    InitTalentsTree(false);

    // #65 (AC Bot/Factory/PlayerbotFactory.cpp:873-878: PlayerbotRepository::Reset + ResetStrategies(false)): the spec may
    // have changed, so rebuild the engines for it. Without this a re-randomized L85 warrior turned fury kept the protection
    // strategies (w67: revenge / shield slam / devastate never resolved). The stored strategies go too.
    if (botAI)
    {
        PlayerbotRepository::instance().Reset(botAI);
        botAI->ResetStrategies(false);  // fix wrong stored strategy
    }

    // #745: AC Randomize -> InitSkills (Bot/Factory/PlayerbotFactory.cpp:853), before InitClassSpells/InitAvailableSpells
    // (:855-856) so the trainer sweep sees the professions. It ends in InitInventorySkill (AC :3408, #707).
    InitSkills();

    // Incremental randomize keeps the existing spec, so still guarantee the class spellbook.
    InitClassSpells();

    // #734, AC PlayerbotFactory::Randomize (Bot/Factory/PlayerbotFactory.cpp:1000-1001): InitGlyphs() on every
    // randomize, after the talents.
    InitGlyphs(false);

    InitPet();

    // issue #136: a full randomize at the bot's current level strips its gear (Prepare) but GiveLevel
    // sends no levelup when the level does not change, so the levelup re-gear never runs.
    if (!incremental && bot->GetLevel() == oldLevel)
        InitEquipment(false);
    // #623, AC PlayerbotFactory::Randomize (Bot/Factory/PlayerbotFactory.cpp:915-920) runs InitEquipment(incremental)
    // on every randomize. The incremental path here never did, so a random bot already at the start level (level 90)
    // kept a weaponless kit through every scheduled re-randomize. When the level changed, the levelup maintenance
    // action re-gears, so only the same-level case is added (AC gates this on equipAndSpecPersistence, which this fork does not have).
    else if (incremental && bot->GetLevel() == oldLevel)
        InitEquipment(true, false);

    // AC PlayerbotFactory::Randomize (Bot/Factory/PlayerbotFactory.cpp:933-938): every randomize gives the bot
    // four bags. Without them a MoP bot carried 16 slots forever, filled them with junk and could not store
    // quest items or reward items.
    InitBags();

    // #534, AC PlayerbotFactory::Randomize (Bot/Factory/PlayerbotFactory.cpp:948 InitFood, :954 InitPotions, :960
    // InitReagents, :996 InitConsumables): after the bags exist, stock food, potions and reagents, then the weapon
    // consumables (needs the spec chosen above). InitAmmo / InitKeyring are gone in 5.4.8. The bags were emptied by ClearInventory earlier in this method.
    InitFood();
    InitPotions();
    InitReagents();
    InitConsumables();

    bot->SetMoney(urand(level * 100000, level * 5 * 100000));
    bot->SetHealth(bot->GetMaxHealth());
    bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));

    bot->SaveToDB(false);
}
 
void BotFactory::Refresh()
{
    // #707: AC PlayerbotFactory::Refresh (Bot/Factory/PlayerbotFactory.cpp:1060) empties the bags on every refresh -
    // the 1-5 h teleport cycle, revive and LFG join (RandomPlayerbotMgr::Refresh, AC :2170-2199). It is what keeps an AC
    // random bot's bags from filling up; nothing else does (AC's "maintenance" strategy is off, AiFactory.cpp:630).
    ClearInventory(bot);

    // #534, AC PlayerbotFactory::Refresh (Bot/Factory/PlayerbotFactory.cpp:1061-1065): restock what ClearInventory just
    // removed, in AC's order (InitAmmo is gone with 5.0.4). Without this a refresh would leave the bot with no
    // food/drink, potions or weapon consumables.
    InitFood();
    InitReagents();
    InitConsumables();
    InitPotions();

    InitPet();

    // #745: AC Refresh -> InitSkills (Bot/Factory/PlayerbotFactory.cpp:1068), after InitPet as on AC: riding (#134,
    // InitMountSpells), the trade skills, and InitInventorySkill (AC :3408, #707), which hands the gathering tools back
    // after ClearInventory.
    InitSkills();

    bot->DurabilityRepairAll(false, 1.0f, false);
    if (bot->isDead())
        bot->ResurrectPlayer(1.0f, false);
    uint32 money = urand(level * 1000, level * 5 * 1000);
    if (bot->GetMoney() < money)
        bot->SetMoney(money);
}
 
void BotFactory::InitPet()
{
    Pet* pet = bot->GetPet();

    /*if (bot->GetClass() == CLASS_HUNTER)
    {
        if (pet)
        {
            bot->RemovePet(PET_REMOVE_ABANDON);
            pet = nullptr;
        }

        // -- Delete all pet slots if possible
        for (uint8 pet_slot_active = 0; pet_slot_active < PetSlot::PET_SLOT_ACTIVE_LAST; ++pet_slot_active)
        {
            uint32 pet_id = bot->GetPetIdBySlot(pet_slot_active);
            if (!pet_id) continue;

            bot->SummonPet(pet_id, bot->GetWorldLocation().GetPositionX(), bot->GetWorldLocation().GetPositionY(),
                           bot->GetWorldLocation().GetPositionZ(), 0.0f, 0);
            bot->RemovePet(PetRemoveMode::PET_REMOVE_ABANDON);
            pet = nullptr;
        }
    }*/

    // #588, AC parity (PlayerbotFactory.cpp:1457-1461, 1518-1530): a hunter must never end with more than one pet in
    // the active slots 0-4. AC keeps the existing CurrentPet (early return) and removes stale pets before creating a
    // new one; in 5.4.8 a respec/dismissal resets the current id while the pets stay in their slots, so re-use the
    // slotted pet (prefer a non-exotic one unless the bot is BM, #202), abandon the surplus, and only create when no
    // slotted pet can be loaded.
    if (!pet && bot->GetClass() == CLASS_HUNTER && bot->IsInWorld())
    {
        std::vector<uint32> owned;
        uint32 curId = bot->GetCurrentPetId();
        if (curId && bot->GetSlotByPetId(curId) >= PET_SLOT_ACTIVE_FIRST && bot->GetSlotByPetId(curId) <= PET_SLOT_ACTIVE_LAST)
            owned.push_back(curId);
        for (uint8 s = PET_SLOT_ACTIVE_FIRST; s <= PET_SLOT_ACTIVE_LAST; ++s)
        {
            uint32 id = bot->GetPetIdBySlot(s);
            if (id && std::find(owned.begin(), owned.end(), id) == owned.end())
                owned.push_back(id);
        }

        auto loadPet = [this](uint32 id) -> Pet*
        {
            Pet* p = new Pet(bot);
            if (!p->LoadPetFromDB(PET_LOAD_BY_ID, id))
            {
                delete p;
                return nullptr;
            }
            return bot->GetPet();
        };

        uint32 keeperId = 0;
        for (size_t i = 0; i < owned.size() && !keeperId; ++i)
        {
            Pet* p = loadPet(owned[i]);
            if (!p)
                continue;
            bool exotic = p->GetCreatureTemplate() && p->GetCreatureTemplate()->IsExotic();
            if (exotic && !bot->CanTameExoticPets())
            {
                bot->RemovePet(PET_REMOVE_ABANDON); // frees the slot and deletes the row
                continue;
            }
            keeperId = owned[i];
        }

        if (keeperId)
        {
            // abandon every other active-slot pet (summon it, then PET_REMOVE_ABANDON, as the old commented block did)
            bot->RemovePet(PET_REMOVE_DISMISS, PET_REMOVE_FLAG_NONE);
            for (uint32 id : owned)
            {
                if (id == keeperId || bot->GetSlotByPetId(id) < 0)
                    continue;
                if (loadPet(id))
                    bot->RemovePet(PET_REMOVE_ABANDON);
                else
                    Pet::DeleteFromDB(id);
            }
            bot->SetCurrentPetId(keeperId);
            pet = loadPet(keeperId);
            if (pet)
                bot->PetSpellInitialize();
        }
    }

    // AC parity (PlayerbotFactory.cpp:1461): a hunter that already has a current pet (stored, not summoned) keeps it
    if (!pet && bot->GetCurrentPetId())
        return;

    if (!pet)
    {
        if (bot->GetClass() != CLASS_HUNTER)
            return;

        Map* map = bot->GetMap();
        if (!map)
            return;

        std::vector<uint32> ids;
        CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates();
        for (CreatureTemplateContainer::const_iterator itr = creatures->begin(); itr != creatures->end(); ++itr)
        {
            if (!itr->second.IsTameable(bot->CanTameExoticPets()))
                continue;
 
            if (itr->second.minlevel > bot->GetLevel())
                continue;
 
            ids.push_back(itr->first);
        }
 
        if (ids.empty())
        {
            TC_LOG_ERROR("playerbots", "No pets available for bot %s (%u level)", bot->GetName().c_str(), bot->GetLevel());
            return;
        }

        for (uint32 i = 0; i < 10; i++)
        {
            uint32 index = urand(0, ids.size() - 1);
            CreatureTemplate const* co = sObjectMgr->GetCreatureTemplate(ids[index]);
            if (!co)
                continue;
            if (co->Name.size() > 21)
                continue;

            int8 newPetSlot = bot->GetSlotForNewPet();
            if (newPetSlot == -1)
                continue;

            // Everything looks OK, create new pet
            pet = bot->CreateTamedPetFrom(co->Entry, 0);
            if (!pet)
                continue;

            // prepare visual effect for levelup
            pet->SetUInt32Value(UNIT_FIELD_LEVEL, bot->GetLevel() - 1);
 
            // add to world
            pet->GetMap()->AddToMap(pet->ToCreature());
 
            // visual effect for levelup
            pet->SetUInt32Value(UNIT_FIELD_LEVEL, bot->GetLevel());
 
            // caster have pet now
            bot->SetMinion(pet, true);
 
            pet->InitTalentForLevel();

            // register in a real active slot exactly like Spell::EffectTameCreature (SpellEffects.cpp:3406-3413)
            bot->AddNewPet(newPetSlot, pet);
            bot->SetCurrentPetId(pet->GetCharmInfo()->GetPetNumber());
            pet->SavePetToDB();
            bot->PetSpellInitialize();
            break;
        }
    }
 
    if (pet)
    {
        pet->InitStatsForLevel(bot->GetLevel());
        pet->SetLevel(bot->GetLevel());
        pet->SetHealth(pet->GetMaxHealth());
    }
    else
    {
        TC_LOG_ERROR("playerbots", "Cannot create pet for bot %s", bot->GetName().c_str());
        return;
    }
 
    // TC_LOG_INFO("playerbots", "Start make spell auto cast for {} spells. {} already auto casted.", pet->m_spells.size(),
    // pet->GetPetAutoSpellSize());
    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;
 
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
        if (!spellInfo)
            continue;
 
        if (spellInfo->IsPassive())
        {
            continue;
        }
        pet->ToggleAutocast(spellInfo, true);
    }
}
#include <fstream>

// #739: AC PlayerbotFactory::InitTalentsTree's spec pick (PlayerbotFactory.cpp:1696-1721) - a weighted draw over
// AiPlayerbot.RandomClassSpecProb.<class>.<specno>. <specno> is the ChrSpecialization TabPage, so a druid has four
// (balance, feral, guardian, restoration) where the old std::rand() % 3 could never reach the 4th. A tab the class
// does not have (ChrSpecialization.dbc) never counts, whatever the config says. AC falls back to specno 0 (and logs)
// when no weight is set, as below.
static uint32 PickRandomSpecTab(Player* bot)
{
    uint8 const cls = bot->GetClass();
    uint32 weight[PlayerbotAIConfig::MAX_SPECNO] = {};
    uint32 pointSum = 0;
    for (uint32 i = 0; i < sChrSpecializationStore.GetNumRows(); ++i)
    {
        ChrSpecializationEntry const* spec = sChrSpecializationStore.LookupEntry(i);
        if (spec && spec->classId == cls && spec->TabPage < PlayerbotAIConfig::MAX_SPECNO)
            weight[spec->TabPage] = sPlayerbotAIConfig->randomClassSpecProb[cls][spec->TabPage];
    }
    for (uint32 w : weight)
        pointSum += w;
    if (pointSum)
    {
        uint32 const point = urand(1, pointSum);
        uint32 currentP = 0;
        for (uint32 i = 0; i < PlayerbotAIConfig::MAX_SPECNO; ++i)
        {
            currentP += weight[i];
            if (point <= currentP)
                return i;
        }
    }
    TC_LOG_ERROR("playerbots", "Fail to select spec num for bot %s! Set to 0.", bot->GetName().c_str());
    return 0;
}

void BotFactory::InitTalentsTree(bool reset)
{
    /*std::map<uint32, std::list<const TalentEntry*>> talents_dbc;
    for (auto entry = sTalentStore.begin(); entry != sTalentStore.end(); ++entry)
    {
        if (talents_dbc.find(entry->PlayerClass) == talents_dbc.end())
            talents_dbc[entry->PlayerClass] = std::list<const TalentEntry*>();
        talents_dbc[entry->PlayerClass].push_back(*entry);
    }

    for (auto& ref : talents_dbc)
    {
        ref.second.sort([](const TalentEntry* a, const TalentEntry* b)
        {
            return (a->Row < b->Row) || (a->Row == b->Row && a->Col < b->Col);
        });
    }

    std::ofstream os("./talent_export.txt", std::ios::app);
    for (const auto& ref : talents_dbc)
    {
        auto classe = ClassToString((Classes)ref.first);
        os << classe << ":\n";
        uint32 currentRow = 0;
        for (const auto& tal : ref.second)
        {
            if (tal->Row != currentRow)
            {
                currentRow = tal->Row;
                os << "\n";
            }
            os << tal->TalentID << "\t";
        }
        os << "\n";
    }
    os.close();*/

    // -- reset spec in case we down level
    if (reset)
    {
        bot->ResetTalents(true, true, true);
    }
    
    // if no spec then pick one random (need to change that to balance)
    if (bot->GetSpecialization() == Specializations::SPEC_NONE)
    {
        // -- Select spec
        if (bot->GetLevel() >= 10)
        {
            uint32 const tab = PickRandomSpecTab(bot);
            WorldPacket p(CMSG_SET_PRIMARY_TALENT_TREE);
            p << tab;
            bot->GetSession()->HandeSetTalentSpecialization(p);
            bot->ActivateSpec(0);
        }
    }

    // issue #98/#65: Player::GiveLevel() (called earlier in Prepare(), before the spec reset/pick
    // above during a full randomize) calls LearnSpecializationSpells() while the bot's spec is still
    // SPEC_NONE (or a stale spec about to be reset), so it grants nothing useful. Nothing re-called
    // it after the spec above was actually finalized, so bots never learned their specialization's
    // core rotation spells (e.g. a Protection Warrior never learned Devastate/Revenge/Shockwave -
    // only whatever InitClassSpells' skill-line sweep happens to cover, like Shield Slam - and the
    // rotation strategy's higher-priority abilities permanently resolved to spell id 0, leaving only
    // the filler ability to ever fire). Re-grant them now that GetTalentSpecialization() is correct;
    // LearnSpell is idempotent so this is safe to call unconditionally on every randomize.
    bot->LearnSpecializationSpells();

    // Phase 1.1 (issue #59): guarantee the class spellbook before talents are (re)learned and
    // before the available-points early return below. Bots whose spec was already set never hit
    // WorldSession::HandeSetTalentSpecialization, so their specialization spells were missing and
    // every class action fell back to auto-attack.
    InitClassSpells();

    // #734: the old tail indexed premadeSpecLink by GetUsedTalentCount() and returned on an empty link - and every
    // link loaded empty (PlayerbotAIConfig.cpp premade loader), so no bot ever learned a talent.
    InitTalentTiers();
}

// #734: ChrSpecialization TabPage of the bot's active spec - the index of CMSG_SET_PRIMARY_TALENT_TREE
// (SkillHandler.cpp HandeSetTalentSpecialization, dbc::GetClassSpecializations(class)[tab]) and of the
// AiPlayerbot.PremadeSpec*.<class>.<tab> keys. PlayerBotSpec::GetSpectab swapped the monk tabs (Windwalker 1,
// Mistweaver 2; ChrSpecialization.dbc: Brewmaster 0, Mistweaver 1, Windwalker 2) until #740; the DBC is still read directly.
static constexpr uint32 MOP_TALENT_TIERS = 6;    // Talent.dbc TierID 0-5
static constexpr uint32 MOP_TALENT_COLUMNS = 3;  // Talent.dbc ColumnIndex 0-2

// #739: AC returns randomClassSpecIndex[class][specTab] from InitTalentsTree and uses it to pick the PremadeSpec entry
// (PlayerbotFactory.cpp:1744, 3672); default index == specno, so unconfigured behaviour is unchanged.
static uint32 GetPremadeSpecIndex(Player* bot, uint32 tab)
{
    uint32 const index = sPlayerbotAIConfig->randomClassSpecIndex[bot->GetClass()][tab];
    return index < MAX_SPECIALIZATIONS - 1 ? index : tab;
}

static uint32 GetSpecTabPage(Player* bot)
{
    if (ChrSpecializationEntry const* spec = sChrSpecializationStore.LookupEntry(uint32(bot->GetSpecialization())))
        if (spec->classId == bot->GetClass() && spec->TabPage < MAX_SPECIALIZATIONS - 1)
            return spec->TabPage;
    return MAX_SPECIALIZATIONS;
}

/// #734 - AC InitTalentsTree -> InitTalentsByTemplate + InitTalents (Bot/Factory/PlayerbotFactory.cpp:1669-1745,
/// 3664-3740) learns the premade spec link for the bot's level and then spends any free point at random. A 5.4.8
/// character has one talent per tier instead: Talent.dbc TierID 0-5, unlocked at 15/30/45/60/75/90
/// (Player::CalculateTalentsPoints = level / 15). For every unlocked tier with no talent yet, learn the configured
/// column (AiPlayerbot.PremadeSpecLink.<class>.<tab>), or a random column when none is configured, through
/// Player::LearnTalent - the CMSG_LEARN_TALENT path - so the talent lands in the talent map as PLAYERSPELL_NEW and
/// Player::_SaveTalents writes character_talent, which inspect (Player::SendInspectResult) reads back.
/// Idempotent: a tier that already holds a talent is left alone.
void BotFactory::InitTalentTiers()
{
    uint32 const tab = GetSpecTabPage(bot);
    if (tab >= MAX_SPECIALIZATIONS - 1)
        return;

    uint32 const tiers = std::min<uint32>(bot->CalculateTalentsPoints(), MOP_TALENT_TIERS);
    if (!tiers)
        return;

    TalentEntry const* grid[MOP_TALENT_TIERS][MOP_TALENT_COLUMNS] = {};
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talent = sTalentStore.LookupEntry(i);
        if (!talent || talent->PlayerClass != bot->GetClass() || talent->TierID >= MOP_TALENT_TIERS ||
            talent->ColumnIndex >= MOP_TALENT_COLUMNS)
            continue;
        grid[talent->TierID][talent->ColumnIndex] = talent;
    }

    // LearnTalent gates on PLAYER_FIELD_MAX_TALENT_TIERS (Player.cpp LearnTalent), which InitTalentForLevel sets.
    if (bot->GetUInt32Value(PLAYER_FIELD_MAX_TALENT_TIERS) < tiers)
        bot->SetUInt32Value(PLAYER_FIELD_MAX_TALENT_TIERS, bot->CalculateTalentsPoints());

    std::vector<uint16> const& link = sPlayerbotAIConfig->premadeSpecLink[bot->GetClass()][GetPremadeSpecIndex(bot, tab)];
    uint8 const activeSpec = bot->GetActiveSpec();
    std::ostringstream learned;
    for (uint32 tier = 0; tier < tiers; ++tier)
    {
        bool hasTier = false;
        for (TalentEntry const* talent : grid[tier])
            if (talent && bot->HasTalent(talent->SpellID, activeSpec))
                hasTier = true;
        if (hasTier)
            continue;

        // A tier spell known outside the talent map (never through a talent) would make LearnTalent refuse the
        // whole tier (its same-tier HasSpell check) and leave the tier grey on inspect; drop it so the tier is learned.
        for (TalentEntry const* talent : grid[tier])
            if (talent && bot->HasSpell(talent->SpellID))
            {
                bot->RemoveSpell(talent->SpellID, false, false);
                learned << " t" << tier << "stray=" << talent->SpellID;
            }

        uint32 column = (tier < link.size() && link[tier] < MOP_TALENT_COLUMNS) ? link[tier] : urand(0, MOP_TALENT_COLUMNS - 1);
        TalentEntry const* pick = grid[tier][column];
        if (!pick)
            continue;

        bool const ok = bot->LearnTalent(uint16(pick->ID));
        learned << " t" << tier << "c" << column << "=" << pick->SpellID << (ok ? "" : "!");
    }

    if (!learned.str().empty())
    {
        bot->SendTalentsInfoData();
        TC_LOG_DEBUG("playerbots", "[TALENT] %s class=%u level=%u tab=%u tiers=%u learned:%s ('!' = refused)",
                     bot->GetName().c_str(), uint32(bot->GetClass()), bot->GetLevel(), tab, tiers,
                     learned.str().c_str());
    }
}

/// #734 - AC PlayerbotFactory::InitGlyphs (Bot/Factory/PlayerbotFactory.cpp:4414-4760): unless incremental, clear
/// the glyphs; then fill each unlocked glyph slot with the configured glyph item of the bot's spec
/// (AiPlayerbot.PremadeSpecGlyph.<class>.<tab>), or a random class glyph of the slot's type when none is configured.
/// 5.4.8 slots (Player::InitGlyphsForLevel): 0/2/4 minor, 1/3/5 major (GlyphSlot.dbc TypeFlags 1/0), unlocked in
/// pairs at 25/50/75 (PLAYER_FIELD_GLYPH_SLOTS_ENABLED; Spell::EffectApplyGlyph enforces the same levels). AC's
/// WotLK slot order table does not apply, so each configured glyph goes to the first free unlocked slot of its own
/// type, in config order (AC's documented order: major 1, minor 1, major 2, minor 2, minor 3, major 3).
/// Player::SetGlyph stores it in the spec's glyph array (saved by _SaveGlyphs, sent by SendInspectResult) and learns
/// the glyph's effect spell. The glyph's apply spell (the item's SPELL_EFFECT_APPLY_GLYPH spell) is learned too, as
/// using a 5.4.8 glyph item does, so the glyph shows as known in the glyph pane.
namespace
{
struct BotGlyphInfo
{
    uint32 glyphId;     // GlyphProperties.dbc
    uint32 applySpell;  // the item spell with SPELL_EFFECT_APPLY_GLYPH
    uint32 typeFlags;   // GlyphProperties TypeFlags: 0 major, 1 minor
    uint32 requiredLevel;
    int32 allowableRace;
};

// AC walks every item template per call (PlayerbotFactory.cpp:4627-4660); the class glyph list never changes, so it
// is built once per class.
bool ResolveGlyphItem(ItemTemplate const* proto, BotGlyphInfo& out)
{
    if (!proto || proto->Class != ITEM_CLASS_GLYPH)
        return false;
    // 5.4.8 item data still carries retired glyph items ("zzOLD Glyph of ...", "Deprecated Glyph of ...").
    if (proto->Name1.find("zzOLD") != std::string::npos || proto->Name1.find("eprecated") != std::string::npos ||
        proto->Name1.find("EPRECATED") != std::string::npos)
        return false;
    for (uint32 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        SpellInfo const* spell = sSpellMgr->GetSpellInfo(uint32(proto->Spells[i].SpellId));
        if (!spell)
            continue;
        for (uint32 e = 0; e < MAX_SPELL_EFFECTS; ++e)
        {
            if (spell->Effects[e].Effect != SPELL_EFFECT_APPLY_GLYPH)
                continue;
            GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(uint32(spell->Effects[e].MiscValue));
            if (!gp)
                continue;
            out.glyphId = gp->Id;
            out.applySpell = spell->Id;
            out.typeFlags = gp->TypeFlags;
            out.requiredLevel = proto->RequiredLevel;
            out.allowableRace = proto->AllowableRace;
            return true;
        }
    }
    return false;
}

std::vector<BotGlyphInfo> const& GetClassGlyphs(uint8 cls)
{
    static std::vector<BotGlyphInfo> byClass[MAX_CLASSES];
    static std::once_flag once;
    std::call_once(once, []()
    {
        for (auto const& entry : *sObjectMgr->GetItemTemplateStore())
        {
            BotGlyphInfo info{};
            if (!ResolveGlyphItem(&entry.second, info))
                continue;
            for (uint8 c = 1; c < MAX_CLASSES; ++c)
                if (entry.second.AllowableClass != -1 && (entry.second.AllowableClass & (1 << (c - 1))))
                    byClass[c].push_back(info);
        }
    });
    return byClass[cls < MAX_CLASSES ? cls : 0];
}
}

void BotFactory::InitGlyphs(bool increment)
{
    bot->InitGlyphsForLevel();
    uint8 const activeSpec = bot->GetActiveSpec();

    if (!increment)
        for (uint8 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)
            if (bot->GetGlyph(activeSpec, slot))
                bot->SetGlyph(slot, 0);  // Player::UnapplyGlyph removes the glyph's effect spell and aura

    uint32 const enabled = bot->GetUInt32Value(PLAYER_FIELD_GLYPH_SLOTS_ENABLED);
    if (!enabled || bot->GetLevel() < 25)
    {
        bot->SendTalentsInfoData();
        return;
    }

    std::set<uint32> chosen;
    for (uint8 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)
        if (uint32 glyph = bot->GetGlyph(activeSpec, slot))
            chosen.insert(glyph);

    auto slotType = [this](uint8 slot) -> int32
    {
        GlyphSlotEntry const* gs = sGlyphSlotStore.LookupEntry(bot->GetGlyphSlot(slot));
        return gs ? int32(gs->TypeFlags) : -1;
    };
    auto freeSlot = [&](uint32 typeFlags) -> int32
    {
        for (uint8 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)
            if ((enabled & (1 << slot)) && !bot->GetGlyph(activeSpec, slot) && slotType(slot) == int32(typeFlags))
                return slot;
        return -1;
    };
    std::ostringstream applied;
    auto apply = [&](uint8 slot, BotGlyphInfo const& info, char const* how)
    {
        if (info.applySpell && !bot->HasSpell(info.applySpell))
            bot->LearnSpell(info.applySpell, false);
        bot->SetGlyph(slot, info.glyphId);
        chosen.insert(info.glyphId);
        applied << " s" << uint32(slot) << "=" << info.glyphId << how;
    };
    auto usable = [this](BotGlyphInfo const& info)
    {
        return info.requiredLevel <= bot->GetLevel() &&
               (info.allowableRace == -1 || (info.allowableRace & int32(bot->GetRaceMask())));
    };

    uint32 const tab = GetSpecTabPage(bot);
    if (tab < MAX_SPECIALIZATIONS - 1)
    {
        for (uint32 itemId : sPlayerbotAIConfig->premadeSpecGlyph[bot->GetClass()][GetPremadeSpecIndex(bot, tab)])
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            BotGlyphInfo info{};
            if (!proto || (proto->AllowableClass & int32(bot->GetClassMask())) == 0 || !ResolveGlyphItem(proto, info) ||
                !usable(info) || chosen.count(info.glyphId))
                continue;
            int32 const slot = freeSlot(info.typeFlags);
            if (slot >= 0)
                apply(uint8(slot), info, "c");
        }
    }

    // AC's else branch: a slot without a configured glyph gets a random class glyph of the slot's type.
    std::vector<BotGlyphInfo> const& pool = GetClassGlyphs(bot->GetClass());
    for (uint8 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)
    {
        if (!(enabled & (1 << slot)) || bot->GetGlyph(activeSpec, slot))
            continue;
        int32 const type = slotType(slot);
        std::vector<BotGlyphInfo const*> ids;
        for (BotGlyphInfo const& info : pool)
            if (int32(info.typeFlags) == type && usable(info) && !chosen.count(info.glyphId))
                ids.push_back(&info);
        if (!ids.empty())
            apply(slot, *ids[urand(0, uint32(ids.size()) - 1)], "r");
    }

    bot->SendTalentsInfoData();
    if (!applied.str().empty())
        TC_LOG_DEBUG("playerbots", "[GLYPH] %s class=%u level=%u tab=%u slots=0x%X applied:%s (c = config, r = random)",
                     bot->GetName().c_str(), uint32(bot->GetClass()), bot->GetLevel(), tab, enabled,
                     applied.str().c_str());
}

/// issue #230 - AC parity, the sweep the MoP port dropped.
/// AC's Bot\Factory\PlayerbotFactory.cpp:3445-3493 InitAvailableSpells() teaches a bot every spell any
/// trainer offers it. Without it a MoP bot only ever knows skill-line rewarded spells, its active
/// specialization's spells and mounts/professions (109-184 spells measured, versus ~350-462 for an AC
/// bot), so every trainer-taught baseline ability - Mind Blast for a Discipline/Holy priest, Judgment,
/// ... - is absent from PlayerSpellMap and SpellIdValue resolves its name to spell id 0
/// (strategy/value/SpellIdValue.cpp:113), silently degrading each class action to its alternative.
/// This core's AiPlayerbot.AutoLearnTrainerSpells (Utils/PlayerbotAIConfig.cpp:370 ->
/// PlayerbotAIConfig.h:261) was loaded and then never used anywhere; it gates the sweep here.
///
/// The body is adapted, not copied: AC enumerates AzerothCore's Trainer::Trainer API
/// (sObjectMgr->GetTrainer, Trainer::Type::Tradeskill/Class, IsTrainerSpellAllowedForBot,
/// CanTeachSpell), none of which exists on this tree. This core keeps the world.npc_trainer rows in
/// ObjectMgr's cache store (ObjectMgr.cpp:9093 LoadTrainerSpell / :9012 AddSpellToTrainer,
/// ObjectMgr.h:1640 GetNpcTrainerSpells, Creature.h:204-233 TrainerSpellData/TrainerSpell) and gates
/// every spell through Player::GetTrainerSpellState (Player.cpp:4557), which already enforces level,
/// reqSkill, class, race, previous rank, required spells and the primary profession limit - the union
/// of AC's CanTeachSpell plus IsTrainerSpellAllowedForBot. So: walk the creature templates flagged
/// UNIT_NPC_FLAG_TRAINER, then learn every spell the bot sees as TRAINER_SPELL_GREEN, branching the
/// same way WorldSession::HandleTrainerBuySpellOpcode does (Handlers/NPCHandler.cpp:413-417) - cast
/// when TrainerSpell::IsCastable(), otherwise LearnSpell.
///
/// Idempotent: GetTrainerSpellState reports TRAINER_SPELL_GRAY for a spell the bot already knows, so
/// re-running this (Refresh, Randomize, the level-up repair at RandomPlayerbotMgr.cpp:975) learns
/// nothing. The bounded pass loop exists only because TrainerSpellMap is an unordered_map:
/// GetTrainerSpellState reports RED until the previous rank of a chain is known, so a single pass
/// would learn rank 1 and then stall on every higher rank.
// #745 (AC PlayerbotFactory.cpp:180-247 GetTrainerSpellTradeSkill + IsTrainerSpellAllowedForBot): the trainer sweep below scans
// every trainer, profession trainers included. Without AC's filter every random bot learned the first two professions the
// sweep reached (164 Blacksmithing + 171 Alchemy on every t745 / t843 bot) instead of its stored firstSkill / secondSkill pair.
namespace { bool IsPrimaryTradeSkill(uint16 skillId); bool IsSecondaryTradeSkill(uint16 skillId); }

static uint16 GetTrainerSpellTradeSkill(TrainerSpell const* trainerSpell)
{
    if (trainerSpell->reqSkill && (IsPrimaryTradeSkill(uint16(trainerSpell->reqSkill)) || IsSecondaryTradeSkill(uint16(trainerSpell->reqSkill))))
        return uint16(trainerSpell->reqSkill);

    // 5.4.8 TrainerSpell carries the spells it teaches (learnedSpell[]); the profession is the trade skill line of one of them.
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        uint32 const spellId = trainerSpell->learnedSpell[i] ? trainerSpell->learnedSpell[i] : (i == 0 ? trainerSpell->spell : 0);
        if (!spellId)
            continue;

        SkillLineAbilityMapBounds const bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);
        for (SkillLineAbilityMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
        {
            uint16 const skillId = uint16(itr->second->SkillLine);
            if (IsPrimaryTradeSkill(skillId) || IsSecondaryTradeSkill(skillId))
                return skillId;
        }
    }

    return 0;
}

static bool IsTrainerSpellAllowedForBot(Player* bot, TrainerSpellData const* trainer, TrainerSpell const* trainerSpell)
{
    if (trainer->trainerType != TRAINER_TYPE_TRADESKILLS || !sRandomPlayerbotMgr->IsRandomBot(bot))
        return true;

    uint16 const skillId = GetTrainerSpellTradeSkill(trainerSpell);
    if (!skillId)
        return false;

    if (IsSecondaryTradeSkill(skillId))
        return true;

    if (!IsPrimaryTradeSkill(skillId))
        return false;

    uint16 const firstSkill = uint16(sRandomPlayerbotMgr->GetValue(bot, "firstSkill"));
    uint16 const secondSkill = uint16(sRandomPlayerbotMgr->GetValue(bot, "secondSkill"));

    if ((IsPrimaryTradeSkill(firstSkill) && skillId == firstSkill) || (IsPrimaryTradeSkill(secondSkill) && skillId == secondSkill))
        return true;

    if (IsPrimaryTradeSkill(firstSkill) || IsPrimaryTradeSkill(secondSkill))
        return false;

    // No stored pair: only ranks of a profession the bot already has (AC :244-246).
    return bot->HasSkill(skillId);
}

static void InitAvailableSpells(Player* bot)
{
    if (!sPlayerbotAIConfig->autoLearnTrainerSpells)
        return;

    // One creature-template scan per thread, not per bot. AC keys this cache by class
    // (PlayerbotFactory.cpp:3447) because it uses the trainer's own class mask; CreatureTemplate on
    // this tree carries no trainer_type/trainer_spell fields, and GetTrainerSpellState already does
    // the per-class filtering at learn time, so the trainer list is class independent and a single
    // shared cache is strictly cheaper. thread_local because Refresh() is not only reached from the
    // randomizer. TrainerSpellData instances live in ObjectMgr's cache store for the process
    // lifetime, so the cached pointers stay valid.
    static thread_local std::vector<TrainerSpellData const*> trainers;
    static thread_local bool scanned = false;

    if (!scanned)
    {
        scanned = true;

        CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates();
        for (CreatureTemplateContainer::const_iterator itr = creatures->begin(); itr != creatures->end(); ++itr)
        {
            CreatureTemplate const* ct = &itr->second;
            if (!(ct->npcflag & UNIT_NPC_FLAG_TRAINER))
                continue;

            if (TrainerSpellData const* data = sObjectMgr->GetNpcTrainerSpells(ct->Entry))
                trainers.push_back(data);
        }
    }

    size_t const before = bot->GetSpellMap().size();

    // Player::CastSpell only queues the cast, it does not apply the spell inline, so GetTrainerSpellState
    // would still report the next pass as GREEN. Remember what has been cast to keep the loop below from
    // queueing the same trainer spell again.
    std::set<uint32> casted;
    uint32 learned = 0;

    for (uint32 pass = 0; pass < 8; ++pass)
    {
        uint32 learnedThisPass = 0;

        for (size_t t = 0; t < trainers.size(); ++t)
        {
            for (TrainerSpellMap::const_iterator ref = trainers[t]->spellList.begin(); ref != trainers[t]->spellList.end(); ++ref)
            {
                TrainerSpell const* trainerSpell = &ref->second;
                if (bot->GetTrainerSpellState(trainerSpell) != TRAINER_SPELL_GREEN)
                    continue;
                if (!IsTrainerSpellAllowedForBot(bot, trainers[t], trainerSpell))   // #745, AC :3481
                    continue;

                if (trainerSpell->IsCastable())
                {
                    if (!casted.insert(trainerSpell->spell).second)
                        continue;

                    bot->CastSpell(bot, trainerSpell->spell, true);
                }
                else
                {
                    bot->LearnSpell(trainerSpell->spell, false);
                }

                ++learnedThisPass;
                ++learned;
            }
        }

        if (!learnedThisPass)
            break;
    }

    TC_LOG_DEBUG("playerbots", "[SPELLS] %s class=%u level=%u trainerSpells +%u spellbook %zu->%zu (gated by AiPlayerbot.AutoLearnTrainerSpells)",
                 bot->GetName().c_str(), uint32(bot->GetClass()), uint32(bot->GetLevel()), learned, before,
                 bot->GetSpellMap().size());
}

/// Phase 1.1 (issue #59). Bots were logging in with an empty spellbook: this core does not grant
/// class abilities to a freshly leveled bot (the specialization-spell DBC path yields nothing here,
/// and the default-skill path never learned the class spell lines). Walk every SkillRaceClassInfo
/// entry for the bot's class and let the core learn the spells bound to that skill line - the same
/// call SetSkill makes when a real player's skill rises. LearnSkillRewardedSpells is idempotent and
/// filters by race, class and level internally.
void BotFactory::InitClassSpells()
{
    size_t const before = bot->GetSpellMap().size();
    uint32 const classMask = 1 << (bot->GetClass() - 1);

    for (uint32 i = 0; i < sSkillRaceClassInfoStore.GetNumRows(); ++i)
    {
        SkillRaceClassInfoEntry const* entry = sSkillRaceClassInfoStore.LookupEntry(i);
        if (!entry || entry->Availability != 1)
            continue;

        if (!(entry->ClassMask & classMask))
            continue;

        bot->LearnSkillRewardedSpells(entry->SkillID, 1000);
    }

    // issue #62 / #102 / #745: riding (InitMountSpells) and the professions are AC's InitSkills now (BotFactory::InitSkills),
    // which every caller runs first, as AC does (Randomize :853-856, Refresh :1068-1070, AutoMaintenanceOnLevelupAction
    // LearnTrainerSpells :80-83), so they are not repeated here.

    // issue #230: AC's InitAvailableSpells - teaches every
    // trainer spell the bot is eligible for. It has to run after InitSkills (profession trainer
    // spells are gated on reqSkill by GetTrainerSpellState) and before the [SPELLS] line below, so the
    // after-count reflects the trainer sweep too.
    InitAvailableSpells(bot);

    // count the mount spells in the spellbook, so we can tell whether random bots are able to ride.
    uint32 mountSpells = 0;
    for (auto const& entry : bot->GetSpellMap())
    {
        if (!entry.second || entry.second->state == PLAYERSPELL_REMOVED || !entry.second->active)
            continue;
        SpellInfo const* info = sSpellMgr->GetSpellInfo(entry.first);
        if (info && info->Effects[0].ApplyAuraName == SPELL_AURA_MOUNTED)
            ++mountSpells;
    }

    TC_LOG_DEBUG("playerbots", "[SPELLS] %s class=%u level=%u spellbook %zu->%zu specStore=%u mounts=%u",
                 bot->GetName().c_str(), uint32(bot->GetClass()), uint32(bot->GetLevel()), before,
                 bot->GetSpellMap().size(), sSpecializationSpellsStore.GetNumRows(), mountSpells);
}

/// Phase 2 (issue #62, extended for #134 - AC parity). A random bot only knows a mount if its class
/// grants one (paladin, warlock, death knight); and even then, on 5.4.8 the mount cast fails
/// SPELL_FAILED_NOT_HERE without riding skill, because SpellInfo::CheckLocation()'s SPELL_AURA_MOUNTED
/// case requires Unit::GetMountCapability() to find a usable MountCapability.dbc row, which is gated
/// on the bot's SKILL_RIDING (762) value (src/server/game/Spells/SpellInfo.cpp CheckLocation(),
/// src/server/game/Entities/Unit/Unit.cpp Unit::GetMountCapability()). AC ports as
/// PlayerbotFactory::InitSkills (Bot/Factory/PlayerbotFactory.cpp :3277-3288): teach riding in the same
/// four tiers real players train, gated by the same AiPlayerbot.Use*MountAtMinLevel keys/defaults.
///
/// This core's SPELL_EFFECT_SKILL handler (src/server/game/Spells/SpellEffects.cpp Spell::EffectSkill)
/// is an empty stub, so - unlike upstream - learning/casting a riding spell here does NOT by itself
/// raise SKILL_RIDING. This follows the same working pattern the old BotFactory::InitProfessions (issue #102, now #745 InitTradeSkills,
/// below) already uses for gathering skills: SetSkill() for the numeric value GetMountCapability()
/// actually reads, LearnSpell() for the matching spellbook entry/tooltip.
static void InitMountsAC(Player* bot);

void BotFactory::InitMountSpells()
{
    uint32 ridingSkill = 0;
    if (bot->GetLevel() >= sPlayerbotAIConfig->useGroundMountAtMinLevel)
    {
        bot->LearnSpell(33388, false);  // Riding (Apprentice) -> 60% ground
        ridingSkill = 75;
    }
    if (bot->GetLevel() >= sPlayerbotAIConfig->useFastGroundMountAtMinLevel)
    {
        bot->LearnSpell(33391, false);  // Riding (Journeyman) -> 100% ground
        ridingSkill = 150;
    }
    if (bot->GetLevel() >= sPlayerbotAIConfig->useFlyMountAtMinLevel)
    {
        bot->LearnSpell(34090, false);  // Riding (Expert) -> 150% fly, unlocks flying
        ridingSkill = 225;

        // MoP-only (issue #134): 5.4.8 gates flying per continent behind a licence spell on top of
        // riding skill (Player::IsKnowHowFlyIn(), Player.cpp ~27738-27775; confirmed against
        // MountCapability.dbc rows 21-29, which require spell 90267 for Eastern Kingdoms/Kalimdor/
        // Deepholm at this same riding-225 tier). Gated with Expert Riding because that is also when
        // live retail unlocks Cataclysm-zone flying training.
        bot->LearnSpell(90267, false);  // Flight Master's License
    }
    if (bot->GetLevel() >= sPlayerbotAIConfig->useFastFlyMountAtMinLevel)
    {
        bot->LearnSpell(34091, false);  // Riding (Artisan) -> 280% fly
        ridingSkill = 300;
    }
    if (bot->GetLevel() >= 80)
    {
        // MoP-only (issue #134): 5.4.8 trains Master Riding at level 80.
        bot->LearnSpell(90265, false);   // Master Riding -> 310% fly (skill 375; MountCapability.dbc
                                          // rows with RidingSkill=375, e.g. ids 240/243/246/282/283)
        ridingSkill = 375;
    }
    if (bot->GetLevel() >= 90)
    {
        // MoP-only (issue #134): the Pandaria flying licence is level 90 on 5.4.8.
        bot->LearnSpell(115913, false);  // Wisdom of the Four Winds (MountCapability.dbc row 41)
    }

    if (ridingSkill)
        bot->SetSkill(SKILL_RIDING, 0, ridingSkill, ridingSkill);

    // AC parity (Bot/Factory/PlayerbotFactory.cpp InitSpecialSpells): AiPlayerbot.RandomBotSpellIds
    // defaults to "54197" (Cold Weather Flying), which MountCapability.dbc also requires for Northrend
    // (rows 15-17). Learned unconditionally, independent of the level gates above, same as upstream.
    for (uint32 const spellId : sPlayerbotAIConfig->randomBotSpellIds)
        if (sSpellMgr->GetSpellInfo(spellId))
            bot->LearnSpell(spellId, false);

    InitMountsAC(bot);
}

/// #134 AC parity: AC PlayerbotFactory::InitMounts (Bot/Factory/PlayerbotFactory.cpp:3944-4063), same
/// spell lists and level gates. Each of the four tiers (slow/fast ground, slow/fast flying) gets one
/// random mount from its list unless the bot already knows one from that list.
static void InitMountsAC(Player* bot)
{
    if (bot->GetLevel() < sPlayerbotAIConfig->useGroundMountAtMinLevel)
        return;

    std::vector<uint32> slow, fast, fslow, ffast;
    switch (bot->GetRace())
    {
        case RACE_HUMAN:
            slow = {470, 6648, 458, 472};
            fast = {23228, 23227, 23229};
            break;
        case RACE_ORC:
            slow = {6654, 6653, 580};
            fast = {23250, 23252, 23251};
            break;
        case RACE_DWARF:
            slow = {6899, 6777, 6898};
            fast = {23238, 23239, 23240};
            break;
        case RACE_NIGHTELF:
            slow = {10789, 8394, 10793};
            fast = {23219, 23220, 63637};
            break;
        case RACE_UNDEAD_PLAYER:
            slow = {17463, 17464, 17462};
            fast = {17465, 23246, 66846};
            break;
        case RACE_TAUREN:
            slow = {18990, 18989, 64657};
            fast = {23249, 23248, 23247};
            break;
        case RACE_GNOME:
            slow = {10969, 17453, 10873, 17454};
            fast = {23225, 23223, 23222};
            break;
        case RACE_TROLL:
            slow = {10796, 10799, 8395};
            fast = {23241, 23242, 23243};
            break;
        case RACE_DRAENEI:
            slow = {34406, 35711, 35710};
            fast = {35713, 35712, 35714};
            break;
        case RACE_BLOODELF:
            slow = {33660, 35020, 35022, 35018};
            fast = {35025, 35025, 35027};
            break;
        default:
            // MoP-only races (goblin, worgen, pandaren) have no AC list. AC's default branch has the
            // factions swapped (Horde gets the human list); the faction's own list is used here.
            if (bot->GetTeamId() == TEAM_HORDE)
            {
                slow = {6654, 6653, 580};
                fast = {23250, 23252, 23251};
            }
            else
            {
                slow = {470, 6648, 458, 472};
                fast = {23228, 23227, 23229};
            }
            break;
    }

    if (bot->GetTeamId() == TEAM_ALLIANCE)
    {
        fslow = {32235, 32239, 32240};
        ffast = {32242, 32289, 32290, 32292};
    }
    else
    {
        fslow = {32244, 32245, 32243};
        ffast = {32295, 32297, 32246, 32296};
    }

    // Beyond AC (owner 2026-10-14): extra flying mounts from AiPlayerbot.RandomBotFlyingMountsExtra[Alliance|Horde]
    // ("spell" or "spell:minLevel", comma separated). They join the two flying tiers' pick pool below, so
    // random bots fly on any TBC/WotLK/Cataclysm/MoP flyer instead of only AC's gryphons / wind riders.
    // Empty keys (the code default) leave AC's behaviour untouched.
    std::vector<std::pair<uint32, uint32>> extraFlyers;  // spell, min level
    uint32 const level = bot->GetLevel();
    if (level >= sPlayerbotAIConfig->useFlyMountAtMinLevel)
    {
        char const* const keys[2] = {"AiPlayerbot.RandomBotFlyingMountsExtra",
                                     bot->GetTeamId() == TEAM_ALLIANCE ? "AiPlayerbot.RandomBotFlyingMountsExtraAlliance"
                                                                       : "AiPlayerbot.RandomBotFlyingMountsExtraHorde"};
        for (char const* key : keys)
        {
            for (std::string const& token : split(sConfigMgr->GetStringDefault(key, "", true), ','))
            {
                uint32 const spell = uint32(strtoul(token.c_str(), nullptr, 10));
                size_t const colon = token.find(':');
                uint32 const minLevel = colon == std::string::npos ? 0 : uint32(strtoul(token.c_str() + colon + 1, nullptr, 10));
                if (!spell || level < minLevel || !sSpellMgr->GetSpellInfo(spell))
                    continue;
                if (std::find_if(extraFlyers.begin(), extraFlyers.end(),
                                 [spell](std::pair<uint32, uint32> const& e) { return e.first == spell; }) == extraFlyers.end())
                    extraFlyers.emplace_back(spell, minLevel);
            }
        }
    }

    auto const LearnMountSpell = [bot](uint32 spell, uint32 type)
    {
        SpellInfo const* info = sSpellMgr->GetSpellInfo(spell);
        if (!info)
            return;
        bot->LearnSpell(spell, false);
        // MountType 263 (Cloud Serpents) needs the Cloud Serpent Riding licence in MountCapability.dbc
        // (rows 285-303, RequiredSpell 130487); without it the cast fails SPELL_FAILED_NOT_HERE.
        if (info->Effects[0].MiscValueB == 263 && sSpellMgr->GetSpellInfo(130487))
            bot->LearnSpell(130487, false);
        TC_LOG_DEBUG("playerbots", "Bot %s (%u) learned %s mount %u", bot->GetName().c_str(), bot->GetLevel(),
                     type == 0 ? "slow" : (type == 1 ? "fast" : "flying"), spell);
    };

    std::vector<uint32> const* const tiers[4] = {&slow, &fast, &fslow, &ffast};
    uint32 const minLevel[4] = {sPlayerbotAIConfig->useGroundMountAtMinLevel,
                                sPlayerbotAIConfig->useFastGroundMountAtMinLevel,
                                sPlayerbotAIConfig->useFlyMountAtMinLevel,
                                sPlayerbotAIConfig->useFastFlyMountAtMinLevel};
    for (uint32 type = 0; type < 4; ++type)
    {
        std::vector<uint32> const& list = *tiers[type];
        if (list.empty() || bot->GetLevel() < minLevel[type])
            continue;

        bool hasMount = false;
        for (uint32 spell : list)
        {
            if (bot->HasSpell(spell))
            {
                hasMount = true;
                break;
            }
        }
        if (hasMount)
            continue;

        // Flying tiers (2, 3) pick uniformly from AC's list plus the configured extras not yet known.
        std::vector<uint32> pool = list;
        if (type >= 2)
            for (auto const& extra : extraFlyers)
                if (!bot->HasSpell(extra.first))
                    pool.push_back(extra.first);
        LearnMountSpell(pool[urand(0, pool.size() - 1)], type);
    }

    // Beyond AC: a bot that already has both AC flyers (every bot created before the extras existed) and no
    // extra at all gets one extra flyer, so existing bots show the new variety on their next Refresh().
    if (!extraFlyers.empty())
    {
        bool hasExtra = false;
        for (auto const& extra : extraFlyers)
            if (bot->HasSpell(extra.first))
            {
                hasExtra = true;
                break;
            }
        if (!hasExtra)
            LearnMountSpell(extraFlyers[urand(0, extraFlyers.size() - 1)].first, 2);
    }
}

// #745: AC PlayerbotFactory trade skills, ported 1:1 (Bot/Factory/PlayerbotFactory.cpp:57-61 tradeSkills, :143-559 the
// profession helpers, :1591-1611 ClearSkills, :2933-3264 InitTradeSkills + specializations, :3275-3443 InitSkills /
// SetRandomSkill; enums and the pair struct from Bot/Factory/PlayerbotFactory.h:105-160). It replaces the fork's
// InitProfessions (herbalism/mining only, #102), so random bots get AC's profession pairs, specializations and the
// secondary skills. The helpers are file-local (AC has them as private statics) so BotFactory.h only gains InitSkills and
// InitTradeSkills. 5.4.8 differences, each marked below: Archaeology is a secondary skill (MoP-only), Monk is a class
// (MoP-only), the blacksmithing / leatherworking / tailoring specializations no longer exist.
namespace
{
// AC PlayerbotFactory::tradeSkills (AC :57-61). MoP-only: + SKILL_ARCHAEOLOGY, a secondary skill since 4.0.1, so AC's
// ClearSkills / UpdateTradeSkills loops cover it like the other secondaries.
uint32 const BotTradeSkills[] = {SKILL_ALCHEMY,       SKILL_ENCHANTING,     SKILL_SKINNING,    SKILL_TAILORING,
                                 SKILL_LEATHERWORKING, SKILL_ENGINEERING,   SKILL_HERBALISM,   SKILL_INSCRIPTION,
                                 SKILL_MINING,         SKILL_BLACKSMITHING, SKILL_COOKING,     SKILL_FIRST_AID,
                                 SKILL_FISHING,        SKILL_JEWELCRAFTING, SKILL_ARCHAEOLOGY};

// AC PlayerbotFactory.h:105-148. Every id exists in the 5.4.8 Spell.dbc (checked against testserver data/dbc).
enum class ProfessionSpecializationSpell : uint32
{
    Weapon = 9787,
    Armor = 9788,
    Hammer = 17040,
    Axe = 17041,
    Sword = 17039,

    LearnWeapon = 9789,
    LearnArmor = 9790,
    LearnHammer = 39099,
    LearnAxe = 39098,
    LearnSword = 39097,

    Dragon = 10656,
    Elemental = 10658,
    Tribal = 10660,

    LearnDragon = 10657,
    LearnElemental = 10659,
    LearnTribal = 10661,

    Spellfire = 26797,
    Mooncloth = 26798,
    Shadoweave = 26801,

    Goblin = 20222,
    Gnomish = 20219,

    LearnGoblin = 20221,
    LearnGnomish = 20220,

    LearnSpellfire = 26796,
    LearnMooncloth = 26799,
    LearnShadoweave = 26800,

    Transmute = 28672,
    Elixir = 28677,
    Potion = 28675,

    LearnTransmute = 28674,
    LearnElixir = 28678,
    LearnPotion = 28676
};

// AC PlayerbotFactory.h:150-160
enum class ProfessionRollType : uint32
{
    Random = 1,
    Class = 2
};

struct WeightedProfessionPair
{
    uint16 firstSkill;
    uint16 secondSkill;
    uint32 weight;
};

// AC PlayerbotAIConfig.cpp:290-291 classMatchingProfessionChance (AiPlayerbot.ClassMatchingProfessionChance, default 30,
// capped at 100). The fork's PlayerbotAIConfig.h has no member for it, so it is read here with AC's name and default.
uint32 ClassMatchingProfessionChance()
{
    static uint32 const value =
        std::min<uint32>(100, uint32(sConfigMgr->GetIntDefault("AiPlayerbot.ClassMatchingProfessionChance", 30)));
    return value;
}

// AC :143-147
bool IsPrimaryTradeSkill(uint16 skillId)
{
    SkillLineEntry const* skillLine = sSkillLineStore.LookupEntry(skillId);
    return skillLine && skillLine->CategoryID == SKILL_CATEGORY_PROFESSION;
}

// AC :149-160. MoP-only: Archaeology.
bool IsSecondaryTradeSkill(uint16 skillId)
{
    switch (skillId)
    {
        case SKILL_COOKING:
        case SKILL_FIRST_AID:
        case SKILL_FISHING:
        case SKILL_ARCHAEOLOGY:
            return true;
        default:
            return false;
    }
}

// AC :254-265
bool IsGatheringTradeSkill(uint16 skillId)
{
    switch (skillId)
    {
        case SKILL_HERBALISM:
        case SKILL_MINING:
        case SKILL_SKINNING:
            return true;
        default:
            return false;
    }
}

// AC :267-270
bool IsCraftingTradeSkill(uint16 skillId) { return IsPrimaryTradeSkill(skillId) && !IsGatheringTradeSkill(skillId); }

// AC :272-298. MoP-only: Archaeology 78670 (SPELL_EFFECT_SKILL on skill 794, like the other starter spells).
uint32 GetProfessionStarterSpell(uint16 skillId)
{
    static std::pair<uint16, uint32> const ProfessionStarterSpells[] = {
        {SKILL_ALCHEMY, 2259},     {SKILL_BLACKSMITHING, 2018}, {SKILL_COOKING, 2550},       {SKILL_ENCHANTING, 7411},
        {SKILL_ENGINEERING, 4036}, {SKILL_FIRST_AID, 3273},     {SKILL_FISHING, 7620},       {SKILL_HERBALISM, 2366},
        {SKILL_INSCRIPTION, 45357}, {SKILL_JEWELCRAFTING, 25229}, {SKILL_LEATHERWORKING, 2108}, {SKILL_MINING, 2575},
        {SKILL_SKINNING, 8613},    {SKILL_TAILORING, 3908},     {SKILL_ARCHAEOLOGY, 78670}};

    for (auto const& entry : ProfessionStarterSpells)
    {
        if (entry.first == skillId)
            return entry.second;
    }

    return 0;
}

// AC :300-353. MoP-only: Monk (AC falls to the warlock list for an unknown class; a monk wears leather, so it takes the
// druid's leather/hybrid list instead of the cloth one).
std::vector<WeightedProfessionPair> GetClassProfessionPairs(Player* bot)
{
    switch (bot->GetClass())
    {
        case CLASS_WARRIOR:
            return {{SKILL_MINING, SKILL_BLACKSMITHING, 45},
                    {SKILL_MINING, SKILL_ENGINEERING, 30},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 15},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 10}};
        case CLASS_PALADIN:
            return {{SKILL_MINING, SKILL_BLACKSMITHING, 45},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 30},
                    {SKILL_MINING, SKILL_ENGINEERING, 15},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 10}};
        case CLASS_DEATH_KNIGHT:
            return {{SKILL_MINING, SKILL_BLACKSMITHING, 45},
                    {SKILL_MINING, SKILL_ENGINEERING, 35},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 20}};
        case CLASS_HUNTER:
            return {{SKILL_SKINNING, SKILL_LEATHERWORKING, 45},
                    {SKILL_MINING, SKILL_ENGINEERING, 35},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 10},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 10}};
        case CLASS_ROGUE:
            return {{SKILL_SKINNING, SKILL_LEATHERWORKING, 35},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 25},
                    {SKILL_MINING, SKILL_ENGINEERING, 25},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 10},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 5}};
        case CLASS_DRUID:
        case CLASS_MONK:
            return {{SKILL_SKINNING, SKILL_LEATHERWORKING, 35},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 35},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 20},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 10}};
        case CLASS_SHAMAN:
            return {{SKILL_HERBALISM, SKILL_ALCHEMY, 35},
                    {SKILL_SKINNING, SKILL_LEATHERWORKING, 25},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 25},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 15}};
        case CLASS_PRIEST:
            return {{SKILL_TAILORING, SKILL_ENCHANTING, 45},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 30},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 25}};
        case CLASS_MAGE:
            return {{SKILL_TAILORING, SKILL_ENCHANTING, 50},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 25},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 25}};
        case CLASS_WARLOCK:
        default:
            return {{SKILL_TAILORING, SKILL_ENCHANTING, 50},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 25},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 25}};
    }
}

// AC :355-367
std::vector<WeightedProfessionPair> GetRandomProfessionPairs()
{
    return {{SKILL_MINING, SKILL_BLACKSMITHING, 20},
            {SKILL_MINING, SKILL_ENGINEERING, 18},
            {SKILL_MINING, SKILL_JEWELCRAFTING, 16},
            {SKILL_SKINNING, SKILL_LEATHERWORKING, 18},
            {SKILL_HERBALISM, SKILL_ALCHEMY, 18},
            {SKILL_HERBALISM, SKILL_INSCRIPTION, 14},
            {SKILL_TAILORING, SKILL_ENCHANTING, 10},
            {SKILL_HERBALISM, SKILL_MINING, 6},
            {SKILL_HERBALISM, SKILL_SKINNING, 5},
            {SKILL_MINING, SKILL_SKINNING, 5}};
}

// AC :369-391
std::pair<uint16, uint16> ChooseProfessionPair(std::vector<WeightedProfessionPair> const& professionPairs)
{
    uint32 totalWeight = 0;
    for (WeightedProfessionPair const& pair : professionPairs)
        totalWeight += pair.weight;

    if (!totalWeight)
        return {SKILL_HERBALISM, SKILL_ALCHEMY};

    uint32 roll = urand(1, totalWeight);
    for (WeightedProfessionPair const& pair : professionPairs)
    {
        if (roll <= pair.weight)
            return {pair.firstSkill, pair.secondSkill};

        roll -= pair.weight;
    }

    WeightedProfessionPair const& fallback = professionPairs.back();
    return {fallback.firstSkill, fallback.secondSkill};
}

// AC :393-453
uint16 ChooseSingleProfession(std::vector<WeightedProfessionPair> const& professionPairs)
{
    std::vector<std::pair<uint16, uint32>> gatheringSkills;
    std::vector<std::pair<uint16, uint32>> craftingSkills;

    auto addWeightedSkill = [](std::vector<std::pair<uint16, uint32>>& skills, uint16 skillId, uint32 weight)
    {
        for (std::pair<uint16, uint32>& skill : skills)
        {
            if (skill.first == skillId)
            {
                skill.second += weight;
                return;
            }
        }

        skills.push_back({skillId, weight});
    };

    for (WeightedProfessionPair const& pair : professionPairs)
    {
        if (IsGatheringTradeSkill(pair.firstSkill))
            addWeightedSkill(gatheringSkills, pair.firstSkill, pair.weight);
        if (IsCraftingTradeSkill(pair.firstSkill))
            addWeightedSkill(craftingSkills, pair.firstSkill, pair.weight);

        if (IsGatheringTradeSkill(pair.secondSkill))
            addWeightedSkill(gatheringSkills, pair.secondSkill, pair.weight);
        if (IsCraftingTradeSkill(pair.secondSkill))
            addWeightedSkill(craftingSkills, pair.secondSkill, pair.weight);
    }

    std::vector<std::pair<uint16, uint32>>* selectedPool = nullptr;
    if (!gatheringSkills.empty() && !craftingSkills.empty())
        selectedPool = urand(0, 1) == 0 ? &gatheringSkills : &craftingSkills;
    else if (!gatheringSkills.empty())
        selectedPool = &gatheringSkills;
    else if (!craftingSkills.empty())
        selectedPool = &craftingSkills;

    if (!selectedPool || selectedPool->empty())
        return SKILL_HERBALISM;

    uint32 totalWeight = 0;
    for (std::pair<uint16, uint32> const& skill : *selectedPool)
        totalWeight += skill.second;

    if (!totalWeight)
        return selectedPool->front().first;

    uint32 roll = urand(1, totalWeight);
    for (std::pair<uint16, uint32> const& skill : *selectedPool)
    {
        if (roll <= skill.second)
            return skill.first;

        roll -= skill.second;
    }

    return selectedPool->back().first;
}

// AC :455-504
uint16 ChooseComplementaryProfession(std::vector<WeightedProfessionPair> const& professionPairs, uint16 existingSkill)
{
    std::vector<std::pair<uint16, uint32>> candidates;
    uint32 totalWeight = 0;

    for (WeightedProfessionPair const& pair : professionPairs)
    {
        uint16 candidate = 0;
        if (pair.firstSkill == existingSkill)
            candidate = pair.secondSkill;
        else if (pair.secondSkill == existingSkill)
            candidate = pair.firstSkill;

        if (!candidate || candidate == existingSkill)
            continue;

        bool merged = false;
        for (std::pair<uint16, uint32>& existingCandidate : candidates)
        {
            if (existingCandidate.first != candidate)
                continue;

            existingCandidate.second += pair.weight;
            merged = true;
            break;
        }

        if (!merged)
            candidates.push_back({candidate, pair.weight});

        totalWeight += pair.weight;
    }

    if (candidates.empty() || !totalWeight)
    {
        std::pair<uint16, uint16> const fallback = ChooseProfessionPair(professionPairs);
        return fallback.first != existingSkill ? fallback.first : fallback.second;
    }

    uint32 roll = urand(1, totalWeight);
    for (std::pair<uint16, uint32> const& candidate : candidates)
    {
        if (roll <= candidate.second)
            return candidate.first;

        roll -= candidate.second;
    }

    return candidates.back().first;
}

// AC :506-519
uint32 GetStoredOrRandomValue(Player* bot, std::string const& key, uint32 minValue, uint32 maxValue)
{
    uint32 value = sRandomPlayerbotMgr->GetValue(bot, key);
    if (value < minValue || value > maxValue)
    {
        value = urand(minValue, maxValue);
        sRandomPlayerbotMgr->SetValue(bot, key, value);
    }

    return value;
}

// AC :521-530 HasAnySpell (renamed: StatsWeightCalculator.cpp has a global HasAnySpell template).
bool BotHasAnySpell(Player* bot, std::vector<uint32> const& spells)
{
    for (uint32 spellId : spells)
    {
        if (bot->HasSpell(spellId))
            return true;
    }

    return false;
}

// AC :532-544
bool LearnProfessionSpecialization(Player* bot, ProfessionSpecializationSpell knownSpell,
                                   ProfessionSpecializationSpell learnSpell)
{
    uint32 const knownSpellId = static_cast<uint32>(knownSpell);
    uint32 const learnSpellId = static_cast<uint32>(learnSpell);

    if (bot->HasSpell(knownSpellId) || !sSpellMgr->GetSpellInfo(learnSpellId))
        return false;

    bot->CastSpell(bot, learnSpellId, true);
    return bot->HasSpell(knownSpellId);
}

// AC PlayerbotFactory::SetRandomSkill (AC :3425-3443), 1:1: value = max = level * 5, keeping the current step.
void SetRandomSkill(Player* bot, uint32 level, uint16 id)
{
    uint32 maxValue = level * 5;

    // do not let skill go beyond limit even if maxlevel > blizzlike
    // if (level > 60)
    // {
    //     maxValue = (level + 10) * 5;
    // }

    // uint32 value = urand(maxValue - level, maxValue);
    uint32 value = maxValue;

    uint16 step = bot->GetSkillValue(id) ? bot->GetSkillStep(id) : 1;

    // if (!bot->HasSkill(id) || value > curValue)
    bot->SetSkill(id, step, value, maxValue);
}

// AC PlayerbotFactory::ClearSkills (AC :1591-1611), trade-skill half. AC's PLAYER_SKILL_INDEX(0/1) reset is the 5.4.8
// PLAYER_FIELD_PROFESSION_SKILL_LINE pair (Player::SetSkill clears them too, Player.cpp:6354-6358). Not ported: AC's unlearn
// of the default race/class skills, which AC re-learns with LearnDefaultSkills after ClearSpells. This fork keeps the
// spellbook on randomize (#521, no ClearSpells), and removing a class skill line here removes every spell of that line
// (Player::SetSkill, Player.cpp:6345-6349), which nothing would re-learn in the same way.
void ClearTradeSkills(Player* bot)
{
    for (uint32 tradeSkill : BotTradeSkills)
        bot->SetSkill(tradeSkill, 0, 0, 0);

    bot->SetUInt32Value(PLAYER_FIELD_PROFESSION_SKILL_LINE, 0);
    bot->SetUInt32Value(PLAYER_FIELD_PROFESSION_SKILL_LINE + 1, 0);
}

// AC PlayerbotFactory::InitAlchemySpecialization (AC :3098-3126). The three masteries still exist on 5.4.8 (Wowpedia
// "Elixir Master": quest reworked in 5.0.4, not removed); AC's skill/level gate is kept.
bool InitAlchemySpecialization(Player* bot)
{
    if (!bot->HasSkill(SKILL_ALCHEMY) || bot->GetBaseSkillValue(SKILL_ALCHEMY) < 325 || bot->GetLevel() <= 67)
        return false;

    if (BotHasAnySpell(bot, {static_cast<uint32>(ProfessionSpecializationSpell::Transmute),
                             static_cast<uint32>(ProfessionSpecializationSpell::Elixir),
                             static_cast<uint32>(ProfessionSpecializationSpell::Potion)}))
        return false;

    switch (GetStoredOrRandomValue(bot, "alchemySpecialization", 1, 3))
    {
        case 1:
            return LearnProfessionSpecialization(bot, ProfessionSpecializationSpell::Transmute,
                                                 ProfessionSpecializationSpell::LearnTransmute);
        case 2:
            return LearnProfessionSpecialization(bot, ProfessionSpecializationSpell::Elixir,
                                                 ProfessionSpecializationSpell::LearnElixir);
        case 3:
        default:
            return LearnProfessionSpecialization(bot, ProfessionSpecializationSpell::Potion,
                                                 ProfessionSpecializationSpell::LearnPotion);
    }
}

// AC PlayerbotFactory::InitEngineeringSpecialization (AC :3128-3151). Gnomish / Goblin engineering still exist on 5.4.8.
bool InitEngineeringSpecialization(Player* bot)
{
    if (!bot->HasSkill(SKILL_ENGINEERING) || bot->GetBaseSkillValue(SKILL_ENGINEERING) < 200 || bot->GetLevel() < 30)
        return false;

    if (BotHasAnySpell(bot, {static_cast<uint32>(ProfessionSpecializationSpell::Goblin),
                             static_cast<uint32>(ProfessionSpecializationSpell::Gnomish)}))
        return false;

    switch (GetStoredOrRandomValue(bot, "engineeringSpecialization", 1, 2))
    {
        case 1:
            return LearnProfessionSpecialization(bot, ProfessionSpecializationSpell::Goblin,
                                                 ProfessionSpecializationSpell::LearnGoblin);
        case 2:
        default:
            return LearnProfessionSpecialization(bot, ProfessionSpecializationSpell::Gnomish,
                                                 ProfessionSpecializationSpell::LearnGnomish);
    }
}

// AC PlayerbotFactory::InitLeatherworkingSpecialization (AC :3153-3181), InitTailoringSpecialization (AC :3183-3211) and
// InitBlacksmithingSpecialization (AC :3213-3264) are not driven on 5.4.8: Dragonscale/Elemental/Tribal leatherworking,
// Spellfire/Mooncloth/Shadoweave tailoring and Armorsmith/Weaponsmith (+ the Master Hammer/Axe/Swordsmith follow-ups) were
// removed in patch 4.0.1 (Wowpedia MediaWiki API, pages "Dragonscale Leatherworking", "Mooncloth tailoring", "Armorsmith",
// "Weaponsmith": {{Removed|patch=4.0.1}} / {{Classic only|patch=4.0.1}}, read 2026-10-15). Their learn spells still sit in
// the 5.4.8 Spell.dbc (ids above), so casting them would hand a bot a dead specialization. The enum ids are kept for AC
// diff parity; the AC bodies, for reference:
//   leatherworking: HasSkill(SKILL_LEATHERWORKING) && base skill >= 225 && level > 40, none of Dragon/Elemental/Tribal
//     known -> GetStoredOrRandomValue("leatherSpecialization", 1, 3) -> Learn{Dragon,Elemental,Tribal};
//   tailoring: HasSkill(SKILL_TAILORING) && base skill >= 350 && level > 59, none of Spellfire/Mooncloth/Shadoweave
//     known -> GetStoredOrRandomValue("tailorSpecialization", 1, 3) -> Learn{Spellfire,Mooncloth,Shadoweave};
//   blacksmithing: HasSkill(SKILL_BLACKSMITHING) && base skill >= 225, neither Armor nor Weapon known ->
//     GetStoredOrRandomValue("blacksmithSpecialization", 1, 2) -> Learn{Armor,Weapon}; then with Weapon known, base
//     skill >= 250, level > 49 and none of Hammer/Axe/Sword known -> GetStoredOrRandomValue(
//     "blacksmithWeaponSpecialization", 1, 3) -> Learn{Hammer,Axe,Sword}.
}  // namespace

// AC PlayerbotFactory::InitTradeSkills (AC :2933-3087), 1:1. MoP-only: Archaeology joins AC's three secondary skills
// from level 20 (the 5.4.8 trainer teaches it from level 20: npc_trainer spell 80836, reqlevel 20).
void BotFactory::InitTradeSkills()
{
    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return;

    uint32 const maxPrimaryTradeSkills = std::min<uint32>(2, sWorld->getIntConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL));

    uint16 firstSkill = uint16(sRandomPlayerbotMgr->GetValue(bot, "firstSkill"));
    uint16 secondSkill = uint16(sRandomPlayerbotMgr->GetValue(bot, "secondSkill"));
    ProfessionRollType professionRollType =
        static_cast<ProfessionRollType>(sRandomPlayerbotMgr->GetValue(bot, "professionRollType"));

    if (professionRollType != ProfessionRollType::Class && professionRollType != ProfessionRollType::Random)
    {
        professionRollType = urand(1, 100) <= ClassMatchingProfessionChance() ? ProfessionRollType::Class
                                                                               : ProfessionRollType::Random;
        sRandomPlayerbotMgr->SetValue(bot, "professionRollType", static_cast<uint32>(professionRollType));
    }

    std::vector<WeightedProfessionPair> professionPairs = professionRollType == ProfessionRollType::Class
                                                              ? GetClassProfessionPairs(bot)
                                                              : GetRandomProfessionPairs();

    std::vector<uint16> knownPrimarySkills;
    for (uint32 tradeSkill : BotTradeSkills)
    {
        if (IsPrimaryTradeSkill(tradeSkill) && bot->HasSkill(tradeSkill))
            knownPrimarySkills.push_back(static_cast<uint16>(tradeSkill));
    }

    std::vector<uint16> primarySkills;
    auto addPrimarySkill = [&primarySkills, maxPrimaryTradeSkills](uint16 skillId)
    {
        if (!skillId || !IsPrimaryTradeSkill(skillId) || primarySkills.size() >= maxPrimaryTradeSkills)
            return;

        if (std::find(primarySkills.begin(), primarySkills.end(), skillId) == primarySkills.end())
            primarySkills.push_back(skillId);
    };

    auto isKnownPrimarySkill = [&knownPrimarySkills](uint16 skillId)
    {
        return std::find(knownPrimarySkills.begin(), knownPrimarySkills.end(), skillId) != knownPrimarySkills.end();
    };

    if (knownPrimarySkills.empty())
    {
        // A full randomization clears skills before this method. Keep a valid stored assignment stable and relearn it.
        addPrimarySkill(firstSkill);
        addPrimarySkill(secondSkill);
    }
    else if (knownPrimarySkills.size() == 1)
    {
        // The real skill is authoritative. Reuse a stored complement only when the stored pair contains that skill.
        uint16 const knownSkill = knownPrimarySkills.front();
        addPrimarySkill(knownSkill);

        if (firstSkill == knownSkill && secondSkill != knownSkill)
            addPrimarySkill(secondSkill);
        else if (secondSkill == knownSkill && firstSkill != knownSkill)
            addPrimarySkill(firstSkill);
    }
    else
    {
        // For contaminated bots, stored values are trusted only when the corresponding skill is actually present.
        if (isKnownPrimarySkill(firstSkill))
            addPrimarySkill(firstSkill);
        if (isKnownPrimarySkill(secondSkill))
            addPrimarySkill(secondSkill);

        for (uint16 skillId : knownPrimarySkills)
            addPrimarySkill(skillId);
    }

    if (primarySkills.empty() && maxPrimaryTradeSkills == 1)
        addPrimarySkill(ChooseSingleProfession(professionPairs));
    else if (primarySkills.empty() && maxPrimaryTradeSkills >= 2)
    {
        std::pair<uint16, uint16> const professionPair = ChooseProfessionPair(professionPairs);
        addPrimarySkill(professionPair.first);
        addPrimarySkill(professionPair.second);
    }
    else if (primarySkills.size() == 1 && maxPrimaryTradeSkills >= 2)
    {
        addPrimarySkill(ChooseComplementaryProfession(professionPairs, primarySkills.front()));

        // Defensive fallback for an empty or malformed pair table.
        if (primarySkills.size() == 1)
        {
            for (uint32 tradeSkill : BotTradeSkills)
            {
                if (IsPrimaryTradeSkill(tradeSkill) && tradeSkill != primarySkills.front())
                {
                    addPrimarySkill(static_cast<uint16>(tradeSkill));
                    break;
                }
            }
        }
    }

    firstSkill = primarySkills.empty() ? 0 : primarySkills[0];
    secondSkill = primarySkills.size() > 1 ? primarySkills[1] : 0;
    sRandomPlayerbotMgr->SetValue(bot, "firstSkill", firstSkill);
    sRandomPlayerbotMgr->SetValue(bot, "secondSkill", secondSkill);

    for (uint16 skillId : knownPrimarySkills)
    {
        if (std::find(primarySkills.begin(), primarySkills.end(), skillId) == primarySkills.end())
            bot->SetSkill(skillId, 0, 0, 0);
    }

    SetRandomSkill(bot, level, SKILL_FIRST_AID);
    SetRandomSkill(bot, level, SKILL_FISHING);
    SetRandomSkill(bot, level, SKILL_COOKING);

    std::vector<uint16> skillsToLearn = {SKILL_FIRST_AID, SKILL_FISHING, SKILL_COOKING};

    // MoP-only: Archaeology, from level 20.
    if (bot->GetLevel() >= 20)
    {
        SetRandomSkill(bot, level, SKILL_ARCHAEOLOGY);
        skillsToLearn.push_back(SKILL_ARCHAEOLOGY);
    }

    skillsToLearn.insert(skillsToLearn.end(), primarySkills.begin(), primarySkills.end());

    uint32 selectedPrimaryStarterSpells = 0;
    for (uint16 skillId : primarySkills)
    {
        uint32 const starterSpellId = GetProfessionStarterSpell(skillId);
        if (starterSpellId && bot->HasSpell(starterSpellId))
            ++selectedPrimaryStarterSpells;
    }

    bot->SetFreePrimaryProfessions(static_cast<uint16>(
        maxPrimaryTradeSkills > selectedPrimaryStarterSpells ? maxPrimaryTradeSkills - selectedPrimaryStarterSpells : 0));

    for (uint16 skillId : skillsToLearn)
    {
        uint32 const spellId = GetProfessionStarterSpell(skillId);
        if (!spellId || bot->HasSpell(spellId))
            continue;

        if (IsPrimaryTradeSkill(skillId) && !bot->GetFreePrimaryProfessionPoints())
            continue;

        bot->LearnSpell(spellId, false);
    }

    for (uint16 skillId : primarySkills)
        SetRandomSkill(bot, level, skillId);

    bot->SetFreePrimaryProfessions(static_cast<uint16>(
        maxPrimaryTradeSkills > primarySkills.size() ? maxPrimaryTradeSkills - primarySkills.size() : 0));

    // AC PlayerbotFactory::InitTradeSpecializations (AC :3089-3096). Leatherworking, tailoring and blacksmithing are not
    // called on 5.4.8 (removed in 4.0.1, see above).
    InitAlchemySpecialization(bot);
    InitEngineeringSpecialization(bot);

    TC_LOG_DEBUG("playerbots", "[PROF] %s level=%u roll=%u primary=%u/%u",
                 bot->GetName().c_str(), uint32(bot->GetLevel()), static_cast<uint32>(professionRollType),
                 uint32(firstSkill), uint32(secondSkill));
}

// AC PlayerbotFactory::InitSkills (AC :3275-3423). Called where AC calls it: Randomize (AC :853, before InitClassSpells /
// InitAvailableSpells), Refresh (AC :1068) and the level-up maintenance (AutoMaintenanceOnLevelupAction LearnTrainerSpells,
// AC :78-85).
void BotFactory::InitSkills()
{
    bot->UpdateSkillsForLevel();

    // AC :3280-3288 riding (SetSkill(SKILL_RIDING, 0) + the four riding spells by AiPlayerbot.Use*MountAtMinLevel) is this
    // fork's InitMountSpells (#134): same keys and tiers, plus the 5.4.8 riding skill value, flight licences and mounts,
    // because the 5.4.8 Spell::EffectSkill does not set the riding skill.
    InitMountSpells();

    // AC :3290-3398 (SetRandomSkill on every class weapon skill, SKILL_DEFENSE / SKILL_UNARMED / SKILL_LOCKPICKING, and
    // the dual-wield / mail / plate skill values) is not ported: 5.4.8 has no weapon, defense or lockpicking skill values
    // since 4.0.1 (Player::UpdateSkillsForLevel skips weapon skills, Player.cpp:6235), the class proficiencies are the
    // SkillRaceClassInfo lines InitClassSpells learns, and AC's WotLK lists would hand out proficiencies 5.4.8 classes no
    // longer have (bows/guns/crossbows/thrown for warriors and rogues, melee weapons for hunters).

    InitTradeSkills();
    InitInventorySkill(bot);
}

void BotFactory::ClearEverything()
{
    bot->GiveLevel(bot->GetClass() == CLASS_DEATH_KNIGHT ? sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL)
                                                        : sWorld->getIntConfig(CONFIG_START_PLAYER_LEVEL));
    bot->SetUInt32Value(PLAYER_FIELD_XP, 0);
    TC_LOG_INFO("playerbots", "Resetting player...");
    bot->ResetTalents(true);
}
  
ObjectGuid BotFactory::GetRandomBot()
{
    GuidVector guids;
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotAccounts.begin();
        i != sPlayerbotAIConfig->randomBotAccounts.end(); i++)
    {
        uint32 accountId = *i;
        if (!AccountMgr::GetCharactersCount(accountId))
            continue;
 
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARS_BY_ACCOUNT_ID);
        stmt->setUInt32(0, accountId);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        if (!result)
            continue;
 
        do
        {
            Field* fields = result->Fetch();
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt32());
            if (!ObjectAccessor::FindPlayer(guid))
                guids.push_back(guid);
        } while (result->NextRow());
    }
 
    if (guids.empty())
        return ObjectGuid::Empty;
 
    uint32 index = urand(0, guids.size() - 1);
    return guids[index];
}
 
std::vector<InventoryType> BotFactory::GetPossibleInventoryTypeListBySlot(EquipmentSlots slot)
{
    std::vector<InventoryType> ret;
    switch (slot)
    {
    case EQUIPMENT_SLOT_HEAD:
        ret.push_back(INVTYPE_HEAD);
        break;
    case EQUIPMENT_SLOT_NECK:
        ret.push_back(INVTYPE_NECK);
        break;
    case EQUIPMENT_SLOT_SHOULDERS:
        ret.push_back(INVTYPE_SHOULDERS);
        break;
    case EQUIPMENT_SLOT_BODY:
        ret.push_back(INVTYPE_BODY);
        break;
    case EQUIPMENT_SLOT_CHEST:
        ret.push_back(INVTYPE_CHEST);
        ret.push_back(INVTYPE_ROBE);
        break;
    case EQUIPMENT_SLOT_WAIST:
        ret.push_back(INVTYPE_WAIST);
        break;
    case EQUIPMENT_SLOT_LEGS:
        ret.push_back(INVTYPE_LEGS);
        break;
    case EQUIPMENT_SLOT_FEET:
        ret.push_back(INVTYPE_FEET);
        break;
    case EQUIPMENT_SLOT_WRISTS:
        ret.push_back(INVTYPE_WRISTS);
        break;
    case EQUIPMENT_SLOT_HANDS:
        ret.push_back(INVTYPE_HANDS);
        break;
    case EQUIPMENT_SLOT_FINGER1:
    case EQUIPMENT_SLOT_FINGER2:
        ret.push_back(INVTYPE_FINGER);
        break;
    case EQUIPMENT_SLOT_TRINKET1:
    case EQUIPMENT_SLOT_TRINKET2:
        ret.push_back(INVTYPE_TRINKET);
        break;
    case EQUIPMENT_SLOT_BACK:
        ret.push_back(INVTYPE_CLOAK);
        break;
    case EQUIPMENT_SLOT_MAINHAND:
        ret.push_back(INVTYPE_WEAPON);
        ret.push_back(INVTYPE_2HWEAPON);
        ret.push_back(INVTYPE_WEAPONMAINHAND);
        ret.push_back(INVTYPE_RANGED);
        // #621, AC PlayerbotFactory.cpp:5349: RANGEDRIGHT (guns, crossbows) also lands in the 5.4.8 main hand.
        ret.push_back(INVTYPE_RANGEDRIGHT);
        break;
    case EQUIPMENT_SLOT_OFFHAND:
        ret.push_back(INVTYPE_WEAPON);
        ret.push_back(INVTYPE_2HWEAPON);
        ret.push_back(INVTYPE_WEAPONOFFHAND);
        ret.push_back(INVTYPE_SHIELD);
        ret.push_back(INVTYPE_HOLDABLE);
        break;
    case EQUIPMENT_SLOT_RANGED:
        ret.push_back(INVTYPE_RANGED);
        ret.push_back(INVTYPE_RANGEDRIGHT);
        break;
    default:
        break;
    }
    return ret;
}

// AC PlayerbotFactory::InitBags (Bot/Factory/PlayerbotFactory.cpp:2799-2823), verbatim: 51809 in bag slots 19-22.
void BotFactory::InitBags(bool destroyOld)
{
    for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; ++slot)
    {
        uint32 newItemId = 51809;
        Item* old_bag = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (old_bag && old_bag->GetTemplate()->ItemId == newItemId)
            continue;

        uint16 dest;
        if (!CanEquipUnseenItem(slot, dest, newItemId))
            continue;

        if (old_bag && destroyOld)
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);

        if (old_bag)
            continue;

        bot->EquipNewItem(dest, newItemId, true);
    }
}

bool BotFactory::CanEquipUnseenItem(uint8 slot, uint16& dest, uint32 item)
{
    dest = 0;

    if (Item* pItem = Item::CreateItem(item, 1, bot, true))
    {
        InventoryResult result = botAI ? botAI->CanEquipItem(slot, dest, pItem, true, true)
            : bot->CanEquipItem(slot, dest, pItem, true, true);
        pItem->RemoveFromUpdateQueueOf(bot);
        delete pItem;
        return result == EQUIP_ERR_OK;
    }

    return false;
}

bool BotFactory::CanEquipItem(ItemTemplate const* proto)
{
    if (proto->Duration != 0)
        return false;

    if (proto->Bonding == BIND_QUEST /*|| proto->Bonding == BIND_WHEN_USE*/)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return true;

    uint32 requiredLevel = proto->RequiredLevel;
    bool hasItem = bot->HasItemCount(proto->ItemId, 1, false);
    if (!requiredLevel && hasItem)
        return false;

    uint32 level = bot->GetLevel();

    if (requiredLevel > level)
        return false;

    return true;
}

// #619, AC PlayerbotFactory.cpp:52-55 initSlotsOrder: equip the main hand before the off hand. The old loop ran
// slot 18 -> 0, so a staff-only spec (feral/balance druid, elemental shaman) took a holdable/shield first, and the
// two-hander then could not displace it into a full backpack (INV_FULL) - 14 level-90 bots were left with no weapon.
static std::vector<int32> const initSlotsOrder = {EQUIPMENT_SLOT_TRINKET1, EQUIPMENT_SLOT_TRINKET2, EQUIPMENT_SLOT_MAINHAND,
    EQUIPMENT_SLOT_OFFHAND, EQUIPMENT_SLOT_RANGED, EQUIPMENT_SLOT_HEAD, EQUIPMENT_SLOT_SHOULDERS, EQUIPMENT_SLOT_CHEST,
    EQUIPMENT_SLOT_LEGS, EQUIPMENT_SLOT_HANDS, EQUIPMENT_SLOT_NECK, EQUIPMENT_SLOT_BODY, EQUIPMENT_SLOT_WAIST,
    EQUIPMENT_SLOT_FEET, EQUIPMENT_SLOT_WRISTS, EQUIPMENT_SLOT_FINGER1, EQUIPMENT_SLOT_FINGER2, EQUIPMENT_SLOT_BACK};

// #626: AC PlayerbotFactory::GetPreferredArmorType (Bot/Factory/PlayerbotFactory.cpp:726-751); the monk row is the 5.4.8
// class AC does not have.
static uint8 GetPreferredArmorType(uint8 cls)
{
    switch (cls)
    {
        case CLASS_WARRIOR:
        case CLASS_PALADIN:
        case CLASS_DEATH_KNIGHT:
            return ITEM_SUBCLASS_ARMOR_PLATE;

        case CLASS_HUNTER:
        case CLASS_SHAMAN:
            return ITEM_SUBCLASS_ARMOR_MAIL;

        case CLASS_ROGUE:
        case CLASS_DRUID:
        case CLASS_MONK:
            return ITEM_SUBCLASS_ARMOR_LEATHER;

        case CLASS_PRIEST:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
            return ITEM_SUBCLASS_ARMOR_CLOTH;

        default:
            return 0;
    }
}

// #626: AC PlayerbotFactory::InitEquipment (Bot/Factory/PlayerbotFactory.cpp:2281-2659). The candidates come from every
// cached item of the bot's last 10 required levels (AC :2417-2479), and the slot takes the candidate with the highest
// StatsWeightCalculator score (AC :2481-2518). The old picker took the LAST candidate. The quality step-down puts greys
// at the end of the list, so `equip blue` put a grey 1H into a protection warrior's main hand. The 1.2x gate then
// compared that grey against the old two-hander and kept the two-hander, so the shield could never be equipped
// (EQUIP_ERR_2HANDED_EQUIPPED): Shield Slam, Devastate and Shield Block failed with result=31 (wave 12, Sandered).
void BotFactory::InitEquipment(bool incremental, bool second_chance)
{
    // #646, AC PlayerbotFactory.cpp:2327: each candidate keeps its best random property (item id, property id).
    std::unordered_map<uint8, std::vector<std::pair<uint32, int32>>> items;
    // #629: AC builds one StatsWeightCalculator per InitEquipment (PlayerbotFactory.cpp:2328) for the incremental gate below.
    StatsWeightCalculator calculator(bot);
    // AC :2330-2358 (SetPvpSpec + the CC-break trinket for a PvP spec) is not ported: the fork has no
    // RandomPlayerbotMgr::IsSpecPvp (see ItemUsageValue.cpp:305).
    uint32 blevel = bot->GetLevel();
    int32 delta = std::min(blevel, 10u);
    // AC PlayerbotAIConfig.cpp:176 preferClassArmorType (default off). The fork's PlayerbotAIConfig has no member for it, so
    // the key is read here with AC's name and default (the same pattern as RandomGearLoweringChance below).
    bool const preferClassArmorType = sConfigMgr->GetBoolDefault("AiPlayerbot.PreferClassArmorType", false);
    // #817, AC PlayerbotAIConfig.cpp:698 limitGearExpansion (default 1; the conf.dist already ships 1). Read here, not in the header.
    bool const limitGearExpansion = sConfigMgr->GetIntDefault("AiPlayerbot.LimitGearExpansion", 1) != 0;

    for (int32 slot : initSlotsOrder)
    {
        // 5.4.8 has no ranged slot (guns, bows and crossbows go to the main hand, #621).
        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_RANGED)
            continue;

        if (level < 50 && (slot == EQUIPMENT_SLOT_TRINKET1 || slot == EQUIPMENT_SLOT_TRINKET2))
            continue;

        // AC :2367-2368
        if (level < 30 && (slot == EQUIPMENT_SLOT_NECK || slot == EQUIPMENT_SLOT_HEAD))
            continue;

        if (level < 20 && (slot == EQUIPMENT_SLOT_FINGER1 || slot == EQUIPMENT_SLOT_FINGER2))
            continue;

        // AC :2373-2376
        if (level < 5 && (slot != EQUIPMENT_SLOT_MAINHAND) && (slot != EQUIPMENT_SLOT_OFFHAND) &&
            (slot != EQUIPMENT_SLOT_FEET) && (slot != EQUIPMENT_SLOT_LEGS) && (slot != EQUIPMENT_SLOT_CHEST) &&
            (slot != EQUIPMENT_SLOT_RANGED))
            continue;

        // AC :2378-2380: exclude resilience weighting for trinkets
        bool isTrinketSlot = (slot == EQUIPMENT_SLOT_TRINKET1 || slot == EQUIPMENT_SLOT_TRINKET2);
        calculator.SetExcludeResilience(isTrinketSlot);

        Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (oldItem && second_chance)
        {
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
        }

        oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        // #629, AC PlayerbotFactory.cpp:2413-2415: the requested quality, lowered one step with randomGearLoweringChance.
        int32 desiredQuality = itemQuality;
        if (urand(0, 100) < 100 * sConfigMgr->GetFloatDefault("AiPlayerbot.RandomGearLoweringChance", 0.0f) &&
            desiredQuality > ITEM_QUALITY_NORMAL)
            desiredQuality--;

        // AC :2417-2479
        do
        {
            for (uint32 requiredLevel = bot->GetLevel(); requiredLevel > uint32(std::max((int32)bot->GetLevel() - delta, 0));
                 requiredLevel--)
            {
                for (InventoryType inventoryType : GetPossibleInventoryTypeListBySlot((EquipmentSlots)slot))
                {
                    for (uint32 itemId : sRandomItemMgr->GetEquipmentNew(requiredLevel, inventoryType))
                    {
                        uint32 skipProb = 25;
                        if (urand(1, 100) <= skipProb)
                            continue;

                        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                        // #817, AC :2431-2440 limitGearExpansion: disable next-expansion gear by item id (23728 = first TBC
                        // item, 35570 = first WotLK item). The ids are ported as AC has them: a level <= 70 bot never gets a
                        // Cata/MoP item (those ids are above 35570), and a level > 70 bot is unrestricted.
                        if (limitGearExpansion && bot->GetLevel() <= 60 && itemId >= 23728)
                            continue;

                        if (limitGearExpansion && bot->GetLevel() <= 70 && itemId >= 35570 && itemId != 36737 &&
                            itemId != 37739 &&
                            itemId != 37740)  // transition point from TBC -> WOTLK isn't as clear, and there are other
                                              // wearable TBC items above 35570 but nothing of significance
                            continue;

                        if (!proto)
                            continue;

                        // #878 (beyond AC, owner 2026-10-22): extend limitGearExpansion to Cata and MoP gear. Ids interleave on
                        // 5.4.8 and the core has no per-item expansion field, so use RequiredLevel/ItemLevel/id (test DB counts
                        // in the #878 report). Heirlooms (quality 7) and low-level items (ItemLevel < 100, so Worgen/Goblin/
                        // Pandaren starting gear and the Cata old-world revamps) are never cut.
                        if (limitGearExpansion && proto->Quality != ITEM_QUALITY_HEIRLOOM && proto->ItemLevel >= 100)
                        {
                            // bot <= 80: no Cataclysm item (zone gear is RequiredLevel 77-85, ids from 52306; WotLK ids stay < 52000)
                            if (bot->GetLevel() <= 80 && itemId >= 52000 && proto->RequiredLevel >= 77)
                                continue;
                            // bot <= 85: no MoP item (RequiredLevel > 85, or RequiredLevel >= 80 with id >= 79000, past Dragon Soul)
                            if (bot->GetLevel() <= 85 &&
                                (proto->RequiredLevel > 85 || (proto->RequiredLevel >= 80 && itemId >= 79000)))
                                continue;
                        }

                        bool shouldCheckGS = desiredQuality > ITEM_QUALITY_NORMAL;

                        if (shouldCheckGS && gearScoreLimit != 0 &&
                            CalcMixedGearScore(proto->ItemLevel, proto->Quality) > gearScoreLimit)
                        {
                            continue;
                        }
                        if (proto->Class != ITEM_CLASS_WEAPON && proto->Class != ITEM_CLASS_ARMOR)
                            continue;

                        if (proto->Quality != uint32(desiredQuality))
                            continue;

                        // AC :2457-2466 CanEquipArmor / CanEquipWeapon. The fork's spec-aware versions are in
                        // RandomItemManager (armor type, weapon and shield/holdable per spec, item stats).
                        if (!sRandomItemMgr->IsEquipCandidate(bot, proto))
                            continue;

                        if (slot == EQUIPMENT_SLOT_OFFHAND && bot->GetClass() == CLASS_ROGUE &&
                            proto->Class != ITEM_CLASS_WEAPON)
                            continue;

                        // #646, AC :2472-2475: keep the best random property of the item's enchantment pool
                        // (StatsWeightCalculator::PickBestRandomPropertyId).
                        int32 bestRandomProp = 0;
                        if (proto->RandomProperty || proto->RandomSuffix)
                            bestRandomProp = calculator.PickBestRandomPropertyId(itemId);
                        items[slot].push_back({itemId, bestRandomProp});
                    }
                }
            }
        } while (items[slot].size() < 25 && desiredQuality-- > ITEM_QUALITY_POOR);

        std::vector<std::pair<uint32, int32>>& ids = items[slot];
        if (ids.empty())
        {
            continue;
        }

        // AC :2481-2518: the highest score wins.
        float bestScoreForSlot = -1;
        uint32 bestItemForSlot = 0;
        int32 bestRandomPropForSlot = 0;
        for (size_t index = 0; index < ids.size(); index++)
        {
            uint32 newItemId = ids[index].first;
            int32 newItemProp = ids[index].second;

            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(newItemId);

            float cur_score = calculator.CalculateItem(newItemId, newItemProp, slot);

            if (cur_score > 0.0f && proto && proto->Class == ITEM_CLASS_ARMOR && preferClassArmorType)
            {
                uint8 preferredArmorType = GetPreferredArmorType(bot->GetClass());
                if (preferredArmorType != 0 && proto->SubClass == preferredArmorType)
                    cur_score *= 3.0f;  // 3x multiplier for preferred armor type
            }

            if (cur_score > bestScoreForSlot)
            {
                // delay heavy check to here
                if (!CanEquipItem(proto))
                    continue;
                uint16 dest;
                if (!CanEquipUnseenItem(slot, dest, newItemId))
                    continue;
                bestScoreForSlot = cur_score;
                bestItemForSlot = newItemId;
                bestRandomPropForSlot = newItemProp;
            }
        }

        if (bestItemForSlot == 0)
        {
            continue;
        }
        uint16 dest;
        if (!CanEquipUnseenItem(slot, dest, bestItemForSlot))
        {
            continue;
        }

        // #629, AC PlayerbotFactory.cpp:2530-2535: an incremental pass only replaces an item when the new one scores at
        // least 1.2x the old one.
        if (incremental && oldItem)
        {
            float old_score = calculator.CalculateItem(oldItem->GetEntry(), oldItem->GetItemRandomPropertyId(), slot);
            if (bestScoreForSlot < 1.2f * old_score)
                continue;
        }
        oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        uint32 const oldEntry = oldItem ? oldItem->GetEntry() : 0;
        // fail to store in bag
        if (oldItem)
        {
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
            //continue;
        }
        Item* newItem = bot->EquipNewItem(dest, bestItemForSlot, true);
        // #646, AC :2553-2562: stamp the chosen random property on the equipped item, with its mods re-applied.
        if (newItem && bestRandomPropForSlot != 0)
        {
            uint8 equipSlot = newItem->GetSlot();
            bot->_ApplyItemMods(newItem, equipSlot, false);
            newItem->SetItemRandomProperties(bestRandomPropForSlot);
            bot->_ApplyItemMods(newItem, equipSlot, true);
        }
        // #622: the old item is destroyed before the new one is equipped, and the result was never checked - when
        // Player::EquipNewItem refused the replacement the slot stayed empty (rogues left without a main hand).
        // Put the old item back so an incremental pass can never take a weapon away.
        if (!newItem && oldEntry && !bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            uint16 oldDest;
            if (CanEquipUnseenItem(slot, oldDest, oldEntry))
                bot->EquipNewItem(oldDest, oldEntry, true);
        }
        bot->AutoUnequipOffhandIfNeed();
    }

    // Secondary init for better equips
    /// @todo: clean up duplicate code
    if (second_chance)
    {
        for (int32 slot : initSlotsOrder)
        {
            if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
                continue;

            if (level < 50 && (slot == EQUIPMENT_SLOT_TRINKET1 || slot == EQUIPMENT_SLOT_TRINKET2))
                continue;

            // AC :2583-2584
            if (level < 30 && (slot == EQUIPMENT_SLOT_NECK || slot == EQUIPMENT_SLOT_HEAD))
                continue;

            if (level < 20 && (slot == EQUIPMENT_SLOT_FINGER1 || slot == EQUIPMENT_SLOT_FINGER2))
                continue;

            // AC :2589-2592
            if (level < 5 && (slot != EQUIPMENT_SLOT_MAINHAND) && (slot != EQUIPMENT_SLOT_OFFHAND) &&
                (slot != EQUIPMENT_SLOT_FEET) && (slot != EQUIPMENT_SLOT_LEGS) && (slot != EQUIPMENT_SLOT_CHEST) &&
                (slot != EQUIPMENT_SLOT_RANGED))
                continue;

            bool isTrinketSlot = (slot == EQUIPMENT_SLOT_TRINKET1 || slot == EQUIPMENT_SLOT_TRINKET2);
            calculator.SetExcludeResilience(isTrinketSlot);

            if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);

            std::vector<std::pair<uint32, int32>>& ids = items[slot];
            if (ids.empty())
                continue;

            // #626, AC :2608-2639: the highest score wins (see the main pass).
            float bestScoreForSlot = -1;
            uint32 bestItemForSlot = 0;
            int32 bestRandomPropForSlot = 0;
            for (size_t index = 0; index < ids.size(); index++)
            {
                uint32 newItemId = ids[index].first;
                int32 newItemProp = ids[index].second;

                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(newItemId);

                float cur_score = calculator.CalculateItem(newItemId, newItemProp, slot);

                if (cur_score > 0.0f && proto && proto->Class == ITEM_CLASS_ARMOR && preferClassArmorType)
                {
                    uint8 preferredArmorType = GetPreferredArmorType(bot->GetClass());
                    if (preferredArmorType != 0 && proto->SubClass == preferredArmorType)
                        cur_score *= 3.0f;  // 3x multiplier for preferred armor type
                }

                if (cur_score > bestScoreForSlot)
                {
                    // delay heavy check to here
                    if (!CanEquipItem(proto))
                        continue;
                    uint16 dest;
                    if (!CanEquipUnseenItem(slot, dest, newItemId))
                        continue;
                    bestScoreForSlot = cur_score;
                    bestItemForSlot = newItemId;
                    bestRandomPropForSlot = newItemProp;
                }
            }

            if (bestItemForSlot == 0)
            {
                continue;
            }
            uint16 dest;
            if (!CanEquipUnseenItem(slot, dest, bestItemForSlot))
            {
                continue;
            }
            Item* newItem = bot->EquipNewItem(dest, bestItemForSlot, true);
            // #646, AC :2646-2655 (second-chance pass): same random-property stamp as the main pass.
            if (newItem && bestRandomPropForSlot != 0)
            {
                uint8 equipSlot = newItem->GetSlot();
                bot->_ApplyItemMods(newItem, equipSlot, false);
                newItem->SetItemRandomProperties(bestRandomPropForSlot);
                bot->_ApplyItemMods(newItem, equipSlot, true);
            }
            bot->AutoUnequipOffhandIfNeed();
        }
    }
}