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

#include "BattlePayMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "ServiceBoost.h"

void WorldSession::HandleBattlePayGetProductList(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BATTLE_PAY_GET_PRODUCT_LIST");

    if (!sBattlePayMgr->IsStoreEnabled())
    {
        WorldPacket data(SMSG_BATTLE_PAY_GET_PRODUCT_LIST_RESPONSE);

        data.WriteBits(0, 19);
        data.WriteBits(0, 19);
        data.WriteBits(0, 20);
        data.FlushBits();

        data << uint32(BATTLE_PAY_CURRENCY_UNKNOWN);
        data << uint32(BATTLE_PAY_STATUS_UNAVAILABLE);
        SendPacket(&data);
        return;
    }

    sBattlePayMgr->SendBattlePayProductList(this);
    sBattlePayMgr->SendPointsBalance(this);
}

void WorldSession::HandleBattlePayStartPurchase(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BATTLE_PAY_START_PURCHASE");

    uint32 productID = recvData.read<uint32>();
    uint32 clientToken = recvData.read<uint32>();

    ObjectGuid guid;

    guid[2] = recvData.ReadBit();
    guid[3] = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    guid[4] = recvData.ReadBit();
    guid[7] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    guid[6] = recvData.ReadBit();

    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[2]);

    uint32 purchaseId = irand(1, 999999); // temp solution

    sBattlePayMgr->AddQuenedPurchaseResponse(PurchaseInfo(this, guid, purchaseId, productID, BATTLE_PAY_PURCHASE_STATUS_READY, BATTLE_PAY_RESULT_OK, clientToken, 0, false), 500);
}

void WorldSession::HandleBattlePayGetPurchaseList(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BATTLE_PAY_GET_PURCHASE_LIST");

    sBattlePayMgr->SendBattlePayPurchaseList(this);
}

void WorldSession::HandleBattlePayConfirmPurchaseResponse(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BATTLE_PAY_CONFIRM_PURCHASE_RESPONSE");

    uint64 currentPrice = recvData.read<uint64>(); // TODO: use it!
    uint32 serverToken = recvData.read<uint32>();
    bool confirm = recvData.read<bool>();

    // #969: the step of this account, and only while it waits for this confirmation. A repeated or
    // late confirm (after BUYING was queued or the purchase finished) used to queue a second delivery
    // and a second charge: two boosts.
    PurchaseInfo* purchase = sBattlePayMgr->GetPurchaseInfo(this);
    if (!purchase || purchase->PurchaseStatus != BATTLE_PAY_PURCHASE_STATUS_ALLOWED_TO_BUY || purchase->ResultCode != BATTLE_PAY_RESULT_OK || !purchase->ServerToken)
    {
        TC_LOG_DEBUG("network", "WorldSession::HandleBattlePayConfirmPurchaseResponse: no purchase waiting for confirmation (account %u), ignored.", GetAccountId());
        return;
    }

    if (confirm && serverToken == purchase->ServerToken)
    {
        PurchaseInfo buying(this, purchase->SelectedPlayer, purchase->PurchaseId, purchase->ProductId, BATTLE_PAY_PURCHASE_STATUS_BUYING, BATTLE_PAY_RESULT_OK, purchase->ClientToken, serverToken, false);
        sBattlePayMgr->AddQuenedPurchaseResponse(buying, 8000); // do not touch timers! everything here should be as it is! decreasing timer will cause errors, for example: window with purchase close automatically and causing random client crashes
    }
    else
    {
        PurchaseInfo declined(this, purchase->SelectedPlayer, purchase->PurchaseId, purchase->ProductId, BATTLE_PAY_PURCHASE_STATUS_ALLOWED_TO_BUY, BATTLE_PAY_RESULT_DECLINED, purchase->ClientToken, 0, false);
        purchase->ServerToken = 0; // a declined purchase can not be confirmed afterwards
        sBattlePayMgr->SendBattlePayPurchaseUpdate(&declined);
    }
}

void WorldSession::HandleBattlePayCharBoost(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BATTLE_PAY_CHAR_BOOST");

    if (!HasBoost())
    {
        TC_LOG_ERROR("network", "WorldSession::HandleBattleCharBoost called, but player has not allowed boost.");
        recvData.rfinish();
        return;
    }

    ObjectGuid guid, guid2;

    recvData.read_skip<uint32>();
    guid2[1] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    guid2[5] = recvData.ReadBit();
    guid2[4] = recvData.ReadBit();
    guid[3] = recvData.ReadBit();
    guid2[6] = recvData.ReadBit();
    guid2[0] = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid2[3] = recvData.ReadBit();
    guid2[7] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    guid2[2] = recvData.ReadBit();
    guid[2] = recvData.ReadBit();
    bool hasCharInfo = !recvData.ReadBit();
    guid[7] = recvData.ReadBit();
    guid[4] = recvData.ReadBit();
    guid[6] = recvData.ReadBit();

    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid2[0]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid2[7]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid2[6]);
    recvData.ReadByteSeq(guid2[4]);
    recvData.ReadByteSeq(guid2[5]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid2[1]);
    recvData.ReadByteSeq(guid2[2]);
    recvData.ReadByteSeq(guid2[3]);
    recvData.ReadByteSeq(guid[5]);

    if (hasCharInfo)
    {
        uint32 charInfo = recvData.read<uint32>();

        sBattlePayMgr->SendBattlePayDistributionUpdate(this, BATTLE_PAY_SERVICE_BOOST, CHARACTER_BOOST_CHOOSED);
        GetBoost()->SetBoostedCharInfo(guid, CHARACTER_BOOST_ITEMS, (charInfo & CHARACTER_BOOST_SPEC_MASK), (charInfo & CHARACTER_BOOST_FACTION_ALLIANCE));

        WorldPacket data(SMSG_CHARACTER_UPGRADE_STARTED, 8);
        data.WriteBit(guid[6]);
        data.WriteBit(guid[2]);
        data.WriteBit(guid[5]);
        data.WriteBit(guid[4]);
        data.WriteBit(guid[7]);
        data.WriteBit(guid[0]);
        data.WriteBit(guid[3]);
        data.WriteBit(guid[1]);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[3]);
        SendPacket(&data);
    }
}
