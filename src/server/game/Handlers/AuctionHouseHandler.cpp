/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "AuctionHouseMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "UpdateMask.h"
#include "Util.h"
#include "AccountMgr.h"

//void called when player click on auctioneer npc
void WorldSession::HandleAuctionHelloOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    uint8 bitOrder[8] = {7, 4, 6, 2, 0, 1, 3, 5};
    recvData.ReadBitInOrder(guid, bitOrder);

    uint8 byteOrder[8] = {4, 2, 6, 0, 5, 3, 7, 1};
    recvData.ReadBytesSeq(guid, byteOrder);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleAuctionHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendAuctionHello((ObjectGuid)guid, unit);
}

//this void causes that auction window is opened
void WorldSession::SendAuctionHello(ObjectGuid guid, Creature* unit)
{
    if (GetPlayer()->getLevel() < sWorld->getIntConfig(CONFIG_AUCTION_LEVEL_REQ))
    {
        SendNotification(GetTrinityString(LANG_AUCTION_REQ), sWorld->getIntConfig(CONFIG_AUCTION_LEVEL_REQ));
        return;
    }

    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntry(unit->getFaction());
    if (!ahEntry)
        return;

    WorldPacket data(SMSG_AUCTION_HELLO_RESPONSE, 12);
    data.WriteBit(guid[2]);
    data.WriteBit(guid[0]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[1]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[6]);
    data.WriteBit(1); // 3.3.3: 1 - AH enabled, 0 - AH disabled
    data.WriteBit(guid[3]);
    data.WriteBit(guid[5]);

    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[4]);
    data << uint32(ahEntry->houseId); // dword18
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[0]);
    SendPacket(&data);
}

//call this method when player bids, creates, or deletes auction
void WorldSession::SendAuctionCommandResult(AuctionEntry* auction, uint32 action, uint32 errorCode, uint32 bidError)
{
    ObjectGuid guid = 0;
    
    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT);
    data << uint32(action);
    data << uint32(auction ? auction->Id : 0);
    data << uint32(bidError);
    data << uint32(errorCode);

    data.WriteBit(!false);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[1]);
    data.WriteBit(guid[2]);
    data.WriteBit(guid[6]);
    data.WriteBit(guid[0]);
    data.WriteBit(guid[5]);
    data.WriteBit(!(action == AUCTION_PLACE_BID || action == ERR_AUCTION_HIGHER_BID));
    data.WriteBit(!(action == ERR_AUCTION_HIGHER_BID));

    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[6]);

    if (action == AUCTION_PLACE_BID || action == ERR_AUCTION_HIGHER_BID)
        data << uint64(auction ? (auction->bid ? auction->GetAuctionOutBid() : 0) : 0);

    if (action == ERR_AUCTION_HIGHER_BID)
        data << uint64(auction ? (auction->bid ? auction->GetAuctionOutBid() : 0) : 0);

    SendPacket(&data);
}

