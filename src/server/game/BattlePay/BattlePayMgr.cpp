/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
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

#include "BattlePayMgr.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "ServiceBoost.h"
#include "BattlePetMgr.h"
#include "Realm.h"
#include "BattlePayAccounting.h"
#include "BattlePayQueries.h"
#include "CryptoRandom.h"
#include "Mail.h"
#include "CharacterCache.h"
#include "World.h"
#include <algorithm>
#include <limits>

#pragma execution_character_set("UTF-8")


BattlePayMgr::BattlePayMgr() : m_enabled(false), m_currency(BATTLE_PAY_CURRENCY_BETA)
{
}

BattlePayMgr::~BattlePayMgr()
{
    for (auto&& product : m_productStore)
        delete product;

    for (auto&& group : m_groupStore)
        delete group;

    for (auto&& entry : m_shopEntryStore)
        delete entry;

    m_productStore.clear();
    m_groupStore.clear();
    m_shopEntryStore.clear();

}

BattlePayMgr* BattlePayMgr::instance()
{
    static BattlePayMgr instance;
    return &instance;
}

void BattlePayMgr::LoadFromDb()
{
    LoadProductsFromDb();
    LoadProductLocalesFromDb();
    LoadProductItemsFromDb();
    LoadGroupsFromDb();
    LoadGroupLocalesFromDb();
    LoadEntriesFromDb();
    LoadEntryLocalesFromDb();
    m_accountingReady = ValidateAccountingSchema();
}

