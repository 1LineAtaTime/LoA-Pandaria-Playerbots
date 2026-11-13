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

#include "ScriptMgr.h"
#include "GridNotifiersImpl.h"
#include "DatabaseEnv.h"
#include "Chat.h"
#include "BattlePayAccounting.h"
#include "BattlePayMgr.h"
#include "Item.h"
#include "Player.h"
#include "Realm.h"
#include "WorldSession.h"
#include <algorithm>
#include <cctype>


enum BattlePayTokenTrinityStrings 
{
    BATTLEPAY_TOKEN_TRINITYSTRING_SUCCESS                             = 30007,    // Thanks for helping the Pandaria 5.4.8 project, you just received donate coins: %f
    BATTLEPAY_TOKEN_TRINITYSTRING_ERR_NOTENOUGH                       = 30008,    // You do not have the necessary token.
    BATTLEPAY_TOKEN_TRINITYSTRING_ERR_INBATTLE                        = 30009,    // You may not use this token whilst you are in combat or present in an arena or battleground.
    BATTLEPAY_TOKEN_TRINITYSTRING_ERR_DISABLED                        = 30010,    // Coins disabled.
};

namespace BattlePay
{
    enum Type:int64
    {
        Coins_1                             = 10000,
        Coins_2                             = 20000,
        Coins_5                             = 50000,
        Coins_10                            = 100000
    };
}

template<int64 Coins>
class battle_pay_token : public ItemScript
{
public:
    battle_pay_token(const char *ScriptName) : ItemScript(ScriptName) { }

    bool OnUse(Player *player, Item *item, const SpellCastTargets &)
    {
        if (player->IsInCombat() || player->InArena() || player->InBattleground())
        {
            ChatHandler(player->GetSession()).PSendSysMessage(player->GetSession()->GetTrinityString(BATTLEPAY_TOKEN_TRINITYSTRING_ERR_INBATTLE));  
        }
        else if (!sWorld->getBoolConfig(CONFIG_WOW_TOKEN))
        {
            ChatHandler(player->GetSession()).PSendSysMessage(player->GetSession()->GetTrinityString(BATTLEPAY_TOKEN_TRINITYSTRING_ERR_DISABLED));
            player->CastSpell(player, 27880, true);
        }
        else
        {
            if (!BattlePayAccounting::CanRedeemPhysicalToken(item->GetCount(), item->GetMaxStackCount()))
            {
                ChatHandler(player->GetSession()).SendSysMessage("This token's stack configuration is not supported by safe redemption. The token has been retained; contact an administrator.");
                return true;
            }
            if (item->GetOwnerGUID() == player->GetGUID())
            {
                if (!player->AddDonateTokenCount(Coins, item->GetGUID().GetCounter()))
                {
                    ChatHandler(player->GetSession()).SendSysMessage("Donation token credit is pending or unavailable. Your token has been retained; retry safely later.");
                    return true;
                }
                uint32 consumed = 1;
                player->DestroyItemCount(item, consumed, true);
                ChatHandler(player->GetSession()).PSendSysMessage(player->GetSession()->GetTrinityString(BATTLEPAY_TOKEN_TRINITYSTRING_SUCCESS), double(Coins)/10000);
                player->SaveToDB();
            }
            else
            {
                ChatHandler(player->GetSession()).PSendSysMessage(player->GetSession()->GetTrinityString(BATTLEPAY_TOKEN_TRINITYSTRING_ERR_NOTENOUGH));
            }
        }
        return true;
    }
};