//this function sends notification, if bidder is online
void WorldSession::SendAuctionBidderNotification(uint32 location, uint32 auctionId, uint64 bidder, uint64 bidSum, uint32 diff, uint32 itemEntry)
{
    ObjectGuid bidderGuid = bidder;

    // Buyout
    if (!bidSum && !diff)
    {
        WorldPacket data(SMSG_AUCTION_BUYOUT_NOTIFICATION, 5 * 4 + 1 + 8);

        uint8 bits[8] = { 4, 7, 5, 2, 0, 1, 6, 3 };
        data.WriteBitInOrder(bidderGuid, bits);

        data.WriteByteSeq(bidderGuid[3]);
        data.WriteByteSeq(bidderGuid[1]);
        data << uint32(location);
        data.WriteByteSeq(bidderGuid[2]);
        data.WriteByteSeq(bidderGuid[7]);
        data.WriteByteSeq(bidderGuid[0]);
        data << uint32(itemEntry);
        data.WriteByteSeq(bidderGuid[6]);
        data.WriteByteSeq(bidderGuid[4]);
        data.WriteByteSeq(bidderGuid[5]);
        data << uint32(diff);
        data << uint32(bidSum);
        data << uint32(auctionId);
        
        SendPacket(&data);
    }
    else
    {
        WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, 5 * 4 + 2 * 8 + 1 + 8);

        uint8 bits[8] = { 1, 0, 4, 5, 6, 2, 7, 3 };
        data.WriteBitInOrder(bidderGuid, bits);

        data << uint64(bidSum);
        data.WriteByteSeq(bidderGuid[6]);
        data.WriteByteSeq(bidderGuid[7]);
        data.WriteByteSeq(bidderGuid[3]);
        data << uint32(auctionId);
        data.WriteByteSeq(bidderGuid[2]);
        data << uint32(itemEntry);
        data << uint32(location);
        data << uint32(0);
        data << uint64(diff);
        data.WriteByteSeq(bidderGuid[5]);
        data.WriteByteSeq(bidderGuid[4]);
        data.WriteByteSeq(bidderGuid[1]);
        data.WriteByteSeq(bidderGuid[0]);
        data << uint32(0);
        SendPacket(&data);
    }
}

// this void causes on client to display: "Your auction sold"
void WorldSession::SendAuctionOwnerNotification(AuctionEntry* auction, bool sold)
{
    WorldPacket data(SMSG_AUCTION_OWNER_BID_NOTIFICATION, 40);

    data << uint64(auction->bid);
    data << float(3600);                    // Time in seconds before money received
    data << uint32(auction->itemEntry);
    data << uint32(0);
    data << uint32(0);                      // Unk
    data << uint32(auction->Id);
    data.WriteBit(sold);

    SendPacket(&data);
}

void WorldSession::SendAuctionRemovedNotification(uint32 auctionId, uint32 itemEntry, int32 randomPropertyId)
{
    // No more sended on 5.4.x official
    /*WorldPacket data(SMSG_AUCTION_REMOVED_NOTIFICATION, (4+4+4));
    data << uint32(auctionId);
    data << uint32(itemEntry);
    data << uint32(randomPropertyId);
    SendPacket(&data);*/
}