void BattlePayMgr::LoadProductsFromDb()
{
    uint32 oldMSTime = getMSTime();

    for (BattlePayProduct* product : m_productStore)
        delete product;
    m_productStore.clear();

    QueryResult result = WorldDatabase.Query("SELECT id, title, description, icon, price, discount, displayId, type, choiceType, flags, flagsInfo FROM battle_pay_product ORDER BY id ASC");
    if (!result)
    {
        TC_LOG_INFO("sql.sql", ">> Loaded 0 Battle Pay store products, table `battle_pay_product` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 id               = fields[0].GetUInt32();
        std::string title       = fields[1].GetString();
        std::string description = fields[2].GetString();
        uint32 icon             = fields[3].GetUInt32();
        uint32 price            = fields[4].GetUInt32();
        uint32 discount         = fields[5].GetUInt32();
        uint32 displayId        = fields[6].GetUInt32();
        uint8 type              = fields[7].GetUInt8();
        uint8 choiceType        = fields[8].GetUInt8();
        uint32 flags            = fields[9].GetUInt32();
        uint32 flagsInfo        = fields[10].GetUInt32();

        if (GetProductId(id))
        {
            TC_LOG_ERROR("sql.sql", "Product id %u defined in `battle_pay_product` already exists, skipped!", id);
            continue;
        }

        if (title.size() > MAX_BATTLE_PAY_PRODUCT_TITLE_SIZE)
        {
            TC_LOG_ERROR("sql.sql", "Title for product id %u defined in `battle_pay_product` is too large (max %d), skipped!", id, MAX_BATTLE_PAY_PRODUCT_TITLE_SIZE);
            continue;
        }

        if (description.size() > MAX_BATTLE_PAY_PRODUCT_DESCRIPTION_SIZE)
        {
            TC_LOG_ERROR("sql.sql", "Description for product id %u defined in `battle_pay_product` is too large (max %d), skipped!", id, MAX_BATTLE_PAY_PRODUCT_DESCRIPTION_SIZE);
            continue;
        }

        uint64 effectivePrice = 0;
        if (!BattlePayAccounting::Price(price, discount, effectivePrice))
        {
            TC_LOG_ERROR("sql.sql", "Product %u has an invalid discount or exceeds the account.dp range; skipped.", id);
            continue;
        }

        m_productStore.push_back(new BattlePayProduct(id, title, description, icon, price, discount, displayId, type, choiceType, flags, flagsInfo));
        count++;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u Battle Pay store products in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void BattlePayMgr::LoadProductLocalesFromDb()
{
    uint32 oldMSTime = getMSTime();

    m_productLocalesMap.clear();

    QueryResult result = WorldDatabase.Query("SELECT id, title_loc1, description_loc1, title_loc2, description_loc2, title_loc3, description_loc3, title_loc4, description_loc4, title_loc5, description_loc5,"
        " title_loc6, description_loc6, title_loc7, description_loc7, title_loc8, description_loc8, title_loc9, description_loc9, title_loc10, description_loc10, title_loc11, description_loc11"
        " FROM locales_battle_pay_product");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 Battle Pay store products locale strings. DB table `locales_battle_pay_product` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 id = fields[0].GetUInt32();

        if (!GetProductId(id))
        {
            TC_LOG_ERROR("sql.sql", "Table `locales_battle_pay_product` (Entry: %u) has locale strings for non-existing Battle Pay product.", id);
            continue;
        }

        BattlePayProductLocale& data = m_productLocalesMap[id];

        for (int i = 1; i < TOTAL_LOCALES; ++i)
        {
            LocaleConstant locale = (LocaleConstant)i;
            ObjectMgr::AddLocaleString(fields[1 + 2 * (i - 1)].GetString(), locale, data.Title);
            ObjectMgr::AddLocaleString(fields[1 + 2 * (i - 1) + 1].GetString(), locale, data.Description);
        }
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %lu Battle Pay store products locale strings in %u ms", (unsigned long)m_productLocalesMap.size(), GetMSTimeDiffToNow(oldMSTime));
}

void BattlePayMgr::LoadProductItemsFromDb()
{
    uint32 oldMSTime = getMSTime();

    m_productItemsMap.clear();

    QueryResult result = WorldDatabase.Query("SELECT id, itemId, count, productId FROM battle_pay_product_items ORDER BY id ASC");
    if (!result)
    {
        TC_LOG_INFO("sql.sql", ">> Loaded 0 Battle Pay store product items, table `battle_pay_product_items` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 id = fields[0].GetUInt32();
        uint32 itemId = fields[1].GetUInt32();
        uint32 itemCount = fields[2].GetUInt32();
        uint32 productId = fields[3].GetUInt32();

        if (!GetProductId(productId))
        {
            TC_LOG_ERROR("sql.sql", "Product id %u defined in `battle_pay_product_items` not exists, skipped!", productId);
            continue;
        }

        if (itemId && !sObjectMgr->GetItemTemplate(itemId))
        {
            TC_LOG_ERROR("sql.sql", "Item id %u for product id %u defined in `battle_pay_product_items` doesn't exist, skipped!", itemId, productId);
            continue;
        }

        if (!itemCount)
        {
            TC_LOG_ERROR("sql.sql", "BattlePay product %u has a zero item count; skipped.", productId);
            continue;
        }

        m_productItemsMap[productId].push_back(BattlePayProductItem(id, itemId, itemCount));
        count++;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u Battle Pay store product items in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void BattlePayMgr::LoadGroupsFromDb()
{
    uint32 oldMSTime = getMSTime();

    for (BattlePayGroup* group : m_groupStore)
        delete group;
    m_groupStore.clear();

    QueryResult result = WorldDatabase.Query("SELECT id, idx, name, icon, type FROM battle_pay_group ORDER BY type ASC");
    if (!result)
    {
        TC_LOG_INFO("sql.sql", ">> Loaded 0 Battle Pay store groups, table `battle_pay_group` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 id        = fields[0].GetUInt32();
        uint32 index     = fields[1].GetUInt32();
        std::string name = fields[2].GetString();
        uint32 icon      = fields[3].GetUInt32();
        uint8 type       = fields[4].GetUInt8();

        if (HasGroupId(id))
        {
            TC_LOG_ERROR("sql.sql", "Group id %u defined in `battle_pay_group` already exists, skipped!", id);
            continue;
        }

        if (HasGroupName(name))
        {
            TC_LOG_ERROR("sql.sql", "Group name %s defined in `battle_pay_group` is invalid because a group of the same name already exists, skipped!", name.c_str());
            continue;
        }

        if (name.size() > MAX_BATTLE_PAY_GROUP_NAME_SIZE)
        {
            TC_LOG_ERROR("sql.sql", "Name for group id %u defined in `battle_pay_group` is too large (max %d), skipped!",
                id, MAX_BATTLE_PAY_GROUP_NAME_SIZE);

            continue;
        }

        if (type >= BATTLE_PAY_GROUP_TYPE_END)
        {
            TC_LOG_ERROR("sql.sql", "Group id %u defined in `battle_pay_group` has invalid group type %u, skipped!", id, type);
            continue;
        }

        m_groupStore.push_back(new BattlePayGroup(id, index, name, icon, type));
        count++;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u Battle Pay store groups in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void BattlePayMgr::LoadGroupLocalesFromDb()
{
    uint32 oldMSTime = getMSTime();

    m_groupLocalesMap.clear();

    QueryResult result = WorldDatabase.Query("SELECT id, name_loc1, name_loc2, name_loc3, name_loc4, name_loc5, name_loc6, name_loc7, name_loc8, name_loc9, name_loc10, name_loc11"
        " FROM locales_battle_pay_group");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 Battle Pay store groups locale strings. DB table `locales_battle_pay_group` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 id = fields[0].GetUInt32();

        if (!HasGroupId(id))
        {
            TC_LOG_ERROR("sql.sql", "Table `locales_battle_pay_group` (Entry: %u) has locale strings for non-existing Battle Pay group.", id);
            continue;
        }

        BattlePayGroupLocale& data = m_groupLocalesMap[id];

        for (int i = 1; i < TOTAL_LOCALES; ++i)
            ObjectMgr::AddLocaleString(fields[i].GetString(), LocaleConstant(i), data.Name);

    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %lu Battle Pay store groups locale strings in %u ms", (unsigned long)m_groupLocalesMap.size(), GetMSTimeDiffToNow(oldMSTime));
}

void BattlePayMgr::LoadEntriesFromDb()
{
    uint32 oldMSTime = getMSTime();

    for (BattlePayShopEntry* entry : m_shopEntryStore)
        delete entry;
    m_shopEntryStore.clear();

    QueryResult result = WorldDatabase.Query("SELECT id, productId, groupId, idx, title, description, icon, displayId, banner, `flags` FROM battle_pay_entry ORDER BY id ASC");
    if (!result)
    {
        TC_LOG_INFO("sql.sql", ">> Loaded 0 Battle Pay store entries, table `battle_pay_entry` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 id               = fields[0].GetUInt32();
        uint32 productId        = fields[1].GetUInt32();
        uint32 groupId          = fields[2].GetUInt32();
        int32 index             = fields[3].GetInt32();
        std::string title       = fields[4].GetString();
        std::string description = fields[5].GetString();
        uint32 icon             = fields[6].GetUInt32();
        uint32 displayId        = fields[7].GetUInt32();
        uint8 banner           = fields[8].GetUInt8();
        uint32 flags            = fields[9].GetUInt32();

        if (HasEntryId(id))
        {
            TC_LOG_ERROR("sql.sql", "Entry id %u defined in `battle_pay_entry` already exists, skipped!", id);
            continue;
        }

        if (!HasGroupId(groupId))
        {
            TC_LOG_ERROR("sql.sql", "Group id %u for entry id %u defined in `battle_pay_entry` is invalid because the group doesn't exists, skipped!", groupId, id);
            continue;
        }

        if (!GetProductId(productId))
        {
            TC_LOG_ERROR("sql.sql", "Product id %u for entry id %u defined in `battle_pay_entry` is invalid because the group doesn't exists, skipped!", productId, id);
            continue;
        }

        if (banner >= BATTLE_PAY_BANNER_TYPE_END)
        {
            TC_LOG_ERROR("sql.sql", "Entry id %u defined in `battle_pay_entry` has invalid banner type %u, skipped!", id, banner);
            continue;
        }

        m_shopEntryStore.push_back(new BattlePayShopEntry(id, productId, groupId, index, title, description, icon, displayId, banner, flags));
        count++;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u Battle Pay store entries in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void BattlePayMgr::LoadEntryLocalesFromDb()
{
    uint32 oldMSTime = getMSTime();

    m_shopEntryLocalesMap.clear();

    QueryResult result = WorldDatabase.Query("SELECT id, title_loc1, description_loc1, title_loc2, description_loc2, title_loc3, description_loc3, title_loc4, description_loc4, title_loc5, description_loc5,"
        " title_loc6, description_loc6, title_loc7, description_loc7, title_loc8, description_loc8, title_loc9, description_loc9, title_loc10, description_loc10, title_loc11, description_loc11"
        " FROM locales_battle_pay_entry");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 Battle Pay store entries locale strings. DB table `locales_battle_pay_entry` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 id = fields[0].GetUInt32();

        if (!HasEntryId(id))
        {
            TC_LOG_ERROR("sql.sql", "Table `locales_battle_pay_entry` (Entry: %u) has locale strings for non-existing Battle Pay entry.", id);
            continue;
        }

        BattlePayShopEntryLocale& data = m_shopEntryLocalesMap[id];

        for (int i = 1; i < TOTAL_LOCALES; ++i)
        {
            LocaleConstant locale = (LocaleConstant)i;
            ObjectMgr::AddLocaleString(fields[1 + 2 * (i - 1)].GetString(), locale, data.Title);
            ObjectMgr::AddLocaleString(fields[1 + 2 * (i - 1) + 1].GetString(), locale, data.Description);
        }
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %lu Battle Pay store entries locale strings in %u ms", (unsigned long)m_shopEntryLocalesMap.size(), GetMSTimeDiffToNow(oldMSTime));
}

BattlePayProduct* BattlePayMgr::GetProductId(uint32 id)
{
    for (auto&& itr : m_productStore)
        if (itr->Id == id)
            return itr;

    return nullptr;
}

bool BattlePayMgr::HasGroupId(uint32 id)
{
    for (auto&& group : m_groupStore)
        if (group->Id == id)
            return true;

    return false;
}

bool BattlePayMgr::HasGroupName(std::string name)
{
    for (auto&& group : m_groupStore)
        if (group->Name == name)
            return true;

    return false;
}

bool BattlePayMgr::HasEntryId(uint32 id)
{
    for (auto&& itemEntry : m_shopEntryStore)
        if (itemEntry->Id == id)
            return true;

    return false;
}

void BattlePayMgr::SendPointsBalance(WorldSession* session)
{
    uint64 balance = 0;
    if (!ReadBalance(session, balance))
        return;
    if (Player* player = session->GetPlayer())
    {
        player->SendBattlePayMessage("BattlePay balance: " + BattlePayAccounting::Display(balance) + " DP");
    }
}

bool BattlePayMgr::ReadBalance(WorldSession* session, uint64& balance)
{
    if (!session)
        return false;
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BATTLEPAY_COINS);
    stmt->setUInt32(0, session->GetAccountId());
    PreparedQueryResult result = LoginDatabase.Query(stmt);
    if (!result || result->Fetch()[0].IsNull())
        return false;
    int64 value = result->Fetch()[0].GetInt64();
    if (value < 0 || uint64(value) > BattlePayAccounting::MaxBalance)
    {
        TC_LOG_ERROR("battlepay", "Account %u has invalid account.dp; purchases refused.", session->GetAccountId());
        return false;
    }
    balance = uint64(value);
    return true;
}

bool BattlePayMgr::UpdatePointsBalance(WorldSession* session, uint64 points)
{
    if (!session || points > BattlePayAccounting::MaxBalance)
        return false;
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_BATTLEPAY_DECREMENT_COINS);
    stmt->setUInt64(0, points);
    stmt->setUInt32(1, session->GetAccountId());
    stmt->setUInt64(2, points);
    uint64 affected = 0;
    bool success = LoginDatabase.DirectExecuteChecked(stmt, &affected) && affected == 1;
    SendPointsBalance(session);
    return success;
}

bool BattlePayMgr::HasPointsBalance(WorldSession* session, uint64 points)
{
    uint64 balance = 0;
    return ReadBalance(session, balance) && BattlePayAccounting::CanDebit(int64(balance), points);
}

namespace
{
    std::string BattlePayNonce()
    {
        static char const hex[] = "0123456789abcdef";
        auto random = Trinity::Crypto::GetRandomBytes<16>();
        std::string result;
        result.reserve(32);
        for (uint8 byte : random)
        {
            result += hex[byte >> 4];
            result += hex[byte & 15];
        }
        return result;
    }

    uint32 BattlePayToken()
    {
        uint32 result = 0;
        do
            Trinity::Crypto::GetRandomBytes(reinterpret_cast<uint8*>(&result), sizeof(result));
        while (!result);
        return result;
    }
}

bool BattlePayMgr::ValidateAccountingSchema()
{
    QueryResult auth = LoginDatabase.Query("SELECT version FROM battlepay_schema WHERE id=1");
    QueryResult chars = CharacterDatabase.Query("SELECT version FROM battlepay_schema WHERE id=1");
    QueryResult authEngines = LoginDatabase.Query("SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME IN ('account','account_boost','battlepay_purchase','battlepay_purchase_item','battlepay_service_delivery','battlepay_log','battlepay_token_credit','wow_token') AND ENGINE='InnoDB'");
    QueryResult charEngines = CharacterDatabase.Query("SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME IN ('characters','mail','mail_items','item_instance','battlepay_delivery') AND ENGINE='InnoDB'");
    QueryResult boostKey = LoginDatabase.Query("SELECT COUNT(*) FROM (SELECT INDEX_NAME FROM information_schema.STATISTICS WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='account_boost' GROUP BY INDEX_NAME,NON_UNIQUE HAVING NON_UNIQUE=0 AND GROUP_CONCAT(COLUMN_NAME ORDER BY SEQ_IN_INDEX) IN ('id,realmid','realmid,id')) AS boost_keys");
    bool ready = auth && chars && authEngines && charEngines && boostKey && boostKey->Fetch()[0].GetUInt64() > 0 && auth->Fetch()[0].GetUInt32() == 1 &&
        chars->Fetch()[0].GetUInt32() == 1 && authEngines->Fetch()[0].GetUInt64() == 8 && charEngines->Fetch()[0].GetUInt64() == 5;
    if (!ready)
        TC_LOG_ERROR("battlepay", "BattlePay disabled: apply the auth and characters accounting migrations and verify all accounting/delivery tables use InnoDB.");
    return ready;
}

PurchaseInfo* BattlePayMgr::GetPurchaseInfo(WorldSession* session)
{
    auto itr = m_pending.find(session);
    return itr == m_pending.end() ? nullptr : itr->second.Purchase.get();
}

void BattlePayMgr::AddQuenedPurchaseResponse(PurchaseInfo* purchase, uint32 timer, bool sendPacket)
{
    PendingPurchase& pending = m_pending[purchase->Session];
    if (pending.Purchase && pending.Purchase->PurchaseId == purchase->PurchaseId)
        purchase->QuotedPrice = pending.Purchase->QuotedPrice;
    else
        pending.QuoteTimer = 300000;
    pending.Purchase.reset(purchase);
    pending.Timer = timer;
    pending.SendPacket = sendPacket;
}

void BattlePayMgr::RemoveSession(WorldSession* session)
{
    // Paid rows survive disconnects and are recovered from the durable ledger.
    m_pending.erase(session);
}

void BattlePayMgr::Update(WorldSession* session, uint32 diff)
{
    if (!m_accountingReady)
        return;
    PendingPurchase& pending = m_pending[session];
    if (pending.RecoveryTimer <= diff)
    {
        pending.RecoveryTimer = 30000;
        RecoverPurchases(session);
    }
    else
        pending.RecoveryTimer -= diff;
    if (pending.Purchase && pending.Purchase->ResultCode == BATTLE_PAY_RESULT_OK && !pending.Purchase->Buyed &&
        pending.Purchase->PurchaseStatus != BATTLE_PAY_PURCHASE_STATUS_BUYING)
    {
        if (pending.QuoteTimer <= diff)
        {
            pending.SendPacket = false;
            LoginDatabase.DirectPExecute("UPDATE battlepay_purchase SET state=3,updated_at=CURRENT_TIMESTAMP WHERE id=%llu AND account_id=%u AND state=0", pending.Purchase->PurchaseId, session->GetAccountId());
            FinishPurchase(*pending.Purchase, BATTLE_PAY_RESULT_DECLINED);
        }
        else
            pending.QuoteTimer -= diff;
    }
    if (!pending.SendPacket)
        return;
    if (pending.Timer > diff)
    {
        pending.Timer -= diff;
        return;
    }
    pending.SendPacket = false;
    // A response can replace the queued state; hold the old state until it returns.
    std::shared_ptr<PurchaseInfo> purchase = pending.Purchase;
    if (purchase)
        SendBattlePayPurchaseUpdate(purchase.get());
}

bool BattlePayMgr::ValidateProduct(WorldSession* session, BattlePayProduct const* product, ObjectGuid target)
{
    if (!product || product->Flags == BATTLE_PAY_PRODUCT_DISABLES)
        return false;
    if (product->Type == BATTLE_PAY_PRODUCT_TYPE_SERVICE)
    {
        if (product->Id != BATTLE_PAY_SERVICE_BOOST || session->HasBoost())
            return false;
        QueryResult entitlement = LoginDatabase.PQuery("SELECT COUNT(*) FROM account_boost WHERE id=%u AND realmid IN (-1,%u) AND counter>0", session->GetAccountId(), realm.Id.Realm);
        return entitlement && entitlement->Fetch()[0].GetUInt64() == 0;
    }
    if (product->Type != BATTLE_PAY_PRODUCT_TYPE_ITEM || !target.IsPlayer())
        return false;
    QueryResult owner = CharacterDatabase.PQuery("SELECT COUNT(*) FROM characters WHERE guid=%u AND account=%u", target.GetCounter(), session->GetAccountId());
    if (!owner || owner->Fetch()[0].GetUInt64() != 1)
        return false;
    auto items = GetItemsByProductId(product->Id);
    if (!items || items->empty())
        return false;
    uint64 stacks = 0;
    for (auto const& item : *items)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.ItemId);
        if (!proto || !item.Count || !proto->GetMaxStackSize())
            return false;
        stacks += (uint64(item.Count) + proto->GetMaxStackSize() - 1) / proto->GetMaxStackSize();
    }
    // Reject malformed catalogs before accepting payment; never silently truncate.
    return stacks <= 120;
}

bool BattlePayMgr::CreatePurchase(PurchaseInfo& purchase, BattlePayProduct const& product)
{
    std::string nonce = BattlePayNonce();
    std::string title = product.Title, description = product.Description;
    if (auto locale = GetProductLocale(product.Id))
    {
        ObjectMgr::GetLocaleString(locale->Title, purchase.Session->GetSessionDbLocaleIndex(), title);
        ObjectMgr::GetLocaleString(locale->Description, purchase.Session->GetSessionDbLocaleIndex(), description);
    }
    LoginDatabase.EscapeString(title);
    LoginDatabase.EscapeString(description);
    auto trans = LoginDatabase.BeginTransaction();
    trans->PAppend("INSERT INTO battlepay_purchase(nonce,account_id,realm_id,character_guid,product_id,product_type,price,title,description) VALUES('%s',%u,%u,%u,%u,%u,%llu,'%s','%s')", nonce, purchase.Session->GetAccountId(), realm.Id.Realm, purchase.SelectedPlayer.GetCounter(), product.Id, product.Type, purchase.QuotedPrice, title, description);
    if (auto items = GetItemsByProductId(product.Id))
        for (auto const& item : *items)
            trans->PAppend("INSERT INTO battlepay_purchase_item(purchase_id,line_id,item_id,item_count) SELECT id,%u,%u,%u FROM battlepay_purchase WHERE nonce='%s'", item.Id, item.ItemId, item.Count, nonce);
    LoginDatabase.AsyncCommitTransaction(trans).m_future.get();
    QueryResult result = LoginDatabase.PQuery("SELECT id,price FROM battlepay_purchase WHERE nonce='%s' AND account_id=%u AND realm_id=%u AND state=0", nonce, purchase.Session->GetAccountId(), realm.Id.Realm);
    if (!result)
        return false;
    purchase.PurchaseId = result->Fetch()[0].GetUInt64();
    return purchase.QuotedPrice == result->Fetch()[1].GetUInt64();
}

void BattlePayMgr::StartPurchase(WorldSession* session, ObjectGuid target, uint32 productId, uint32 clientToken)
{
    // Starting again while confirmation/delivery is pending cannot replace its owner.
    if (PurchaseInfo* pending = GetPurchaseInfo(session))
        if (pending->PurchaseStatus != BATTLE_PAY_PURCHASE_STATUS_BUYED && pending->ResultCode == BATTLE_PAY_RESULT_OK)
        {
            WorldPacket data(SMSG_BATTLE_PAY_START_PURCHASE_RESPONSE);
            data << uint64(0) << clientToken << uint32(BATTLE_PAY_RESULT_SHOP_ERROR);
            session->SendPacket(&data);
            return;
        }
    if (target.IsEmpty() && session->GetPlayer())
        target = session->GetPlayer()->GetGUID();
    BattlePayProduct* product = GetProductId(productId);
    auto purchase = std::make_unique<PurchaseInfo>(session, target, 0, productId, BATTLE_PAY_PURCHASE_STATUS_READY, BATTLE_PAY_RESULT_OK, clientToken, 0, false);
    if (!IsStoreEnabled() || !ValidateProduct(session, product, target) ||
        !BattlePayAccounting::Price(product->Price, product->Discount, purchase->QuotedPrice))
    {
        purchase->ResultCode = BATTLE_PAY_RESULT_UNAVAILABLE;
        AddQuenedPurchaseResponse(purchase.release(), 500, true);
        return;
    }
    if (!HasPointsBalance(session, purchase->QuotedPrice))
    {
        WorldPacket data(SMSG_BATTLE_PAY_START_PURCHASE_RESPONSE);
        data << uint64(0) << clientToken << uint32(BATTLE_PAY_RESULT_INSUFFICIENT_BALANCE);
        session->SendPacket(&data);
        return;
    }
    if (!CreatePurchase(*purchase, *product))
    {
        purchase->ResultCode = BATTLE_PAY_RESULT_UNAVAILABLE;
        AddQuenedPurchaseResponse(purchase.release(), 500, true);
        return;
    }
    AddQuenedPurchaseResponse(purchase.release(), 500, true);
}

void BattlePayMgr::ConfirmPurchase(WorldSession* session, uint64 price, uint32 token, bool confirm)
{
    PurchaseInfo* current = GetPurchaseInfo(session);
    if (!current || current->Session != session || current->ResultCode != BATTLE_PAY_RESULT_OK)
        return;
    if (!confirm)
    {
        // The 5.4.8 client sends a zero price when the confirmation frame closes.
        // A cancellation must still match the outstanding server token.
        if (!BattlePayAccounting::CanCancel(current->PurchaseStatus, token, current->ServerToken))
            return;
        LoginDatabase.DirectPExecute("UPDATE battlepay_purchase SET state=3,updated_at=CURRENT_TIMESTAMP WHERE id=%llu AND account_id=%u AND state=0", current->PurchaseId, session->GetAccountId());
        FinishPurchase(*current, BATTLE_PAY_RESULT_DECLINED);
        return;
    }
    if (!BattlePayAccounting::CanConfirm(current->PurchaseStatus, token, current->ServerToken, price, current->QuotedPrice))
        return;
    auto purchase = new PurchaseInfo(*current);
    purchase->PurchaseStatus = BATTLE_PAY_PURCHASE_STATUS_BUYING;
    // Keep the existing client timing. Repeated confirmations now fail the state check.
    AddQuenedPurchaseResponse(purchase, 8000, true);
}

bool BattlePayMgr::ChargePurchase(PurchaseInfo const& purchase)
{
    if (!IsStoreEnabled())
        return false;
    LoginDatabase.DirectPExecute(BattlePayQueries::Charge, purchase.PurchaseId, purchase.Session->GetAccountId(), realm.Id.Realm);
    // This read resolves a lost acknowledgement without replaying a deduction.
    QueryResult result = LoginDatabase.PQuery("SELECT state,price FROM battlepay_purchase WHERE id=%llu AND account_id=%u AND realm_id=%u", purchase.PurchaseId, purchase.Session->GetAccountId(), realm.Id.Realm);
    return result && (result->Fetch()[0].GetUInt8() == 1 || result->Fetch()[0].GetUInt8() == 2) && result->Fetch()[1].GetUInt64() == purchase.QuotedPrice;
}

void BattlePayMgr::RecoverPurchases(WorldSession* session)
{
    LoginDatabase.DirectPExecute("UPDATE battlepay_purchase SET state=3,updated_at=CURRENT_TIMESTAMP WHERE account_id=%u AND realm_id=%u AND state=0 AND created_at<CURRENT_TIMESTAMP-INTERVAL 5 MINUTE", session->GetAccountId(), realm.Id.Realm);
    QueryResult result = LoginDatabase.PQuery("SELECT id FROM battlepay_purchase WHERE account_id=%u AND realm_id=%u AND state=1 ORDER BY id LIMIT 20", session->GetAccountId(), realm.Id.Realm);
    if (result)
        do
        {
            uint64 id = result->Fetch()[0].GetUInt64();
            PurchaseInfo* pending = GetPurchaseInfo(session);
            if (!pending || pending->PurchaseId != id || pending->ResultCode != BATTLE_PAY_RESULT_OK || pending->PurchaseStatus == BATTLE_PAY_PURCHASE_STATUS_BUYED)
                FulfillPurchase(id, session);
        }
        while (result->NextRow());
}

void BattlePayMgr::FinishPurchase(PurchaseInfo& purchase, uint32 result)
{
    if (result != BATTLE_PAY_RESULT_OK && purchase.PurchaseId)
        LoginDatabase.DirectPExecute("UPDATE battlepay_purchase SET state=3,updated_at=CURRENT_TIMESTAMP WHERE id=%llu AND account_id=%u AND state=0", purchase.PurchaseId, purchase.Session->GetAccountId());
    purchase.ResultCode = result;
    purchase.PurchaseStatus = result == BATTLE_PAY_RESULT_OK || result == BATTLE_PAY_RESULT_SHOP_ERROR
        ? BATTLE_PAY_PURCHASE_STATUS_BUYED : BATTLE_PAY_PURCHASE_STATUS_ALLOWED_TO_BUY;
    purchase.Buyed = true;
    purchase.ServerToken = 0;
    SendBattlePayPurchaseUpdate(&purchase);
    if (result == BATTLE_PAY_RESULT_OK)
        SendPointsBalance(purchase.Session);
}

bool BattlePayMgr::RefundPurchase(uint64 purchaseId, uint32 accountId)
{
    // A durable cancellation fence in the character DB prevents any later
    // delivery attempt from winning the same receipt key after this refund.
    QueryResult receipt = CharacterDatabase.PQuery("SELECT state FROM battlepay_delivery WHERE purchase_id=%llu AND account_id=%u", purchaseId, accountId);
    if (!receipt || receipt->Fetch()[0].GetUInt8() != 0)
        return false;
    LoginDatabase.DirectPExecute(BattlePayQueries::Refund, purchaseId, accountId);
    QueryResult result = LoginDatabase.PQuery("SELECT state FROM battlepay_purchase WHERE id=%llu AND account_id=%u", purchaseId, accountId);
    bool refunded = result && result->Fetch()[0].GetUInt8() == 4;
    if (!refunded)
        TC_LOG_ERROR("battlepay", "Purchase %llu refund pending; check DB availability and account.dp range.", purchaseId);
    return refunded;
}

bool BattlePayMgr::FulfillPurchase(uint64 purchaseId, WorldSession* session)
{
    QueryResult purchase = LoginDatabase.PQuery("SELECT account_id,character_guid,product_id,product_type,price,title,description,state FROM battlepay_purchase WHERE id=%llu AND account_id=%u AND realm_id=%u", purchaseId, session->GetAccountId(), realm.Id.Realm);
    if (!purchase)
        return false;
    Field* fields = purchase->Fetch();
    uint32 accountId = fields[0].GetUInt32();
    uint32 target = fields[1].GetUInt32();
    uint32 productId = fields[2].GetUInt32();
    uint8 type = fields[3].GetUInt8();
    uint64 price = fields[4].GetUInt64();
    std::string title = fields[5].GetString();
    std::string body = fields[6].GetString();
    uint8 state = fields[7].GetUInt8();
    if (state != 1)
        return state == 2;

    if (type == BATTLE_PAY_PRODUCT_TYPE_SERVICE)
    {
        if (productId != BATTLE_PAY_SERVICE_BOOST)
        {
            TC_LOG_ERROR("battlepay", "Unsupported paid service purchase %llu requires review; no further charge or delivery attempted.", purchaseId);
            return false;
        }
        auto trans = LoginDatabase.BeginTransaction();
        trans->PAppend("INSERT INTO battlepay_service_delivery(purchase_id) VALUES(%llu)", purchaseId);
        trans->PAppend("INSERT INTO account_boost(id,realmid,counter) VALUES(%u,%u,1) ON DUPLICATE KEY UPDATE counter=counter+1", accountId, realm.Id.Realm);
        trans->PAppend("INSERT INTO battlepay_log(accountId,characterGuid,realm,item,price) VALUES(%u,%u,%u,0,%llu)", accountId, target, realm.Id.Realm, price);
        trans->PAppend("UPDATE battlepay_purchase SET state=2,updated_at=CURRENT_TIMESTAMP WHERE id=%llu AND state=1", purchaseId);
        LoginDatabase.AsyncCommitTransaction(trans).m_future.get();
        QueryResult result = LoginDatabase.PQuery("SELECT state FROM battlepay_purchase WHERE id=%llu", purchaseId);
        if (!result || result->Fetch()[0].GetUInt8() != 2)
            return false;
        session->SetBoost(true);
        SendBattlePayDistributionUpdate(session, BATTLE_PAY_SERVICE_BOOST, CHARACTER_BOOST_ALLOW);
        SendPointsBalance(session);
        return true;
    }
    if (type != BATTLE_PAY_PRODUCT_TYPE_ITEM)
        return false;

    QueryResult receipt = CharacterDatabase.PQuery("SELECT state,attempt FROM battlepay_delivery WHERE purchase_id=%llu", purchaseId);
    if (receipt && receipt->Fetch()[0].GetUInt8() == 0)
    {
        RefundPurchase(purchaseId, accountId);
        SendPointsBalance(session);
        return false;
    }

    if (!receipt)
    {
        // COUNT always returns a row; an unavailable DB is never mistaken for
        // a deleted character and cannot trigger compensation.
        QueryResult owner = CharacterDatabase.PQuery("SELECT COUNT(*) FROM characters WHERE guid=%u AND account=%u", target, accountId);
        if (!owner)
            return false;
        QueryResult snapshot = LoginDatabase.PQuery("SELECT item_id,item_count FROM battlepay_purchase_item WHERE purchase_id=%llu ORDER BY line_id", purchaseId);
        if (!snapshot)
            return false; // Could be an unavailable auth DB; retain the paid row.

        struct DeliveryItem { uint32 Id; uint32 Count; };
        std::vector<DeliveryItem> lines;
        uint64 stacks = 0;
        bool deliverable = owner->Fetch()[0].GetUInt64() == 1;
        do
        {
            uint32 id = snapshot->Fetch()[0].GetUInt32(), count = snapshot->Fetch()[1].GetUInt32();
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(id);
            if (!proto || !count || !proto->GetMaxStackSize())
                deliverable = false;
            else
            {
                stacks += (uint64(count) + proto->GetMaxStackSize() - 1) / proto->GetMaxStackSize();
                lines.push_back({ id, count });
            }
        } while (snapshot->NextRow());
        deliverable = deliverable && stacks > 0 && stacks <= 120;

        std::string attempt = BattlePayNonce();
        if (!deliverable)
        {
            // This insert races safely with delivery: only one primary-key
            // receipt can commit, and a failed insert is never a refund grant.
            CharacterDatabase.DirectPExecute("INSERT IGNORE INTO battlepay_delivery(purchase_id,account_id,character_guid,attempt,state) VALUES(%llu,%u,%u,'%s',0)", purchaseId, accountId, target, attempt);
            RefundPurchase(purchaseId, accountId);
            SendPointsBalance(session);
            return false;
        }

        auto trans = CharacterDatabase.BeginTransaction();
        std::string claim = Trinity::StringFormat("INSERT INTO battlepay_delivery(purchase_id,account_id,character_guid,attempt,state) SELECT %llu,%u,guid,'%s',1 FROM characters WHERE guid=%u AND account=%u", purchaseId, accountId, attempt, target, accountId);
        trans->AppendChecked(claim.c_str(), 1);
        // INSERT SELECT holds a shared lock on the target until this transaction
        // commits, preventing deletion/account transfer while mail is inserted.
        std::vector<std::unique_ptr<Item>> items;
        std::vector<std::unique_ptr<Mail>> mails;
        for (DeliveryItem line : lines)
            while (line.Count)
            {
                if (mails.empty() || mails.back()->items.size() == MAX_MAIL_ITEMS)
                {
                    auto mail = std::make_unique<Mail>();
                    mail->messageID = sObjectMgr->GenerateMailID();
                    mail->messageType = MAIL_NORMAL;
                    mail->stationery = MAIL_STATIONERY_GM;
                    mail->mailTemplateId = 0;
                    mail->sender = 0;
                    mail->receiver = target;
                    mail->subject = title;
                    mail->body = body;
                    mail->expire_time = time(nullptr) + 180 * DAY;
                    mail->deliver_time = time(nullptr);
                    mail->money = 0;
                    mail->COD = 0;
                    mail->checked = MAIL_CHECK_MASK_HAS_BODY;
                    mail->state = MAIL_STATE_UNCHANGED;
                    mails.push_back(std::move(mail));
                }
                uint32 count = std::min(line.Count, sObjectMgr->GetItemTemplate(line.Id)->GetMaxStackSize());
                std::unique_ptr<Item> item(Item::CreateItem(line.Id, count, nullptr));
                if (!item)
                    return false; // No transaction has been submitted; no receipt exists.
                item->SetOwnerGUID(ObjectGuid(HighGuid::Player, target));
                item->SaveToDB(trans);
                mails.back()->AddItem(item->GetGUID().GetCounter(), line.Id);
                auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL_ITEM);
                stmt->setUInt32(0, mails.back()->messageID);
                stmt->setUInt32(1, item->GetGUID().GetCounter());
                stmt->setUInt32(2, target);
                trans->Append(stmt);
                items.push_back(std::move(item));
                line.Count -= count;
            }
        for (auto const& mail : mails)
        {
            auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL);
            stmt->setUInt32(0, mail->messageID);
            stmt->setUInt8(1, mail->messageType);
            stmt->setUInt8(2, mail->stationery);
            stmt->setUInt16(3, 0);
            stmt->setUInt32(4, 0);
            stmt->setUInt32(5, target);
            stmt->setString(6, mail->subject);
            stmt->setString(7, mail->body);
            stmt->setBool(8, true);
            stmt->setUInt64(9, mail->expire_time);
            stmt->setUInt64(10, mail->deliver_time);
            stmt->setUInt32(11, 0);
            stmt->setUInt32(12, 0);
            stmt->setUInt32(13, mail->checked);
            trans->Append(stmt);
        }
        CharacterDatabase.AsyncCommitTransaction(trans).m_future.get();
        receipt = CharacterDatabase.PQuery("SELECT state,attempt FROM battlepay_delivery WHERE purchase_id=%llu", purchaseId);
        if (!receipt || receipt->Fetch()[0].GetUInt8() != 1)
            return false;
        // Update in-memory mail only after proving our exact attempt committed.
        // A failed/replayed attempt must never publish its transient item IDs.
        if (receipt->Fetch()[1].GetString() == attempt)
        {
            Player* player = session->GetPlayer();
            if (player && player->GetGUID().GetCounter() != target)
                player = nullptr;
            for (auto& mail : mails)
            {
                sCharacterCache->IncreaseCharacterMailCount(ObjectGuid(HighGuid::Player, target));
                if (player)
                {
                    player->AddNewMailDeliverTime(mail->deliver_time);
                    if (player->IsMailsLoaded())
                        player->AddMail(mail.release());
                }
            }
            if (player && player->IsMailsLoaded())
                for (auto& item : items)
                    player->AddMItem(item.release());
        }
    }

    // Character receipt and auth accounting are separate durable transactions.
    // Receipt existence, rather than a second delivery, reconciles a crash here.
    auto trans = LoginDatabase.BeginTransaction();
    std::string claim = Trinity::StringFormat("INSERT INTO battlepay_service_delivery(purchase_id) VALUES(%llu)", purchaseId);
    trans->AppendChecked(claim.c_str(), 1); // Shared completion key for exactly-once legacy logging.
    trans->PAppend("INSERT INTO battlepay_log(accountId,characterGuid,realm,item,price) SELECT account_id,character_guid,realm_id,COALESCE((SELECT item_id FROM battlepay_purchase_item WHERE purchase_id=%llu ORDER BY line_id LIMIT 1),0),price FROM battlepay_purchase WHERE id=%llu AND state=1", purchaseId, purchaseId);
    trans->PAppend("UPDATE battlepay_purchase SET state=2,updated_at=CURRENT_TIMESTAMP WHERE id=%llu AND state=1", purchaseId);
    LoginDatabase.AsyncCommitTransaction(trans).m_future.get();
    QueryResult complete = LoginDatabase.PQuery("SELECT state FROM battlepay_purchase WHERE id=%llu", purchaseId);
    SendPointsBalance(session);
    return complete && complete->Fetch()[0].GetUInt8() == 2;
}

void BattlePayMgr::SendBattlePayDistributionList(WorldSession* session)
{
    // TODO: finish this
    WorldPacket data(SMSG_BATTLE_PAY_GET_DISTRIBUTION_LIST_RESPONSE, 4 + 4);
    data << uint32(0);
    data.WriteBits(0, 19);
    data.FlushBits();
    session->SendPacket(&data);
}

void BattlePayMgr::SendBattlePayPurchaseList(WorldSession* session)
{
    // TODO: finish this
    WorldPacket data(SMSG_BATTLE_PAY_GET_PURCHASE_LIST_RESPONSE, 4 + 4);
    data.WriteBits(0, 19);
    data.FlushBits();
    data << uint32(0);
    session->SendPacket(&data);
}

void BattlePayMgr::SendBattlePayProductList(WorldSession* session)
{
    bool hasItemInfo = false, unkBit1 = false, unkBit2 = false, unkBit3 = false, unkBit4 = false, hasBattlePetResult = false, unkBit5 = false, unkBit6 = false, unkBit7 = false;
    uint64 accountBalance = 0;
    bool hasAccountBalance = ReadBalance(session, accountBalance);

    WorldPacket data(SMSG_BATTLE_PAY_GET_PRODUCT_LIST_RESPONSE);
    data.WriteBits(m_shopEntryStore.size(), 19);
    data.WriteBits(m_productStore.size(), 19);

    for (auto&& product : m_productStore)
    {
        BattlePayProductItemsVector const* items = GetItemsByProductId(product->Id);

        std::string productTitle = product->Title;
        std::string productDescription = product->Description;
        LocaleConstant localeConstant = session->GetSessionDbLocaleIndex();
        if (localeConstant != LOCALE_enUS)
            if (BattlePayProductLocale const* locProd = GetProductLocale(product->Id))
            {
                ObjectMgr::GetLocaleString(locProd->Title, localeConstant, productTitle);
                ObjectMgr::GetLocaleString(locProd->Description, localeConstant, productDescription);
            }

        data.WriteBits(product->ChoiceType, 2);
        data.WriteBits(items->size(), 20);

        for (auto&& item : *items)
        {
            bool hasProduct = false;
            if (product->Type == BATTLE_PAY_PRODUCT_TYPE_SERVICE)
            {
                if (product->Id == BATTLE_PAY_SERVICE_BOOST)
                    if (session->HasBoost())
                        hasProduct = true;
            }
            else if (product->Type == BATTLE_PAY_PRODUCT_TYPE_ITEM)
            {
                ItemTemplate const* itemTemp = sObjectMgr->GetItemTemplate(item.ItemId);
                uint32 productSpell = itemTemp ? (itemTemp->Spells ? (itemTemp->Spells[0].SpellTrigger == 6 ? itemTemp->Spells[0].SpellId : itemTemp->Spells[1].SpellId) : 0) : 0;
                if (SpellInfo const* spell = sSpellMgr->GetSpellInfo(productSpell))
                {
                    if (spell->IsAbilityOfSkillType(SKILL_MOUNTS))
                    {
                        if (Player* player = session->GetPlayer())
                        {
                            if (player->HasSpell(productSpell))
                                hasProduct = true;
                        }
                        else
                        {
                            if (CharacterDatabase.PQuery("SELECT spell FROM account_spell WHERE account = '%u' AND spell = '%u'", session->GetAccountId(), productSpell))
                                hasProduct = true;
                        }
                    }
                    else if (spell->IsAbilityOfSkillType(SKILL_COMPANIONS))
                    {
                        uint16 speciesId = sObjectMgr->BattlePetGetSpeciesFromItem(itemTemp->ItemId);
                        if (Player* player = session->GetPlayer())
                        {
                            if (player->GetBattlePetMgr().GetBattlePetCount(speciesId))
                                hasProduct = true;
                        }
                        else
                        {
                            if (CharacterDatabase.PQuery("SELECT species FROM account_battle_pet WHERE accountId = '%u' AND species = '%u'", session->GetAccountId(), speciesId))
                                hasProduct = true;
                        }
                    }
                }
            }

            data.WriteBit(false);
            data.WriteBit(hasProduct);
            data.WriteBit(hasItemInfo);
            if (hasItemInfo)
            {
                data.WriteBits(0, 10);
                data.WriteBits(0, 13);
                data.WriteBit(unkBit1);
                data.WriteBit(unkBit2);
                data.WriteBit(unkBit3);
                data.WriteBit(unkBit4);
                data.WriteBits(0, 10);
            }
            data.WriteBit(hasBattlePetResult);
            if (hasBattlePetResult)
                data.WriteBits(0, 4);
        }
        data.WriteBit(!productTitle.empty() ? true : false);
        if (!productTitle.empty())
        {
            data.WriteBit(product->DisplayId ? true : false);
            data.WriteBits(productTitle.size(), 10);
            data.WriteBits(productDescription.size(), 13);
            data.WriteBits(0, 10);
            data.WriteBit(product->FlagsInfo ? true : false);
            data.WriteBit(unkBit5);
            data.WriteBit(product->Icon ? true : false);
        }
    }

    data.WriteBits(m_groupStore.size(), 20);

    for (auto&& entry : m_shopEntryStore)
    {
        std::string entryTitle = entry->Title;
        std::string entryDescription = entry->Description;
        LocaleConstant localeConstant = session->GetSessionDbLocaleIndex();
        if (localeConstant != LOCALE_enUS)
            if (BattlePayShopEntryLocale const* locEntry = GetShopEntryLocale(entry->Id))
            {
                ObjectMgr::GetLocaleString(locEntry->Title, localeConstant, entryTitle);
                ObjectMgr::GetLocaleString(locEntry->Description, localeConstant, entryDescription);
            }

        data.WriteBit(!entryTitle.empty() ? true : false);
        if (!entryTitle.empty())
        {
            data.WriteBits(entryTitle.size(), 10);
            data.WriteBit(unkBit6);
            data.WriteBits(0, 10);
            data.WriteBits(entryDescription.size(), 13);
            data.WriteBit(entry->DisplayId);
            data.WriteBit(entry->Icon);
            data.WriteBit(unkBit7);
        }
    }

    for (auto&& group : m_groupStore)
    {
        std::string groupName = group->Name;
        LocaleConstant localeConstant = session->GetSessionDbLocaleIndex();
        if (localeConstant != LOCALE_enUS)
            if (BattlePayGroupLocale const* locGroup = GetGroupLocale(group->Id))
                ObjectMgr::GetLocaleString(locGroup->Name, localeConstant, groupName);

        data.WriteBits(groupName.size(), 8);
    }

    data.FlushBits();

    for (auto&& group : m_groupStore)
    {
        std::string groupName = group->Name;
        LocaleConstant localeConstant = session->GetSessionDbLocaleIndex();
        if (localeConstant != LOCALE_enUS)
            if (BattlePayGroupLocale const* locGroup = GetGroupLocale(group->Id))
                ObjectMgr::GetLocaleString(locGroup->Name, localeConstant, groupName);

        data << uint32(group->Index);
        data.WriteString(groupName);
        data << uint8(group->Type);
        data << uint32(group->Icon);
        data << uint32(group->Id);
    }

    for (auto&& product : m_productStore)
    {
        BattlePayProductItemsVector const* items = GetItemsByProductId(product->Id);

        std::string productTitle = product->Title;
        std::string productDescription = product->Description;
        LocaleConstant localeConstant = session->GetSessionDbLocaleIndex();
        if (localeConstant != LOCALE_enUS)
            if (BattlePayProductLocale const* locProd = GetProductLocale(product->Id))
            {
                ObjectMgr::GetLocaleString(locProd->Title, localeConstant, productTitle);
                ObjectMgr::GetLocaleString(locProd->Description, localeConstant, productDescription);
            }

        data << uint8(product->Type);
        for (auto&& item : *items)
        {
            if (hasItemInfo)
            {
                if (unkBit2)
                    data << uint32(0);
                if (unkBit3)
                    data << uint32(0);
                if (unkBit4)
                    data << uint32(0);
                data.WriteString(std::string());
                if (unkBit1)
                    data << uint32(0);
                data.WriteString(std::string());
                data.WriteString(std::string());
            }
            data << uint32(item.ItemId);
            data << uint32(item.Count);
            data << uint32(item.Id);
        }
        if (!productTitle.empty())
        {
            if (product->DisplayId)
                data << uint32(product->DisplayId);
            if (product->Icon)
                data << uint32(product->Icon);
            data.WriteString(std::string());
            data.WriteString(productTitle);
            if (unkBit5)
                data << uint32(0);
            if (product->FlagsInfo)
                data << uint32(product->FlagsInfo);
            data.WriteString(productDescription);
        }
        data << uint32(product->Id);
        data << uint32(product->Flags);

        uint64 price = 0, currentPrice = 0, effectiveDp = 0;
        BattlePayAccounting::Price(product->Price, product->Discount, effectiveDp);
        BattlePayAccounting::WirePrice(product->Price, price);
        BattlePayAccounting::WirePrice(effectiveDp, currentPrice);

        // The existing catalog uses product 1 as a non-purchasable balance tile.
        // Match its metadata as well as its ID so a legitimate product 1 is not
        // silently repurposed by a different world database.
        if (hasAccountBalance && product->Id == 1 && product->Price == 0 &&
            product->Type == BATTLE_PAY_PRODUCT_TYPE_SERVICE && product->Title == "Your balance")
        {
            BattlePayAccounting::WirePrice(accountBalance, currentPrice);
            price = currentPrice;
        }

        data << uint64(price);
        data << uint64(currentPrice);
    }

    for (auto&& entry : m_shopEntryStore)
    {
        std::string entryTitle = entry->Title;
        std::string entryDescription = entry->Description;
        LocaleConstant localeConstant = session->GetSessionDbLocaleIndex();
        if (localeConstant != LOCALE_enUS)
            if (BattlePayShopEntryLocale const* locEntry = GetShopEntryLocale(entry->Id))
            {
                ObjectMgr::GetLocaleString(locEntry->Title, localeConstant, entryTitle);
                ObjectMgr::GetLocaleString(locEntry->Description, localeConstant, entryDescription);
            }

        if (!entryTitle.empty())
        {
            if (unkBit6)
                data << uint32(0);
            data.WriteString(std::string());
            if (unkBit7)
                data << uint32(unkBit7);
            if (entry->Icon)
                data << uint32(entry->Icon);
            if (entry->DisplayId)
                data << uint32(entry->DisplayId);
            data.WriteString(entryDescription);
            data.WriteString(entryTitle);
        }

        data << uint32(entry->Flags);
        data << uint8(entry->Banner);
        data << uint32(entry->Id);
        data << int32(entry->Index);
        data << uint32(entry->GroupId);
        data << uint32(entry->ProductId);
    }
    
    data << uint32(GetStoreCurrency());
    data << uint32(BATTLE_PAY_STATUS_OK);

    session->SendPacket(&data);
}

void BattlePayMgr::SendBattlePayDistributionUpdate(WorldSession* session, uint32 productId, uint8 status)
{
    ObjectGuid guid = session->GetPlayer() ? session->GetPlayer()->GetGUID() : ObjectGuid(HighGuid::WowAccount, session->GetAccountId());
    ObjectGuid guid2 = ObjectGuid::Empty;

    BattlePayProduct* product = GetProductId(productId);
    bool HasBattlePayProduct = product;

    std::string title = product->Title;
    std::string description = product->Description;
    LocaleConstant localeConstant = session->GetSessionDbLocaleIndex();
    if (localeConstant != LOCALE_enUS)
        if (BattlePayShopEntryLocale const* locEntry = GetShopEntryLocale(product->Id))
        {
            ObjectMgr::GetLocaleString(locEntry->Title, localeConstant, title);
            ObjectMgr::GetLocaleString(locEntry->Description, localeConstant, description);
        }

    WorldPacket data(SMSG_BATTLE_PAY_DISTRIBUTION_UPDATE);
    data.WriteBit(guid[5]);
    data.WriteBit(guid[0]);
    data.WriteBit(HasBattlePayProduct);
    data.WriteBit(guid[1]);
    data.WriteBit(guid2[4]);
    data.WriteBit(guid2[7]);
    data.WriteBit(guid2[0]);
    data.WriteBit(0); // Revoked
    data.WriteBit(guid2[1]);
    data.WriteBit(guid2[2]);
    if (HasBattlePayProduct)
    {
        data.WriteBits(1, 2);
        data.WriteBits(0, 20);
        data.WriteBit(!title.empty());
        if (!title.empty())
        {
            data.WriteBits(0, 10);
            data.WriteBit(0);
            data.WriteBits(title.length(), 10);
            data.WriteBit(0);
            data.WriteBit(0);
            data.WriteBits(description.length(), 13);
            data.WriteBit(0); // bool11
        }
    }

    data.WriteBit(guid[7]);
    data.WriteBit(guid2[6]);
    data.WriteBit(guid[2]);
    data.WriteBit(guid2[5]);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[6]);
    data.WriteBit(guid2[3]);
    data.WriteBit(guid[4]);

    data.FlushBits();

    if (HasBattlePayProduct)
    {
        data << int32(0);
        data << int64(0);
        data.WriteString(description);
        data.WriteString(title);
        // if (bool11)
            //data << int32(0);
        data << int64(0);
        data << int8(product->Type);
        data << int32(0);
    }

    data << int32(product->Id);
    data.WriteByteSeq(guid2[4]);
    data << int64(0);
    data.WriteByteSeq(guid2[1]);
    data.WriteByteSeq(guid2[5]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[0]);
    data << int32(0);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid2[0]);
    data.WriteByteSeq(guid2[7]);
    data << int32(0);
    data << int32(status);
    data.WriteByteSeq(guid2[6]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid2[3]);
    data.WriteByteSeq(guid2[2]);


    session->SendPacket(&data);
}

void BattlePayMgr::SendBattlePayPurchaseUpdate(PurchaseInfo* purchase)
{
    if (!purchase->GetSession())
        return;

    if (purchase->PurchaseStatus == BATTLE_PAY_PURCHASE_STATUS_READY)
    {
        WorldPacket data(SMSG_BATTLE_PAY_START_PURCHASE_RESPONSE);
        // The 5.4.8 client reads PurchaseId, ClientToken, then ResultCode.
        data << uint64(purchase->PurchaseId);
        data << uint32(purchase->ClientToken);
        data << uint32(purchase->ResultCode == BATTLE_PAY_RESULT_OK ? 1 : purchase->ResultCode);
        purchase->GetSession()->SendPacket(&data);
    }

    std::string wallet = purchase->PurchaseStatus == BATTLE_PAY_PURCHASE_STATUS_ALLOWED_TO_BUY || purchase->PurchaseStatus == BATTLE_PAY_PURCHASE_STATUS_BUYING || purchase->PurchaseStatus == BATTLE_PAY_PURCHASE_STATUS_BUYED ? "Account" : "";

    WorldPacket data(SMSG_BATTLE_PAY_PURCHASE_UPDATE);
    data.WriteBits(1, 19); // PurchaseCount
    for (uint8 i = 0; i < 1; i++)
        data.WriteBits(wallet.size(), 8);
    data.FlushBits();
    for (uint8 i = 0; i < 1; i++)
    {
        data << uint32(purchase->ResultCode);
        data << uint64(purchase->PurchaseId);
        data << uint32(purchase->ProductId);
        data << uint32(purchase->PurchaseStatus);
        data.WriteString(wallet);
    }
    purchase->GetSession()->SendPacket(&data);

    if (purchase->PurchaseStatus == BATTLE_PAY_PURCHASE_STATUS_READY && purchase->ResultCode == BATTLE_PAY_RESULT_OK)
    {
        if (sBattlePayMgr->IsStoreEnabled())
            sBattlePayMgr->AddQuenedPurchaseResponse(new PurchaseInfo(purchase->GetSession(), purchase->SelectedPlayer, purchase->PurchaseId, purchase->ProductId, BATTLE_PAY_PURCHASE_STATUS_ALLOWED_TO_BUY, BATTLE_PAY_RESULT_OK, purchase->ClientToken, 0, false), 500, true);
        else
            sBattlePayMgr->AddQuenedPurchaseResponse(new PurchaseInfo(purchase->GetSession(), purchase->SelectedPlayer, purchase->PurchaseId, purchase->ProductId, BATTLE_PAY_PURCHASE_STATUS_READY, BATTLE_PAY_RESULT_UNAVAILABLE, purchase->ClientToken, 0, false), 500, true);
    }

    if (purchase->PurchaseStatus == BATTLE_PAY_PURCHASE_STATUS_ALLOWED_TO_BUY && purchase->ResultCode == BATTLE_PAY_RESULT_OK && !purchase->Buyed)
    {
        BattlePayProduct* product = GetProductId(purchase->ProductId);
        if (!IsStoreEnabled() || !ValidateProduct(purchase->Session, product, purchase->SelectedPlayer))
        {
            auto failed = new PurchaseInfo(*purchase);
            failed->ResultCode = BATTLE_PAY_RESULT_SHOP_ERROR;
            failed->Buyed = true;
            AddQuenedPurchaseResponse(failed, 200, true);
            return;
        }
        purchase->ServerToken = BattlePayToken();
        data.Initialize(SMSG_BATTLE_PAY_CONFIRM_PURCHASE);
        uint64 wirePrice = 0;
        BattlePayAccounting::WirePrice(purchase->QuotedPrice, wirePrice);
        data << purchase->ServerToken << wirePrice << purchase->PurchaseId;
        purchase->Session->SendPacket(&data);
    }

    if (purchase->PurchaseStatus == BATTLE_PAY_PURCHASE_STATUS_BUYING && !purchase->Buyed)
    {
        // Recheck configuration, ownership and the catalog before making a durable charge.
        if (!ValidateProduct(purchase->Session, GetProductId(purchase->ProductId), purchase->SelectedPlayer) ||
            !ChargePurchase(*purchase))
        {
            FinishPurchase(*purchase, BATTLE_PAY_RESULT_SHOP_ERROR);
            return;
        }
        data.Initialize(SMSG_BATTLE_PAY_DELIVERY_ENDED);
        // Preserve the existing 5.4.8 wire shape; fulfillment uses the entire snapshot.
        data.WriteBits(1, 22);
        data.FlushBits();
        auto items = GetItemsByProductId(purchase->ProductId);
        data << uint32(items && !items->empty() ? items->front().ItemId : 0);
        data << uint64(0);
        purchase->Session->SendPacket(&data);
        auto delivery = new PurchaseInfo(*purchase);
        delivery->Buyed = true;
        AddQuenedPurchaseResponse(delivery, 1000, true);
        return;
    }

    if (purchase->PurchaseStatus == BATTLE_PAY_PURCHASE_STATUS_BUYING && purchase->Buyed)
        FinishPurchase(*purchase, FulfillPurchase(purchase->PurchaseId, purchase->Session) ? BATTLE_PAY_RESULT_OK : BATTLE_PAY_RESULT_SHOP_ERROR);
}