// A stackable account-currency voucher. Wallet and durable item decrement use
// one InnoDB transaction on the character connection, including a receipt.
class item_orb_of_loyalty : public ItemScript
{
public:
    item_orb_of_loyalty() : ItemScript("item_orb_of_loyalty") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const&) override
    {
        if (!player || !item || item->GetEntry() != 35626 || !player->GetSession() ||
            item->GetOwnerGUID() != player->GetGUID() || !item->GetCount())
            return true;
        auto refuse = [player, item](char const* message, bool reconnect = false)
        {
            ChatHandler(player->GetSession()).SendSysMessage(message);
            player->SendEquipError(EQUIP_ERR_CLIENT_LOCKED_OUT, item, nullptr);
            // The baseline save cleared this item's dirty state. If commit
            // outcome is uncertain, don't keep a stale inventory copy active.
            // Logout saves no changed item row; reconnect loads durable truth.
            if (reconnect)
                player->GetSession()->KickPlayer();
            return true;
        };
        if (player->GetSession()->IsBot() || item->IsInTrade())
            return refuse("Only a player can redeem an Orb of Loyalty from their own inventory.");

        auto const* auth = LoginDatabase.GetConnectionInfo();
        auto const* characters = CharacterDatabase.GetConnectionInfo();
        std::string schema = auth->database;
        if (auth->host != characters->host || auth->port_or_socket != characters->port_or_socket || schema.empty() ||
            !std::all_of(schema.begin(), schema.end(), [](unsigned char c) { return std::isalnum(c) || c == '_'; }))
            return refuse("Orb redemption requires auth and characters on the same database server. Your orb was retained.");

        QueryResult engines = CharacterDatabase.PQuery(
            "SELECT COUNT(*) FROM information_schema.TABLES WHERE ENGINE='InnoDB' AND "
            "((TABLE_SCHEMA=DATABASE() AND TABLE_NAME IN ('item_instance','character_inventory','battlepay_orb_redemption')) "
            "OR (TABLE_SCHEMA='%s' AND TABLE_NAME='account'))", schema.c_str());
        if (!engines || engines->Fetch()[0].GetUInt32() != 4)
            return refuse("Orb redemption storage is not ready. Your orb was retained.");

        auto commit = [](CharacterDatabaseTransaction transaction)
        {
            try { return CharacterDatabase.AsyncCommitTransaction(transaction).m_future.get(); }
            catch (std::exception const&)
            {
                TC_LOG_ERROR("battlepay", "Orb redemption database task raised an exception; durable state will be reloaded");
                return false;
            }
        };
        uint32 account = player->GetSession()->GetAccountId();
        QueryResult balance = CharacterDatabase.PQuery("SELECT COALESCE(dp,0) FROM `%s`.account WHERE id=%u", schema.c_str(), account);
        if (!balance || balance->Fetch()[0].GetInt64() < 0 || balance->Fetch()[0].GetInt64() > INT32_MAX - 100)
            return refuse("Your account cannot receive another 100 Battle Shop Coins. Your orb was retained.");

        uint32 itemGuid = item->GetGUID().GetCounter();
        uint32 characterGuid = player->GetGUID().GetCounter();
        uint32 before = item->GetCount();
        // Flush pending inventory changes first, including newly granted items.
        // A failed baseline must never proceed to a wallet update.
        auto baseline = CharacterDatabase.BeginTransaction();
        player->SaveInventoryAndGoldToDB(baseline);
        if (!commit(baseline))
            return refuse("Orb redemption could not save inventory. Reconnect to reload your saved items.", true);

        auto trans = CharacterDatabase.BeginTransaction();
        std::string credit = Trinity::StringFormat(
            "UPDATE `%s`.account a INNER JOIN item_instance i ON i.guid=%u AND i.owner_guid=%u "
            "INNER JOIN character_inventory ci ON ci.item=i.guid AND ci.guid=i.owner_guid "
            "SET a.dp=COALESCE(a.dp,0)+100,i.count=i.count-1 WHERE a.id=%u AND COALESCE(a.dp,0)>=0 AND COALESCE(a.dp,0)<=2147483547 "
            "AND i.itemEntry=35626 AND i.count=%u AND i.count>0", schema.c_str(), itemGuid, characterGuid, account, before);
        // One wallet row plus one item row must change. The expected count also
        // prevents retrying an already-committed transaction from crediting twice.
        trans->AppendChecked(credit.c_str(), 2);
        trans->PAppend("INSERT INTO battlepay_orb_redemption(realm_id,account_id,character_guid,item_guid,count_before,dp_added) "
            "VALUES(%u,%u,%u,%u,%u,100)", realm.Id.Realm, account, characterGuid, itemGuid, before);
        trans->PAppend("DELETE ci FROM character_inventory ci INNER JOIN item_instance i ON i.guid=ci.item "
            "WHERE i.guid=%u AND i.owner_guid=%u AND i.itemEntry=35626 AND i.count=0", itemGuid, characterGuid);
        trans->PAppend("DELETE FROM item_instance WHERE guid=%u AND owner_guid=%u AND itemEntry=35626 AND count=0", itemGuid, characterGuid);
        if (!commit(trans))
        {
            TC_LOG_ERROR("battlepay", "Orb redemption transaction failed or is uncertain: account %u character %u item %u before %u; reconnect required", account, characterGuid, itemGuid, before);
            return refuse("Orb redemption did not confirm. Reconnect to load the saved wallet and inventory; no automatic compensation was guessed.", true);
        }

        // Publish the inventory change only after the wallet/item commit.
        uint32 consumed = 1;
        player->DestroyItemCount(item, consumed, true);
        ChatHandler(player->GetSession()).SendSysMessage("Added 100 Battle Shop Coins to your account.");
        sBattlePayMgr->SendBattlePayProductList(player->GetSession());
        return true;
    }
};

void AddSC_wow_token()
{
    new item_orb_of_loyalty();
    new battle_pay_token<BattlePay::Coins_1>("wow_token_1");
    new battle_pay_token<BattlePay::Coins_2>("wow_token_2");
    new battle_pay_token<BattlePay::Coins_5>("wow_token_5");
    new battle_pay_token<BattlePay::Coins_10>("wow_token_10");
}