//this void creates new auction and adds auction to some auctionhouse
void WorldSession::HandleAuctionSellItem(WorldPacket& recvData)
{
    ObjectGuid auctioneer;
    uint64 bid, buyout;
    uint32 itemsCount, etime;

    recvData >> bid >> etime >> buyout;
    auctioneer[2] = recvData.ReadBit();
    auctioneer[6] = recvData.ReadBit();
    auctioneer[5] = recvData.ReadBit();
    auctioneer[3] = recvData.ReadBit();
    auctioneer[1] = recvData.ReadBit();
    auctioneer[4] = recvData.ReadBit();
    auctioneer[7] = recvData.ReadBit();
    auctioneer[0] = recvData.ReadBit();
    itemsCount = recvData.ReadBits(5);

    ObjectGuid itemGUIDs[MAX_AUCTION_ITEMS]; // 160 slot = 4x 36 slot bag + backpack 16 slot
    uint32 count[MAX_AUCTION_ITEMS];

    memset(itemGUIDs, 0, sizeof(itemGUIDs));

    if (itemsCount > MAX_AUCTION_ITEMS)
    {
        SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
        return;
    }

    for (uint8 i = 0; i < itemsCount; i++)
    {
        itemGUIDs[i][7] = recvData.ReadBit();
        itemGUIDs[i][3] = recvData.ReadBit();
        itemGUIDs[i][5] = recvData.ReadBit();
        itemGUIDs[i][1] = recvData.ReadBit();
        itemGUIDs[i][6] = recvData.ReadBit();
        itemGUIDs[i][2] = recvData.ReadBit();
        itemGUIDs[i][0] = recvData.ReadBit();
        itemGUIDs[i][4] = recvData.ReadBit();
    }

    for (uint8 i = 0; i < itemsCount; i++)
    {
	 ObjectGuid guid = itemGUIDs[i];

	 recvData.ReadByteSeq(guid[0]);
	 recvData.ReadByteSeq(guid[2]);
	 recvData.ReadByteSeq(guid[4]);
	 recvData.ReadByteSeq(guid[5]);
	 recvData.ReadByteSeq(guid[7]);
	 recvData.ReadByteSeq(guid[3]);
	 recvData.ReadByteSeq(guid[6]);
	 recvData.ReadByteSeq(guid[1]);
     recvData >> count[i];

	 for (uint32 j = 0; j < MAX_AUCTION_ITEMS; ++j)
	 {
            if (guid != 0 && itemGUIDs[j] != 0 && guid == itemGUIDs[j])
            {
                // duplicate guid, cheating
                SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
                return;
            }
	 }

	 itemGUIDs[i] = guid;

	 if (!itemGUIDs[i] || !count[i] || count[i] > 1000)
	     return;
    }

    recvData.ReadByteSeq(auctioneer[3]);
    recvData.ReadByteSeq(auctioneer[5]);
    recvData.ReadByteSeq(auctioneer[1]);
    recvData.ReadByteSeq(auctioneer[2]);
    recvData.ReadByteSeq(auctioneer[0]);
    recvData.ReadByteSeq(auctioneer[7]);
    recvData.ReadByteSeq(auctioneer[6]);
    recvData.ReadByteSeq(auctioneer[4]);

    if (!bid || !etime)
        return;

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(auctioneer, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
        return;

    AuctionHouseEntry const* auctionHouseEntry = AuctionHouseMgr::GetAuctionHouseEntry(creature->getFaction());
    if (!auctionHouseEntry)
        return;

    etime *= MINUTE;

    switch (etime)
    {
        case 1*MIN_AUCTION_TIME:
        case 2*MIN_AUCTION_TIME:
        case 4*MIN_AUCTION_TIME:
            break;
        default:
            return;
    }

    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    Item* items[MAX_AUCTION_ITEMS];

    uint32 finalCount = 0;
    uint32 true_item_entry = 0;

    for (uint32 i = 0; i < itemsCount; ++i)
    {
        Item* item = _player->GetItemByGuid(itemGUIDs[i]);

        if (!item)
        {
            SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_ITEM_NOT_FOUND);
            return;
        }

        if (!true_item_entry)
        {
            true_item_entry = item->GetEntry();
        }
        else
        {
            if (item->GetEntry() != true_item_entry)
            {
                SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_ITEM_NOT_FOUND);
                return;
            }
        }

        if (sAuctionMgr->GetAItem(item->GetGUIDLow()) || !item->CanBeTraded() || item->IsNotEmptyBag() ||
            item->GetTemplate()->Flags & ITEM_PROTO_FLAG_CONJURED || item->GetUInt32Value(ITEM_FIELD_DURATION) ||
            item->GetCount() < count[i])
        {
            SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
            return;
        }

        items[i] = item;
        finalCount += count[i];
    }

    if (!finalCount)
    {
        SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
        return;
    }

    if (!GetPlayer()->HasItemCount(true_item_entry, finalCount))
    {
        SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_ITEM_NOT_FOUND);
        return;
    }

    for (uint32 i = 0; i < itemsCount; ++i)
    {
        Item* item = items[i];

        if (item->GetMaxStackCount() < finalCount)
        {
            SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
            return;
        }
    }

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(creature->getFaction());

    for (uint32 i = 0; i < itemsCount; ++i)
    {
        Item* item = items[i];

        uint32 auctionTime = uint32(etime * sWorld->getRate(RATE_AUCTION_TIME));
        uint32 deposit = sAuctionMgr->GetAuctionDeposit(auctionHouseEntry, etime, item, finalCount);
        if (!_player->HasEnoughMoney((uint64)deposit))
        {
            SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_NOT_ENOUGHT_MONEY);
            return;
        }

        _player->ModifyMoney(-int32(deposit));

        AuctionEntry* AH = new AuctionEntry;
        AH->Id = sObjectMgr->GenerateAuctionID();

        if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
            AH->auctioneer = 174444;
        else
            AH->auctioneer = creature->GetSpawnId();

        ASSERT(sObjectMgr->GetCreatureData(AH->auctioneer)); // Tentative de vendre un item a un pnj qui n'existe pas, mieux vaut crash ici sinon l'item en question risque de disparaitre tout simplement

        // Required stack size of auction matches to current item stack size, just move item to auctionhouse
        if (itemsCount == 1 && item->GetCount() == count[i])
        {
            if (GetSecurity() > SEC_PLAYER && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
            {
                sLog->outCommand(GetAccountId(), "", GetPlayer()->GetGUIDLow(), GetPlayer()->GetName(), 0, "", 0, "",
                                "GM %s (Account: %u) create auction: %s (Entry: %u Count: %u)",
                                GetPlayerName().c_str(), GetAccountId(), item->GetTemplate()->Name1.c_str(), item->GetEntry(), item->GetCount());
            }

            AH->itemGUIDLow = item->GetGUIDLow();
            AH->itemEntry = item->GetEntry();
            AH->itemCount = item->GetCount();
            AH->owner = _player->GetGUIDLow();
            AH->startbid = bid;
            AH->bidder = 0;
            AH->bid = 0;
            AH->buyout = buyout;
            AH->expire_time = time(NULL) + auctionTime;
            AH->deposit = deposit;
            AH->auctionHouseEntry = auctionHouseEntry;

            sLog->outInfo(LOG_FILTER_NETWORKIO, "CMSG_AUCTION_SELL_ITEM: Player %s (guid %d) is selling item %s entry %u (guid %d) to auctioneer %u with count %u with initial bid %u with buyout %u and with time %u (in sec) in auctionhouse %u", _player->GetName(), _player->GetGUIDLow(), item->GetTemplate()->Name1.c_str(), item->GetEntry(), item->GetGUIDLow(), AH->auctioneer, item->GetCount(), bid, buyout, auctionTime, AH->GetHouseId());
            sAuctionMgr->AddAItem(item);
            auctionHouse->AddAuction(AH);

            _player->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);

            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            item->DeleteFromInventoryDB(trans);
            item->SaveToDB(trans);
            AH->SaveToDB(trans);
            _player->SaveInventoryAndGoldToDB(trans);
            CharacterDatabase.CommitTransaction(trans);

            SendAuctionCommandResult(AH, AUCTION_SELL_ITEM, ERR_AUCTION_OK);

            GetPlayer()->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION, 1);
            break;
        }
        else // Required stack size of auction does not match to current item stack size, clone item and set correct stack size
        {
            Item* newItem = item->CloneItem(finalCount, _player);
            if (!newItem)
            {
                sLog->outError(LOG_FILTER_NETWORKIO, "CMSG_AUCTION_SELL_ITEM: Could not create clone of item %u", item->GetEntry());
                SendAuctionCommandResult(NULL, AUCTION_SELL_ITEM, ERR_AUCTION_DATABASE_ERROR);
                
                delete AH;
                return;
            }

            if (GetSecurity() > SEC_PLAYER && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
            {
                sLog->outCommand(GetAccountId(), "", GetPlayer()->GetGUIDLow(), GetPlayer()->GetName(), 0, "", 0, "",
                                "GM %s (Account: %u) create auction: %s (Entry: %u Count: %u)",
                                GetPlayerName().c_str(), GetAccountId(), item->GetTemplate()->Name1.c_str(), item->GetEntry(), item->GetCount());
            }

            AH->itemGUIDLow = newItem->GetGUIDLow();
            AH->itemEntry = newItem->GetEntry();
            AH->itemCount = newItem->GetCount();
            AH->owner = _player->GetGUIDLow();
            AH->startbid = bid;
            AH->bidder = 0;
            AH->bid = 0;
            AH->buyout = buyout;
            AH->expire_time = time(NULL) + auctionTime;
            AH->deposit = deposit;
            AH->auctionHouseEntry = auctionHouseEntry;

            sLog->outInfo(LOG_FILTER_NETWORKIO, "CMSG_AUCTION_SELL_ITEM: Player %s (guid %d) is selling item %s entry %u (guid %d) to auctioneer %u with count %u with initial bid %u with buyout %u and with time %u (in sec) in auctionhouse %u", _player->GetName(), _player->GetGUIDLow(), newItem->GetTemplate()->Name1.c_str(), newItem->GetEntry(), newItem->GetGUIDLow(), AH->auctioneer, newItem->GetCount(), bid, buyout, auctionTime, AH->GetHouseId());
            sAuctionMgr->AddAItem(newItem);
            auctionHouse->AddAuction(AH);

            for (uint32 j = 0; j < itemsCount; ++j)
            {
                Item* item2 = items[j];

                // Item stack count equals required count, ready to delete item - cloned item will be used for auction
                if (item2->GetCount() == count[j])
                {
                    _player->MoveItemFromInventory(item2->GetBagSlot(), item2->GetSlot(), true);

                    SQLTransaction trans = CharacterDatabase.BeginTransaction();
                    item2->DeleteFromInventoryDB(trans);
                    item2->DeleteFromDB(trans);
                    CharacterDatabase.CommitTransaction(trans);
                }
                else // Item stack count is bigger than required count, update item stack count and save to database - cloned item will be used for auction
                {
                    item2->SetCount(item2->GetCount() - count[j]);
                    item2->SetState(ITEM_CHANGED, _player);
                    _player->ItemRemovedQuestCheck(item2->GetEntry(), count[j]);
                    item2->SendUpdateToPlayer(_player);

                    SQLTransaction trans = CharacterDatabase.BeginTransaction();
                    item2->SaveToDB(trans);
                    CharacterDatabase.CommitTransaction(trans);
                }
            }

            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            newItem->SaveToDB(trans);
            AH->SaveToDB(trans);
            _player->SaveInventoryAndGoldToDB(trans);
            CharacterDatabase.CommitTransaction(trans);

            SendAuctionCommandResult(AH, AUCTION_SELL_ITEM, ERR_AUCTION_OK);

            GetPlayer()->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION, 1);
            break;
        }
    }

    WorldPacket data(SMSG_AUCTION_OWNER_LIST_RESULT, (4+4+4));
    data << uint32(0);                                     // amount place holder

    uint32 count2 = 0;
    uint32 totalcount = 0;

    auctionHouse->BuildListOwnerItems(data, _player, count2, totalcount);
    data.put<uint32>(0, count2);
    data << uint32(totalcount);
    data << uint32(0);
    SendPacket(&data);
}

