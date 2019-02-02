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
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include "Arena.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "Chat.h"
#include "Language.h"
#include "Log.h"
#include "LFG.h"
#include "Player.h"
#include "Object.h"
#include "Opcodes.h"
#include "DisableMgr.h"
#include "Group.h"
#include "InterRealmOpcodes.h"

void WorldSession::HandleBattlemasterHelloOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_BATTLEMASTER_HELLO Message from (GUID: %u TypeId:%u)", GUID_LOPART(guid), GuidHigh2TypeId(GUID_HIPART(guid)));

    Creature* unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if (!unit->isBattleMaster())                             // it's not battlemaster
        return;

    // Stop the npc if moving
    unit->StopMoving();

    BattlegroundTypeId bgTypeId = sBattlegroundMgr->GetBattleMasterBG(unit->GetEntry());

    if (!_player->GetBGAccessByLevel(bgTypeId))
    {
                                                            // temp, must be gossip message...
        SendNotification(LANG_YOUR_BG_LEVEL_REQ_ERROR);
        return;
    }

    SendBattleGroundList(guid, bgTypeId);
}

void WorldSession::SendBattleGroundList(uint64 guid, BattlegroundTypeId bgTypeId)
{
    WorldPacket data;
    sBattlegroundMgr->BuildBattlegroundListPacket(&data, guid, _player, bgTypeId);
    SendPacket(&data);
}

void WorldSession::HandleBattlemasterJoinOpcode(WorldPacket& recvData)
{
    if (sWorld->getBoolConfig(CONFIG_INTERREALM_ENABLE))
    {
        InterRealmSession* tunnel = sWorld->GetInterRealmSession();
        if (tunnel && !tunnel->IsTunnelOpened())
        {
            ChatHandler(this).PSendSysMessage(LANG_INTERREALM_DISABLED);
            return;
        }

        HandleIRBattlemasterJoinOpcode(recvData, tunnel);
        return;
    }

    ObjectGuid guid;
    uint32 bgTypeId_ = 0;
    uint32 instanceId = 0;
    uint8 joinAsGroup = 0;
    uint8 role;
    bool isPremade = false;
    Group* grp = NULL;
    uint32 disabledBgs[2];

    for (int i = 0; i < 2; i++)
    {
        recvData >> disabledBgs[i];
    }

    guid[3] = recvData.ReadBit();
    guid[7] = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid[6] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    role = recvData.ReadBit() == 1 ? ROLE_DAMAGE : 0;
    guid[2] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    joinAsGroup = recvData.ReadBit();
    guid[4] = recvData.ReadBit();

    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[1]);

    if (role != ROLE_DAMAGE)
        recvData >> role;

    bgTypeId_ = GUID_LOPART(guid);

    if (!sBattlemasterListStore.LookupEntry(bgTypeId_))
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "Battleground: invalid bgtype (%u) received. possible cheater? player guid %u", bgTypeId_, _player->GetGUIDLow());
        return;
    }

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, bgTypeId_, NULL))
    {
        ChatHandler(this).PSendSysMessage(LANG_BG_DISABLED);
        return;
    }

    BattlegroundTypeId bgTypeId = BattlegroundTypeId(bgTypeId_);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_BATTLEMASTER_JOIN Message from (GUID: %u TypeId:%u)", GUID_LOPART(guid), GuidHigh2TypeId(GUID_HIPART(guid)));

    // can do this, since it's battleground, not arena
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, 0);
    BattlegroundQueueTypeId bgQueueTypeIdRandom = BattlegroundMgr::BGQueueTypeId(BATTLEGROUND_RB, 0);

    // ignore if player is already in BG
    if (_player->InBattleground())
        return;

    // get bg instance or bg template if instance not found
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);

    if (!bg)
        return;

    // expected bracket entry
    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->getLevel());
    if (!bracketEntry)
        return;

    GroupJoinBattlegroundResult err = ERR_BATTLEGROUND_NONE;

    // check queue conditions
    if (!joinAsGroup)
    {
        // check Deserter debuff
        if (!_player->CanJoinToBattleground())
        {
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
            _player->GetSession()->SendPacket(&data);
            return;
        }

        if (_player->GetBattlegroundQueueIndex(bgQueueTypeIdRandom) < PLAYER_MAX_BATTLEGROUND_QUEUES)
        {
            //player is already in random queue
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, ERR_IN_RANDOM_BG);
            _player->GetSession()->SendPacket(&data);
            return;
        }

        if (_player->InBattlegroundQueue() && bgTypeId == BATTLEGROUND_RB)
        {
            //player is already in queue, can't start random queue
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, ERR_IN_NON_RANDOM_BG);
            _player->GetSession()->SendPacket(&data);
            return;
        }

        // check if already in queue
        if (_player->GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
            //player is already in this queue
            return;

        // check if has free queue slots
        if (!_player->HasFreeBattlegroundQueueId())
        {
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, ERR_BATTLEGROUND_TOO_MANY_QUEUES);
            _player->GetSession()->SendPacket(&data);
            return;
        }

        _player->SetBattleGroundRoles(role);

        BattlegroundQueue& bgQueue = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId];

        GroupQueueInfo* ginfo = bgQueue.AddGroup(_player, NULL, bgTypeId, bracketEntry, 0, false, isPremade, 0, 0, disabledBgs);
        uint32 avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
        uint32 queueSlot = _player->AddBattlegroundQueueId(bgQueueTypeId);

        // add joined time data
        _player->AddBattlegroundQueueJoinTime(bgTypeId, ginfo->JoinTime);

        WorldPacket data; // send status packet (in queue)
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, _player, queueSlot, STATUS_WAIT_QUEUE, avgTime, ginfo->JoinTime, ginfo->ArenaType);
        SendPacket(&data);

        sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s", bgQueueTypeId, bgTypeId, _player->GetGUIDLow(), _player->GetName());
    }
    else
    {
        grp = _player->GetGroup();

        if (!grp)
            return;

        if (grp->GetLeaderGUID() != _player->GetGUID())
            return;

        err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, 0, bg->GetMaxPlayersPerTeam(), false, 0);
        isPremade = (grp->GetMembersCount() >= bg->GetMinPlayersPerTeam());

        BattlegroundQueue& bgQueue = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId];
        GroupQueueInfo* ginfo = NULL;
        uint32 avgTime = 0;

        if (!err)
        {
            sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: the following players are joining as group:");
            ginfo = bgQueue.AddGroup(_player, grp, bgTypeId, bracketEntry, 0, false, isPremade, 0, 0, disabledBgs);
            avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
        }

        for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* member = itr->getSource();
            if (!member)
                continue;   // this should never happen

            if (err)
            {
                WorldPacket data;
                sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, err);
                member->GetSession()->SendPacket(&data);
                continue;
            }

            // add to queue
            uint32 queueSlot = member->AddBattlegroundQueueId(bgQueueTypeId);

            // add joined time data
            member->AddBattlegroundQueueJoinTime(bgTypeId, ginfo->JoinTime);

            WorldPacket data; // send status packet (in queue)
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, member, queueSlot, STATUS_WAIT_QUEUE, avgTime, ginfo->JoinTime, ginfo->ArenaType);
            member->GetSession()->SendPacket(&data);

            sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s", bgQueueTypeId, bgTypeId, member->GetGUIDLow(), member->GetName());
        }
        sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: group end");

    }

    sBattlegroundMgr->ScheduleQueueUpdate(0, 0, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId(), disabledBgs);

}

