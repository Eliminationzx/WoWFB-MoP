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

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Guild.h"
#include "GossipDef.h"
#include "SocialMgr.h"

// Helper for getting guild object of session's player.
// If guild does not exist, sends error (if necessary).
inline Guild* _GetPlayerGuild(WorldSession* session, bool sendError = false)
{
    if (uint32 guildId = session->GetPlayer()->GetGuildId())    // If guild id = 0, player is not in guild
        if (Guild* guild = sGuildMgr->GetGuildById(guildId))   // Find guild by id
            return guild;
    if (sendError)
        Guild::SendCommandResult(session, GUILD_CREATE_S, ERR_GUILD_PLAYER_NOT_IN_GUILD);
    return NULL;
}

void WorldSession::HandleGuildQueryOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_QUERY");

    ObjectGuid guildGuid;
    ObjectGuid playerGuid;

    guildGuid[0] = recvPacket.ReadBit();
    guildGuid[4] = recvPacket.ReadBit();
    playerGuid[0] = recvPacket.ReadBit();
    guildGuid[1] = recvPacket.ReadBit();
    guildGuid[7] = recvPacket.ReadBit();
    guildGuid[6] = recvPacket.ReadBit();
    playerGuid[6] = recvPacket.ReadBit();
    playerGuid[2] = recvPacket.ReadBit();
    guildGuid[5] = recvPacket.ReadBit();
    playerGuid[5] = recvPacket.ReadBit();
    playerGuid[7] = recvPacket.ReadBit();
    playerGuid[1] = recvPacket.ReadBit();
    playerGuid[3] = recvPacket.ReadBit();
    playerGuid[4] = recvPacket.ReadBit();
    guildGuid[2] = recvPacket.ReadBit();
    guildGuid[3] = recvPacket.ReadBit();

    recvPacket.ReadByteSeq(guildGuid[7]);
    recvPacket.ReadByteSeq(playerGuid[4]);
    recvPacket.ReadByteSeq(playerGuid[0]);
    recvPacket.ReadByteSeq(playerGuid[3]);
    recvPacket.ReadByteSeq(playerGuid[6]);
    recvPacket.ReadByteSeq(playerGuid[7]);
    recvPacket.ReadByteSeq(guildGuid[2]);
    recvPacket.ReadByteSeq(guildGuid[5]);
    recvPacket.ReadByteSeq(guildGuid[0]);
    recvPacket.ReadByteSeq(guildGuid[3]);
    recvPacket.ReadByteSeq(playerGuid[2]);
    recvPacket.ReadByteSeq(playerGuid[5]);
    recvPacket.ReadByteSeq(guildGuid[1]);
    recvPacket.ReadByteSeq(playerGuid[1]);
    recvPacket.ReadByteSeq(guildGuid[6]);
    recvPacket.ReadByteSeq(guildGuid[4]);

    // If guild doesn't exist or player is not part of the guild send error
    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
    {
        if (guild->IsMember(playerGuid))
        {
            guild->HandleQuery(this);
            return;
        }
    }
    else if (!GetInterRealmBG())
    {
        Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_PLAYER_NOT_IN_GUILD);
    }
}

void WorldSession::HandleGuildInviteOpcode(WorldPacket& recvPacket)
{
    time_t now = time(NULL);
    if (now - timeLastGuildInviteCommand < 5)
        return;
    else
       timeLastGuildInviteCommand = now;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_INVITE");

    uint32 nameLength = recvPacket.ReadBits(9);

    std::string invitedName = recvPacket.ReadString(nameLength);

    auto delimeter_pos = invitedName.find("-");
    if (delimeter_pos > 0)
        invitedName = invitedName.substr(0, delimeter_pos);

    if (normalizePlayerName(invitedName))
        if (Guild* guild = _GetPlayerGuild(this, true))
            guild->HandleInviteMember(this, invitedName);
}