// this function is called when client bids or buys out auction
void WorldSession::HandleAuctionPlaceBid(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_AUCTION_PLACE_BID");

    ObjectGuid auctioneer;
    uint32 auctionId;
    uint64 price;

    recvData >> price;
    recvData >> auctionId;

    auctioneer[2] = recvData.ReadBit();
    auctioneer[5] = recvData.ReadBit();
    auctioneer[4] = recvData.ReadBit();
    auctioneer[1] = recvData.ReadBit();
    auctioneer[0] = recvData.ReadBit();
    auctioneer[3] = recvData.ReadBit();
    auctioneer[6] = recvData.ReadBit();
    auctioneer[7] = recvData.ReadBit();

    recvData.ReadByteSeq(auctioneer[4]);
    recvData.ReadByteSeq(auctioneer[7]);
    recvData.ReadByteSeq(auctioneer[5]);
    recvData.ReadByteSeq(auctioneer[3]);
    recvData.ReadByteSeq(auctioneer[6]);
    recvData.ReadByteSeq(auctioneer[2]);
    recvData.ReadByteSeq(auctioneer[1]);
    recvData.ReadByteSeq(auctioneer[0]);

    if (!auctionId || !price)
        return;                                             // check for cheaters

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(auctioneer, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleAuctionPlaceBid - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(auctioneer)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(creature->getFaction());

    AuctionEntry* auction = auctionHouse->GetAuction(auctionId);
    Player* player = GetPlayer();

    if (!auction || auction->owner == player->GetGUIDLow())
    {
        //you cannot bid your own auction:
        //SendAuctionCommandResult(NULL, AUCTION_PLACE_BID, ERR_AUCTION_BID_OWN);
        return;
    }

    // cheating
    if (price <= auction->bid || price < auction->startbid)
        return;

    // price too low for next bid if not buyout
    if ((price < auction->buyout || auction->buyout == 0) &&
        price < auction->bid + auction->GetAuctionOutBid())
    {
        // client already test it but just in case ...
        //SendAuctionCommandResult(auction, AUCTION_PLACE_BID, ERR_AUCTION_HIGHER_BID);
        return;
    }

    if (!player->HasEnoughMoney(price))
    {
        // client already test it but just in case ...
        //SendAuctionCommandResult(auction, AUCTION_PLACE_BID, ERR_AUCTION_NOT_ENOUGHT_MONEY);
        return;
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    if (price < auction->buyout || auction->buyout == 0)
    {
        if (auction->bidder > 0)
        {
            if (auction->bidder == player->GetGUIDLow())
                player->ModifyMoney(-int64(price - auction->bid));
            else
            {
                // mail to last bidder and return money
                sAuctionMgr->SendAuctionOutbiddedMail(auction, price, GetPlayer(), trans);
                player->ModifyMoney(-int64(price));
            }
        }
        else
            player->ModifyMoney(-int64(price));

        auction->bidder = player->GetGUIDLow();
        auction->bid = price;
        GetPlayer()->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID, price);

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_AUCTION_BID);
        stmt->setUInt32(0, auction->bidder);
        stmt->setUInt32(1, auction->bid);
        stmt->setUInt32(2, auction->Id);
        trans->Append(stmt);

        //SendAuctionCommandResult(auction, AUCTION_PLACE_BID, ERR_AUCTION_OK);
    }
    else
    {
        //buyout:
        if (player->GetGUIDLow() == auction->bidder)
            player->ModifyMoney(-int64(auction->buyout - auction->bid));
        else
        {
            player->ModifyMoney(-int64(auction->buyout));
            if (auction->bidder)                          //buyout for bidded auction ..
                sAuctionMgr->SendAuctionOutbiddedMail(auction, auction->buyout, GetPlayer(), trans);
        }
        auction->bidder = player->GetGUIDLow();
        auction->bid = auction->buyout;
        GetPlayer()->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID, auction->buyout);

        //- Mails must be under transaction control too to prevent data loss
        sAuctionMgr->SendAuctionSalePendingMail(auction, trans);
        sAuctionMgr->SendAuctionSuccessfulMail(auction, trans);
        sAuctionMgr->SendAuctionWonMail(auction, trans);

        //SendAuctionCommandResult(auction, AUCTION_PLACE_BID, ERR_AUCTION_OK);

        auction->DeleteFromDB(trans);

        uint32 itemEntry = auction->itemEntry;
        sAuctionMgr->RemoveAItem(auction->itemGUIDLow);
        auctionHouse->RemoveAuction(auction, itemEntry);
    }

    player->SaveInventoryAndGoldToDB(trans);
    CharacterDatabase.CommitTransaction(trans);
}