void WorldSession::HandleBattlegroundPlayerPositionsOpcode(WorldPacket& /*recvData*/)
{
    Battleground* bg = _player->GetBattleground();
    if (!bg)                                                 // can't be received if player not in battleground
        return;

    uint32 count = 0;
    std::vector<Player*> players;

    if (Player* plr = ObjectAccessor::FindPlayer(bg->GetFlagPickerGUID(TEAM_ALLIANCE)))
    {
        players.push_back(plr);
        ++count;
    }

    if (Player* plr = ObjectAccessor::FindPlayer(bg->GetFlagPickerGUID(TEAM_HORDE)))
    {
        players.push_back(plr);
        ++count;
    }

    WorldPacket data(SMSG_BATTLEGROUND_PLAYER_POSITIONS, (20 + count)/8 + (count*18));

    data.WriteBits(count, 20);

    for (auto itr : players)
    {
        ObjectGuid guid = itr->GetGUID();

        uint8 bits[8] = { 3, 5, 2, 0, 4, 6, 1, 7 };
        data.WriteBitInOrder(guid, bits);
    }

    for (auto itr : players)
    {
        ObjectGuid guid = itr->GetGUID();

        data << uint8(itr->GetTeamId() == TEAM_ALLIANCE ? 1 : 2);
        data.WriteByteSeq(guid[6]);
        data << float(itr->GetPositionX());
        data.WriteByteSeq(guid[7]);
        data << float(itr->GetPositionY());
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[2]);
        data << uint8(itr->GetTeamId() == TEAM_ALLIANCE ? 3 : 2); // 244 and 83 ?
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[5]);
    }

    SendPacket(&data);
}

void WorldSession::HandlePVPLogDataOpcode(WorldPacket& /*recvData*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd MSG_PVP_LOG_DATA Message");

    Battleground* bg = _player->GetBattleground();
    if (!bg)
        return;

    // Prevent players from sending BuildPvpLogDataPacket in an arena except for when sent in BattleGround::EndBattleGround.
    if (bg->isArena())
        return;

    WorldPacket data;
    sBattlegroundMgr->BuildPvpLogDataPacket(&data, bg);
    SendPacket(&data);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent MSG_PVP_LOG_DATA Message");
}

void WorldSession::HandleBattlefieldListOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_BATTLEFIELD_LIST Message");

    uint32 bgTypeId;
    recvData >> bgTypeId;                                  // id from DBC

    BattlemasterListEntry const* bl = sBattlemasterListStore.LookupEntry(bgTypeId);
    if (!bl)
    {
        sLog->outDebug(LOG_FILTER_BATTLEGROUND, "BattlegroundHandler: invalid bgtype (%u) with player (Name: %s, GUID: %u) received.", bgTypeId, _player->GetName(), _player->GetGUIDLow());
        return;
    }

    // InterRealm
    InterRealmSession* tunnel = sWorld->GetInterRealmSession();
    if (tunnel && tunnel->IsTunnelOpened())
    {
        WorldPacket pckt(IR_CMSG_BATTLEGROUND_LIST_QUERY, 8 + 4 + 1 + 1);

        pckt << uint64(GetPlayer()->GetGUID());
        pckt << uint32(bgTypeId);
        pckt << uint8(GetPlayer()->GetRandomWinner());
        pckt << uint8(GetPlayer()->getLevel());

        tunnel->SendPacket(&pckt);
    }
    else
    {
        WorldPacket data;
        sBattlegroundMgr->BuildBattlegroundListPacket(&data, 0, _player, BattlegroundTypeId(bgTypeId));
        SendPacket(&data);
    }
}

void WorldSession::HandleBattleFieldPortOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_BATTLEFIELD_PORT Message");

    if (sWorld->getBoolConfig(CONFIG_INTERREALM_ENABLE))
    {
        InterRealmSession* tunnel = sWorld->GetInterRealmSession();
        if (!tunnel || !tunnel->IsTunnelOpened())
        {
            ChatHandler(this).PSendSysMessage(LANG_INTERREALM_DISABLED);
            return;
        }

        HandleIRBattlefieldPortOpcode(recvData, tunnel);
        return;
    }

    uint32 time;
    uint32 queueSlot;                                            // guessed
    uint32 unk;                                       // type id from dbc
    uint8 action;                                           // enter battle 0x1, leave queue 0x0
    ObjectGuid guid;
    
    action = recvData.ReadBit();
    recvData >> queueSlot;
    recvData >> time;
    recvData >> unk;

    guid[7] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    guid[4] = recvData.ReadBit();
    guid[2] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    guid[6] = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid[3] = recvData.ReadBit();

    uint8 byteOrder[8] = { 0, 7, 2, 6, 3, 5, 1, 4 };
    recvData.ReadBytesSeq(guid, byteOrder);

    if (!_player->InBattlegroundQueue())
    {
        sLog->outDebug(LOG_FILTER_BATTLEGROUND, "BattlegroundHandler: Invalid CMSG_BATTLEFIELD_PORT received from player (Name: %s, GUID: %u), he is not in bg_queue.", _player->GetName(), _player->GetGUIDLow());
        return;
    }

    if (queueSlot > PLAYER_MAX_BATTLEGROUND_QUEUES)
    {
        sLog->OutPandashan("HandleBattleFieldPortOpcode queueSlot %u", queueSlot);
        return;
    }

    BattlegroundQueueTypeId bgQueueTypeId = _player->GetBattlegroundQueueTypeId(queueSlot);
    if (bgQueueTypeId == BATTLEGROUND_QUEUE_NONE)
    {
        sLog->outDebug(LOG_FILTER_BATTLEGROUND, "BattlegroundHandler: invalid queueSlot (%u) received.", queueSlot);
        return;
    }

    if (bgQueueTypeId >= MAX_BATTLEGROUND_QUEUE_TYPES)
    {
        sLog->OutPandashan("HandleBattleFieldPortOpcode bgQueueTypeId %u", bgQueueTypeId);
        return;
    }

    BattlegroundQueue& bgQueue = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId];

    //we must use temporary variable, because GroupQueueInfo pointer can be deleted in BattlegroundQueue::RemovePlayer() function
    GroupQueueInfo ginfo;
    if (!bgQueue.GetPlayerGroupInfoData(_player->GetGUID(), &ginfo))
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "BattlegroundHandler: itrplayerstatus not found.");
        return;
    }
    // if action == 1, then instanceId is required
    if (!ginfo.IsInvitedToBGInstanceGUID && action == 1)
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "BattlegroundHandler: instance not found.");
        return;
    }

    BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(bgQueueTypeId);
    // BGTemplateId returns BATTLEGROUND_AA when it is arena queue.
    // Do instance id search as there is no AA bg instances.
    Battleground* bg = sBattlegroundMgr->GetBattleground(ginfo.IsInvitedToBGInstanceGUID, bgTypeId == BATTLEGROUND_AA ? BATTLEGROUND_TYPE_NONE : bgTypeId);

    // bg template might and must be used in case of leaving queue, when instance is not created yet
    if (!bg && action == 0)
        bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "HandleBattleFieldPortOpcode: bg_template not found for type id %u, queueId %u.", bgTypeId, bgQueueTypeId);
        return;
    }

    // get real bg type
    bgTypeId = bg->GetTypeID();

    // expected bracket entry
    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->getLevel());
    if (!bracketEntry)
        return;

    //some checks if player isn't cheating - it is not exactly cheating, but we cannot allow it
    if (action == 1 && ginfo.ArenaType == 0)
    {
        //if player is trying to enter battleground (not arena!) and he has deserter debuff, we must just remove him from queue
        if (!_player->CanJoinToBattleground())
        {
            //send bg command result to show nice message
            WorldPacket data2;
            sBattlegroundMgr->BuildStatusFailedPacket(&data2, bg, _player, 0, ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
            _player->GetSession()->SendPacket(&data2);
            action = 0;
            sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: player %s (%u) has a deserter debuff, do not port him to battleground!", _player->GetName(), _player->GetGUIDLow());
        }
        //if player don't match battleground max level, then do not allow him to enter! (this might happen when player leveled up during his waiting in queue
        if (_player->getLevel() > bg->GetMaxLevel())
        {
            sLog->outError(LOG_FILTER_NETWORKIO, "Battleground: Player %s (%u) has level (%u) higher than maxlevel (%u) of battleground (%u)! Do not port him to battleground!",
                _player->GetName(), _player->GetGUIDLow(), _player->getLevel(), bg->GetMaxLevel(), bg->GetTypeID());
            action = 0;
        }
    }

    WorldPacket data;
    switch (action)
    {
        case 1:                                         // port to battleground
            if (!_player->IsInvitedForBattlegroundQueueType(bgQueueTypeId))
                return;                                 // cheating?

            if (!_player->InBattleground())
                _player->SetBattlegroundEntryPoint();

            // resurrect the player
            if (!_player->isAlive())
            {
                _player->ResurrectPlayer(1.0f);
                _player->SpawnCorpseBones();
            }

            // stop taxi flight at port
            _player->GetMotionMaster()->MovementExpired();
            if (_player->isInFlight())
                _player->CleanupAfterTaxiFlight();

            // remove battleground queue status from BGmgr
            bgQueue.RemovePlayer(_player->GetGUID(), false);
            // this is still needed here if battleground "jumping" shouldn't add deserter debuff
            // also this is required to prevent stuck at old battleground after SetBattlegroundId set to new
            if (Battleground* currentBg = _player->GetBattleground())
                currentBg->RemovePlayerAtLeave(_player->GetGUID(), false, true);

            // set the destination instance id
            _player->SetBattlegroundId(bg->GetInstanceID(), bgTypeId);
            // set the destination team
            _player->SetBGTeam(ginfo.Team);
            // bg->HandleBeforeTeleportToBattleground(_player);
            sBattlegroundMgr->SendToBattleground(_player, ginfo.IsInvitedToBGInstanceGUID, bgTypeId);
            // add only in HandleMoveWorldPortAck()
            // bg->AddPlayer(_player, team);
            sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: player %s (%u) joined battle for bg %u, bgtype %u, queue type %u.", _player->GetName(), _player->GetGUIDLow(), bg->GetInstanceID(), bg->GetTypeID(), bgQueueTypeId);
            break;
        case 0:                                         // leave queue
            if (bg->isArena() && bg->GetStatus() > STATUS_WAIT_QUEUE)
                return;
            // if player leaves rated arena match before match start, it is counted as he played but he lost
            /*if (ginfo.IsRated && ginfo.IsInvitedToBGInstanceGUID)
            {
                if (ginfo.group)
                {
                    sLog->outDebug(LOG_FILTER_BATTLEGROUND, "UPDATING memberLost's personal arena rating for %u by opponents rating: %u, because he has left queue!", GUID_LOPART(_player->GetGUID()), ginfo.OpponentsTeamRating);
                    ginfo.group->MemberLost(_player, ginfo.OpponentsMatchmakerRating);
                }
            }*/

            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, _player, queueSlot, STATUS_NONE, _player->GetBattlegroundQueueJoinTime(bgTypeId), 0, 0);
            SendPacket(&data);

            _player->RemoveBattlegroundQueueId(bgQueueTypeId);  // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
            bgQueue.RemovePlayer(_player->GetGUID(), true);

            if (ginfo.IsRatedBG)
            {
                for (auto plrInfo : ginfo.Players)
                {
                    if (Player* player = ObjectAccessor::FindPlayer(plrInfo.first))
                    {
                        player->RemoveBattlegroundQueueId(bgQueueTypeId);  // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
                        bgQueue.RemovePlayer(player->GetGUID(), true);
                        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, player, queueSlot, STATUS_NONE, player->GetBattlegroundQueueJoinTime(bgTypeId), 0, 0);
                        SendPacket(&data);
                    }
                }
            }

            // player left queue, we should update it - do not update Arena Queue
            if (!ginfo.ArenaType)
                sBattlegroundMgr->ScheduleQueueUpdate(ginfo.ArenaMatchmakerRating, ginfo.ArenaType, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());

            sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: player %s (%u) left queue for bgtype %u, queue type %u.", _player->GetName(), _player->GetGUIDLow(), bg->GetTypeID(), bgQueueTypeId);
            break;
        default:
            sLog->outError(LOG_FILTER_NETWORKIO, "Battleground port: unknown action %u", action);
            break;
    }
}

void WorldSession::HandleLeaveBattlefieldOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_LEAVE_BATTLEFIELD Message");

    bool l_WargameInvite = _player && _player->IsInvitedToWargame();

    if (sWorld->getBoolConfig(CONFIG_INTERREALM_ENABLE) && !l_WargameInvite)
    {
        InterRealmSession* tunnel = sWorld->GetInterRealmSession();
        if (!tunnel || !tunnel->IsTunnelOpened())
        {
            ChatHandler(this).PSendSysMessage(LANG_INTERREALM_DISABLED);
            return;
        }
        
        HandleIRLeaveBattlefieldOpcode(recvData, tunnel);
        return;
    }

    if (l_WargameInvite)
        _player->SetWargameInvite(false);

    uint32 queueSlot;
    uint32 joinTime;
    uint32 clientInstanceId;
    ObjectGuid playerGuid;

    bool action = recvData.ReadBit();

    recvData >> queueSlot;
    recvData >> clientInstanceId;
    recvData >> joinTime;

    playerGuid[6] = recvData.ReadBit();
    playerGuid[7] = recvData.ReadBit();
    playerGuid[1] = recvData.ReadBit();
    playerGuid[2] = recvData.ReadBit();
    playerGuid[0] = recvData.ReadBit();
    playerGuid[5] = recvData.ReadBit();
    playerGuid[4] = recvData.ReadBit();
    playerGuid[3] = recvData.ReadBit();

    recvData.ReadByteSeq(playerGuid[7]);
    recvData.ReadByteSeq(playerGuid[2]);
    recvData.ReadByteSeq(playerGuid[1]);
    recvData.ReadByteSeq(playerGuid[5]);
    recvData.ReadByteSeq(playerGuid[4]);
    recvData.ReadByteSeq(playerGuid[0]);
    recvData.ReadByteSeq(playerGuid[3]);
    recvData.ReadByteSeq(playerGuid[6]);

    if (_player->InBattleground() || _player->InArena())
    {
        if (_player->InArena())
        if (_player->GetBattleground()->GetStatus() == STATUS_WAIT_JOIN)
            return;

        _player->LeaveBattleground();
    }
    else if (_player->InBattlegroundQueue())
    {
        BattlegroundQueueTypeId bgQueueTypeId = _player->GetBattlegroundQueueTypeId(queueSlot);
        if (bgQueueTypeId == BATTLEGROUND_QUEUE_NONE)
        {
            sLog->outDebug(LOG_FILTER_BATTLEGROUND, "HandleLeaveBattlefieldOpcode: invalid queueSlot (%u) received.", queueSlot);
            return;
        }

        if (bgQueueTypeId >= MAX_BATTLEGROUND_QUEUE_TYPES)
        {
            sLog->OutPandashan("HandleLeaveBattlefieldOpcode: bgQueueTypeId %u", bgQueueTypeId);
            return;
        }

        BattlegroundQueue& bgQueue = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId];

        //we must use temporary variable, because GroupQueueInfo pointer can be deleted in BattlegroundQueue::RemovePlayer() function
        GroupQueueInfo ginfo;
        if (!bgQueue.GetPlayerGroupInfoData(_player->GetGUID(), &ginfo))
        {
            sLog->outError(LOG_FILTER_NETWORKIO, "HandleLeaveBattlefieldOpcode: itrplayerstatus not found.");
            return;
        }

        BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(bgQueueTypeId);
        // BGTemplateId returns BATTLEGROUND_AA when it is arena queue.
        // Do instance id search as there is no AA bg instances.
        Battleground* bg = sBattlegroundMgr->GetBattleground(ginfo.IsInvitedToBGInstanceGUID, bgTypeId == BATTLEGROUND_AA ? BATTLEGROUND_TYPE_NONE : bgTypeId);

        // bg template might and must be used in case of leaving queue, when instance is not created yet
        if (!bg && action == false)
            bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bg)
        {
            sLog->outError(LOG_FILTER_NETWORKIO, "HandleLeaveBattlefieldOpcode: bg_template not found for type id %u, queueId %u.", bgTypeId, bgQueueTypeId);
            return;
        }

        // get real bg type
        bgTypeId = bg->GetTypeID();

        // expected bracket entry
        PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->getLevel());
        if (!bracketEntry)
            return;

        //some checks if player isn't cheating - it is not exactly cheating, but we cannot allow it
        if (action == true && ginfo.ArenaType == 0)
        {
            //if player is trying to enter battleground (not arena!) and he has deserter debuff, we must just remove him from queue
            if (!_player->CanJoinToBattleground())
            {
                //send bg command result to show nice message
                WorldPacket data2;
                sBattlegroundMgr->BuildStatusFailedPacket(&data2, bg, _player, 0, ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
                _player->GetSession()->SendPacket(&data2);
                action = false;
                sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: player %s (%u) has a deserter debuff, do not port him to battleground!", _player->GetName(), _player->GetGUIDLow());
            }
            //if player don't match battleground max level, then do not allow him to enter! (this might happen when player leveled up during his waiting in queue
            if (_player->getLevel() > bg->GetMaxLevel())
            {
                sLog->outError(LOG_FILTER_NETWORKIO, "Battleground: Player %s (%u) has level (%u) higher than maxlevel (%u) of battleground (%u)! Do not port him to battleground!",
                    _player->GetName(), _player->GetGUIDLow(), _player->getLevel(), bg->GetMaxLevel(), bg->GetTypeID());
                action = false;
            }
        }

        if (action == true)
        {

            if (!_player->IsInvitedForBattlegroundQueueType(bgQueueTypeId))
                return;                                 // cheating?

            if (!_player->InBattleground())
                _player->SetBattlegroundEntryPoint();

            // resurrect the player
            if (!_player->isAlive())
            {
                _player->ResurrectPlayer(1.0f);
                _player->SpawnCorpseBones();
            }

            // stop taxi flight at port
            _player->GetMotionMaster()->MovementExpired();
            if (_player->isInFlight())
                _player->CleanupAfterTaxiFlight();

            // remove battleground queue status from BGmgr
            bgQueue.RemovePlayer(_player->GetGUID(), false);
            // this is still needed here if battleground "jumping" shouldn't add deserter debuff
            // also this is required to prevent stuck at old battleground after SetBattlegroundId set to new
            if (Battleground* currentBg = _player->GetBattleground())
                currentBg->RemovePlayerAtLeave(_player->GetGUID(), false, true);

            // set the destination instance id
            _player->SetBattlegroundId(bg->GetInstanceID(), bgTypeId);
            // set the destination team
            _player->SetBGTeam(ginfo.Team);
            // bg->HandleBeforeTeleportToBattleground(_player);
            sBattlegroundMgr->SendToBattleground(_player, ginfo.IsInvitedToBGInstanceGUID, bgTypeId);
            // add only in HandleMoveWorldPortAck()
            // bg->AddPlayer(_player, team);
            sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: player %s (%u) joined battle for bg %u, bgtype %u, queue type %u.", _player->GetName(), _player->GetGUIDLow(), bg->GetInstanceID(), bg->GetTypeID(), bgQueueTypeId);
        }
        else
        {
            // leave queue
            if (bg->isArena() && bg->GetStatus() > STATUS_WAIT_QUEUE)
                return;

            WorldPacket data;
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, _player, queueSlot, STATUS_NONE, _player->GetBattlegroundQueueJoinTime(bgTypeId), 0, 0);
            SendPacket(&data);

            _player->RemoveBattlegroundQueueId(bgQueueTypeId);  // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
            bgQueue.RemovePlayer(_player->GetGUID(), true);

            if (ginfo.IsRatedBG)
            {
                for (auto plrInfo : ginfo.Players)
                {
                    if (Player* player = ObjectAccessor::FindPlayer(plrInfo.first))
                    {
                        player->RemoveBattlegroundQueueId(bgQueueTypeId);  // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
                        bgQueue.RemovePlayer(player->GetGUID(), true);
                        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, player, queueSlot, STATUS_NONE, player->GetBattlegroundQueueJoinTime(bgTypeId), 0, 0);
                        SendPacket(&data);
                    }
                }
                
                GetPlayer()->CastDeserterDebuff();
            }

            // player left queue, we should update it - do not update Arena Queue
            if (!ginfo.ArenaType)
                sBattlegroundMgr->ScheduleQueueUpdate(ginfo.ArenaMatchmakerRating, ginfo.ArenaType, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());
        }
    }
}