void WorldSession::HandleGuildRemoveOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_REMOVE");

    ObjectGuid playerGuid;

    uint8 bitOrder[8] = { 3, 1, 6, 0, 7, 2, 5, 4 };
    recvPacket.ReadBitInOrder(playerGuid, bitOrder);

    uint8 byteOrder[8] = { 2, 6, 0, 1, 4, 3, 5, 7 };
    recvPacket.ReadBytesSeq(playerGuid, byteOrder);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleRemoveMember(this, playerGuid);
}

void WorldSession::HandleGuildMasterReplaceOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_REPLACE_GUILD_MASTER");

    Guild* guild = _GetPlayerGuild(this, true);

    if (!guild)
        return; // Cheat

    uint32 logoutTime = guild->GetMemberLogoutTime(guild->GetLeaderGUID());

    if (!logoutTime)
        return;

    time_t now = time(NULL);

    if (time_t(logoutTime + 3 * MONTH) > now)
        return; // Cheat

    guild->SwitchGuildLeader(GetPlayer()->GetGUID());
}

void WorldSession::HandleGuildAcceptOpcode(WorldPacket& /*recvPacket*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_ACCEPT");
    // Player cannot be in guild
    if (!GetPlayer()->GetGuildId())
        // Guild where player was invited must exist
        if (Guild* guild = sGuildMgr->GetGuildById(GetPlayer()->GetGuildIdInvited()))
            guild->HandleAcceptMember(this);
}

void WorldSession::HandleGuildDeclineOpcode(WorldPacket& /*recvPacket*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_DECLINE");

    GetPlayer()->SetGuildIdInvited(0);
    GetPlayer()->SetInGuild(0);
}

void WorldSession::HandleGuildRosterOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_ROSTER");

    recvPacket.rfinish();

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleRoster(this);
}

void WorldSession::HandleGuildAssignRankOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_ASSIGN_MEMBER_RANK");

    ObjectGuid targetGuid;

    uint32 rankId;
    recvPacket >> rankId;

    uint8 bitOrder[8] = { 7, 5, 2, 1, 0, 3, 6, 4 };
    recvPacket.ReadBitInOrder(targetGuid, bitOrder);

    uint8 byteOrder[8] = { 7, 3, 0, 5, 2, 1, 6, 4 };
    recvPacket.ReadBytesSeq(targetGuid, byteOrder);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetMemberRank(this, targetGuid, GetPlayer()->GetGUID(), rankId);
}

void WorldSession::HandleGuildLeaveOpcode(WorldPacket& /*recvPacket*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_LEAVE");

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleLeaveMember(this);
}

void WorldSession::HandleGuildDisbandOpcode(WorldPacket& /*recvPacket*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_DISBAND");

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleDisband(this);
}

void WorldSession::HandleGuildLeaderOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_LEADER");

    std::string name;
    uint32 len = recvPacket.ReadBits(9);

    name = recvPacket.ReadString(len);

    size_t pos = name.find('-');
    if (pos > 0)
    {
        name = name.substr(0, pos);
        if (normalizePlayerName(name))
            if (Guild* guild = _GetPlayerGuild(this, true))
                guild->HandleSetLeader(this, name);
    }
    else
    {
        if (normalizePlayerName(name))
            if (Guild* guild = _GetPlayerGuild(this, true))
                guild->HandleSetLeader(this, name);
    }
}

void WorldSession::HandleGuildMOTDOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_MOTD");

    uint32 motdLength = recvPacket.ReadBits(10);
    std::string motd = recvPacket.ReadString(motdLength);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetMOTD(this, motd);
}

void WorldSession::HandleSwapRanks(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_SWITCH_RANK");

    uint32 id = 0;
    bool up = false;

    recvPacket >> id;
    up = recvPacket.ReadBit();
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSwapRanks(this, id, up);
}

void WorldSession::HandleGuildSetNoteOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_SET_NOTE");

    ObjectGuid playerGuid;

    playerGuid[1] = recvPacket.ReadBit();
    playerGuid[5] = recvPacket.ReadBit();
    bool ispublic = recvPacket.ReadBit();      // 0 == Officer, 1 == Public
    playerGuid[2] = recvPacket.ReadBit();
    uint32 noteLength = recvPacket.ReadBits(8);
    playerGuid[0] = recvPacket.ReadBit();
    playerGuid[4] = recvPacket.ReadBit();
    playerGuid[3] = recvPacket.ReadBit();
    playerGuid[7] = recvPacket.ReadBit();
    playerGuid[6] = recvPacket.ReadBit();

    recvPacket.ReadByteSeq(playerGuid[7]);
    recvPacket.ReadByteSeq(playerGuid[0]);
    recvPacket.ReadByteSeq(playerGuid[4]);
    recvPacket.ReadByteSeq(playerGuid[1]);
    std::string note = recvPacket.ReadString(noteLength);
    recvPacket.ReadByteSeq(playerGuid[5]);
    recvPacket.ReadByteSeq(playerGuid[6]);
    recvPacket.ReadByteSeq(playerGuid[3]);
    recvPacket.ReadByteSeq(playerGuid[2]);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetMemberNote(this, note, playerGuid, ispublic);
}

void WorldSession::HandleGuildQueryRanksOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_QUERY_RANKS");

    ObjectGuid guildGuid;

    uint8 bitOrder[8] = { 2, 7, 0, 5, 6, 3, 4, 1 };
    recvData.ReadBitInOrder(guildGuid, bitOrder);

    uint8 byteOrder[8] = { 6, 0, 1, 7, 2, 3, 4, 5 };
    recvData.ReadBytesSeq(guildGuid, byteOrder);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
        if (guild->IsMember(_player->GetGUID()))
            guild->HandleGuildRanks(this);
}

void WorldSession::HandleGuildAddRankOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_ADD_RANK");

    uint32 rankId;
    recvPacket >> rankId;

    uint32 length = recvPacket.ReadBits(7);

    std::string rankName = recvPacket.ReadString(length);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleAddNewRank(this, rankName); //, rankId);
}

void WorldSession::HandleGuildDelRankOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_DEL_RANK");

    uint32 rankId;
    recvPacket >> rankId;

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleRemoveRank(this, rankId);
}

void WorldSession::HandleGuildChangeInfoTextOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_INFO_TEXT");

    uint32 length = recvPacket.ReadBits(11);

    std::string info = recvPacket.ReadString(length);

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetInfo(this, info);
}

void WorldSession::HandleSaveGuildEmblemOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_SAVE_GUILD_EMBLEM");

    EmblemInfo emblemInfo;
    emblemInfo.ReadPacket(recvPacket);

    ObjectGuid playerGuid;

    uint8 bitsOrder[8] = { 6, 0, 1, 5, 2, 4, 7, 3 };
    recvPacket.ReadBitInOrder(playerGuid, bitsOrder);

    uint8 bytesOrder[8] = { 7, 1, 0, 2, 6, 5, 4, 3 };
    recvPacket.ReadBytesSeq(playerGuid, bytesOrder);

    Player* player = ObjectAccessor::FindPlayer(playerGuid);
    if (!player)
        return;

    if (GetPlayer()->GetGUID() != player->GetGUID())
        return;

    // Remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (Guild* guild = _GetPlayerGuild(this))
        guild->HandleSetEmblem(this, emblemInfo);
    else
        // "You are not part of a guild!";
        Guild::SendSaveEmblemResult(this, ERR_GUILDEMBLEM_NOGUILD);
}

void WorldSession::HandleGuildEventLogQueryOpcode(WorldPacket& /* recvPacket */)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received (CMSG_GUILD_EVENT_LOG_QUERY)");

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendEventLog(this);
}