//this void is called when auction_owner cancels his auction
void WorldSession::HandleAuctionRemoveItem(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_AUCTION_REMOVE_ITEM");

    ObjectGuid auctioneer;
    uint32 auctionId;

    recvData >> auctionId;

    uint8 bitOrder[8] = {1, 6, 0, 5, 2, 7, 4, 3};
    recvData.ReadBitInOrder(auctioneer, bitOrder);

    uint8 byteOrder[8] = {0, 4, 7, 3, 6, 1, 5, 2};
    recvData.ReadBytesSeq(auctioneer, byteOrder);

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(auctioneer, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleAuctionRemoveItem - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(auctioneer)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(creature->getFaction());

    AuctionEntry* auction = auctionHouse->GetAuction(auctionId);
    Player* player = GetPlayer();

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    if (auction && auction->owner == player->GetGUIDLow())
    {
        Item* pItem = sAuctionMgr->GetAItem(auction->itemGUIDLow);
        if (pItem)
        {
            if (auction->bidder > 0)                        // If we have a bidder, we have to send him the money he paid
            {
                uint64 auctionCut = auction->GetAuctionCut();
                if (!player->HasEnoughMoney((uint64)auctionCut))          //player doesn't have enough money, maybe message needed
                    return;
                sAuctionMgr->SendAuctionCancelledToBidderMail(auction, trans, pItem);
                player->ModifyMoney(-int64(auctionCut));
            }

            // item will deleted or added to received mail list
            MailDraft(auction->BuildAuctionMailSubject(AUCTION_CANCELED), AuctionEntry::BuildAuctionMailBody(0, 0, auction->buyout, auction->deposit, 0))
                .AddItem(pItem)
                .SendMailTo(trans, player, auction, MAIL_CHECK_MASK_COPIED);
        }
        else
        {
            sLog->outError(LOG_FILTER_NETWORKIO, "Auction id: %u got non existing item (item guid : %u)!", auction->Id, auction->itemGUIDLow);
            SendAuctionCommandResult(NULL, AUCTION_CANCEL, ERR_AUCTION_DATABASE_ERROR);
            return;
        }
    }
    else
    {
        SendAuctionCommandResult(NULL, AUCTION_CANCEL, ERR_AUCTION_DATABASE_ERROR);
        //this code isn't possible ... maybe there should be assert
        sLog->outError(LOG_FILTER_NETWORKIO, "CHEATER: %u tried to cancel auction (id: %u) of another player or auction is NULL", player->GetGUIDLow(), auctionId);
        return;
    }

    //inform player, that auction is removed
    SendAuctionCommandResult(auction, AUCTION_CANCEL, ERR_AUCTION_OK);

    // Now remove the auction

    player->SaveInventoryAndGoldToDB(trans);
    auction->DeleteFromDB(trans);
    CharacterDatabase.CommitTransaction(trans);

    uint32 itemEntry = auction->itemEntry;
    sAuctionMgr->RemoveAItem(auction->itemGUIDLow);
    auctionHouse->RemoveAuction(auction, itemEntry);
}

//called when player lists his bids
void WorldSession::HandleAuctionListBidderItems(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_AUCTION_LIST_BIDDER_ITEMS");

    ObjectGuid guid;                                        //NPC guid
    uint32 listfrom;                                        //page of auctions
    uint32 outbiddedCount;                                  //count of outbidded auctions
    std::list<uint32> outbiddedAuctionIds;

    recvData >> listfrom;                                   // not used in fact (this list not have page control in client)
    outbiddedCount = recvData.ReadBits(7);

    uint8 bitOrder[8] = {6, 0, 2, 1, 7, 3, 4, 5};
    recvData.ReadBitInOrder(guid, bitOrder);

    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[3]);

    uint32 t;
    for (uint8 i = 0; i < outbiddedCount; i++)
    {
        recvData >> t;
        outbiddedAuctionIds.push_back(t);
    }

    recvData.ReadByteSeq(guid[2]);
    
    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleAuctionListBidderItems - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(creature->getFaction());

    WorldPacket data(SMSG_AUCTION_BIDDER_LIST_RESULT, (4+4+4));
    Player* player = GetPlayer();
    data << uint32(0);                                     //add 0 as count
    uint32 count = 0;
    uint32 totalcount = 0;

    for (std::list<uint32>::iterator itr = outbiddedAuctionIds.begin(); itr != outbiddedAuctionIds.end(); ++itr)
    {
        AuctionEntry* auction = auctionHouse->GetAuction(*itr);
        if (auction && auction->BuildAuctionInfo(data))
        {
            ++totalcount;
            ++count;
        }
    }

    auctionHouse->BuildListBidderItems(data, player, count, totalcount);
    data.put<uint32>(0, count);                           // add count to placeholder
    data << totalcount;
    data << uint32(300);                                  // Official datas
    SendPacket(&data);
}

