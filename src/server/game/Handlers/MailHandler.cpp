/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "DatabaseEnv.h"
#include "Mail.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Language.h"
#include "DBCStores.h"
#include "Item.h"
#include "AccountMgr.h"

void WorldSession::HandleSendMail(WorldPacket& recvData)
{
    ObjectGuid mailbox;
    uint64 money, COD;
    std::string receiver, subject, body;
    uint32 bodyLength, subjectLength, receiverLength;
    uint32 unk1, stationery;

    uint32 oldMSTime = getMSTime();

    recvData >> unk1;
    recvData >> COD;
    recvData >> stationery;
    recvData >> money;

    uint8 items_count = recvData.ReadBits(5);

    if (items_count > MAX_MAIL_ITEMS)                       // client limit
    {
        GetPlayer()->SendMailResult(0, MAIL_SEND, MAIL_ERR_TOO_MANY_ATTACHMENTS);
        recvData.rfinish();                   // set to end to avoid warnings spam
        return;
    }
    
    mailbox[0] = recvData.ReadBit();

    ObjectGuid itemGUIDs[MAX_MAIL_ITEMS];

    uint8 bitOrder[8] = { 6, 5, 1, 3, 0, 4, 7, 2 };
    for (uint8 i = 0; i < items_count; ++i)
        recvData.ReadBitInOrder(itemGUIDs[i], bitOrder);
    

    mailbox[2] = recvData.ReadBit();
    mailbox[3] = recvData.ReadBit();
    mailbox[7] = recvData.ReadBit();
    mailbox[5] = recvData.ReadBit();
    bodyLength = recvData.ReadBits(11);
    receiverLength = recvData.ReadBits(9);
    //receiverLength = receiverLength << 1;
    //bool receiverbit = recvData.ReadBit(); // receiver length's last bit is 1
    //if (receiverbit)
    //    receiverLength |= 1;
    mailbox[1] = recvData.ReadBit();
    mailbox[4] = recvData.ReadBit();
    mailbox[6] = recvData.ReadBit();
    subjectLength = recvData.ReadBits(9);
    //subjectLength = subjectLength << 1;
    //bool subjectbit = recvData.ReadBit(); // subject length's last bit is 1
    //if (subjectbit)
    //    subjectLength |= 1;

    recvData.ReadByteSeq(mailbox[0]);
    recvData.ReadByteSeq(mailbox[2]);
    recvData.ReadByteSeq(mailbox[6]);
    recvData.ReadByteSeq(mailbox[3]);
    recvData.ReadByteSeq(mailbox[4]);
    recvData.ReadByteSeq(mailbox[5]);
    

    for (uint8 i = 0; i < items_count; ++i)
    {
        recvData.ReadByteSeq(itemGUIDs[i][6]);
        recvData.ReadByteSeq(itemGUIDs[i][3]);
        recvData.ReadByteSeq(itemGUIDs[i][0]);
        recvData.ReadByteSeq(itemGUIDs[i][5]);
        recvData.ReadByteSeq(itemGUIDs[i][7]);
        recvData.ReadByteSeq(itemGUIDs[i][4]);
        recvData.read_skip<uint8>();           // item slot in mail, not used
        recvData.ReadByteSeq(itemGUIDs[i][1]);
        recvData.ReadByteSeq(itemGUIDs[i][2]);
    }

    subject = recvData.ReadString((subjectLength));
    recvData.ReadByteSeq(mailbox[1]);
    receiver = recvData.ReadString((receiverLength));
    recvData.ReadByteSeq(mailbox[7]);
    body = recvData.ReadString(bodyLength);

    Player* player = _player;

    if (!GetPlayer()->GetGameObjectIfCanInteractWith(mailbox, GAMEOBJECT_TYPE_MAILBOX))
    {
        player->SendMailResult(0, MAIL_SEND, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    if (receiver.empty())
    {
        player->SendMailResult(0, MAIL_SEND, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    auto delimeter_pos = receiver.find("-");
    if (delimeter_pos > 0)
        receiver = receiver.substr(0, delimeter_pos);

    if (player->getLevel() < sWorld->getIntConfig(CONFIG_MAIL_LEVEL_REQ))
    {
        SendNotification(GetTrinityString(LANG_MAIL_SENDER_REQ), sWorld->getIntConfig(CONFIG_MAIL_LEVEL_REQ));
        player->SendMailResult(0, MAIL_SEND, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    uint64 rc = 0;
    if (normalizePlayerName(receiver))
        rc = sObjectMgr->GetPlayerGUIDByName(receiver);

    if (!rc)
    {
        player->SendMailResult(0, MAIL_SEND, MAIL_ERR_RECIPIENT_NOT_FOUND);
        return;
    }

    if (player->GetGUID() == rc)
    {
        player->SendMailResult(0, MAIL_SEND, MAIL_ERR_CANNOT_SEND_TO_SELF);
        return;
    }

    uint32 cost = items_count ? 30 * items_count : 30;  // price hardcoded in client

    uint64 reqmoney = cost + money;
    if (!player->HasEnoughMoney(reqmoney) && !player->isGameMaster())
    {
        player->SendMailResult(0, MAIL_SEND, MAIL_ERR_NOT_ENOUGH_MONEY);
        return;
    }

    Player* receive = ObjectAccessor::FindPlayer(rc);

    uint32 rc_team = 0;
    uint8 mails_count = 0;                                  //do not allow to send to one player more than 100 mails

    if (receive)
    {
        rc_team = receive->GetTeam();
        mails_count = receive->GetMailSize();
    }

    //do not allow to have more than 100 mails in mailbox.. mails count is in opcode uint8!!! - so max can be 255..
    if (mails_count > 100)
    {
        player->SendMailResult(0, MAIL_SEND, MAIL_ERR_RECIPIENT_CAP_REACHED);
        return;
    }

    // test the receiver's Faction... or all items are account bound
    bool accountBound = items_count ? true : false;
    for (uint8 i = 0; i < items_count; ++i)
    {
        Item* item = player->GetItemByGuid(itemGUIDs[i]);
        if (item)
        {
            ItemTemplate const* itemProto = item->GetTemplate();
            if (!itemProto || !(itemProto->Flags & ITEM_PROTO_FLAG_BIND_TO_ACCOUNT))
            {
                accountBound = false;
                break;
            }
        }
    }

    uint32 rc_account = receive
        ? receive->GetSession()->GetAccountId()
        : sObjectMgr->GetPlayerAccountIdByGUID(rc);

    Item* items[MAX_MAIL_ITEMS];

    for (uint8 i = 0; i < items_count; ++i)
    {
        if (!itemGUIDs[i])
        {
            player->SendMailResult(0, MAIL_SEND, MAIL_ERR_MAIL_ATTACHMENT_INVALID);
            return;
        }

        Item* item = player->GetItemByGuid(itemGUIDs[i]);

        // prevent sending bag with items (cheat: can be placed in bag after adding equipped empty bag to mail)
        if (!item)
        {
            player->SendMailResult(0, MAIL_SEND, MAIL_ERR_MAIL_ATTACHMENT_INVALID);
            return;
        }

        if (!item->CanBeTraded(true))
        {
            player->SendMailResult(0, MAIL_SEND, MAIL_ERR_EQUIP_ERROR, EQUIP_ERR_MAIL_BOUND_ITEM);
            return;
        }

        if (item->IsBoundAccountWide() && item->IsSoulBound() && player->GetSession()->GetAccountId() != rc_account)
        {
            player->SendMailResult(0, MAIL_SEND, MAIL_ERR_EQUIP_ERROR, EQUIP_ERR_NOT_SAME_ACCOUNT);
            return;
        }

        if (item->GetTemplate()->Flags & ITEM_PROTO_FLAG_CONJURED || item->GetUInt32Value(ITEM_FIELD_DURATION))
        {
            player->SendMailResult(0, MAIL_SEND, MAIL_ERR_EQUIP_ERROR, EQUIP_ERR_MAIL_BOUND_ITEM);
            return;
        }

        if (COD && item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAPPED))
        {
            player->SendMailResult(0, MAIL_SEND, MAIL_ERR_CANT_SEND_WRAPPED_COD);
            return;
        }

        if (item->IsNotEmptyBag())
        {
            player->SendMailResult(0, MAIL_SEND, MAIL_ERR_EQUIP_ERROR, EQUIP_ERR_DESTROY_NONEMPTY_BAG);
            return;
        }

        items[i] = item;
    }

    // Check for spamming
    if (!UpdateAntispamCount())
    {
        player->SendMailResult(0, MAIL_SEND, MAIL_ERR_INTERNAL_ERROR);
        SendNotification(GetTrinityString(LANG_ANTISPAM_ERROR));
        return;
    }

    // Check for special symbols
    if (!checkMailText(subject) ||  !checkMailText(body))
    {
        player->SendMailResult(0, MAIL_SEND, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    player->SendMailResult(0, MAIL_SEND, MAIL_OK);

    player->ModifyMoney(-int64(reqmoney));
    player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL, cost);

    bool needItemDelay = false;

    MailDraft draft(subject, body);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    if (items_count > 0 || money > 0)
    {
        if (items_count > 0)
        {
            for (uint8 i = 0; i < items_count; ++i)
            {
                Item* item = items[i];
                if (!AccountMgr::IsPlayerAccount(GetSecurity()) && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
                {
                    sLog->outCommand(GetAccountId(), "", GetPlayer()->GetGUIDLow(), GetPlayer()->GetName(),
                                    rc_account, "", 0, receiver.c_str(),
                                    "GM %s (Account: %u) mail item: %s (Entry: %u Count: %u) to player: %s (Account: %u)",
                                    GetPlayerName().c_str(), GetAccountId(), item->GetTemplate()->Name1.c_str(), item->GetEntry(), item->GetCount(), receiver.c_str(), rc_account);
                }

                item->SetNotRefundable(GetPlayer()); // makes the item no longer refundable
                player->MoveItemFromInventory(items[i]->GetBagSlot(), item->GetSlot(), true);

                item->DeleteFromInventoryDB(trans);     // deletes item from character's inventory
                item->SetOwnerGUID(rc);
                item->SaveToDB(trans);                  // recursive and not have transaction guard into self, item not in inventory and can be save standalone

                draft.AddItem(item);
            }

            // if item send to character at another account, then apply item delivery delay
            needItemDelay = player->GetSession()->GetAccountId() != rc_account;
        }

        if (money > 0 && !AccountMgr::IsPlayerAccount(GetSecurity()) && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
        {
            //TODO: character guid
            sLog->outCommand(GetAccountId(), "", GetPlayer()->GetGUIDLow(), GetPlayer()->GetName(),
                            rc_account, "", 0, receiver.c_str(),
                            "GM %s (Account: %u) mail money: %u to player: %s (Account: %u)",
                            GetPlayerName().c_str(), GetAccountId(), money, receiver.c_str(), rc_account);
        }
    }

    // Guild Mail
    if (receive && receive->GetGuildId() && player->GetGuildId())
        if (player->HasAura(83951) && (player->GetGuildId() == receive->GetGuildId()))
            needItemDelay = false;

    // If theres is an item, there is a one hour delivery delay if sent to another account's character.
    uint32 deliver_delay = needItemDelay ? sWorld->getIntConfig(CONFIG_MAIL_DELIVERY_DELAY) : 0;

    // will delete item or place to receiver mail list
    draft
        .AddMoney(money)
        .AddCOD(COD)
        .SendMailTo(trans, MailReceiver(receive, GUID_LOPART(rc)), MailSender(player), body.empty() ? MAIL_CHECK_MASK_COPIED : MAIL_CHECK_MASK_HAS_BODY, deliver_delay);

    player->SaveInventoryAndGoldToDB(trans);
    CharacterDatabase.CommitTransaction(trans);
}

// Called when mail is read
void WorldSession::HandleMailMarkAsRead(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_MAIL_MARK_AS_READ");

    ObjectGuid mailbox;
    uint32 mailId;

    recvData >> mailId;

    mailbox[4] = recvData.ReadBit();
    mailbox[0] = recvData.ReadBit();
    mailbox[2] = recvData.ReadBit();
    mailbox[7] = recvData.ReadBit();
    mailbox[1] = recvData.ReadBit();
    mailbox[6] = recvData.ReadBit();
    mailbox[3] = recvData.ReadBit();
    recvData.ReadBit();
    mailbox[5] = recvData.ReadBit();

    uint8 bytesOrder[8] = { 2, 5, 0, 6, 1, 3, 7, 4 };
    recvData.ReadBytesSeq(mailbox, bytesOrder);

    if (!GetPlayer()->GetGameObjectIfCanInteractWith(mailbox, GAMEOBJECT_TYPE_MAILBOX))
        return;

    Player* player = _player;
    Mail* m = player->GetMail(mailId);
    if (m)
    {
        if (player->unReadMails)
            --player->unReadMails;
        m->checked = m->checked | MAIL_CHECK_MASK_READ;
        player->m_mailsUpdated = true;
        m->state = MAIL_STATE_CHANGED;
    }
}

// Called when client deletes mail
void WorldSession::HandleMailDelete(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_MAIL_DELETE");

    uint32 mailId;
    recvData.read_skip<uint32>();
    recvData >> mailId;                       // mailTemplateId
    
    Mail* m = _player->GetMail(mailId);
    Player* player = _player;
    player->m_mailsUpdated = true;
    if (m)
    {
        // delete shouldn't show up for COD mails
        if (m->COD)
        {
            player->SendMailResult(mailId, MAIL_DELETED, MAIL_ERR_INTERNAL_ERROR);
            return;
        }

        m->state = MAIL_STATE_DELETED;
    }
    player->SendMailResult(mailId, MAIL_DELETED, MAIL_OK);
}

void WorldSession::HandleMailReturnToSender(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_MAIL_RETURN_TO_SENDER");

    ObjectGuid owner;
    uint32 mailId;
    recvData >> mailId;

    uint8 bitOrder[8] = { 1, 7, 6, 3, 4, 0, 2, 5 };
    recvData.ReadBitInOrder(owner, bitOrder);

    uint8 byteOrder[8] = { 7, 3, 6, 0, 2, 1, 5, 4 };
    recvData.ReadBytesSeq(owner, byteOrder);

    //if (!GetPlayer()->GetGameObjectIfCanInteractWith(mailbox, GAMEOBJECT_TYPE_MAILBOX))
    //    return;

    Player* player = _player;
    Mail* m = player->GetMail(mailId);
    if (!m || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL))
    {
        player->SendMailResult(mailId, MAIL_RETURNED_TO_SENDER, MAIL_ERR_INTERNAL_ERROR);
        return;
    }
    //we can return mail now
    //so firstly delete the old one
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_BY_ID);
    stmt->setUInt32(0, mailId);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_ITEM_BY_ID);
    stmt->setUInt32(0, mailId);
    trans->Append(stmt);

    player->RemoveMail(mailId);

    // only return mail if the player exists (and delete if not existing)
    if (m->messageType == MAIL_NORMAL && m->sender)
    {
        MailDraft draft(m->subject, m->body);
        if (m->mailTemplateId)
            draft = MailDraft(m->mailTemplateId, false);     // items already included

        if (m->HasItems())
        {
            for (MailItemInfoVec::iterator itr2 = m->items.begin(); itr2 != m->items.end(); ++itr2)
            {
                Item* item = player->GetMItem(itr2->item_guid);
                if (item)
                    draft.AddItem(item);
                else
                {
                    //WTF?
                }

                player->RemoveMItem(itr2->item_guid);
            }
        }
        draft.AddMoney(m->money).SendReturnToSender(GetAccountId(), m->receiver, m->sender, trans);
    }

    CharacterDatabase.CommitTransaction(trans);

    delete m;                                               //we can deallocate old mail
    player->SendMailResult(mailId, MAIL_RETURNED_TO_SENDER, MAIL_OK);
}

// Called when player takes item attached in mail
void WorldSession::HandleMailTakeItem(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_MAIL_TAKE_ITEM");

    ObjectGuid mailbox;
    uint32 mailId;
    uint32 itemId;

    recvData >> itemId;
    recvData >> mailId;
    

    uint8 bitsOrder[8] = { 3, 2, 0, 5, 6, 1, 7, 4 };
    recvData.ReadBitInOrder(mailbox, bitsOrder);

    uint8 bytesOrder[8] = { 3, 2, 0, 5, 6, 7, 1, 4 };
    recvData.ReadBytesSeq(mailbox, bytesOrder);

    if (!GetPlayer()->GetGameObjectIfCanInteractWith(mailbox, GAMEOBJECT_TYPE_MAILBOX))
        return;

    Player* player = _player;

    Mail* m = player->GetMail(mailId);
    if (!m || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL))
    {
        player->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    // prevent cheating with skip client money check
    if (!player->HasEnoughMoney(uint64(m->COD)))
    {
        player->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_ERR_NOT_ENOUGH_MONEY);
        return;
    }

    Item* item = player->GetMItem(itemId);

    ItemPosCountVec dest;
    uint8 msg = _player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item, false);
    if (msg == EQUIP_ERR_OK)
    {
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        m->RemoveItem(itemId);
        m->removedItems.push_back(itemId);

        if (m->COD > 0)                                     //if there is COD, take COD money from player and send them to sender by mail
        {
            uint64 sender_guid = MAKE_NEW_GUID(m->sender, 0, HighGuid::Player);
            Player* receive = ObjectAccessor::FindPlayer(sender_guid);

            uint32 sender_accId = 0;

            if (!AccountMgr::IsPlayerAccount(GetSecurity()) && sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
            {
                std::string sender_name;
                if (receive)
                {
                    sender_accId = receive->GetSession()->GetAccountId();
                    sender_name = receive->GetName();
                }
                else
                {
                    // can be calculated early
                    sender_accId = sObjectMgr->GetPlayerAccountIdByGUID(sender_guid);

                    if (!sObjectMgr->GetPlayerNameByGUID(sender_guid, sender_name))
                        sender_name = sObjectMgr->GetTrinityStringForDBCLocale(LANG_UNKNOWN);
                }
                sLog->outCommand(GetAccountId(), "", GetPlayer()->GetGUIDLow(), GetPlayer()->GetName(),
                                sender_accId, "", sender_guid, sender_name.c_str(),
                                "GM %s (Account: %u) receive mail item: %s (Entry: %u Count: %u) and send COD money: %u to player: %s (Account: %u)",
                                GetPlayerName().c_str(), GetAccountId(), item->GetTemplate()->Name1.c_str(), item->GetEntry(), item->GetCount(), m->COD, sender_name.c_str(), sender_accId);
            }
            else if (!receive)
                sender_accId = sObjectMgr->GetPlayerAccountIdByGUID(sender_guid);

            // check player existence
            if (receive || sender_accId)
            {
                MailDraft(m->subject, "")
                    .AddMoney(m->COD)
                    .SendMailTo(trans, MailReceiver(receive, m->sender), MailSender(MAIL_NORMAL, m->receiver), MAIL_CHECK_MASK_COD_PAYMENT);
            }

            player->ModifyMoney(-int32(m->COD));
        }
        m->COD = 0;
        m->state = MAIL_STATE_CHANGED;
        player->m_mailsUpdated = true;
        player->RemoveMItem(item->GetGUIDLow());

        uint32 count = item->GetCount();                      // save counts before store and possible merge with deleting
        item->SetState(ITEM_UNCHANGED);                       // need to set this state, otherwise item cannot be removed later, if neccessary
        player->MoveItemToInventory(dest, item, true);

        player->SaveInventoryAndGoldToDB(trans);
        player->_SaveMail(trans);
        CharacterDatabase.CommitTransaction(trans);

        player->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_OK, 0, itemId, count);
    }
    else
        player->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_ERR_EQUIP_ERROR, msg);
}

void WorldSession::HandleMailTakeMoney(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_MAIL_TAKE_MONEY");

    ObjectGuid mailbox;
    uint64 money;
    uint32 mailId;

    recvData >> money;
    recvData >> mailId;

    uint8 bitsOrder[8] = { 3, 5, 4, 2, 6, 1, 0, 7 };
    recvData.ReadBitInOrder(mailbox, bitsOrder);

    uint8 bytesOrder[8] = { 5, 6, 0, 2, 3, 7, 1, 4 };
    recvData.ReadBytesSeq(mailbox, bytesOrder);

    if (!GetPlayer()->GetGameObjectIfCanInteractWith(mailbox, GAMEOBJECT_TYPE_MAILBOX))
        return;

    Player* player = _player;

    Mail* m = player->GetMail(mailId);
    if ((!m || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL)) ||
        (money > 0 && m->money != money))
    {
        player->SendMailResult(mailId, MAIL_MONEY_TAKEN, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    // Don't take money if it exceed the max amount
    if (player->GetMoney() + money > MAX_MONEY_AMOUNT)
    {
        player->SendMailResult(mailId, MAIL_MONEY_TAKEN, MAIL_ERR_EQUIP_ERROR);
        return;
    }

    player->SendMailResult(mailId, MAIL_MONEY_TAKEN, MAIL_OK);

    player->ModifyMoney(money);
    m->money = 0;
    m->state = MAIL_STATE_CHANGED;
    player->m_mailsUpdated = true;

    // save money and mail to prevent cheating
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    player->SaveGoldToDB(trans);
    player->_SaveMail(trans);
    CharacterDatabase.CommitTransaction(trans);
}

// Called when player lists his received mails
void WorldSession::HandleGetMailList(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_GET_MAIL_LIST");

    ObjectGuid mailBoxGuid;

    uint8 bitsOrder[8] = { 5, 3, 6, 0, 1, 4, 7, 2 };
    recvData.ReadBitInOrder(mailBoxGuid, bitsOrder);

    uint8 bytesOrder[8] = { 4, 7, 3, 5, 6, 0, 1, 2 };
    recvData.ReadBytesSeq(mailBoxGuid, bytesOrder);

    if (!GetPlayer()->GetGameObjectIfCanInteractWith(mailBoxGuid, GAMEOBJECT_TYPE_MAILBOX))
        return;

    Player* player = _player;

    //load players mails, and mailed items
    if (!player->m_mailsLoaded)
        player->_LoadMail();

    // client can't work with packets > max int16 value
    const uint32 maxPacketSize = 32767;

    uint32 mailsCount = 0;                                 // real send to client mails amount
    uint32 realCount  = 0;                                 // real mails amount

    WorldPacket data(SMSG_MAIL_LIST_RESULT);
    ByteBuffer dataBuffer;

    time_t cur_time = time(NULL);
    for (PlayerMails::iterator itr = player->GetMailBegin(); itr != player->GetMailEnd(); ++itr)
    {
        // Only first 50 mails are displayed
        if (mailsCount >= 50)
        {
            realCount += 1;
            continue;
        }

        // skip deleted or not delivered (deliver delay not expired) mails
        if ((*itr)->state == MAIL_STATE_DELETED || cur_time < (*itr)->deliver_time)
            continue;

        uint8 item_count = (*itr)->items.size();            // max count is MAX_MAIL_ITEMS (12)

        size_t next_mail_size = 2+4+1+((*itr)->messageType == MAIL_NORMAL ? 8 : 4)+4*8+((*itr)->subject.size()+1)+((*itr)->body.size()+1)+1+item_count*(1+4+4+MAX_INSPECTED_ENCHANTMENT_SLOT*3*4+4+4+4+4+4+4+1);

        if (data.wpos()+next_mail_size > maxPacketSize)
        {
            realCount += 1;
            continue;
        }

        ++realCount;
        ++mailsCount;
    }

    data.WriteBits(mailsCount, 18);

    mailsCount = 0;

    for (PlayerMails::iterator itr = player->GetMailBegin(); itr != player->GetMailEnd(); ++itr)
    {
        // Only first 50 mails are displayed
        if (mailsCount >= 50)
            continue;

        // skip deleted or not delivered (deliver delay not expired) mails
        if ((*itr)->state == MAIL_STATE_DELETED || cur_time < (*itr)->deliver_time)
            continue;

        uint8 item_count = (*itr)->items.size();                    // max count is MAX_MAIL_ITEMS (12)

        size_t next_mail_size = 2 + 4 + 1 + ((*itr)->messageType == MAIL_NORMAL ? 8 : 4) + 4 * 8 + ((*itr)->subject.size() + 1) + ((*itr)->body.size() + 1) + 1 + item_count*(1 + 4 + 4 + MAX_INSPECTED_ENCHANTMENT_SLOT * 3 * 4 + 4 + 4 + 4 + 4 + 4 + 4 + 1);

        if (data.wpos() + next_mail_size > maxPacketSize)
            continue;


        if ((*itr)->messageType == MAIL_CREATURE || (*itr)->messageType == MAIL_GAMEOBJECT || (*itr)->messageType == MAIL_AUCTION)
            data.WriteBit(1);                                       // hasSenderEntry
        else
            data.WriteBit(0);

        data.WriteBits((*itr)->subject.size(), 8);
        data.WriteBit(false); // ?

        if ((*itr)->messageType == MAIL_NORMAL)
            data.WriteBit(true);
        else
            data.WriteBit(false);

        data.WriteBit(false); // ?

        if ((*itr)->messageType == MAIL_NORMAL)
        {
            ObjectGuid senderGuid = (*itr)->sender;
            uint8 bitsOrder[8] = { 6, 0, 3, 2, 5, 7, 1, 4 };
            data.WriteBitInOrder(senderGuid, bitsOrder);
        }

        data.WriteBits((*itr)->body.size(), 13);
        data.WriteBits(item_count, 17);

        for (uint8 i = 0; i < item_count; ++i)
        {
            data.WriteBit(0);

            Item* item = player->GetMItem((*itr)->items[i].item_guid);

            dataBuffer << uint32(item ? item->GetCount() : 0);
            dataBuffer << uint32(item ? item->GetEntry() : 0);
            dataBuffer << uint32(0); // ? uint32(item ? item->GetItemSuffixFactor() : 0);
            dataBuffer << uint32(0); // ? 
            
            dataBuffer << uint8(i);

            for (uint8 j = 0; j < MAX_INSPECTED_ENCHANTMENT_SLOT; ++j)
            {
                dataBuffer << uint32(item ? item->GetEnchantmentId((EnchantmentSlot)j) : 0);
                dataBuffer << uint32(item ? item->GetEnchantmentDuration((EnchantmentSlot)j) : 0);
                dataBuffer << uint32(item ? item->GetEnchantmentCharges((EnchantmentSlot)j) : 0);
            }

            dataBuffer << uint32(item ? item->GetGUIDLow() : 0);
            dataBuffer << uint32(item ? item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) : 0);
            dataBuffer << int32(0); // int32(item ? item->GetItemRandomPropertyId() : 0);

            if (item)
                item->BuildItemModifiers(&dataBuffer);
            else
                dataBuffer << uint32(0);

            dataBuffer << uint32(item ? item->GetUInt32Value(ITEM_FIELD_DURABILITY) : 0);
        }

        dataBuffer << uint64((*itr)->money);                        // Gold

        // if first unk bit then << uint32

        dataBuffer << uint32(0); // ??

        if ((*itr)->messageType == MAIL_NORMAL)
        {
            ObjectGuid senderGuid = (*itr)->sender;
            uint8 bytesOrder[8] = { 7, 0, 6, 5, 4, 2, 3, 1 };
            dataBuffer.WriteBytesSeq(senderGuid, bytesOrder);
        }

        dataBuffer << uint8((*itr)->messageType);                   // Message Type

        if ((*itr)->subject.size())
            dataBuffer.append((*itr)->subject.c_str(), (*itr)->subject.size());

        dataBuffer << uint32(0); // ??

        dataBuffer << uint32((*itr)->stationery);                   // stationery (Stationery.dbc)
        dataBuffer << uint32((*itr)->checked);

        // if second unk bit then << uint32

        dataBuffer << float(((*itr)->expire_time - time(NULL)) / DAY);  // Time
        
        if ((*itr)->body.size())
            dataBuffer.append((*itr)->body.c_str(), (*itr)->body.size());
        
        dataBuffer << uint32((*itr)->messageID);                    // Message ID
        
        if ((*itr)->messageType == MAIL_CREATURE || (*itr)->messageType == MAIL_GAMEOBJECT || (*itr)->messageType == MAIL_AUCTION)
            dataBuffer << uint32((*itr)->sender);                   // creature/gameobject entry, auction id
        
        dataBuffer << uint64((*itr)->COD);

        ++mailsCount;
    }

    data.append(dataBuffer);

    data << uint32(realCount);

    SendPacket(&data);

    // recalculate m_nextMailDelivereTime and unReadMails
    _player->UpdateNextMailTimeAndUnreads();
}

// Used when player copies mail body to his inventory
void WorldSession::HandleMailCreateTextItem(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_MAIL_CREATE_TEXT_ITEM");

    ObjectGuid mailbox;
    uint32 mailId;

    recvData >> mailId;

    uint8 bitsOrder[8] = { 3, 1, 0, 7, 4, 5, 2, 6 };
    recvData.ReadBitInOrder(mailbox, bitsOrder);

    uint8 bytesOrder[8] = { 7, 0, 4, 6, 5, 2, 3, 1 };
    recvData.ReadBytesSeq(mailbox, bytesOrder);

    if (!GetPlayer()->GetGameObjectIfCanInteractWith(mailbox, GAMEOBJECT_TYPE_MAILBOX))
        return;

    Player* player = _player;

    Mail* m = player->GetMail(mailId);
    if (!m || (m->body.empty() && !m->mailTemplateId) || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL))
    {
        player->SendMailResult(mailId, MAIL_MADE_PERMANENT, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    Item* bodyItem = new Item;                              // This is not bag and then can be used new Item.
    if (!bodyItem->Create(sObjectMgr->GetGenerator<HighGuid::Item>().Generate(), MAIL_BODY_ITEM_TEMPLATE, player))
    {
        delete bodyItem;
        return;
    }

    // in mail template case we need create new item text
    if (m->mailTemplateId)
    {
        MailTemplateEntry const* mailTemplateEntry = sMailTemplateStore.LookupEntry(m->mailTemplateId);
        if (!mailTemplateEntry)
        {
            player->SendMailResult(mailId, MAIL_MADE_PERMANENT, MAIL_ERR_INTERNAL_ERROR);
            return;
        }

        bodyItem->SetText(mailTemplateEntry->content);
    }
    else
        bodyItem->SetText(m->body);

    bodyItem->SetUInt32Value(ITEM_FIELD_CREATOR, m->sender);
    bodyItem->SetFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_MAIL_TEXT_MASK);

    sLog->outInfo(LOG_FILTER_NETWORKIO, "HandleMailCreateTextItem mailid=%u", mailId);

    ItemPosCountVec dest;
    uint8 msg = _player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, bodyItem, false);
    if (msg == EQUIP_ERR_OK)
    {
        m->checked = m->checked | MAIL_CHECK_MASK_COPIED;
        m->state = MAIL_STATE_CHANGED;
        player->m_mailsUpdated = true;

        player->StoreItem(dest, bodyItem, true);
        player->SendMailResult(mailId, MAIL_MADE_PERMANENT, MAIL_OK);
    }
    else
    {
        player->SendMailResult(mailId, MAIL_MADE_PERMANENT, MAIL_ERR_EQUIP_ERROR, msg);
        delete bodyItem;
    }
}

// TODO Fix me! ... this void has probably bad condition, but good data are sent
void WorldSession::HandleQueryNextMailTime(WorldPacket & /*recvData*/)
{
    if (!_player->m_mailsLoaded)
        _player->_LoadMail();

    if (_player->unReadMails == 0)
    {
        WorldPacket data(SMSG_QUERY_NEXT_MAIL_TIME, 4 + 3);
        data << float(-1);
        data.WriteBits(0, 20);
        SendPacket(&data);
        return;
    }

    WorldPacket data(SMSG_QUERY_NEXT_MAIL_TIME, 4 + 3 + 2 + 48);
    data << float(0);

    auto wpos = data.wpos();
    auto bpos = data.bitpos();
    data.WriteBits(0, 20);

    int count = 0;

    ByteBuffer dataBuffer2(_player->unReadMails * 21);

    time_t now = time(NULL);

    auto const& mails = _player->GetPlayerMails();
    for (auto m : mails)
    {
        // must be not checked yet
        if (m->checked & MAIL_CHECK_MASK_READ)
            continue;

        // and already delivered
        if (now < m->deliver_time)
            continue;

        ObjectGuid senderGuid = (m->messageType == MAIL_NORMAL) ? MAKE_NEW_GUID(m->sender, 0, HighGuid::Player) : 0;

        data.WriteBit(senderGuid[4]);
        data.WriteBit(senderGuid[2]);
        data.WriteBit(false); // bit1
        data.WriteBit(false); // bit2
        data.WriteBit(senderGuid[1]);
        data.WriteBit(senderGuid[3]);
        data.WriteBit(senderGuid[6]);
        data.WriteBit(senderGuid[5]);
        data.WriteBit(senderGuid[0]);
        data.WriteBit(senderGuid[7]);

        dataBuffer2.WriteByteSeq(senderGuid[6]);
        // if bit2 dataBuffer2 << uint32
        dataBuffer2.WriteByteSeq(senderGuid[4]);
        dataBuffer2.WriteByteSeq(senderGuid[2]);
        dataBuffer2.WriteByteSeq(senderGuid[0]);
        dataBuffer2 << float(m->deliver_time - now);
        dataBuffer2 << uint32(m->messageType != MAIL_NORMAL ? m->sender : 0);
        dataBuffer2.WriteByteSeq(senderGuid[1]);
        dataBuffer2.WriteByteSeq(senderGuid[7]);
        dataBuffer2 << uint8(m->messageType);
        dataBuffer2 << uint32(m->stationery);
        // if bit1 dataBuffer2 << uint32
        dataBuffer2.WriteByteSeq(senderGuid[3]);
        dataBuffer2.WriteByteSeq(senderGuid[5]);

        ++count;
    }

    data.PutBits(count, 20, wpos, bpos);

    if (count)
        data.append(dataBuffer2);

    SendPacket(&data);
}