void WorldSession::HandleGuildBankMoneyWithdrawn(WorldPacket& /* recvData */)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received (CMSG_GUILD_BANK_MONEY_WITHDRAWN_QUERY)");

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendMoneyInfo(this);
}

void WorldSession::HandleGuildPermissions(WorldPacket& /* recvData */)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received (CMSG_GUILD_PERMISSIONS)");

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendPermissions(this);
}

// Called when clicking on Guild bank gameobject
void WorldSession::HandleGuildBankerActivate(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received (CMSG_GUILD_BANKER_ACTIVATE)");

    ObjectGuid GoGuid;
    bool fullSlotList;

    GoGuid[3] = recvData.ReadBit();
    GoGuid[2] = recvData.ReadBit();
    GoGuid[7] = recvData.ReadBit();
    GoGuid[4] = recvData.ReadBit();
    GoGuid[6] = recvData.ReadBit();
    GoGuid[0] = recvData.ReadBit();
    GoGuid[5] = recvData.ReadBit();
    fullSlotList = recvData.ReadBit(); // 0 = only slots updated in last operation are shown. 1 = all slots updated
    GoGuid[1] = recvData.ReadBit();

    uint8 bytesOrder[8] = { 7, 1, 4, 2, 0, 3, 6, 5 };
    recvData.ReadBytesSeq(GoGuid, bytesOrder);

    if (GetPlayer()->GetGameObjectIfCanInteractWith(GoGuid, GAMEOBJECT_TYPE_GUILD_BANK))
    {
        if (Guild* guild = _GetPlayerGuild(this))
            guild->SendBankList(this, 0, true, true);
        else
            Guild::SendCommandResult(this, GUILD_BANK, ERR_GUILD_PLAYER_NOT_IN_GUILD);
    }
}

