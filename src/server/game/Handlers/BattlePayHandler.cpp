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

    sBattlePayMgr->StartPurchase(this, guid, productID, clientToken);
}

void WorldSession::HandleBattlePayGetPurchaseList(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BATTLE_PAY_GET_PURCHASE_LIST");

    sBattlePayMgr->SendBattlePayPurchaseList(this);
}

void WorldSession::HandleBattlePayConfirmPurchaseResponse(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_BATTLE_PAY_CONFIRM_PURCHASE_RESPONSE");

    uint64 currentPrice = recvData.read<uint64>();
    uint32 serverToken = recvData.read<uint32>();
    bool confirm = recvData.read<bool>();

    sBattlePayMgr->ConfirmPurchase(this, currentPrice, serverToken, confirm);
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

        if (!GetBoost()->SetBoostedCharInfo(guid, CHARACTER_BOOST_ITEMS, (charInfo & CHARACTER_BOOST_SPEC_MASK), (charInfo & CHARACTER_BOOST_FACTION_ALLIANCE)))
            return;
        sBattlePayMgr->SendBattlePayDistributionUpdate(this, BATTLE_PAY_SERVICE_BOOST, CHARACTER_BOOST_CHOOSED);

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
