/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "ObjectMgr.h"
#include "Player.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include "BlackMarket/BlackMarketMgr.h"
#include "Log.h"
#include "Language.h"
#include "Opcodes.h"
#include "UpdateMask.h"
#include "Util.h"
#include "AccountMgr.h"

void WorldSession::HandleBlackMarketHello(WorldPacket& recvData)
{
    ObjectGuid guid;

    uint8 bits[8] = { 6, 4, 2, 7, 5, 3, 0, 1 };
    recvData.ReadBitInOrder(guid, bits);

    uint8 bytes[8] = { 3, 2, 1, 6, 0, 5, 7, 4 };
    recvData.ReadBytesSeq(guid, bytes);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BLACK_MARKET);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleBlackMarketHello - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // Remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendBlackMarketHello(guid);
}

void WorldSession::SendBlackMarketHello(uint64 npcGuid)
{
    WorldPacket data(SMSG_BLACKMARKET_OPEN_RESULT, 9);
    ObjectGuid guid = npcGuid;

    data.WriteBit(guid[2]);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[1]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[5]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[0]);
    data.WriteBit(1);                // unk
    data.WriteBit(guid[6]);

    uint8 bytes[8] = { 7, 1, 4, 3, 2, 6, 0, 5 };
    data.WriteBytesSeq(guid, bytes);

    SendPacket(&data);
}

void WorldSession::HandleBlackMarketRequestItems(WorldPacket& recvData)
{
    ObjectGuid guid;
    uint32 unk;

    recvData >> unk;

    uint8 bits[8] = { 4, 2, 6, 3, 1, 7, 5, 0 };
    recvData.ReadBitInOrder(guid, bits);

    uint8 bytes[8] = { 6, 4, 0, 7, 5, 2, 1, 3 };
    recvData.ReadBytesSeq(guid, bytes);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BLACK_MARKET);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleBlackMarketRequestItems - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    SendBlackMarketRequestItemsResult();
}

void WorldSession::SendBlackMarketRequestItemsResult()
{
    WorldPacket data;
    sBlackMarketMgr->BuildBlackMarketAuctionsPacket(data, GetPlayer()->GetGUIDLow());
    SendPacket(&data);
}

void WorldSession::HandleBlackMarketBid(WorldPacket& recvData)
{
    ObjectGuid guid;
    uint32 itemid, id;
    uint64 price;

    recvData >> price;
    recvData >> itemid;
    recvData >> id;

    uint8 bits[8] = { 4, 2, 1, 0, 6, 7, 3, 5 };
    recvData.ReadBitInOrder(guid, bits);

    uint8 bytes[8] = { 4, 0, 3, 1, 7, 5, 2, 6 };
    recvData.ReadBytesSeq(guid, bytes);

    sLog->outDebug(LOG_FILTER_NETWORKIO, ">> HandleBid >> id : %u, price : %u, itemid : %u", id, price, itemid);

    if (!price)
        return;

    Creature* creature = _player->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BLACK_MARKET);
    if (!creature)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleBlackMarketBid - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    BMAuctionEntry* auction = sBlackMarketMgr->GetAuction(id);
    if (!auction)
        return;

    if (auction->bidder == _player->GetGUIDLow()) // Trying to cheat
        return;

    if (!_player->HasEnoughMoney(price))
        return;

    if (auction->bid >= price && price != auction->bm_template->startBid) // Trying to cheat
        return;

    _player->ModifyMoney(-int64(price));

    sBlackMarketMgr->UpdateAuction(auction, price, _player);

    SendBlackMarketBidResult(itemid, id);
    SendBlackMarketRequestItemsResult();
}

void WorldSession::SendBlackMarketBidResult(uint32 itemEntry, uint32 auctionId)
{
    WorldPacket data(SMSG_BLACKMARKET_BID_ON_ITEM_RESULT, 12);

    data << uint32(auctionId);
    data << uint32(0);
    data << uint32(itemEntry);
    SendPacket(&data);
}