// Called when opening guild bank tab only (first one)
void WorldSession::HandleGuildBankQueryTab(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received (CMSG_GUILD_BANK_QUERY_TAB)");

    ObjectGuid GoGuid;
    uint8 tabId;
    bool fullSlotList;

    recvData >> tabId;

    GoGuid[4] = recvData.ReadBit();
    GoGuid[0] = recvData.ReadBit();
    GoGuid[1] = recvData.ReadBit();
    GoGuid[7] = recvData.ReadBit();
    fullSlotList = recvData.ReadBit(); // 0 = only slots updated in last operation are shown. 1 = all slots updated
    GoGuid[3] = recvData.ReadBit();
    GoGuid[6] = recvData.ReadBit();
    GoGuid[2] = recvData.ReadBit();
    GoGuid[5] = recvData.ReadBit();

    uint8 bytesOrder[8] = { 0, 6, 1, 7, 5, 2, 3, 4 };
    recvData.ReadBytesSeq(GoGuid, bytesOrder);

    if (GetPlayer()->GetGameObjectIfCanInteractWith(GoGuid, GAMEOBJECT_TYPE_GUILD_BANK))
        if (Guild* guild = _GetPlayerGuild(this))
        {
            guild->SendBankList(this, tabId, true, true);
            guild->SendMoneyInfo(this);
        }
}

void WorldSession::HandleGuildBankDepositMoney(WorldPacket& recvData)
{
    ObjectGuid goGuid;
    uint64 money;

    recvData >> money;

    uint8 bitsOrder[8] = { 4, 7, 5, 1, 2, 3, 0, 6 };
    recvData.ReadBitInOrder(goGuid, bitsOrder);

    uint8 bytesOrder[8] = { 2, 0, 4, 6, 5, 1, 3, 7 };
    recvData.ReadBytesSeq(goGuid, bytesOrder);

    if (_player->GetGameObjectIfCanInteractWith(goGuid, GAMEOBJECT_TYPE_GUILD_BANK))
    {
        if (money && _player->HasEnoughMoney(money))
        {
            if (Guild* guild = _GetPlayerGuild(this))
            {
                uint64 amount = guild->GetBankMoney();
                if ((amount + money) > MAX_MONEY_AMOUNT)
                    guild->SendCommandResult(this, GUILD_BANK, ERR_GUILD_TOO_MUCH_MONEY);
                else
                    guild->HandleMemberDepositMoney(this, money);
            }
        }
    }
}