void WorldSession::HandleLeaveBattlefieldAtEndOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_LEAVE_BATTLEFIELD_AT_END Message");

    if (_player->InBattleground() || _player->InArena())
    {
        if (_player->InArena())
            if (_player->GetBattleground()->GetStatus() == STATUS_WAIT_JOIN)
                return;

        _player->LeaveBattleground();
    }
}

void WorldSession::HandleBattlefieldStatusOpcode(WorldPacket& /*recvData*/)
{
    // empty opcode
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_BATTLEFIELD_STATUS Message");

    if (sWorld->getBoolConfig(CONFIG_INTERREALM_ENABLE))
    {
        InterRealmSession* tunnel = sWorld->GetInterRealmSession();
        if (!tunnel || !tunnel->IsTunnelOpened())
        {
            ChatHandler(this).PSendSysMessage(LANG_INTERREALM_DISABLED);
            return;
        }

        HandleIRBattlefieldStatusOpcode(tunnel);
        return;
    }

    WorldPacket data;
    // we must update all queues here
    Battleground* bg = NULL;
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        BattlegroundQueueTypeId bgQueueTypeId = _player->GetBattlegroundQueueTypeId(i);
        if (!bgQueueTypeId)
            continue;
        BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(bgQueueTypeId);
        uint8 arenaType = BattlegroundMgr::BGArenaType(bgQueueTypeId);
        if (bgTypeId == _player->GetBattlegroundTypeId())
        {
            bg = _player->GetBattleground();
            //i cannot check any variable from player class because player class doesn't know if player is in 2v2 / 3v3 or 5v5 arena
            //so i must use bg pointer to get that information
            if (bg && bg->GetArenaType() == arenaType)
            {
                // this line is checked, i only don't know if GetElapsedTime() is changing itself after bg end!
                // send status in Battleground
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, _player, i, STATUS_IN_PROGRESS, _player->GetBattlegroundQueueJoinTime(bgTypeId), bg->GetElapsedTime(), arenaType);
                SendPacket(&data);
                continue;
            }
        }

        //we are sending update to player about queue - he can be invited there!
        //get GroupQueueInfo for queue status
        BattlegroundQueue& bgQueue = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId];
        GroupQueueInfo ginfo;
        if (!bgQueue.GetPlayerGroupInfoData(_player->GetGUID(), &ginfo))
            continue;
        if (ginfo.IsInvitedToBGInstanceGUID)
        {
            bg = sBattlegroundMgr->GetBattleground(ginfo.IsInvitedToBGInstanceGUID, bgTypeId);
            if (!bg)
                continue;

            // send status invited to Battleground
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, GetPlayer(), i, STATUS_WAIT_JOIN, getMSTimeDiff(getMSTime(), ginfo.RemoveInviteTime), _player->GetBattlegroundQueueJoinTime(bgTypeId), arenaType);
            SendPacket(&data);
        }
        else
        {
            bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            if (!bg)
                continue;

            // expected bracket entry
            PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->getLevel());
            if (!bracketEntry)
                continue;

            uint32 avgTime = bgQueue.GetAverageQueueWaitTime(&ginfo, bracketEntry->GetBracketId());
            // send status in Battleground Queue
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, GetPlayer(), i, STATUS_WAIT_QUEUE, avgTime, _player->GetBattlegroundQueueJoinTime(bgTypeId), arenaType);
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleBattlemasterJoinArena(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_BATTLEMASTER_JOIN_ARENA");

    if (sWorld->getBoolConfig(CONFIG_INTERREALM_ENABLE))
    {
        InterRealmSession* tunnel = sWorld->GetInterRealmSession();
        if (!tunnel || !tunnel->IsTunnelOpened())
        {
            ChatHandler(this).PSendSysMessage(LANG_INTERREALM_DISABLED);
            return;
        }

        HandleIRBattlemasterJoinArena(recvData, tunnel);
        return;
    }

    uint8 arenaslot;                                        // 2v2, 3v3 or 5v5

    recvData >> arenaslot;

    // ignore if we already in BG or BG queue
    if (_player->InBattleground())
        return;

    uint32 arenaRating = 0;
    uint32 matchmakerRating = 0;

    uint8 arenatype = Arena::GetTypeBySlot(arenaslot);

    //check existance
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA);
    if (!bg)
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "Battleground: template bg (all arenas) not found");
        return;
    }

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, BATTLEGROUND_AA, NULL))
    {
        ChatHandler(this).PSendSysMessage(LANG_ARENA_DISABLED);
        return;
    }

    BattlegroundTypeId bgTypeId = bg->GetTypeID();
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, arenatype);

    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->getLevel());
    if (!bracketEntry)
        return;

    GroupJoinBattlegroundResult err = ERR_BATTLEGROUND_NONE;

    Group* grp = _player->GetGroup();

    // no group found, error
    if (!grp)
        return;

    if (grp->GetLeaderGUID() != _player->GetGUID())
        return;

    uint32 playerDivider = 0;
    for (GroupReference const* ref = grp->GetFirstMember(); ref != NULL; ref = ref->next())
    {
        if (Player const* groupMember = ref->getSource())
        {
            arenaRating += groupMember->GetArenaPersonalRating(arenaslot);
            matchmakerRating += groupMember->GetArenaMatchMakerRating(arenaslot);
            ++playerDivider;
        }
    }

    if (!playerDivider)
        return;

    arenaRating /= playerDivider;
    matchmakerRating /= playerDivider;

    if (arenaRating <= 0)
        arenaRating = 1;
    if (matchmakerRating <= 0)
        matchmakerRating = 1;

    BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);

    uint32 avgTime = 0;
    GroupQueueInfo* ginfo = NULL;

    err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, arenatype, arenatype, true, arenaslot);
    if (!err || (err && sBattlegroundMgr->isArenaTesting()))
    {
        sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: leader %s queued with matchmaker rating %u for type %u", _player->GetName(), matchmakerRating, arenatype);

        ginfo = bgQueue.AddGroup(_player, grp, bgTypeId, bracketEntry, arenatype, true, false, arenaRating, matchmakerRating);
        avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
    }

    for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* member = itr->getSource();
        if (!member)
            continue;

        if (err && !sBattlegroundMgr->isArenaTesting())
        {
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, err);
            member->GetSession()->SendPacket(&data);
            continue;
        }

         // add to queue
        uint32 queueSlot = member->AddBattlegroundQueueId(bgQueueTypeId);

        if (!ginfo)
        {
            sLog->OutPandashan("NULL ginfo !!!!");
            return;
        }

        // add joined time data
        member->AddBattlegroundQueueJoinTime(bgTypeId, ginfo->JoinTime);

        WorldPacket data; // send status packet (in queue)
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, member, queueSlot, STATUS_WAIT_QUEUE, avgTime, ginfo->JoinTime, arenatype);
        member->GetSession()->SendPacket(&data);

        sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: player joined queue for arena as group bg queue type %u bg type %u: GUID %u, NAME %s", bgQueueTypeId, bgTypeId, member->GetGUIDLow(), member->GetName());
    }

    sBattlegroundMgr->ScheduleQueueUpdate(matchmakerRating, arenatype, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());
}

