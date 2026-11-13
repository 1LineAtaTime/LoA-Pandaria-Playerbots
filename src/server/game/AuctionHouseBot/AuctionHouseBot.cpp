/*
* This file is part of the Pandaria 5.4.8 Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "AuctionHouseBot.h"
#include "AuctionHouseBotItemFilter.h"
#include "AccountMgr.h"
#include "AuctionHouseBotBuyer.h"
#include "AuctionHouseBotSeller.h"
#include "AuctionHouseMgr.h"
#include "Config.h"
#include "Containers.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Item.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include <algorithm>
#include <cctype>
#include <unordered_set>

AuctionBotConfig* AuctionBotConfig::instance()
{
    static AuctionBotConfig instance;
    return &instance;
}

bool AuctionBotConfig::Initialize()
{
    GetConfigFromFile();
    _AHBotCharacters.clear();
    _AHBotCharacterIds.clear();
    for (uint32 i = 0; i < MAX_AUCTION_HOUSE_TYPE; ++i)
        _lastSeller[i] = _lastBuyer[i] = ObjectGuid::Empty;
    _randomActorCount = 0;
    for (auto& actors : _AHBotActorCharacters)
        actors.clear();

    if (!GetConfig(CONFIG_AHBOT_BUYER_ENABLED) && !GetConfig(CONFIG_AHBOT_SELLER_ENABLED))
    {
        TC_LOG_INFO("ahbot", "AHBOT is Disabled.");
        return false;
    }

    if (GetConfig(CONFIG_AHBOT_ALLIANCE_ITEM_AMOUNT_RATIO) == 0 && GetConfig(CONFIG_AHBOT_HORDE_ITEM_AMOUNT_RATIO) == 0 && GetConfig(CONFIG_AHBOT_NEUTRAL_ITEM_AMOUNT_RATIO) == 0 &&
        !GetConfig(CONFIG_AHBOT_BUYER_ALLIANCE_ENABLED) && !GetConfig(CONFIG_AHBOT_BUYER_HORDE_ENABLED) && !GetConfig(CONFIG_AHBOT_BUYER_NEUTRAL_ENABLED))
    {
        TC_LOG_INFO("ahbot", "All feature of AuctionHouseBot are disabled!");
        return false;
    }

    if (GetConfig(CONFIG_AHBOT_ALLIANCE_ITEM_AMOUNT_RATIO) == 0 && GetConfig(CONFIG_AHBOT_HORDE_ITEM_AMOUNT_RATIO) == 0 && GetConfig(CONFIG_AHBOT_NEUTRAL_ITEM_AMOUNT_RATIO) == 0)
        TC_LOG_INFO("ahbot", "AuctionHouseBot SELLER is disabled!");

    if (!GetConfig(CONFIG_AHBOT_BUYER_ALLIANCE_ENABLED) && !GetConfig(CONFIG_AHBOT_BUYER_HORDE_ENABLED) && !GetConfig(CONFIG_AHBOT_BUYER_NEUTRAL_ENABLED))
        TC_LOG_INFO("ahbot", "AuctionHouseBot BUYER is disabled!");

    _itemsPerCycleBoost = GetConfig(CONFIG_AHBOT_ITEMS_PER_CYCLE_BOOST);
    _itemsPerCycleNormal = GetConfig(CONFIG_AHBOT_ITEMS_PER_CYCLE_NORMAL);

    if (uint32 ahBotAccId = GetConfig(CONFIG_AHBOT_ACCOUNT_ID))
    {
        uint32 configuredCharacterGuid = sConfigMgr->GetIntDefault("AuctionHouseBot.GUID", 0);
        // check character count
        if (AccountMgr::GetCharactersCount(GetConfig(CONFIG_AHBOT_ACCOUNT_ID)))
        {
            // find account guids associated with ahbot account
            uint32 count = 0;
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARS_BY_ACCOUNT_ID);
            stmt->setUInt32(0, ahBotAccId);
            if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
            {
                do
                {
                    Field* fields = result->Fetch();
                    uint32 characterGuid = fields[0].GetUInt32();
                    if (!configuredCharacterGuid || characterGuid == configuredCharacterGuid)
                    {
                        _AHBotCharacters.push_back(ObjectGuid::Create<HighGuid::Player>(characterGuid));
                        _AHBotCharacterIds.insert(characterGuid);
                        ++count;
                    }
                } while (result->NextRow());
            }

            if (configuredCharacterGuid && !count)
            {
                TC_LOG_ERROR("ahbot", "AuctionHouseBot character GUID %u is not on account %u; AuctionHouseBot is disabled.", configuredCharacterGuid, ahBotAccId);
                return false;
            }

            TC_LOG_DEBUG("ahbot", "AuctionHouseBot found %u characters", count);
        }
        else
            TC_LOG_WARN("ahbot", "AuctionHouseBot Account ID %u has no associated characters.", ahBotAccId);
    }

    if (sConfigMgr->GetBoolDefault("AuctionHouseBot.UseRandomPlayerbots", false))
    {
        // The Playerbots module creates exactly prefix0 ... prefix(N-1) during
        // config load. Resolve those accounts by name instead of assuming IDs.
        int32 accountCount = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAccountCount", 0);
        std::string prefix = sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotAccountPrefix", "rndbot");
        if (accountCount < 0 || accountCount > 1000)
            accountCount = 0;

        for (int32 i = 0; i < accountCount; ++i)
        {
            std::string accountName = prefix + std::to_string(i);
            std::transform(accountName.begin(), accountName.end(), accountName.begin(), [](unsigned char c) { return char(std::toupper(c)); });
            uint32 accountId = AccountMgr::GetId(accountName);
            if (!accountId)
                continue;

            QueryResult characters = CharacterDatabase.PQuery("SELECT guid,race FROM characters WHERE account=%u", accountId);
            if (!characters)
                continue;

            do
            {
                Field* fields = characters->Fetch();
                ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt32());
                if (!_AHBotCharacterIds.insert(guid.GetCounter()).second)
                    continue;

                _AHBotCharacters.push_back(guid);
                _AHBotActorCharacters[AUCTION_HOUSE_NEUTRAL].push_back(guid);
                uint32 team = Player::TeamForRace(fields[1].GetUInt8());
                if (team == ALLIANCE)
                    _AHBotActorCharacters[AUCTION_HOUSE_ALLIANCE].push_back(guid);
                else if (team == HORDE)
                    _AHBotActorCharacters[AUCTION_HOUSE_HORDE].push_back(guid);
            }
            while (characters->NextRow());
        }

        _randomActorCount = uint32(_AHBotActorCharacters[AUCTION_HOUSE_NEUTRAL].size());
        TC_LOG_INFO("ahbot", "AuctionHouseBot loaded %u random actors (%u Alliance, %u Horde); legacy bot owners remain recognized.",
            uint32(_AHBotActorCharacters[AUCTION_HOUSE_NEUTRAL].size()),
            uint32(_AHBotActorCharacters[AUCTION_HOUSE_ALLIANCE].size()),
            uint32(_AHBotActorCharacters[AUCTION_HOUSE_HORDE].size()));
    }

    if (_AHBotActorCharacters[AUCTION_HOUSE_NEUTRAL].empty())
        for (auto& actors : _AHBotActorCharacters)
            actors = _AHBotCharacters;

    return true;
}

void AuctionBotConfig::SetConfig(AuctionBotConfigUInt32Values index, char const* fieldname, uint32 defvalue)
{
    SetConfig(index, sConfigMgr->GetIntDefault(fieldname, defvalue));

    if (int32(GetConfig(index)) < 0)
    {
        TC_LOG_ERROR("ahbot", "AHBot: %s (%i) can't be negative. Using %u instead.", fieldname, int32(GetConfig(index)), defvalue);
        SetConfig(index, defvalue);
    }
}

void AuctionBotConfig::SetConfigMax(AuctionBotConfigUInt32Values index, char const* fieldname, uint32 defvalue, uint32 maxvalue)
{
    SetConfig(index, sConfigMgr->GetIntDefault(fieldname, defvalue));

    if (GetConfig(index) > maxvalue)
    {
        TC_LOG_ERROR("ahbot", "AHBot: %s (%u) must be in range 0...%u. Using %u instead.", fieldname, GetConfig(index), maxvalue, maxvalue);
        SetConfig(index, maxvalue);
    }
}

void AuctionBotConfig::SetConfigMinMax(AuctionBotConfigUInt32Values index, char const* fieldname, uint32 defvalue, uint32 minvalue, uint32 maxvalue)
{
    SetConfig(index, sConfigMgr->GetIntDefault(fieldname, defvalue));

    if (GetConfig(index) > maxvalue)
    {
        TC_LOG_ERROR("ahbot", "AHBot: %s (%u) must be in range %u...%u. Using %u instead.", fieldname, GetConfig(index), minvalue, maxvalue, maxvalue);
        SetConfig(index, maxvalue);
    }

    if (GetConfig(index) < minvalue)
    {
        TC_LOG_ERROR("ahbot", "AHBot: %s (%u) must be in range %u...%u. Using %u instead.", fieldname, GetConfig(index), minvalue, maxvalue, minvalue);
        SetConfig(index, minvalue);
    }
}

void AuctionBotConfig::SetConfig(AuctionBotConfigBoolValues index, char const* fieldname, bool defvalue)
{
    SetConfig(index, sConfigMgr->GetBoolDefault(fieldname, defvalue));
}

void AuctionBotConfig::SetConfig(AuctionBotConfigFloatValues index, char const* fieldname, float defvalue)
{
    SetConfig(index, sConfigMgr->GetFloatDefault(fieldname, defvalue));
}

//Get AuctionHousebot configuration file
void AuctionBotConfig::GetConfigFromFile()
{
    SetConfig(CONFIG_AHBOT_ACCOUNT_ID, "AuctionHouseBot.Account", 0);

    SetConfigMax(CONFIG_AHBOT_ALLIANCE_ITEM_AMOUNT_RATIO, "AuctionHouseBot.Alliance.Items.Amount.Ratio", 100, 10000);
    SetConfigMax(CONFIG_AHBOT_HORDE_ITEM_AMOUNT_RATIO, "AuctionHouseBot.Horde.Items.Amount.Ratio", 100, 10000);
    SetConfigMax(CONFIG_AHBOT_NEUTRAL_ITEM_AMOUNT_RATIO, "AuctionHouseBot.Neutral.Items.Amount.Ratio", 100, 10000);

    SetAHBotIncludes(sConfigMgr->GetStringDefault("AuctionHouseBot.forceIncludeItems", ""));
    SetAHBotExcludes(sConfigMgr->GetStringDefault("AuctionHouseBot.forceExcludeItems", ""));

    SetConfig(CONFIG_AHBOT_BUYER_ALLIANCE_ENABLED, "AuctionHouseBot.Buyer.Alliance.Enabled", false);
    SetConfig(CONFIG_AHBOT_BUYER_HORDE_ENABLED, "AuctionHouseBot.Buyer.Horde.Enabled", false);
    SetConfig(CONFIG_AHBOT_BUYER_NEUTRAL_ENABLED, "AuctionHouseBot.Buyer.Neutral.Enabled", false);

    SetConfig(CONFIG_AHBOT_BUYER_CHANCE_FACTOR, "AuctionHouseBot.Buyer.ChanceFactor", 2.0f);

    SetConfig(CONFIG_AHBOT_ITEMS_VENDOR, "AuctionHouseBot.Items.Vendor", false);
    SetConfig(CONFIG_AHBOT_ITEMS_LOOT, "AuctionHouseBot.Items.Loot", true);
    SetConfig(CONFIG_AHBOT_ITEMS_MISC, "AuctionHouseBot.Items.Misc", false);

    SetConfig(CONFIG_AHBOT_BIND_NO, "AuctionHouseBot.Bind.No", true);
    SetConfig(CONFIG_AHBOT_BIND_PICKUP, "AuctionHouseBot.Bind.Pickup", false);
    SetConfig(CONFIG_AHBOT_BIND_EQUIP, "AuctionHouseBot.Bind.Equip", true);
    SetConfig(CONFIG_AHBOT_BIND_USE, "AuctionHouseBot.Bind.Use", true);
    SetConfig(CONFIG_AHBOT_BIND_QUEST, "AuctionHouseBot.Bind.Quest", false);
    SetConfig(CONFIG_AHBOT_LOCKBOX_ENABLED, "AuctionHouseBot.LockBox.Enabled", false);

    SetConfig(CONFIG_AHBOT_BUYPRICE_SELLER, "AuctionHouseBot.BuyPrice.Seller", true);

    SetConfig(CONFIG_AHBOT_ITEMS_PER_CYCLE_BOOST, "AuctionHouseBot.ItemsPerCycle.Boost", 1000);
    SetConfig(CONFIG_AHBOT_ITEMS_PER_CYCLE_NORMAL, "AuctionHouseBot.ItemsPerCycle.Normal", 20);

    SetConfig(CONFIG_AHBOT_ITEM_MIN_ITEM_LEVEL, "AuctionHouseBot.Items.ItemLevel.Min", 0);
    SetConfig(CONFIG_AHBOT_ITEM_MAX_ITEM_LEVEL, "AuctionHouseBot.Items.ItemLevel.Max", 0);
    SetConfig(CONFIG_AHBOT_ITEM_MIN_REQ_LEVEL, "AuctionHouseBot.Items.ReqLevel.Min", 0);
    SetConfig(CONFIG_AHBOT_ITEM_MAX_REQ_LEVEL, "AuctionHouseBot.Items.ReqLevel.Max", 0);
    SetConfig(CONFIG_AHBOT_ITEM_MIN_SKILL_RANK, "AuctionHouseBot.Items.ReqSkill.Min", 0);
    SetConfig(CONFIG_AHBOT_ITEM_MAX_SKILL_RANK, "AuctionHouseBot.Items.ReqSkill.Max", 0);

    SetConfig(CONFIG_AHBOT_ITEM_GRAY_AMOUNT, "AuctionHouseBot.Items.Amount.Gray", 0);
    SetConfig(CONFIG_AHBOT_ITEM_WHITE_AMOUNT, "AuctionHouseBot.Items.Amount.White", 2000);
    SetConfig(CONFIG_AHBOT_ITEM_GREEN_AMOUNT, "AuctionHouseBot.Items.Amount.Green", 2500);
    SetConfig(CONFIG_AHBOT_ITEM_BLUE_AMOUNT, "AuctionHouseBot.Items.Amount.Blue", 1500);
    SetConfig(CONFIG_AHBOT_ITEM_PURPLE_AMOUNT, "AuctionHouseBot.Items.Amount.Purple", 500);
    SetConfig(CONFIG_AHBOT_ITEM_ORANGE_AMOUNT, "AuctionHouseBot.Items.Amount.Orange", 0);
    SetConfig(CONFIG_AHBOT_ITEM_YELLOW_AMOUNT, "AuctionHouseBot.Items.Amount.Yellow", 0);

    SetConfigMax(CONFIG_AHBOT_CLASS_CONSUMABLE_PRIORITY, "AuctionHouseBot.Class.Consumable", 6, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_CONTAINER_PRIORITY, "AuctionHouseBot.Class.Container", 4, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_WEAPON_PRIORITY, "AuctionHouseBot.Class.Weapon", 8, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_GEM_PRIORITY, "AuctionHouseBot.Class.Gem", 3, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_ARMOR_PRIORITY, "AuctionHouseBot.Class.Armor", 8, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_REAGENT_PRIORITY, "AuctionHouseBot.Class.Reagent", 1, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_PROJECTILE_PRIORITY, "AuctionHouseBot.Class.Projectile", 2, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_TRADEGOOD_PRIORITY, "AuctionHouseBot.Class.TradeGood", 10, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_GENERIC_PRIORITY, "AuctionHouseBot.Class.Generic", 1, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_RECIPE_PRIORITY, "AuctionHouseBot.Class.Recipe", 6, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_QUIVER_PRIORITY, "AuctionHouseBot.Class.Quiver", 1, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_QUEST_PRIORITY, "AuctionHouseBot.Class.Quest", 1, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_KEY_PRIORITY, "AuctionHouseBot.Class.Key", 1, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_MISC_PRIORITY, "AuctionHouseBot.Class.Misc", 5, 10);
    SetConfigMax(CONFIG_AHBOT_CLASS_GLYPH_PRIORITY, "AuctionHouseBot.Class.Glyph", 3, 10);

    SetConfig(CONFIG_AHBOT_ALLIANCE_PRICE_RATIO, "AuctionHouseBot.Alliance.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_HORDE_PRICE_RATIO, "AuctionHouseBot.Horde.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_NEUTRAL_PRICE_RATIO, "AuctionHouseBot.Neutral.Price.Ratio", 100);

    SetConfig(CONFIG_AHBOT_ITEM_GRAY_PRICE_RATIO, "AuctionHouseBot.Items.Gray.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_ITEM_WHITE_PRICE_RATIO, "AuctionHouseBot.Items.White.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_ITEM_GREEN_PRICE_RATIO, "AuctionHouseBot.Items.Green.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_ITEM_BLUE_PRICE_RATIO, "AuctionHouseBot.Items.Blue.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_ITEM_PURPLE_PRICE_RATIO, "AuctionHouseBot.Items.Purple.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_ITEM_ORANGE_PRICE_RATIO, "AuctionHouseBot.Items.Orange.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_ITEM_YELLOW_PRICE_RATIO, "AuctionHouseBot.Items.Yellow.Price.Ratio", 100);

    SetConfig(CONFIG_AHBOT_CLASS_CONSUMABLE_PRICE_RATIO, "AuctionHouseBot.Class.Consumable.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_CONTAINER_PRICE_RATIO, "AuctionHouseBot.Class.Container.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_WEAPON_PRICE_RATIO, "AuctionHouseBot.Class.Weapon.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_GEM_PRICE_RATIO, "AuctionHouseBot.Class.Gem.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_ARMOR_PRICE_RATIO, "AuctionHouseBot.Class.Armor.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_REAGENT_PRICE_RATIO, "AuctionHouseBot.Class.Reagent.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_PROJECTILE_PRICE_RATIO, "AuctionHouseBot.Class.Projectile.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_TRADEGOOD_PRICE_RATIO, "AuctionHouseBot.Class.TradeGood.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_GENERIC_PRICE_RATIO, "AuctionHouseBot.Class.Generic.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_RECIPE_PRICE_RATIO, "AuctionHouseBot.Class.Recipe.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_QUIVER_PRICE_RATIO, "AuctionHouseBot.Class.Quiver.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_QUEST_PRICE_RATIO, "AuctionHouseBot.Class.Quest.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_KEY_PRICE_RATIO, "AuctionHouseBot.Class.Key.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_MISC_PRICE_RATIO, "AuctionHouseBot.Class.Misc.Price.Ratio", 100);
    SetConfig(CONFIG_AHBOT_CLASS_GLYPH_PRICE_RATIO, "AuctionHouseBot.Class.Glyph.Price.Ratio", 100);

    SetConfig(CONFIG_AHBOT_CLASS_CONSUMABLE_ALLOW_ZERO, "AuctionHouseBot.Class.Consumable.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_CONTAINER_ALLOW_ZERO, "AuctionHouseBot.Class.Container.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_WEAPON_ALLOW_ZERO, "AuctionHouseBot.Class.Weapon.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_GEM_ALLOW_ZERO, "AuctionHouseBot.Class.Gem.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_ARMOR_ALLOW_ZERO, "AuctionHouseBot.Class.Armor.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_REAGENT_ALLOW_ZERO, "AuctionHouseBot.Class.Reagent.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_PROJECTILE_ALLOW_ZERO, "AuctionHouseBot.Class.Projectile.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_TRADEGOOD_ALLOW_ZERO, "AuctionHouseBot.Class.TradeGood.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_RECIPE_ALLOW_ZERO, "AuctionHouseBot.Class.Recipe.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_QUIVER_ALLOW_ZERO, "AuctionHouseBot.Class.Quiver.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_QUEST_ALLOW_ZERO, "AuctionHouseBot.Class.Quest.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_KEY_ALLOW_ZERO, "AuctionHouseBot.Class.Key.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_MISC_ALLOW_ZERO, "AuctionHouseBot.Class.Misc.Allow.Zero", false);
    SetConfig(CONFIG_AHBOT_CLASS_GLYPH_ALLOW_ZERO, "AuctionHouseBot.Class.Glyph.Allow.Zero", false);

    SetConfig(CONFIG_AHBOT_MINTIME, "AuctionHouseBot.MinTime", 1);
    SetConfig(CONFIG_AHBOT_MAXTIME, "AuctionHouseBot.MaxTime", 72);

    SetConfigMinMax(CONFIG_AHBOT_BUYER_RECHECK_INTERVAL, "AuctionHouseBot.Buyer.Recheck.Interval", 20, 1, DAY / MINUTE);
    SetConfig(CONFIG_AHBOT_BUYER_BASEPRICE_GRAY, "AuctionHouseBot.Buyer.Baseprice.Gray", 3504);
    SetConfig(CONFIG_AHBOT_BUYER_BASEPRICE_WHITE, "AuctionHouseBot.Buyer.Baseprice.White", 5429);
    SetConfig(CONFIG_AHBOT_BUYER_BASEPRICE_GREEN, "AuctionHouseBot.Buyer.Baseprice.Green", 21752);
    SetConfig(CONFIG_AHBOT_BUYER_BASEPRICE_BLUE, "AuctionHouseBot.Buyer.Baseprice.Blue", 36463);
    SetConfig(CONFIG_AHBOT_BUYER_BASEPRICE_PURPLE, "AuctionHouseBot.Buyer.Baseprice.Purple", 87124);
    SetConfig(CONFIG_AHBOT_BUYER_BASEPRICE_ORANGE, "AuctionHouseBot.Buyer.Baseprice.Orange", 214347);
    SetConfig(CONFIG_AHBOT_BUYER_BASEPRICE_YELLOW, "AuctionHouseBot.Buyer.Baseprice.Yellow", 407406);
    SetConfig(CONFIG_AHBOT_BUYER_CHANCEMULTIPLIER_GRAY, "AuctionHouseBot.Buyer.ChanceMultiplier.Gray", 100);
    SetConfig(CONFIG_AHBOT_BUYER_CHANCEMULTIPLIER_WHITE, "AuctionHouseBot.Buyer.ChanceMultiplier.White", 100);
    SetConfig(CONFIG_AHBOT_BUYER_CHANCEMULTIPLIER_GREEN, "AuctionHouseBot.Buyer.ChanceMultiplier.Green", 100);
    SetConfig(CONFIG_AHBOT_BUYER_CHANCEMULTIPLIER_BLUE, "AuctionHouseBot.Buyer.ChanceMultiplier.Blue", 100);
    SetConfig(CONFIG_AHBOT_BUYER_CHANCEMULTIPLIER_PURPLE, "AuctionHouseBot.Buyer.ChanceMultiplier.Purple", 100);
    SetConfig(CONFIG_AHBOT_BUYER_CHANCEMULTIPLIER_ORANGE, "AuctionHouseBot.Buyer.ChanceMultiplier.Orange", 100);
    SetConfig(CONFIG_AHBOT_BUYER_CHANCEMULTIPLIER_YELLOW, "AuctionHouseBot.Buyer.ChanceMultiplier.Yellow", 100);

    SetConfig(CONFIG_AHBOT_SELLER_ENABLED, "AuctionHouseBot.Seller.Enabled", false);
    SetConfig(CONFIG_AHBOT_BUYER_ENABLED, "AuctionHouseBot.Buyer.Enabled", false);

    SetConfig(CONFIG_AHBOT_CLASS_MISC_MOUNT_MIN_REQ_LEVEL, "AuctionHouseBot.Class.Misc.Mount.ReqLevel.Min", 0);
    SetConfig(CONFIG_AHBOT_CLASS_MISC_MOUNT_MAX_REQ_LEVEL, "AuctionHouseBot.Class.Misc.Mount.ReqLevel.Max", 0);
    SetConfig(CONFIG_AHBOT_CLASS_MISC_MOUNT_MIN_SKILL_RANK, "AuctionHouseBot.Class.Misc.Mount.ReqSkill.Min", 0);
    SetConfig(CONFIG_AHBOT_CLASS_MISC_MOUNT_MAX_SKILL_RANK, "AuctionHouseBot.Class.Misc.Mount.ReqSkill.Max", 0);
    SetConfig(CONFIG_AHBOT_CLASS_GLYPH_MIN_REQ_LEVEL, "AuctionHouseBot.Class.Glyph.ReqLevel.Min", 0);
    SetConfig(CONFIG_AHBOT_CLASS_GLYPH_MAX_REQ_LEVEL, "AuctionHouseBot.Class.Glyph.ReqLevel.Max", 0);
    SetConfig(CONFIG_AHBOT_CLASS_GLYPH_MIN_ITEM_LEVEL, "AuctionHouseBot.Class.Glyph.ItemLevel.Min", 0);
    SetConfig(CONFIG_AHBOT_CLASS_GLYPH_MAX_ITEM_LEVEL, "AuctionHouseBot.Class.Glyph.ItemLevel.Max", 0);
    SetConfig(CONFIG_AHBOT_CLASS_TRADEGOOD_MIN_ITEM_LEVEL, "AuctionHouseBot.Class.TradeGood.ItemLevel.Min", 0);
    SetConfig(CONFIG_AHBOT_CLASS_TRADEGOOD_MAX_ITEM_LEVEL, "AuctionHouseBot.Class.TradeGood.ItemLevel.Max", 0);
    SetConfig(CONFIG_AHBOT_CLASS_CONTAINER_MIN_ITEM_LEVEL, "AuctionHouseBot.Class.Container.ItemLevel.Min", 0);
    SetConfig(CONFIG_AHBOT_CLASS_CONTAINER_MAX_ITEM_LEVEL, "AuctionHouseBot.Class.Container.ItemLevel.Max", 0);

    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_CONSUMABLE, "AuctionHouseBot.Class.RandomStackRatio.Consumable", 20);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_CONTAINER, "AuctionHouseBot.Class.RandomStackRatio.Container", 0);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_WEAPON, "AuctionHouseBot.Class.RandomStackRatio.Weapon", 0);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_GEM, "AuctionHouseBot.Class.RandomStackRatio.Gem", 20);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_ARMOR, "AuctionHouseBot.Class.RandomStackRatio.Armor", 0);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_REAGENT, "AuctionHouseBot.Class.RandomStackRatio.Reagent", 100);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_PROJECTILE, "AuctionHouseBot.Class.RandomStackRatio.Projectile", 100);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_TRADEGOOD, "AuctionHouseBot.Class.RandomStackRatio.TradeGood", 50);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_GENERIC, "AuctionHouseBot.Class.RandomStackRatio.Generic", 100);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_RECIPE, "AuctionHouseBot.Class.RandomStackRatio.Recipe", 0);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_QUIVER, "AuctionHouseBot.Class.RandomStackRatio.Quiver", 0);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_QUEST, "AuctionHouseBot.Class.RandomStackRatio.Quest", 100);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_KEY, "AuctionHouseBot.Class.RandomStackRatio.Key", 100);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_MISC, "AuctionHouseBot.Class.RandomStackRatio.Misc", 100);
    SetConfig(CONFIG_AHBOT_CLASS_RANDOMSTACKRATIO_GLYPH, "AuctionHouseBot.Class.RandomStackRatio.Glyph", 0);

    SetConfig(CONFIG_AHBOT_BIDPRICE_MIN, "AuctionHouseBot.BidPrice.Min", 0.6f);
    SetConfig(CONFIG_AHBOT_BIDPRICE_MAX, "AuctionHouseBot.BidPrice.Max", 0.9f);
}

char const* AuctionBotConfig::GetHouseTypeName(AuctionHouseType houseType)
{
    static char const* names[MAX_AUCTION_HOUSE_TYPE] = { "Neutral", "Alliance", "Horde" };
    return names[houseType];
}

// Picks a random character from the list of AHBot chars
ObjectGuid AuctionBotConfig::GetRandChar(AuctionHouseType houseType) const
{
    auto const& actors = _AHBotActorCharacters[houseType];
    if (actors.empty())
        return ObjectGuid::Empty;

    if (_lastSeller[houseType] && urand(0, 99) < 65)
        return _lastSeller[houseType];
    return _lastSeller[houseType] = Trinity::Containers::SelectRandomContainerElement(actors);
}

// Picks a random AHBot character, but excludes a specific one. This is used
// to have another character than the auction owner place bids
ObjectGuid AuctionBotConfig::GetRandCharExclude(ObjectGuid exclude, AuctionHouseType houseType) const
{
    auto const& actors = _AHBotActorCharacters[houseType];
    if (actors.empty())
        return ObjectGuid::Empty;

    std::vector<ObjectGuid> filteredCharacters;
    filteredCharacters.reserve(actors.size());

    for (ObjectGuid charId : actors)
        if (charId != exclude)
            filteredCharacters.push_back(charId);

    if (filteredCharacters.empty())
        return ObjectGuid::Empty;

    if (_lastBuyer[houseType] && _lastBuyer[houseType] != exclude && urand(0, 99) < 65)
        return _lastBuyer[houseType];
    return _lastBuyer[houseType] = Trinity::Containers::SelectRandomContainerElement(filteredCharacters);
}

bool AuctionBotConfig::IsBotChar(ObjectGuid characterID) const
{
    return !characterID || _AHBotCharacterIds.count(characterID.GetCounter()) != 0;
}

uint32 AuctionBotConfig::GetConfigItemAmountRatio(AuctionHouseType houseType) const
{
    switch (houseType)
    {
        case AUCTION_HOUSE_ALLIANCE:
            return GetConfig(CONFIG_AHBOT_ALLIANCE_ITEM_AMOUNT_RATIO);
        case AUCTION_HOUSE_HORDE:
            return GetConfig(CONFIG_AHBOT_HORDE_ITEM_AMOUNT_RATIO);
        default:
            return GetConfig(CONFIG_AHBOT_NEUTRAL_ITEM_AMOUNT_RATIO);
    }
}

uint32 AuctionBotConfig::GetConfigPriceRatio(AuctionHouseType houseType) const
{
    switch (houseType)
    {
        case AUCTION_HOUSE_ALLIANCE:
            return GetConfig(CONFIG_AHBOT_ALLIANCE_PRICE_RATIO);
        case AUCTION_HOUSE_HORDE:
            return GetConfig(CONFIG_AHBOT_HORDE_PRICE_RATIO);
        default:
            return GetConfig(CONFIG_AHBOT_NEUTRAL_PRICE_RATIO);
    }
}

bool AuctionBotConfig::GetConfigBuyerEnabled(AuctionHouseType houseType) const
{
    switch (houseType)
    {
        case AUCTION_HOUSE_ALLIANCE:
            return GetConfig(CONFIG_AHBOT_BUYER_ALLIANCE_ENABLED);
        case AUCTION_HOUSE_HORDE:
            return GetConfig(CONFIG_AHBOT_BUYER_HORDE_ENABLED);
        default:
            return GetConfig(CONFIG_AHBOT_BUYER_NEUTRAL_ENABLED);
    }
}

uint32 AuctionBotConfig::GetConfigItemQualityAmount(AuctionQuality quality) const
{
    switch (quality)
    {
        case AUCTION_QUALITY_GRAY:
            return GetConfig(CONFIG_AHBOT_ITEM_GRAY_AMOUNT);
        case AUCTION_QUALITY_WHITE:
            return GetConfig(CONFIG_AHBOT_ITEM_WHITE_AMOUNT);
        case AUCTION_QUALITY_GREEN:
            return GetConfig(CONFIG_AHBOT_ITEM_GREEN_AMOUNT);
        case AUCTION_QUALITY_BLUE:
            return GetConfig(CONFIG_AHBOT_ITEM_BLUE_AMOUNT);
        case AUCTION_QUALITY_PURPLE:
            return GetConfig(CONFIG_AHBOT_ITEM_PURPLE_AMOUNT);
        case AUCTION_QUALITY_ORANGE:
            return GetConfig(CONFIG_AHBOT_ITEM_ORANGE_AMOUNT);
        default:
            return GetConfig(CONFIG_AHBOT_ITEM_YELLOW_AMOUNT);
    }
}

AuctionHouseBot::AuctionHouseBot(): _buyer(nullptr), _seller(nullptr), _operationSelector(0)
{
}

AuctionHouseBot::~AuctionHouseBot()
{
    delete _buyer;
    delete _seller;
}

void AuctionHouseBot::InitializeAgents()
{
    if (sAuctionBotConfig->GetConfig(CONFIG_AHBOT_SELLER_ENABLED))
    {
        delete _seller;

        _seller = new AuctionBotSeller();
        if (!_seller->Initialize())
        {
            delete _seller;
            _seller = nullptr;
        }
    }

    if (sAuctionBotConfig->GetConfig(CONFIG_AHBOT_BUYER_ENABLED))
    {
        delete _buyer;

        _buyer = new AuctionBotBuyer();
        if (!_buyer->Initialize())
        {
            delete _buyer;
            _buyer = nullptr;
        }
    }
}

void AuctionHouseBot::Initialize()
{
    if (sAuctionBotConfig->Initialize())
    {
        ExpireRestrictedBotAuctions();
        InitializeAgents();
    }
}

void AuctionHouseBot::ExpireRestrictedBotAuctions()
{
    uint32 count = 0;
    uint32 refreshedLegacy = 0;
    uint32 legacyGuid = sConfigMgr->GetIntDefault("AuctionHouseBot.GUID", 0);
    uint32 legacyRefreshLimit = std::min<uint32>(1000, std::max<int32>(0, sConfigMgr->GetIntDefault("AuctionHouseBot.RefreshLegacyListings", 0)));
    if (!sConfigMgr->GetBoolDefault("AuctionHouseBot.UseRandomPlayerbots", false) || !sAuctionBotConfig->GetRandomActorCount())
        legacyRefreshLimit = 0;
    int32 maxRequiredLevel = sAuctionBotConfig->GetConfig(CONFIG_AHBOT_ITEM_MAX_REQ_LEVEL);
    std::unordered_set<uint32> visited;
    for (uint32 i = 0; i < MAX_AUCTION_HOUSE_TYPE; ++i)
    {
        AuctionHouseObject* house = sAuctionMgr->GetAuctionsMap(AuctionHouseType(i));
        for (auto itr = house->GetAuctionsBegin(); itr != house->GetAuctionsEnd(); ++itr)
        {
            AuctionEntry* auction = itr->second;
            // Cross-faction auctions can expose the same map through all houses.
            if (!visited.insert(auction->Id).second)
                continue;
            if (!auction->owner || !sAuctionBotConfig->IsBotChar(auction->owner) || auction->bid || auction->bidder)
                continue;
            Item* item = sAuctionMgr->GetAItem(auction->itemGUIDLow);
            if (!item || !item->GetTemplate())
                continue;

            ItemTemplate const* prototype = item->GetTemplate();
            if (AuctionHouseBotItemFilter::HasRestrictedMarker(prototype->Name1) ||
                (maxRequiredLevel && prototype->GetRequiredLevel() > maxRequiredLevel))
            {
                auction->expire_time = GameTime::GetGameTime();
                ++count;
            }
            else if (legacyGuid && auction->owner.GetCounter() == legacyGuid && refreshedLegacy < legacyRefreshLimit)
            {
                // Replace a bounded number of old single-owner listings with
                // new random-character listings through normal expiry/refill.
                auction->expire_time = GameTime::GetGameTime();
                ++refreshedLegacy;
            }
        }
    }
    TC_LOG_INFO("ahbot", "AuctionHouseBot queued %u unbid restricted listings for normal expiration.", count);
    TC_LOG_INFO("ahbot", "AuctionHouseBot queued %u legacy single-owner listings for gradual refresh.", refreshedLegacy);
}

void AuctionHouseBot::SetItemsRatio(uint32 al, uint32 ho, uint32 ne)
{
    if (_seller)
        _seller->SetItemsRatio(al, ho, ne);
}

void AuctionHouseBot::SetItemsRatioForHouse(AuctionHouseType house, uint32 val)
{
    if (_seller)
        _seller->SetItemsRatioForHouse(house, val);
}

void AuctionHouseBot::SetItemsAmount(uint32(&vals)[MAX_AUCTION_QUALITY])
{
    if (_seller)
        _seller->SetItemsAmount(vals);
}

void AuctionHouseBot::SetItemsAmountForQuality(AuctionQuality quality, uint32 val)
{
    if (_seller)
        _seller->SetItemsAmountForQuality(quality, val);
}

void AuctionHouseBot::ReloadAllConfig()
{
    sAuctionBotConfig->Reload();
    InitializeAgents();
}

void AuctionHouseBot::PrepareStatusInfos(AuctionHouseBotStatusInfo& statusInfo)
{
    for (uint32 i = 0; i < MAX_AUCTION_HOUSE_TYPE; ++i)
    {
        statusInfo[i].ItemsCount = 0;

        for (int j = 0; j < MAX_AUCTION_QUALITY; ++j)
            statusInfo[i].QualityInfo[j] = 0;

        AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(AuctionHouseType(i));
        for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = auctionHouse->GetAuctionsBegin(); itr != auctionHouse->GetAuctionsEnd(); ++itr)
        {
            AuctionEntry* auctionEntry = itr->second;
            if (Item* item = sAuctionMgr->GetAItem(auctionEntry->itemGUIDLow))
            {
                ItemTemplate const* prototype = item->GetTemplate();
                if (!auctionEntry->owner || sAuctionBotConfig->IsBotChar(auctionEntry->owner)) // Add only ahbot items
                {
                    if (prototype->GetQuality() < MAX_AUCTION_QUALITY)
                        ++statusInfo[i].QualityInfo[prototype->GetQuality()];

                    ++statusInfo[i].ItemsCount;
                }
            }
        }
    }
}

void AuctionHouseBot::Rebuild(bool all)
{
    for (uint32 i = 0; i < MAX_AUCTION_HOUSE_TYPE; ++i)
    {
        AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(AuctionHouseType(i));
        for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = auctionHouse->GetAuctionsBegin(); itr != auctionHouse->GetAuctionsEnd(); ++itr)
            if (!itr->second->owner || sAuctionBotConfig->IsBotChar(itr->second->owner)) // ahbot auction
                if (all || itr->second->bid == 0)           // expire now auction if no bid or forced
                    itr->second->expire_time = GameTime::GetGameTime();
    }
}

AuctionHouseBot* AuctionHouseBot::instance()
{
    static AuctionHouseBot instance;
    return &instance;
}

void AuctionHouseBot::Update()
{
    // nothing do...
    if (!_buyer && !_seller)
        return;

    // scan all possible update cases until first success
    for (uint32 count = 0; count < 2 * MAX_AUCTION_HOUSE_TYPE; ++count)
    {
        bool successStep = false;

        if (_operationSelector < MAX_AUCTION_HOUSE_TYPE)
        {
            if (_seller)
                successStep = _seller->Update(AuctionHouseType(_operationSelector));
        }
        else
        {
            if (_buyer)
                successStep = _buyer->Update(AuctionHouseType(_operationSelector - MAX_AUCTION_HOUSE_TYPE));
        }

        ++_operationSelector;
        if (_operationSelector >= 2 * MAX_AUCTION_HOUSE_TYPE)
            _operationSelector = 0;

        // one success update per call
        if (successStep)
            break;
    }
}