void WorldSession::HandleGuildBankWithdrawMoney(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received (CMSG_GUILD_BANK_WITHDRAW_MONEY)");

    uint64 money;
    ObjectGuid GoGuid;

    recvData >> money;

    uint8 bitsOrder[8] = { 0, 3, 7, 4, 5, 1, 6, 2 };
    recvData.ReadBitInOrder(GoGuid, bitsOrder);

    uint8 bytesOrder[8] = { 4, 2, 0, 5, 3, 6, 1, 7 };
    recvData.ReadBytesSeq(GoGuid, bytesOrder);

    if (money)
        if (GetPlayer()->GetGameObjectIfCanInteractWith(GoGuid, GAMEOBJECT_TYPE_GUILD_BANK))
            if (Guild* guild = _GetPlayerGuild(this))
                guild->HandleMemberWithdrawMoney(this, money);
}

void WorldSession::HandleGuildBankSwapItems(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received (CMSG_GUILD_BANK_SWAP_ITEMS)");

    ObjectGuid GoGuid;
    uint32 amountSplited = 0;
    uint32 originalItemId = 0;
    uint32 itemId = 0;
    uint32 autostoreCount = 0;
    uint8 srcTabSlot = 0;
    uint8 toChar = 0;
    uint8 srcTabId = 0;
    uint8 dstTabId = 0;
    uint8 dstTabSlot = 0;
    uint8 plrSlot = 0;
    uint8 plrBag = 0;
    bool bit52 = false;
    bool hasDstTab = false;
    bool bankToBank = false;
    bool hasSrcTabSlot = false;
    bool hasPlrSlot = false;
    bool hasItemId = false;
    bool hasPlrBag = false;
    bool autostore = false;

    recvData >> amountSplited;
    recvData >> dstTabSlot;
    recvData >> originalItemId;
    recvData >> toChar >> srcTabId;

    bit52 = recvData.ReadBit();
    hasPlrBag = !recvData.ReadBit();
    GoGuid[2] = recvData.ReadBit();
    hasItemId = !recvData.ReadBit();
    GoGuid[3] = recvData.ReadBit();
    hasDstTab = !recvData.ReadBit();
    GoGuid[0] = recvData.ReadBit();
    hasPlrSlot = !recvData.ReadBit();
    bankToBank = recvData.ReadBit();
    GoGuid[7] = recvData.ReadBit();
    GoGuid[6] = recvData.ReadBit();
    GoGuid[1] = recvData.ReadBit();
    hasSrcTabSlot = !recvData.ReadBit();
    autostore = !recvData.ReadBit();
    GoGuid[5] = recvData.ReadBit();
    GoGuid[4] = recvData.ReadBit();

    uint8 bytesOrder[8] = { 7, 1, 2, 4, 0, 5, 6, 3 };
    recvData.ReadBytesSeq(GoGuid, bytesOrder);

    if (hasSrcTabSlot)
        srcTabSlot = recvData.read<uint8>();
    if (hasItemId)
        itemId = recvData.read<uint32>();
    if (hasPlrBag)
        plrBag = recvData.read<uint8>();
    if (hasPlrSlot)
        plrSlot = recvData.read<uint8>();
    if (hasDstTab)
        dstTabId = recvData.read<uint8>();
    if (autostore)
        autostoreCount = recvData.read<uint32>();

    if (!GetPlayer()->GetGameObjectIfCanInteractWith(GoGuid, GAMEOBJECT_TYPE_GUILD_BANK))
    {
        recvData.rfinish();                   // Prevent additional spam at rejected packet
        return;
    }

    Guild* guild = _GetPlayerGuild(this);
    if (!guild)
    {
        recvData.rfinish();                   // Prevent additional spam at rejected packet
        return;
    }

    if (bankToBank)
        guild->SwapItems(GetPlayer(), dstTabId, srcTabSlot, srcTabId, dstTabSlot, amountSplited);
    else if (autostore)
        guild->AutoStoreItemInInventory(GetPlayer(), srcTabId, dstTabSlot, autostoreCount);
    else
    {
        // Player <-> Bank
        // Allow to work with inventory only
        if (!Player::IsInventoryPos(plrBag, plrSlot) && !(plrBag == NULL_BAG && plrSlot == NULL_SLOT))
            GetPlayer()->SendEquipError(EQUIP_ERR_INTERNAL_BAG_ERROR, NULL);
        else
            guild->SwapItemsWithInventory(GetPlayer(), toChar, srcTabId, dstTabSlot, plrBag, plrSlot, amountSplited);
    }
}