void WorldSession::HandleBattlemasterJoinRated(WorldPacket & recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_BATTLEMASTER_JOIN_RATED");

    if (sWorld->getBoolConfig(CONFIG_INTERREALM_ENABLE))
    {
        InterRealmSession* tunnel = sWorld->GetInterRealmSession();
        if (!tunnel || !tunnel->IsTunnelOpened())
        {
            ChatHandler(this).PSendSysMessage(LANG_INTERREALM_DISABLED);
            return;
        }

        HandleIRBattlemasterJoinRated(recvData, tunnel);
        return;
    }

    // ignore if we already in BG or BG queue
    if (_player->InBattleground())
        return;

    uint32 personalRating = 0;
    uint32 matchmakerRating = 0;

    //check existance
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_RATED_10_VS_10);
    if (!bg)
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "Battleground: template bg (10 vs 10) not found");
        return;
    }

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, BATTLEGROUND_RATED_10_VS_10, NULL))
        return;

    BattlegroundTypeId bgTypeId = bg->GetTypeID();
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, 0);

    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->getLevel());
    if (!bracketEntry)
        return;

    GroupJoinBattlegroundResult err = ERR_BATTLEGROUND_NONE;

    Group* grp = _player->GetGroup();

    // no group found, error
    if (!grp)
        return;

    if (grp->GetLeaderGUID() != _player->GetGUID())
        return;

    uint32 playerDivider = 0;
    for (GroupReference const* ref = grp->GetFirstMember(); ref != NULL; ref = ref->next())
    {
        if (Player const* groupMember = ref->getSource())
        {
            personalRating += groupMember->GetArenaPersonalRating(SLOT_RBG);
            matchmakerRating += groupMember->GetArenaMatchMakerRating(SLOT_RBG);
            ++playerDivider;
        }
    }

    if (!playerDivider)
        return;

    personalRating /= playerDivider;
    matchmakerRating /= playerDivider;

    if (personalRating <= 0)
        personalRating = 0;
    if (matchmakerRating <= 0)
        matchmakerRating = 1;

    BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);

    uint32 avgTime = 0;
    GroupQueueInfo* ginfo;

    err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, 10, 10, true, 0);
    if (!err)
    {
        sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: leader %s queued");

        ginfo = bgQueue.AddGroup(_player, grp, bgTypeId, bracketEntry, 0, false, true, personalRating, matchmakerRating);
        avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
    }

    for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* member = itr->getSource();
        if (!member)
            continue;

        if (err)
        {
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, err);
            member->GetSession()->SendPacket(&data);
            continue;
        }

         // add to queue
        uint32 queueSlot = member->AddBattlegroundQueueId(bgQueueTypeId);

        // add joined time data
        member->AddBattlegroundQueueJoinTime(bgTypeId, ginfo->JoinTime);

        WorldPacket data; // send status packet (in queue)
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, member, queueSlot, STATUS_WAIT_QUEUE, avgTime, ginfo->JoinTime, ginfo->ArenaType);
        member->GetSession()->SendPacket(&data);

        sLog->outDebug(LOG_FILTER_BATTLEGROUND, "Battleground: player joined queue for rated battleground as group bg queue type %u bg type %u: GUID %u, NAME %s", bgQueueTypeId, bgTypeId, member->GetGUIDLow(), member->GetName());
    }
    sBattlegroundMgr->ScheduleQueueUpdate(matchmakerRating, 0, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());
}