//this void sends player info about his auctions
void WorldSession::HandleAuctionListOwnerItems(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_AUCTION_LIST_OWNER_ITEMS");

    uint32 listfrom;
    ObjectGuid guid;

    recvData >> listfrom;                                  // not used in fact (this list not have page control in client)

    uint8 bitOrder[8] = {6, 0, 7, 2, 3, 1, 5, 4};
    recvData.ReadBitInOrder(guid, bitOrder);

    uint8 byteOrder[8] = {2, 5, 6, 4, 3, 7, 1, 0};
    recvData.ReadBytesSeq(guid, byteOrder);

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleAuctionListOwnerItems - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(creature->getFaction());

    WorldPacket data(SMSG_AUCTION_OWNER_LIST_RESULT, (4+4+4));
    data << uint32(0);                                     // amount place holder

    uint32 count = 0;
    uint32 totalcount = 0;

    auctionHouse->BuildListOwnerItems(data, _player, count, totalcount);
    data.put<uint32>(0, count);
    data << uint32(totalcount);
    data << uint32(0);
    SendPacket(&data);
}

//this void is called when player clicks on search button
void WorldSession::HandleAuctionListItems(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_AUCTION_LIST_ITEMS");

    uint8 levelmin, levelmax, unkbyte;
    uint32 listfrom, auctionSlotID, auctionMainCategory, auctionSubCategory, quality, dword148;
    ObjectGuid guid;

    recvData >> auctionSubCategory;
    recvData >> levelmin;
    recvData >> quality;
    recvData >> listfrom;
    recvData >> unkbyte;
    recvData >> auctionMainCategory;
    recvData >> auctionSlotID;
    recvData >> levelmax;
    recvData >> dword148;

    // this block looks like it uses some lame byte packing or similar...
    for (uint32 i = 0; i < dword148; i++)
        recvData.read_skip<uint8>();

    auto searchedname_len = recvData.ReadBits(8);
    bool test1 = recvData.ReadBit();
    guid[4] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    guid[6] = recvData.ReadBit();
    guid[2] = recvData.ReadBit();
    bool usable = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid[7] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    guid[3] = recvData.ReadBit();

    std::string searchedname = recvData.ReadString(searchedname_len);
    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[3]);

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
    if (!creature)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleAuctionListItems - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(creature->getFaction());

    //sLog->outDebug(LOG_FILTER_GENERAL, "Auctionhouse search (GUID: %u TypeId: %u)",, list from: %u, searchedname: %s, levelmin: %u, levelmax: %u, auctionSlotID: %u, auctionMainCategory: %u, auctionSubCategory: %u, quality: %u, usable: %u",
    //  GUID_LOPART(guid), GuidHigh2TypeId(GUID_HIPART(guid)), listfrom, searchedname.c_str(), levelmin, levelmax, auctionSlotID, auctionMainCategory, auctionSubCategory, quality, usable);

    WorldPacket data(SMSG_AUCTION_LIST_RESULT, (4+4+4));
    uint32 count = 0;
    uint32 totalcount = 0;
    data << uint32(0);

    // converting string that we try to find to lower case
    std::wstring wsearchedname;
    if (!Utf8toWStr(searchedname, wsearchedname))
        return;

    wstrToLower(wsearchedname);

    auctionHouse->BuildListAuctionItems(data, _player,
        wsearchedname, listfrom, levelmin, levelmax, usable,
        auctionSlotID, auctionMainCategory, auctionSubCategory, quality,
        count, totalcount);

    data.put<uint32>(0, count);
    data << uint32(totalcount);
    data << uint32(300);                                  //unk 2.3.0
    SendPacket(&data);
}

void WorldSession::HandleAuctionListPendingSales(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_AUCTION_LIST_PENDING_SALES");

    WorldPacket data(SMSG_AUCTION_LIST_PENDING_SALES, 4);
    data.WriteBits(0, 2);
    data << uint32(0);
    SendPacket(&data);
}