void WorldSession::HandleGuildBankBuyTab(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received (CMSG_GUILD_BANK_BUY_TAB)");

    uint8 tabId;
    ObjectGuid GoGuid;
    recvData >> tabId;

    uint8 bitsOrder[8] = { 7, 6, 1, 2, 5, 3, 0, 4 };
    recvData.ReadBitInOrder(GoGuid, bitsOrder);

    uint8 bytesOrder[8] = { 0, 7, 3, 4, 1, 6, 5, 2 };
    recvData.ReadBytesSeq(GoGuid, bytesOrder);

    // Only for SPELL_EFFECT_UNLOCK_GUILD_VAULT_TAB, this prevent cheating
    if (tabId > 5)
        return;

    if (!GoGuid || GetPlayer()->GetGameObjectIfCanInteractWith(GoGuid, GAMEOBJECT_TYPE_GUILD_BANK))
        if (Guild* guild = _GetPlayerGuild(this))
            guild->HandleBuyBankTab(this, tabId);
}

void WorldSession::HandleGuildBankUpdateTab(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received (CMSG_GUILD_BANK_UPDATE_TAB)");

    uint8 tabId;
    ObjectGuid GoGuid;
    uint32 nameLen, iconLen;
    std::string name;
    std::string icon;

    recvData >> tabId;

    iconLen = recvData.ReadBits(9);

    GoGuid[0] = recvData.ReadBit();
    GoGuid[3] = recvData.ReadBit();
    GoGuid[4] = recvData.ReadBit();
    GoGuid[7] = recvData.ReadBit();
    GoGuid[2] = recvData.ReadBit();
    GoGuid[5] = recvData.ReadBit();
    GoGuid[6] = recvData.ReadBit();

    nameLen = recvData.ReadBits(7);
    GoGuid[1] = recvData.ReadBit();

    recvData.ReadByteSeq(GoGuid[3]);
    recvData.ReadByteSeq(GoGuid[2]);
    recvData.ReadByteSeq(GoGuid[5]);

    name = recvData.ReadString(nameLen);
    icon = recvData.ReadString(iconLen);

    recvData.ReadByteSeq(GoGuid[4]);
    recvData.ReadByteSeq(GoGuid[0]);
    recvData.ReadByteSeq(GoGuid[1]);
    recvData.ReadByteSeq(GoGuid[6]);
    recvData.ReadByteSeq(GoGuid[7]);

    if (!name.empty() && !icon.empty())
        if (GetPlayer()->GetGameObjectIfCanInteractWith(GoGuid, GAMEOBJECT_TYPE_GUILD_BANK))
            if (Guild* guild = _GetPlayerGuild(this))
                guild->HandleSetBankTabInfo(this, tabId, name, icon);
}

void WorldSession::HandleGuildBankLogQuery(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received (MSG_GUILD_BANK_LOG_QUERY)");

    uint32 tabId;
    recvData >> tabId;

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendBankLog(this, tabId);
}

void WorldSession::HandleQueryGuildBankTabText(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_BANK_QUERY_TEXT");

    uint32 tabId;
    recvData >> tabId;

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendBankTabText(this, tabId);
}