void WorldSession::HandleQueryBattlefieldState(WorldPacket& recvData)
{
    //uint32 unk;

    recvData.rfinish();

    if (_player->InBattleground())
    {
        if (Battleground* bg = _player->GetBattleground())
        {
            int countdownSec = (bg->GetMaxCountdownTimer() - ceil(float(bg->GetElapsedTime()) / 1000));
            _player->SendStartTimer(countdownSec, bg->GetMaxCountdownTimer(), PVP_TIMER);
        }
    }
}

void WorldSession::HandleStartWarGame(WorldPacket& p_Packet)
{
    ObjectGuid l_OpposingPartyMemberGUID;
    ObjectGuid l_QueueID;

    p_Packet.read_skip<uint32>();
    p_Packet.read_skip<uint32>();

    l_OpposingPartyMemberGUID[3] = p_Packet.ReadBit();
    l_QueueID[6]                 = p_Packet.ReadBit();
    l_OpposingPartyMemberGUID[5] = p_Packet.ReadBit();
    l_QueueID[1]                 = p_Packet.ReadBit();
    l_OpposingPartyMemberGUID[7] = p_Packet.ReadBit();
    l_QueueID[3]                 = p_Packet.ReadBit();
    l_OpposingPartyMemberGUID[4] = p_Packet.ReadBit();
    l_OpposingPartyMemberGUID[0] = p_Packet.ReadBit();
    l_QueueID[5]                 = p_Packet.ReadBit();
    l_QueueID[7]                 = p_Packet.ReadBit();
    l_OpposingPartyMemberGUID[2] = p_Packet.ReadBit();
    l_QueueID[2]                 = p_Packet.ReadBit();
    l_QueueID[4]                 = p_Packet.ReadBit();
    l_OpposingPartyMemberGUID[6] = p_Packet.ReadBit();
    l_QueueID[0]                 = p_Packet.ReadBit();
    l_OpposingPartyMemberGUID[1] = p_Packet.ReadBit();

    p_Packet.ReadByteSeq(l_OpposingPartyMemberGUID[0]);
    p_Packet.ReadByteSeq(l_QueueID[5]);
    p_Packet.ReadByteSeq(l_OpposingPartyMemberGUID[7]);
    p_Packet.ReadByteSeq(l_QueueID[4]);
    p_Packet.ReadByteSeq(l_QueueID[6]);
    p_Packet.ReadByteSeq(l_OpposingPartyMemberGUID[2]);
    p_Packet.ReadByteSeq(l_QueueID[2]);
    p_Packet.ReadByteSeq(l_OpposingPartyMemberGUID[6]);
    p_Packet.ReadByteSeq(l_QueueID[0]);
    p_Packet.ReadByteSeq(l_QueueID[3]);
    p_Packet.ReadByteSeq(l_OpposingPartyMemberGUID[1]);
    p_Packet.ReadByteSeq(l_QueueID[7]);
    p_Packet.ReadByteSeq(l_QueueID[1]);
    p_Packet.ReadByteSeq(l_OpposingPartyMemberGUID[5]);
    p_Packet.ReadByteSeq(l_OpposingPartyMemberGUID[3]);
    p_Packet.ReadByteSeq(l_OpposingPartyMemberGUID[4]);

    if (_player->GetGroup() == nullptr)
        return;

    if (_player->GetGroup()->GetLeaderGUID() != _player->GetGUID())
        return;

    /// - Try to find the opposing party leader
    Player* l_OpposingPartyLeader = sObjectAccessor->FindPlayer(l_OpposingPartyMemberGUID);
    if (l_OpposingPartyLeader == nullptr)
        return;

    if (l_OpposingPartyLeader->GetGroup() == nullptr)
        return;

    if (l_OpposingPartyLeader->GetGroup()->GetLeaderGUID() != l_OpposingPartyLeader->GetGUID())
        return;

    /// Already one invite requested
    if (_player->HasWargameRequest())
        return;

    /// The request is delete after 60 secs (handle in Player::Update)
    WargameRequest* l_Request = new WargameRequest();
    l_Request->OpposingPartyMemberGUID = l_OpposingPartyMemberGUID;
    l_Request->CreationDate            = time(nullptr);
    l_Request->QueueID                 = l_QueueID;

    _player->SetWargameRequest(l_Request);

    sBattlegroundMgr->SendCheckWargameEntry(_player, l_OpposingPartyLeader, l_QueueID);
}

void WorldSession::HandleAcceptWarGameInvite(WorldPacket& p_Packet)
{
    ObjectGuid l_OpposingPartyMember;
    ObjectGuid l_QueueID;
    bool       l_Accept;

    l_OpposingPartyMember[1] = p_Packet.ReadBit();
    l_QueueID[5] = p_Packet.ReadBit();
    l_QueueID[1] = p_Packet.ReadBit();
    l_QueueID[0] = p_Packet.ReadBit();
    l_OpposingPartyMember[5] = p_Packet.ReadBit();
    l_QueueID[6] = p_Packet.ReadBit();
    l_QueueID[7] = p_Packet.ReadBit();
    l_OpposingPartyMember[3] = p_Packet.ReadBit();
    l_OpposingPartyMember[4] = p_Packet.ReadBit();
    l_OpposingPartyMember[2] = p_Packet.ReadBit();
    l_OpposingPartyMember[7] = p_Packet.ReadBit();
    l_OpposingPartyMember[6] = p_Packet.ReadBit();
    l_QueueID[2] = p_Packet.ReadBit();
    l_Accept = p_Packet.ReadBit();
    l_QueueID[3] = p_Packet.ReadBit();
    l_QueueID[2] = p_Packet.ReadBit();
    l_OpposingPartyMember[0] = p_Packet.ReadBit();

    p_Packet.ReadByteSeq(l_OpposingPartyMember[6]);
    p_Packet.ReadByteSeq(l_OpposingPartyMember[7]);
    p_Packet.ReadByteSeq(l_QueueID[7]);
    p_Packet.ReadByteSeq(l_OpposingPartyMember[5]);
    p_Packet.ReadByteSeq(l_OpposingPartyMember[2]);
    p_Packet.ReadByteSeq(l_QueueID[5]);
    p_Packet.ReadByteSeq(l_OpposingPartyMember[0]);
    p_Packet.ReadByteSeq(l_QueueID[4]);
    p_Packet.ReadByteSeq(l_QueueID[0]);
    p_Packet.ReadByteSeq(l_QueueID[1]);
    p_Packet.ReadByteSeq(l_OpposingPartyMember[1]);
    p_Packet.ReadByteSeq(l_OpposingPartyMember[4]);
    p_Packet.ReadByteSeq(l_QueueID[6]);
    p_Packet.ReadByteSeq(l_QueueID[2]);
    p_Packet.ReadByteSeq(l_QueueID[3]);
    p_Packet.ReadByteSeq(l_OpposingPartyMember[3]);

    BattlegroundTypeId l_BattlegroundTypeId = BattlegroundTypeId(l_QueueID & 0xFFFF);

    Group* l_Group = _player->GetGroup();
    if (l_Group == nullptr
        || !l_Group->IsLeader(_player->GetGUID()))
        return;

    Player* l_OpposingPartyLeader = sObjectAccessor->FindPlayer(l_OpposingPartyMember);
    if (l_OpposingPartyLeader == nullptr)
        return;

    Group* l_OpposingGroup = l_OpposingPartyLeader->GetGroup();
    if (l_OpposingGroup == nullptr
        || !l_OpposingGroup->IsLeader(l_OpposingPartyLeader->GetGUID()))
        return;

    /// Check if the invitation realy exist, and isn't expired
    if (!l_OpposingPartyLeader->HasWargameRequest())
        return;

    WargameRequest* l_Request = l_OpposingPartyLeader->GetWargameRequest();
    if (l_Request->QueueID != l_QueueID
        || l_Request->OpposingPartyMemberGUID != _player->GetGUID())
        return;

    if (!l_Accept)
    {
        ///@TODO: Send notification to opposing party ?
        return;
    }

    /// ignore if player is already in BG
    if (_player->InBattleground())
        return;

    sBattlegroundMgr->InitWargame(l_Group, l_OpposingGroup, l_BattlegroundTypeId);
}

// TODO: move to interrealmsession block
void BuildArenaInfoBlock(Player* player, ByteBuffer& packet)
{
    for (uint8 i = 0; i < MAX_PVP_SLOT; ++i)
    {
        packet << player->GetArenaPersonalRating(i);
        packet << player->GetBestRatingOfWeek(i);
        packet << player->GetBestRatingOfSeason(i);
        packet << player->GetArenaMatchMakerRating(i);
        packet << player->GetWeekGames(i);
        packet << player->GetWeekWins(i);
        packet << player->GetPrevWeekWins(i);
        packet << player->GetSeasonGames(i);
        packet << player->GetSeasonWins(i);
    }
}

// Interrealm functions
void WorldSession::HandleIRBattlemasterJoinOpcode(WorldPacket& recvData, InterRealmSession* tunnel)
{
    ObjectGuid guid;
    uint32 bgTypeId_ = 0;
    uint32 instanceId = 0;
    uint8 joinAsGroup = 0;
    uint8 role;
    bool isPremade = false;
    Group* grp = NULL;
    uint32 disabledBgs[2];

    for (int i = 0; i < 2; i++)
    {
        recvData >> disabledBgs[i];
    }

    guid[3] = recvData.ReadBit();
    guid[7] = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid[6] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    role = recvData.ReadBit() == 1 ? ROLE_DAMAGE : 0;
    guid[2] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    joinAsGroup = recvData.ReadBit();
    guid[4] = recvData.ReadBit();

    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[1]);

    if (role != ROLE_DAMAGE)
        recvData >> role;

    bgTypeId_ = GUID_LOPART(guid);

    if (!sBattlemasterListStore.LookupEntry(bgTypeId_))
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "Battleground: invalid bgtype (%u) received. possible cheater? player guid %u", bgTypeId_, _player->GetGUIDLow());
        return;
    }

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, bgTypeId_, NULL))
    {
        ChatHandler(this).PSendSysMessage(LANG_BG_DISABLED);
        return;
    }

    BattlegroundTypeId bgTypeId = BattlegroundTypeId(bgTypeId_);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_BATTLEMASTER_JOIN Message from (GUID: %u TypeId:%u)", GUID_LOPART(guid), GuidHigh2TypeId(GUID_HIPART(guid)));

    // can do this, since it's battleground, not arena
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, 0);
    BattlegroundQueueTypeId bgQueueTypeIdRandom = BattlegroundMgr::BGQueueTypeId(BATTLEGROUND_RB, 0);

    // ignore if player is already in BG
    if (_player->InBattleground())
        return;

    // get bg instance or bg template if instance not found
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);

    if (!bg)
        return;

    // expected bracket entry
    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->getLevel());
    if (!bracketEntry)
        return;

    GroupJoinBattlegroundResult err = ERR_BATTLEGROUND_NONE;

    // check queue conditions
    if (!joinAsGroup)
    {
        // check Deserter debuff
        if (!_player->CanJoinToBattleground())
        {
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
            _player->GetSession()->SendPacket(&data);
            return;
        }

        if (_player->GetBattlegroundQueueIndex(bgQueueTypeIdRandom) < PLAYER_MAX_BATTLEGROUND_QUEUES)
        {
            //player is already in random queue
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, ERR_IN_RANDOM_BG);
            _player->GetSession()->SendPacket(&data);
            return;
        }

        if (_player->InBattlegroundQueue() && bgTypeId == BATTLEGROUND_RB)
        {
            //player is already in queue, can't start random queue
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, ERR_IN_NON_RANDOM_BG);
            _player->GetSession()->SendPacket(&data);
            return;
        }

        // check if already in queue
        if (_player->GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
            //player is already in this queue
            return;

        // check if has free queue slots
        if (!_player->HasFreeBattlegroundQueueId())
        {
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, ERR_BATTLEGROUND_TOO_MANY_QUEUES);
            _player->GetSession()->SendPacket(&data);
            return;
        }
    }
    else
    {
        grp = _player->GetGroup();

        if (!grp)
            return;

        if (grp->GetLeaderGUID() != _player->GetGUID())
            return;

        err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, 0, bg->GetMaxPlayersPerTeam(), false, 0);
        isPremade = (grp->GetMembersCount() >= bg->GetMinPlayersPerTeam());

        BattlegroundQueue& bgQueue = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId];
        GroupQueueInfo* ginfo = NULL;
        uint32 avgTime = 0;

        if (err)
        {
            for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* member = itr->getSource();
                if (!member)
                    continue;   // this should never happen

                WorldPacket data;
                sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, _player, 0, err);
                member->GetSession()->SendPacket(&data);
            }

            return;
        }
    }

    // All's clear, create ir packet

    if (!joinAsGroup)
    {
        tunnel->SendRegisterPlayer(_player, instanceId, guid, bgTypeId);
    }
    else
    {
        tunnel->SendRegisterGroup(grp, instanceId, guid, bgTypeId);
    }
}