void WorldSession::HandleSetGuildBankTabText(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_SET_GUILD_BANK_TEXT");

    uint32 tabId;
    recvData >> tabId;

    uint32 textLen = recvData.ReadBits(14);

    std::string text = recvData.ReadString(textLen);

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SetBankTabText(tabId, text);
}

void WorldSession::HandleGuildQueryXPOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUERY_GUILD_XP");

    ObjectGuid guildGuid;

    uint8 bitOrder[8] = { 3, 2, 7, 5, 6, 0, 1, 4 };
    recvPacket.ReadBitInOrder(guildGuid, bitOrder);

    uint8 byteOrder[8] = { 3, 7, 2, 1, 5, 4, 0, 6 };
    recvPacket.ReadBytesSeq(guildGuid, byteOrder);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
        if (guild->IsMember(_player->GetGUID()))
            guild->SendGuildXP(this);
}

void WorldSession::HandleGuildSetRankPermissionsOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_SET_RANK_PERMISSIONS");

    Guild* guild = _GetPlayerGuild(this, true);
    if (!guild)
    {
        recvPacket.rfinish();
        return;
    }

    uint32 oldRankId;
    uint32 rankId;
    uint32 oldRights;
    uint32 newRights;
    uint32 moneyPerDay;

    recvPacket >> oldRankId;

    GuildBankRightsAndSlotsVec rightsAndSlots(GUILD_BANK_MAX_TABS);
    for (uint8 tabId = 0; tabId < GUILD_BANK_MAX_TABS; ++tabId)
    {
        uint32 bankRights;
        uint32 slots;

        recvPacket >> bankRights;
        recvPacket >> slots;

        rightsAndSlots[tabId] = GuildBankRightsAndSlots(uint8(bankRights), slots);
    }

    recvPacket >> moneyPerDay;
    recvPacket >> newRights;
    recvPacket >> rankId;
    recvPacket >> oldRights;
    
    uint32 nameLength = recvPacket.ReadBits(7);
    std::string rankName = recvPacket.ReadString(nameLength);
    
    moneyPerDay = std::min<uint64>(uint64(moneyPerDay*GOLD), uint64(std::numeric_limits<uint32>::max));

    guild->HandleSetRankInfo(this, rankId, rankName, newRights, moneyPerDay, rightsAndSlots);
}

void WorldSession::HandleGuildRequestPartyState(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_REQUEST_PARTY_STATE");

    ObjectGuid guildGuid;

    uint8 bitOrder[8] = { 5, 3, 4, 0, 2, 6, 1, 7 };
    recvData.ReadBitInOrder(guildGuid, bitOrder);

    uint8 byteOrder[8] = { 5, 4, 3, 1, 7, 0, 2, 6 };
    recvData.ReadBytesSeq(guildGuid, byteOrder);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
        guild->HandleGuildPartyRequest(this);
}