void WorldSession::HandleIRBattlemasterJoinArena(WorldPacket& packet, InterRealmSession* tunnel)
{
    uint8 arenaslot;                                        // 2v2, 3v3 or 5v5

    packet >> arenaslot;

    // ignore if we already in BG or BG queue
    if (_player->InBattleground())
        return;

    if (GetInterRealmBG())
        return;

    uint32 arenaRating = 0;
    uint32 matchmakerRating = 0;

    uint8 arenatype = Arena::GetTypeBySlot(arenaslot);

    //check existance
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA);
    if (!bg)
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "Battleground: template bg (all arenas) not found");
        return;
    }

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, BATTLEGROUND_AA, NULL))
    {
        ChatHandler(this).PSendSysMessage(LANG_ARENA_DISABLED);
        return;
    }

    BattlegroundTypeId bgTypeId = bg->GetTypeID();
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, arenatype);

    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->getLevel());
    if (!bracketEntry)
        return;

    Group* grp = _player->GetGroup();

    // no group found, error
    if (!grp)
        return;

    if (grp->GetLeaderGUID() != _player->GetGUID())
        return;

    GroupJoinBattlegroundResult err = ERR_BATTLEGROUND_NONE;
    err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, arenatype, arenatype, true, arenaslot);
    for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* member = itr->getSource();
        if (!member)
            return;

        if (err)
        {
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, member, 0, err);
            member->GetSession()->SendPacket(&data);
            continue;
        }
    }

    if (err)
        return;

    tunnel->SendRegisterArena(grp, arenaslot);
}

void WorldSession::HandleIRBattlemasterJoinRated(WorldPacket& packet, InterRealmSession* tunnel)
{
    
    // ignore if we already in BG or BG queue
    if (_player->InBattleground())
        return;

    if (GetInterRealmBG())
        return;

    uint32 personalRating = 0;
    uint32 matchmakerRating = 0;

    //check existance
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_RATED_10_VS_10);
    if (!bg)
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "Battleground: template bg (10 vs 10) not found");
        return;
    }

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, BATTLEGROUND_RATED_10_VS_10, NULL))
        return;

    BattlegroundTypeId bgTypeId = bg->GetTypeID();
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, 0);

    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->getLevel());
    if (!bracketEntry)
        return;

    Group* grp = _player->GetGroup();

    // no group found, error
    if (!grp)
        return;

    if (grp->GetLeaderGUID() != _player->GetGUID())
        return;

    uint32 playerDivider = 0;
    for (GroupReference const* ref = grp->GetFirstMember(); ref != NULL; ref = ref->next())
    {
        if (Player const* groupMember = ref->getSource())
        {
            personalRating += groupMember->GetArenaPersonalRating(SLOT_RBG);
            matchmakerRating += groupMember->GetArenaMatchMakerRating(SLOT_RBG);
            ++playerDivider;
        }
    }

    if (!playerDivider)
        return;

    personalRating /= playerDivider;
    matchmakerRating /= playerDivider;

    if (personalRating <= 0)
        personalRating = 0;
    if (matchmakerRating <= 0)
        matchmakerRating = 1;

    GroupJoinBattlegroundResult err = ERR_BATTLEGROUND_NONE;

    err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, 2, 10, false, 0);

    for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* member = itr->getSource();
        if (!member)
            return;

        if (err)
        {
            WorldPacket data;
            sBattlegroundMgr->BuildStatusFailedPacket(&data, bg, member, 0, err);
            member->GetSession()->SendPacket(&data);
            continue;
        }
    }

    if (err)
        return;

    tunnel->SendRegisterRated(grp, personalRating, matchmakerRating);
}

void WorldSession::HandleIRBattlefieldPortOpcode(WorldPacket &recvData, InterRealmSession* tunnel)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_BATTLEFIELD_PORT Message");

    uint32 time;
    uint32 queueSlot;                                            // guessed
    uint32 unk;                                       // type id from dbc
    uint8 action;                                           // enter battle 0x1, leave queue 0x0
    ObjectGuid guid;
    
    action = recvData.ReadBit();
    recvData >> queueSlot;
    recvData >> time;
    recvData >> unk;

    guid[7] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    guid[4] = recvData.ReadBit();
    guid[2] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    guid[6] = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid[3] = recvData.ReadBit();

    uint8 byteOrder[8] = { 0, 7, 2, 6, 3, 5, 1, 4 };
    recvData.ReadBytesSeq(guid, byteOrder);

    if (action = 1)
    {
        _player->RemoveAurasByType(SPELL_AURA_MOUNTED);

        if (!_player->InBattleground())
            _player->SetBattlegroundEntryPoint();

        // resurrect the player
        if (!_player->isAlive())
        {
            _player->ResurrectPlayer(1.0f);
            _player->SpawnCorpseBones();
        }
        // stop taxi flight at port
        if (_player->isInFlight())
        {
            _player->GetMotionMaster()->MovementExpired();
            _player->CleanupAfterTaxiFlight();
        }

        if (uint64 lootGuid = GetPlayer()->GetLootGUID())
            DoLootRelease(lootGuid);

        SetBattlegroundPortData(GetPlayer()->GetGUID(), time, queueSlot, action);
        GetPlayer()->SaveToDB(false, true);
    }
    else
    {
        WorldPacket pckt(IR_CMSG_BATTLEFIELD_PORT, 8 + 4 + 4 + 1);

        pckt << uint64(GetPlayer()->GetGUID());
        pckt << uint32(time);
        pckt << uint32(queueSlot);
        pckt << uint8(action);

        tunnel->SendPacket(&pckt);
    }
}

void WorldSession::HandleIRLeaveBattlefieldOpcode(WorldPacket &recvData, InterRealmSession* tunnel)
{
    uint32 queueSlot;
    uint32 joinTime;
    uint32 clientInstanceId;
    ObjectGuid playerGuid;

    uint8 action = recvData.ReadBit() ? 1 : 0;

    recvData >> queueSlot;
    recvData >> clientInstanceId;
    recvData >> joinTime;

    playerGuid[6] = recvData.ReadBit();
    playerGuid[7] = recvData.ReadBit();
    playerGuid[1] = recvData.ReadBit();
    playerGuid[2] = recvData.ReadBit();
    playerGuid[0] = recvData.ReadBit();
    playerGuid[5] = recvData.ReadBit();
    playerGuid[4] = recvData.ReadBit();
    playerGuid[3] = recvData.ReadBit();

    recvData.ReadByteSeq(playerGuid[7]);
    recvData.ReadByteSeq(playerGuid[2]);
    recvData.ReadByteSeq(playerGuid[1]);
    recvData.ReadByteSeq(playerGuid[5]);
    recvData.ReadByteSeq(playerGuid[4]);
    recvData.ReadByteSeq(playerGuid[0]);
    recvData.ReadByteSeq(playerGuid[3]);
    recvData.ReadByteSeq(playerGuid[6]);


    if (action == 1)
    {
        if (!_player->InBattleground())
            _player->SetBattlegroundEntryPoint();

        // resurrect the player
        if (!_player->isAlive())
        {
            _player->ResurrectPlayer(1.0f);
            _player->SpawnCorpseBones();
        }
        // stop taxi flight at port
        if (_player->isInFlight())
        {
            _player->GetMotionMaster()->MovementExpired();
            _player->CleanupAfterTaxiFlight();
        }

        if (uint64 lootGuid = GetPlayer()->GetLootGUID())
            DoLootRelease(lootGuid);

        SetBattlegroundPortData(GetPlayer()->GetGUID(), joinTime, queueSlot, action);
        GetPlayer()->SaveToDB(false, true);
    }
    else
    {
        WorldPacket pckt(IR_CMSG_BATTLEFIELD_PORT, 8 + 4 + 4 + 1);

        pckt << uint64(GetPlayer()->GetGUID());
        pckt << uint32(joinTime);
        pckt << uint32(queueSlot);
        pckt << uint8(0);

        tunnel->SendPacket(&pckt);
    }
}

void WorldSession::HandleIRBattlefieldStatusOpcode(InterRealmSession* tunnel)
{
    WorldPacket pckt(IR_CMSG_BATTLEFIELD_STATUS_QUERY, 8);
    pckt << uint64(GetPlayer()->GetGUID());
    tunnel->SendPacket(&pckt);
}