void WorldSession::HandleGuildRequestMaxDailyXP(WorldPacket& recvPacket)
{
    ObjectGuid guildGuid;

    uint8 bitOrder[8] = {2, 5, 3, 7, 4, 1, 0, 6};
    recvPacket.ReadBitInOrder(guildGuid, bitOrder);

    uint8 byteOrder[8] = {7, 3, 2, 1, 0, 5, 6, 4};
    recvPacket.ReadBytesSeq(guildGuid, byteOrder);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
    {
        if (guild->IsMember(_player->GetGUID()))
        {
            WorldPacket data(SMSG_GUILD_MAX_DAILY_XP, 8);
            data << uint64(sWorld->getIntConfig(CONFIG_GUILD_DAILY_XP_CAP));
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleAutoDeclineGuildInvites(WorldPacket& recvPacket)
{
    bool enable;
    enable = recvPacket.ReadBit();

    GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_AUTO_DECLINE_GUILD, enable);
}

void WorldSession::HandleGuildRewardsQueryOpcode(WorldPacket& recvPacket)
{
    uint32 unk = 0;
    recvPacket >> unk;

    if (Guild* guild = sGuildMgr->GetGuildById(_player->GetGuildId()))
    {
        std::vector<GuildReward> const& rewards = sGuildMgr->GetGuildRewards();

        auto size = rewards.size();

        WorldPacket data(SMSG_GUILD_REWARDS_LIST, 4 + 3 + (size*28) + size/8);
        data << uint32(time(NULL));
        data.WriteBits(size, 19);

        for (GuildReward const& reward : rewards)
        {
            data.WriteBits(reward.AchievementId > 0 ? 1 : 0, 22);
        }

        for (GuildReward const& reward : rewards)
        {
            if (reward.AchievementId)
                data << uint32(reward.AchievementId);
                
            data << uint64(reward.Price);
            data << uint32(reward.Standing);
            data << uint32(reward.Racemask);
            data << uint32(reward.Entry);
            data << uint32(0);
        }

        SendPacket(&data);
    }
}

void WorldSession::HandleGuildQueryNewsOpcode(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_QUERY_NEWS");

    ObjectGuid guildGuid;

    uint8 bitsOrder[8] = { 4, 3, 5, 1, 2, 6, 0, 7 };
    recvPacket.ReadBitInOrder(guildGuid, bitsOrder);

    uint8 bytesOrder[8] = { 0, 1, 5, 6, 4, 3, 2, 7 };
    recvPacket.ReadBytesSeq(guildGuid, bytesOrder);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
    {
        if (guild->IsMember(_player->GetGUID()))
        {
            WorldPacket data;
            guild->GetNewsLog().BuildNewsData(data);
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleGuildNewsUpdateStickyOpcode(WorldPacket& recvPacket)
{
    uint32 newsId;
    bool sticky;
    ObjectGuid guid;

    recvPacket >> newsId;

    guid[3] = recvPacket.ReadBit();
    guid[2] = recvPacket.ReadBit();
    guid[7] = recvPacket.ReadBit();
    sticky = recvPacket.ReadBit();
    guid[5] = recvPacket.ReadBit();
    guid[0] = recvPacket.ReadBit();
    guid[6] = recvPacket.ReadBit();
    guid[1] = recvPacket.ReadBit();
    guid[4] = recvPacket.ReadBit();

    uint8 byteOrder[8] = {3, 4, 7, 2, 5, 1, 6, 0};
    recvPacket.ReadBytesSeq(guid, byteOrder);

    if (Guild* guild = sGuildMgr->GetGuildById(_player->GetGuildId()))
    {
        if (GuildNewsEntry* newsEntry = guild->GetNewsLog().GetNewsById(newsId))
        {
            if (sticky)
                newsEntry->Flags |= 1;
            else
                newsEntry->Flags &= ~1;
            WorldPacket data;
            guild->GetNewsLog().BuildNewsData(newsId, *newsEntry, data);
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleGuildRequestChallengeUpdate(WorldPacket& /*p_RecvData*/)
{
    if (_player->GetGuild() == nullptr)
        return;

    _player->GetGuild()->SendGuildChallengeUpdated(this);
}

void WorldSession::HandleGuildRequestGuildRecipes(WorldPacket& recvPacket)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GUILD_REQUEST_CHALLENGE_UPDATE");

    ObjectGuid guildGuid;

    uint8 bitsOrder[8] = { 5, 0, 7, 2, 6, 1, 4, 3 };
    recvPacket.ReadBitInOrder(guildGuid, bitsOrder);

    uint8 bytesOrder[8] = { 6, 4, 5, 1, 3, 0, 7, 2 };
    recvPacket.ReadBytesSeq(guildGuid, bytesOrder);

    if (Guild* guild = sGuildMgr->GetGuildByGuid(guildGuid))
        if (guild->IsMember(_player->GetGUID()))
            guild->SendGuildRecipes(this);
}