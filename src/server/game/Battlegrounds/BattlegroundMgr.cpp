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
#include "ObjectMgr.h"
#include "World.h"
#include "WorldPacket.h"

#include "Arena.h"
#include "BattlegroundMgr.h"
#include "BattlegroundAV.h"
#include "BattlegroundAB.h"
#include "BattlegroundEY.h"
#include "BattlegroundWS.h"
#include "BattlegroundNA.h"
#include "BattlegroundBE.h"
#include "BattlegroundAA.h"
#include "BattlegroundRL.h"
#include "BattlegroundSA.h"
#include "BattlegroundDS.h"
#include "BattlegroundRV.h"
#include "BattlegroundIC.h"
#include "BattlegroundRB.h"
#include "BattlegroundRBG.h"
#include "BattlegroundTV.h"
#include "BattlegroundTP.h"
#include "BattlegroundBFG.h"
#include "BattlegroundKT.h"
#include "BattlegroundSSM.h"
#include "BattlegroundDG.h"
#include "BattlegroundTTP.h"
#include "Chat.h"
#include "Map.h"
#include "MapInstanced.h"
#include "MapManager.h"
#include "Player.h"
#include "GameEventMgr.h"
#include "SharedDefines.h"
#include "Formulas.h"
#include "DisableMgr.h"
#include "LFG.h"
#include "Group.h"
#include <string>

/*********************************************************/
/***            BATTLEGROUND MANAGER                   ***/
/*********************************************************/

const extern BattlegroundTypeId battlegroundTypes[] = {
    BATTLEGROUND_TYPE_NONE,
    BATTLEGROUND_AV,
    BATTLEGROUND_WS,
    BATTLEGROUND_AB,
    BATTLEGROUND_NA,
    BATTLEGROUND_BE,
    BATTLEGROUND_AA,
    BATTLEGROUND_EY,
    BATTLEGROUND_RL,
    BATTLEGROUND_SA,
    BATTLEGROUND_DS,
    BATTLEGROUND_RV,
    BATTLEGROUND_IC,
    BATTLEGROUND_RB,
    BATTLEGROUND_RATED_10_VS_10,
    BATTLEGROUND_RATED_15_VS_15,
    BATTLEGROUND_RATED_25_VS_25,
    BATTLEGROUND_TP,
    BATTLEGROUND_BFG,
    BATTLEGROUND_EYR,
    BATTLEGROUND_KT,
    BATTLEGROUND_CTF3,
    BATTLEGROUND_SSM,
    BATTLEGROUND_TV,
    BATTLEGROUND_DG,
    BATTLEGROUND_TTP
};

BattlegroundMgr::BattlegroundMgr() : m_ArenaTesting(false)
{
    for (auto& bg : m_Battlegrounds)
    {
        bg.second.clear();
    }
    m_NextRatedArenaUpdate = sWorld->getIntConfig(CONFIG_ARENA_RATED_UPDATE_TIMER);
    m_Testing=false;

    InitBattlegroundQueues();
}

BattlegroundMgr::~BattlegroundMgr()
{
    DeleteAllBattlegrounds();
}

void BattlegroundMgr::DeleteAllBattlegrounds()
{
    for (auto typeId : battlegroundTypes)
    {
        BattlegroundSetMap::iterator i = m_Battlegrounds.find(typeId);
        if (i == m_Battlegrounds.end())
            continue;

        auto& set = (*i).second;
        for (BattlegroundSet::iterator itr = set.begin(); itr != set.end();)
        {
            Battleground* bg = itr->second;
            set.erase(itr++);

            if (!m_ClientBattlegroundIds[typeId][bg->GetBracketId()].empty())
                m_ClientBattlegroundIds[typeId][bg->GetBracketId()].erase(bg->GetClientInstanceID());
            delete bg;
        }
    }

    // destroy template battlegrounds that listed only in queues (other already terminated)
    for (auto& slot : BGFreeSlotQueue)
    {
        // ~Battleground call unregistring BG from queue
        while (!slot.second.empty())
            delete slot.second.front();
    }
}

// used to update running battlegrounds, and delete finished ones
void BattlegroundMgr::Update(uint32 diff)
{
    BattlegroundSet::iterator itr, next;
    for (auto typeId : battlegroundTypes)
    {
        itr = m_Battlegrounds[typeId].begin();
        // skip updating battleground template
        if (itr != m_Battlegrounds[typeId].end())
            ++itr;
        for (; itr != m_Battlegrounds[typeId].end(); itr = next)
        {
            next = itr;
            ++next;
            itr->second->Update(diff);
            // use the SetDeleteThis variable
            // direct deletion caused crashes
            if (itr->second->ToBeDeleted())
            {
                Battleground* bg = itr->second;
                m_Battlegrounds[typeId].erase(itr);
                if (!m_ClientBattlegroundIds[typeId][bg->GetBracketId()].empty())
                    m_ClientBattlegroundIds[typeId][bg->GetBracketId()].erase(bg->GetClientInstanceID());

                delete bg;
            }
        }
    }

    // update events timer
    for (int qtype = BATTLEGROUND_QUEUE_NONE; qtype < MAX_BATTLEGROUND_QUEUE_TYPES; ++qtype)
        m_BattlegroundQueues[qtype].UpdateEvents(diff);

    // update scheduled queues
    if (!m_QueueUpdateScheduler.empty())
    {
        std::vector<QueueSchedulerItem*> scheduled;
        {
            //copy vector and clear the other
            scheduled = std::vector<QueueSchedulerItem*>(m_QueueUpdateScheduler);
            m_QueueUpdateScheduler.clear();
            //release lock
        }

        for (uint8 i = 0; i < scheduled.size(); i++)
        {
            uint32 arenaMMRating = scheduled[i]->_arenaMMRating;
            uint8 arenaType = scheduled[i]->_arenaType;
            BattlegroundQueueTypeId bgQueueTypeId = scheduled[i]->_bgQueueTypeId;
            BattlegroundTypeId bgTypeId = scheduled[i]->_bgTypeId;
            BattlegroundBracketId bracket_id = scheduled[i]->_bracket_id;
            uint32 const* disabledBGs = scheduled[i]->_disabledBGs.data();
            m_BattlegroundQueues[bgQueueTypeId].BattlegroundQueueUpdate(diff, bgTypeId, bracket_id, arenaType, arenaMMRating > 0, arenaMMRating, disabledBGs);
        }
           
        /*for (std::vector<QueueSchedulerItem*>::iterator itr = scheduled.begin();
            itr != scheduled.end(); ++itr)
            delete *itr;*/
    }

    // if rating difference counts, maybe force-update queues
    if (sWorld->getIntConfig(CONFIG_ARENA_RATED_UPDATE_TIMER))
    {
        // it's time to force update
        if (m_NextRatedArenaUpdate < diff)
        {
            // forced update for rated arenas (scan all, but skipped non rated)
            sLog->outDebug(LOG_FILTER_BATTLEGROUND, "BattlegroundMgr: UPDATING ARENA QUEUES");
            for (int qtype = BATTLEGROUND_QUEUE_2v2; qtype <= BATTLEGROUND_QUEUE_5v5; ++qtype)
                for (int bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
                {
                    m_BattlegroundQueues[qtype].BattlegroundQueueUpdate(diff,
                        BATTLEGROUND_AA, BattlegroundBracketId(bracket),
                        BattlegroundMgr::BGArenaType(BattlegroundQueueTypeId(qtype)), true, 0);

                }

            m_NextRatedArenaUpdate = sWorld->getIntConfig(CONFIG_ARENA_RATED_UPDATE_TIMER);
        }
        else
            m_NextRatedArenaUpdate -= diff;
    }
}

void BattlegroundMgr::BuildBattlegroundStatusPacket(WorldPacket* data, Battleground* bg, Player * pPlayer, uint8 QueueSlot, uint8 StatusID, uint32 Time1, uint32 Time2, uint8 arenatype, uint8 uiFrame)
{
    // we can be in 2 queues in same time...
    if (!bg)
        StatusID = STATUS_NONE;

    uint8 minlevel = 0;
    if (bg)
    {
        PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), pPlayer->getLevel());
        if (bracketEntry)
            minlevel = bracketEntry->minLevel;
        else
            minlevel = bg->GetMinLevel();
    }

    

    ObjectGuid player_guid = pPlayer->GetGUID();
    ObjectGuid bg_guid = bg ? bg->GetGUID() : 0;

    switch (StatusID)
    {
        case STATUS_NONE:
        {
            data->Initialize(SMSG_BATTLEFIELD_STATUS);

            uint8 bitOrder[8] = {2, 3, 7, 6, 4, 1, 5, 0};
            data->WriteBitInOrder(player_guid, bitOrder);

            *data << uint32(QueueSlot);                 // Queue slot
            data->WriteByteSeq(player_guid[4]);
            data->WriteByteSeq(player_guid[2]);
            data->WriteByteSeq(player_guid[1]);
            data->WriteByteSeq(player_guid[6]);
            *data << uint32(Time1);                     // Join Time
            data->WriteByteSeq(player_guid[3]);
            data->WriteByteSeq(player_guid[5]);
            if (bg)
                *data << uint32(bg->IsRandom() ? uint32(BATTLEGROUND_RB) : bg->GetClientInstanceID());
            else
                *data << uint32(0);
            //*data << uint32(bg ? bg->GetClientInstanceID() : 0);                 // Client Instance Id
            data->WriteByteSeq(player_guid[0]);
            data->WriteByteSeq(player_guid[7]);
            break;
        }
        case STATUS_WAIT_QUEUE:
        {
            data->Initialize(SMSG_BATTLEFIELD_STATUS_QUEUED);

            *data << uint32(QueueSlot); //*data << uint32(bg->isArena() ? bg->GetMaxPlayersPerTeam() : 1);
            *data << uint32(GetMSTimeDiffToNow(Time2));                     // Estimated Wait Time
            *data << uint32(bg->IsRandom() ? uint32(BATTLEGROUND_RB) : bg->GetClientInstanceID()); // Client Instance ID
            //*data << uint32(bg->GetClientInstanceID());
            *data << uint32(Time1);
            *data << uint8(0); // byte32
            *data << uint32(Time2);                     //Time of the join
            *data << uint8(0); // byte30
            *data << uint32(bg->isArena() ? bg->GetMaxPlayersPerTeam() : 1); //*data << uint32(QueueSlot);
            *data << uint8(minlevel); //BG Min level
            
            data->WriteBit(bg_guid[1]);
            data->WriteBit(1);   // byte4C
            data->WriteBit(bg_guid[5]);
            data->WriteBit(player_guid[1]);
            data->WriteBit(player_guid[2]);
            data->WriteBit(player_guid[6]);
            data->WriteBit(bg_guid[7]);
            data->WriteBit(bg_guid[3]);
            data->WriteBit(0);   // Eligible In Queue byte45
            data->WriteBit(player_guid[7]);
            data->WriteBit(player_guid[0]);
            data->WriteBit(0);   // byte44
            data->WriteBit(bg_guid[4]);
            data->WriteBit(player_guid[4]);
            data->WriteBit(player_guid[5]);
            data->WriteBit(player_guid[3]);
            data->WriteBit(bg_guid[6]);
            data->WriteBit(bg_guid[2]);
            data->WriteBit(bg->isRated());   // byte38
            data->WriteBit(bg_guid[0]);

            data->WriteByteSeq(player_guid[4]);
            data->WriteByteSeq(bg_guid[5]);
            data->WriteByteSeq(bg_guid[6]);
            data->WriteByteSeq(player_guid[6]);
            data->WriteByteSeq(player_guid[1]);
            data->WriteByteSeq(bg_guid[3]);
            data->WriteByteSeq(bg_guid[7]);
            data->WriteByteSeq(bg_guid[1]);
            data->WriteByteSeq(player_guid[0]);
            data->WriteByteSeq(player_guid[2]);
            data->WriteByteSeq(bg_guid[2]);
            data->WriteByteSeq(player_guid[7]);
            data->WriteByteSeq(player_guid[5]);
            data->WriteByteSeq(player_guid[3]);
            data->WriteByteSeq(bg_guid[4]);
            data->WriteByteSeq(bg_guid[0]);
            break;
        }
        case STATUS_WAIT_JOIN:
        {
            data->Initialize(SMSG_BATTLEFIELD_STATUS_NEED_CONFIRMATION, 44);

            uint8 role = pPlayer->GetBattleGroundRoles();

            data->WriteBit(player_guid[3]);
            data->WriteBit(bg->isRated()); // byte38
            data->WriteBit(bg_guid[5]);
            data->WriteBit(player_guid[2]);
            data->WriteBit(player_guid[7]);
            data->WriteBit(bg_guid[4]);
            data->WriteBit(bg_guid[7]);
            data->WriteBit(player_guid[0]);
            data->WriteBit(bg_guid[2]);
            data->WriteBit(bg_guid[3]);
            data->WriteBit(player_guid[6]);
            data->WriteBit(player_guid[4]);
            data->WriteBit(player_guid[1]);
            data->WriteBit(bg_guid[6]);
            data->WriteBit(role == ROLE_DAMAGE);
            data->WriteBit(bg_guid[1]);
            data->WriteBit(bg_guid[0]);
            data->WriteBit(player_guid[5]);

            data->WriteByteSeq(bg_guid[3]);
            *data << uint32(bg->IsRandom() ? uint32(BATTLEGROUND_RB) : bg->GetClientInstanceID());
            //*data << uint32(bg->GetClientInstanceID());
            data->WriteByteSeq(player_guid[2]);
            data->WriteByteSeq(bg_guid[7]);
            data->WriteByteSeq(player_guid[5]);
            *data << uint32(bg->GetMapId());
            data->WriteByteSeq(player_guid[0]);
            data->WriteByteSeq(player_guid[4]);
            data->WriteByteSeq(bg_guid[0]);
            data->WriteByteSeq(bg_guid[1]);
            data->WriteByteSeq(bg_guid[6]);
            data->WriteByteSeq(bg_guid[5]);
            data->WriteByteSeq(player_guid[7]);
            *data << uint32(bg->isArena() ? bg->GetMaxPlayersPerTeam() : 1); //*data << uint32(QueueSlot);
            *data << uint8(minlevel);
            data->WriteByteSeq(player_guid[6]);
            *data << uint32(Time2);
            if (role != ROLE_DAMAGE)
            {
                // Client use sent value like this
                // Role = 1 << (val + 1)
                if (role == ROLE_TANK)
                    *data << uint8(0);
                else
                    *data << uint8(1);
            }
            data->WriteByteSeq(player_guid[1]);
            *data << uint32(QueueSlot); //*data << uint32(bg->isArena() ? bg->GetMaxPlayersPerTeam() : 1);
            *data << uint8(0); // byte32
            data->WriteByteSeq(bg_guid[4]);
            data->WriteByteSeq(bg_guid[2]);
            *data << uint32(Time2);                     // Time until closed
            data->WriteByteSeq(player_guid[3]);
            *data << uint8(0); // byte30

            break;
        }
        case STATUS_IN_PROGRESS:
        case STATUS_WAIT_LEAVE:
        {
            data->Initialize(SMSG_BATTLEFIELD_STATUS_ACTIVE, 49);

            data->WriteBit(player_guid[5]);
            data->WriteBit(bg_guid[4]);
            data->WriteBit(bg_guid[6]);
            data->WriteBit(player_guid[1]);
            data->WriteBit(bg_guid[5]);
            data->WriteBit(player_guid[3]);
            data->WriteBit(bg_guid[0]);
            data->WriteBit(bg_guid[2]);
            data->WriteBit(player_guid[7]);
            data->WriteBit(bg_guid[3]);
            data->WriteBit(player_guid[6]);
            data->WriteBit(bg_guid[7]);
            data->WriteBit(player_guid[4]);
            data->WriteBit(player_guid[0]);
            data->WriteBit(pPlayer->GetBGTeam() == HORDE ? 0 : 1);     // Battlefield Faction ( 0 horde, 1 alliance )
            data->WriteBit(bg->isRated());
            data->WriteBit(player_guid[2]);
            data->WriteBit(bg_guid[1]);
            data->WriteBit(false); // TournamentRules

            data->WriteByteSeq(bg_guid[7]);
            *data << uint8(0); // byte30
            data->WriteByteSeq(player_guid[6]);
            *data << uint32(bg->GetStatus() == STATUS_WAIT_LEAVE ? Time1 : (bg->GetPrematureCountDown() ? bg->GetPrematureCountDownTimer() : 0));  // Time to Close
            *data << uint8(bg->GetMinLevel());          // Min Level
            data->WriteByteSeq(bg_guid[2]);
            data->WriteByteSeq(bg_guid[1]);
            data->WriteByteSeq(player_guid[3]);
            data->WriteByteSeq(bg_guid[3]);
            *data << uint32(Time2); // Elapsed Time
            data->WriteByteSeq(bg_guid[5]);
            *data << uint32(QueueSlot); //*data << uint32(bg->isArena() ? bg->GetMaxPlayersPerTeam() : 1);                 // Queue slot
            data->WriteByteSeq(bg_guid[4]);
            data->WriteByteSeq(player_guid[7]);
            data->WriteByteSeq(player_guid[5]);
            data->WriteByteSeq(player_guid[0]);
            *data << uint32(bg->IsRandom() ? uint32(BATTLEGROUND_RB) : bg->GetClientInstanceID()); // Client Instance ID     
            *data << uint32(bg->isArena() ? bg->GetMaxPlayersPerTeam() : 1); //*data << uint32(QueueSlot);
            data->WriteByteSeq(player_guid[4]);
            *data << uint8(0); // byte32
            data->WriteByteSeq(bg_guid[0]);
            *data << uint32(bg->GetMapId());            // Map Id
            data->WriteByteSeq(bg_guid[6]);
            data->WriteByteSeq(player_guid[1]);
            data->WriteByteSeq(player_guid[2]);
            *data << uint32(GetMSTimeDiffToNow(Time2));

            break;
        }
        /*case STATUS_WAIT_LEAVE:
        {
            data->Initialize(SMSG_BATTLEFIELD_STATUS_WAIT_FOR_GROUPS, 48);

            data->WriteBit(player_guid[4]);
            data->WriteBit(player_guid[3]);
            data->WriteBit(bg_guid[5]);
            data->WriteBit(bg_guid[0]);
            data->WriteBit(bg_guid[1]);
            data->WriteBit(bg->isRated());              // Is Rated
            data->WriteBit(bg_guid[7]);
            data->WriteBit(player_guid[1]);
            data->WriteBit(player_guid[0]);
            data->WriteBit(player_guid[5]);
            data->WriteBit(bg_guid[3]);
            data->WriteBit(bg_guid[6]);
            data->WriteBit(player_guid[6]);
            data->WriteBit(player_guid[7]);
            data->WriteBit(bg_guid[2]);
            data->WriteBit(bg_guid[4]);
            data->WriteBit(player_guid[2]);
            data->FlushBits();
            
            *data << uint32(0);  // unk32 1
            *data << uint32(0);  // unk32 2
            data->WriteByteSeq(player_guid[0]);
            data->WriteByteSeq(player_guid[7]);
            data->WriteByteSeq(bg_guid[6]);
            data->WriteByteSeq(player_guid[4]);
            *data << uint8(0);  // unk byte1
            *data << uint8(0);  // unk byte2
            data->WriteByteSeq(player_guid[2]);
            data->WriteByteSeq(player_guid[3]);
            data->WriteByteSeq(bg_guid[4]);
            data->WriteByteSeq(bg_guid[3]);
            data->WriteByteSeq(bg_guid[0]);
            data->WriteByteSeq(bg_guid[1]);
            data->WriteByteSeq(bg_guid[5]);
            data->WriteByteSeq(bg_guid[7]);
            *data << uint32(0);  // unk32 3
            *data << uint8(0);  // unk byte3
            data->WriteByteSeq(bg_guid[2]);
            data->WriteByteSeq(player_guid[1]);
            data->WriteByteSeq(player_guid[5]);
            *data << uint32(0);  // unk32 4
            *data << uint32(0);  // unk32 5
            *data << uint8(0);  // unk byte4
            *data << uint8(0);  // unk byte5
            *data << uint8(0);  // unk byte6
            *data << uint8(0);  // unk byte7
            data->WriteByteSeq(player_guid[6]);
            *data << uint32(0);  // unk32 6*/

          /*  

            *data << uint8(0);                          // unk
            *data << uint32(bg->GetStatus());           // Status
            *data << uint32(QueueSlot);                 // Queue slot
            *data << uint32(Time1);                     // Time until closed
            *data << uint32(0);                         // unk
            *data << uint8(0);                          // unk
            *data << uint8(0);                          // unk
            *data << uint8(bg->GetMinLevel());          // Min Level
            *data << uint8(0);                          // unk
            *data << uint8(0);                          // unk
            *data << uint32(bg->GetMapId());            // Map Id
            *data << uint32(Time2);                     // Time
            *data << uint8(0);                          // unk

            data->WriteBit(bg_guid[0]);
            data->WriteBit(bg_guid[1]);
            data->WriteBit(bg_guid[7]);
            data->WriteBit(player_guid[7]);
            data->WriteBit(player_guid[0]);
            data->WriteBit(bg_guid[4]);
            data->WriteBit(player_guid[6]);
            data->WriteBit(player_guid[2]);
            data->WriteBit(player_guid[3]);
            data->WriteBit(bg_guid[3]);
            data->WriteBit(player_guid[4]);
            data->WriteBit(bg_guid[5]);
            data->WriteBit(player_guid[5]);
            data->WriteBit(bg_guid[2]);
            data->WriteBit(bg->isRated());              // Is Rated
            data->WriteBit(player_guid[1]);
            data->WriteBit(bg_guid[6]);

            data->FlushBits();

            data->WriteByteSeq(player_guid[0]);
            data->WriteByteSeq(bg_guid[4]);
            data->WriteByteSeq(player_guid[3]);
            data->WriteByteSeq(bg_guid[1]);
            data->WriteByteSeq(bg_guid[0]);
            data->WriteByteSeq(bg_guid[2]);
            data->WriteByteSeq(player_guid[2]);
            data->WriteByteSeq(bg_guid[7]);
            data->WriteByteSeq(player_guid[1]);
            data->WriteByteSeq(player_guid[6]);
            data->WriteByteSeq(bg_guid[6]);
            data->WriteByteSeq(bg_guid[5]);
            data->WriteByteSeq(player_guid[5]);
            data->WriteByteSeq(player_guid[4]);
            data->WriteByteSeq(player_guid[7]);
            data->WriteByteSeq(bg_guid[3]);
            break;
        }*/
    }
}

void BattlegroundMgr::BuildPvpLogDataPacket(WorldPacket* data, Battleground* bg)
{
    bool isArena = bg->isArena();
    bool finished = bg->GetStatus() == STATUS_WAIT_LEAVE;
    bool finishedArena = isArena && finished;

    int32 count1 = 0;
    int32 count2 = 0;

    data->Initialize(SMSG_PVP_LOG_DATA);

    uint32 nPlayers[BG_TEAMS_COUNT] = { 0 };

    data->WriteBit(finishedArena);
    data->WriteBit(false);

    size_t countPos = data->wpos();
    uint8 bitPos = data->bitpos();
    data->WriteBits(0, 19);

    for (Battleground::BattlegroundScoreMap::const_iterator itr2 = bg->GetPlayerScoresBegin(); itr2 != bg->GetPlayerScoresEnd(); ++itr2)
    {
        ObjectGuid guid = itr2->first;

        if (!bg->IsPlayerInBattleground(guid))
        {
            sLog->outError(LOG_FILTER_BATTLEGROUND, "Player " UI64FMTD " has scoreboard entry for battleground %u but is not in battleground!", uint64(guid), bg->GetTypeID(true));
            continue;
        }

        Player* player = ObjectAccessor::FindPlayer(guid);
        if (!player)
            continue;

        uint32 index = bg->GetTeamIndexByTeamId(player->GetBGTeam()) == BG_TEAM_ALLIANCE;
        ++nPlayers[index];

        ++count1;
        if (count1 <= 80)
        {
            data->WriteBit(guid[4]);
            data->WriteBit(player->IsInWorld());           // IsOnline
            data->WriteBit(guid[0]);
            data->WriteBit(!isArena);
            data->WriteBit(guid[7]);
            data->WriteBit(guid[5]);
            data->WriteBit(finishedArena);         // unk1
            data->WriteBit(guid[1]);
            data->WriteBit(finishedArena);         // unk2
            if (bg->isArena() || bg->IsRatedBG())
                data->WriteBit(bg->GetPlayerTeam(guid) == ALLIANCE);
            else
                data->WriteBit(player->GetTeam() == ALLIANCE);
            switch (bg->GetTypeID(true))                             // Custom values
            {
            case BATTLEGROUND_RB:
                switch (bg->GetMapId())
                {
                case 489:
                case 529:
                case 607:
                case 628:
                case 726:
                    data->WriteBits(0x00000002, 22);
                    break;
                case 761:
                    data->WriteBits(0x00000002, 22);
                    break;
                case 30:
                    data->WriteBits(0x00000005, 22);
                    break;
                case 566:
                    data->WriteBits(0x00000001, 22);
                    break;
                case 1105:
                    data->WriteBits(0x00000004, 22);
                    break;
                case 727:
                    data->WriteBits(0x00000001, 22);
                    break;
                default:
                    data->WriteBits(0, 22);
                    break;
                }
                break;
            case BATTLEGROUND_AV:
                data->WriteBits(0x00000005, 22);
                break;
            case BATTLEGROUND_EY:
            case BATTLEGROUND_EYR:
                data->WriteBits(0x00000001, 22);
                break;
            case BATTLEGROUND_WS:
            case BATTLEGROUND_AB:
            case BATTLEGROUND_SA:
            case BATTLEGROUND_IC:
            case BATTLEGROUND_TP:
            case BATTLEGROUND_BFG:
            case BATTLEGROUND_KT:
                data->WriteBits(0x00000002, 22);
                break;
            case BATTLEGROUND_DG:
                data->WriteBits(0x00000004, 22);
                break;
            case BATTLEGROUND_SSM:
                data->WriteBits(0x00000001, 22);
                break;
            default:
                data->WriteBits(0, 22);
                break;
            }
            data->WriteBit(guid[2]);
            data->WriteBit(guid[3]);
            data->WriteBit(finishedArena);           // unk3
            data->WriteBit(finishedArena);           // unk4
            data->WriteBit(guid[6]);
        }
    }

    data->WriteBit(finished);

    for (Battleground::BattlegroundScoreMap::const_iterator itr2 = bg->GetPlayerScoresBegin(); itr2 != bg->GetPlayerScoresEnd(); ++itr2)
    {
        ObjectGuid guid = itr2->first;
        if (!bg->IsPlayerInBattleground(guid))
        {
            sLog->outError(LOG_FILTER_BATTLEGROUND, "Player " UI64FMTD " has scoreboard entry for battleground %u but is not in battleground!", uint64(guid), bg->GetTypeID(true));
            continue;
        }

        Player* player = ObjectAccessor::FindPlayer(guid);
        if (!player)
            continue;

        BattlegroundScore const* score = itr2->second;

        ++count2;
        if (count2 <= 80)
        {
            data->WriteByteSeq(guid[5]);
            *data << uint32(score->HealingDone);
            if (!isArena)
            {
                *data << uint32(score->HonorableKills);
                *data << uint32(score->BonusHonor / 100);
                *data << uint32(score->Deaths);

            }
            data->WriteByteSeq(guid[4]);
            data->WriteByteSeq(guid[0]);
            if (finishedArena)
                *data << int32(50);
            data->WriteByteSeq(guid[2]);
            *data << uint32(score->DamageDone);
            data->WriteByteSeq(guid[6]);
            data->WriteByteSeq(guid[7]);
            *data << uint32(player->GetPrimaryTalentTree(player->GetActiveSpec()));
            data->WriteByteSeq(guid[3]);
            data->WriteByteSeq(guid[1]);
            
            if (finishedArena)
                *data << int32(16);
            if (finishedArena)
                *data << int32(22);
            switch (bg->GetTypeID(true))                             // Custom values
            {
            case BATTLEGROUND_RB:
                switch (bg->GetMapId())
                {
                case 489:
                    *data << uint32(((BattlegroundWGScore*)score)->FlagCaptures);        // flag captures
                    *data << uint32(((BattlegroundWGScore*)score)->FlagReturns);         // flag returns
                    break;
                case 566:
                    *data << uint32(((BattlegroundEYScore*)score)->FlagCaptures);        // flag captures
                    break;
                case 529:
                    *data << uint32(((BattlegroundABScore*)score)->BasesAssaulted);      // bases asssulted
                    *data << uint32(((BattlegroundABScore*)score)->BasesDefended);       // bases defended
                    break;
                case 30:
                    *data << uint32(((BattlegroundAVScore*)score)->GraveyardsAssaulted); // GraveyardsAssaulted
                    *data << uint32(((BattlegroundAVScore*)score)->GraveyardsDefended);  // GraveyardsDefended
                    *data << uint32(((BattlegroundAVScore*)score)->TowersAssaulted);     // TowersAssaulted
                    *data << uint32(((BattlegroundAVScore*)score)->TowersDefended);      // TowersDefended
                    *data << uint32(((BattlegroundAVScore*)score)->MinesCaptured);       // MinesCaptured
                    break;
                case 607:
                    *data << uint32(((BattlegroundSAScore*)score)->demolishers_destroyed);
                    *data << uint32(((BattlegroundSAScore*)score)->gates_destroyed);
                    break;
                case 628:                                   // IC
                    *data << uint32(((BattlegroundICScore*)score)->BasesAssaulted);       // bases asssulted
                    *data << uint32(((BattlegroundICScore*)score)->BasesDefended);        // bases defended
                    break;
                case 726:
                    *data << uint32(((BattlegroundTPScore*)score)->FlagCaptures);         // flag captures
                    *data << uint32(((BattlegroundTPScore*)score)->FlagReturns);          // flag returns
                    break;
                case 761:
                    *data << uint32(((BattlegroundBFGScore*)score)->BasesAssaulted);      // bases asssulted
                    *data << uint32(((BattlegroundBFGScore*)itr2->second)->BasesDefended);       // bases defended
                    break;
                case 1105:
                    *data << uint32(((BattlegroundDGScore*)score)->m_CapturedCart);       // flag captures
                    *data << uint32(((BattlegroundDGScore*)score)->m_ReturnedCart);       // flag returns
                    *data << uint32(((BattlegroundDGScore*)score)->m_AssaultedMines);     // bases asssulted
                    *data << uint32(((BattlegroundDGScore*)score)->m_DefendedMines);      // bases defended
                    break;
                case 727:
                    *data << uint32(((BattlegroundSMScore*)score)->MineCartCaptures);
                    break;
                }
                break;
            case BATTLEGROUND_AV:
                *data << uint32(((BattlegroundAVScore*)score)->GraveyardsAssaulted); // GraveyardsAssaulted
                *data << uint32(((BattlegroundAVScore*)score)->GraveyardsDefended);  // GraveyardsDefended
                *data << uint32(((BattlegroundAVScore*)score)->TowersAssaulted);     // TowersAssaulted
                *data << uint32(((BattlegroundAVScore*)score)->TowersDefended);      // TowersDefended
                *data << uint32(((BattlegroundAVScore*)score)->MinesCaptured);       // MinesCaptured
                break;
            case BATTLEGROUND_WS:
                *data << uint32(((BattlegroundWGScore*)score)->FlagCaptures);        // flag captures
                *data << uint32(((BattlegroundWGScore*)score)->FlagReturns);         // flag returns
                break;
            case BATTLEGROUND_AB:
                *data << uint32(((BattlegroundABScore*)score)->BasesAssaulted);      // bases asssulted
                *data << uint32(((BattlegroundABScore*)score)->BasesDefended);       // bases defended
                break;
            case BATTLEGROUND_EY:
            case BATTLEGROUND_EYR:
                *data << uint32(((BattlegroundEYScore*)score)->FlagCaptures);        // flag captures
                break;
            case BATTLEGROUND_SA:
                *data << uint32(((BattlegroundSAScore*)score)->demolishers_destroyed);
                *data << uint32(((BattlegroundSAScore*)score)->gates_destroyed);
                break;
            case BATTLEGROUND_IC:
                *data << uint32(((BattlegroundICScore*)score)->BasesAssaulted);       // bases asssulted
                *data << uint32(((BattlegroundICScore*)score)->BasesDefended);        // bases defended
                break;
            case BATTLEGROUND_TP:
                *data << uint32(((BattlegroundTPScore*)score)->FlagCaptures);         // flag captures
                *data << uint32(((BattlegroundTPScore*)score)->FlagReturns);          // flag returns
                break;
            case BATTLEGROUND_BFG:
                *data << uint32(((BattlegroundBFGScore*)score)->BasesAssaulted);      // bases asssulted
                *data << uint32(((BattlegroundBFGScore*)score)->BasesDefended);       // bases defended
                break;
            case BATTLEGROUND_KT:
                *data << uint32(((BattleGroundKTScore*)score)->OrbHandles);
                *data << uint32(((BattleGroundKTScore*)score)->Score * 10);
                break;
            case BATTLEGROUND_DG:
                *data << uint32(((BattlegroundDGScore*)score)->m_CapturedCart);       // flag captures
                *data << uint32(((BattlegroundDGScore*)score)->m_ReturnedCart);       // flag returns
                *data << uint32(((BattlegroundDGScore*)score)->m_AssaultedMines);     // bases asssulted
                *data << uint32(((BattlegroundDGScore*)score)->m_DefendedMines);      // bases defended
                break;
            case BATTLEGROUND_SSM:
                *data << uint32(((BattlegroundSMScore*)score)->MineCartCaptures);
                break;
            }
            if (finishedArena)
                *data << int32(score->RatingChange);
            *data << uint32(score->KillingBlows);
        }
    }

    ASSERT(count1 == count2);

    data->PutBits(count1, 19, countPos, bitPos);

    *data << uint8(nPlayers[1]);
    *data << uint8(nPlayers[0]);
    
    if (finished)
        *data << uint8(bg->GetWinner());

    if (finishedArena)
    {
        auto const& score0 = bg->_arenaTeamScores[BG_TEAM_ALLIANCE];
        auto const& score1 = bg->_arenaTeamScores[BG_TEAM_HORDE];

        *data << int32(score0.OldRating);
        *data << int32(score1.MatchmakerRating);
        *data << int32(score1.NewRating);
        *data << int32(score0.NewRating);
        *data << int32(score1.OldRating);
        *data << int32(score0.MatchmakerRating);
    }
}

void BattlegroundMgr::BuildStatusFailedPacket(WorldPacket* data, Battleground* bg, Player* player, uint8 QueueSlot, GroupJoinBattlegroundResult result)
{
    ObjectGuid player_guid = player->GetGUID(); // player who caused the error
    ObjectGuid bg_guid = bg->GetGUID();
    ObjectGuid unkguid = 0;

    data->Initialize(SMSG_BATTLEFIELD_STATUS_FAILED);

    data->WriteBit(unkguid[5]);
    data->WriteBit(player_guid[1]);
    data->WriteBit(player_guid[6]);
    data->WriteBit(player_guid[3]);
    data->WriteBit(bg_guid[4]);
    data->WriteBit(unkguid[4]);
    data->WriteBit(unkguid[0]);
    data->WriteBit(bg_guid[2]);
    data->WriteBit(player_guid[4]);
    data->WriteBit(unkguid[1]);
    data->WriteBit(unkguid[6]);
    data->WriteBit(bg_guid[7]);
    data->WriteBit(player_guid[2]);
    data->WriteBit(unkguid[3]);
    data->WriteBit(bg_guid[0]);
    data->WriteBit(player_guid[5]);
    data->WriteBit(player_guid[0]);
    data->WriteBit(bg_guid[3]);
    data->WriteBit(bg_guid[5]);
    data->WriteBit(player_guid[7]);
    data->WriteBit(unkguid[2]);
    data->WriteBit(unkguid[7]);
    data->WriteBit(bg_guid[6]);
    data->WriteBit(bg_guid[1]);

    *data << uint32(player->GetBattlegroundQueueJoinTime(bg->GetTypeID())); // Join Time RANDOM
    data->WriteByteSeq(unkguid[2]);
    data->WriteByteSeq(player_guid[4]);
    data->WriteByteSeq(player_guid[3]);
    data->WriteByteSeq(player_guid[7]);
    data->WriteByteSeq(bg_guid[3]);
    data->WriteByteSeq(player_guid[2]);
    data->WriteByteSeq(bg_guid[7]);
    *data << uint32(QueueSlot);                 // Queue slot
    data->WriteByteSeq(unkguid[0]);
    data->WriteByteSeq(bg_guid[2]);
    data->WriteByteSeq(unkguid[4]);
    data->WriteByteSeq(unkguid[5]);
    *data << uint32(bg->IsRandom() ? uint32(BATTLEGROUND_RB) : bg->GetClientInstanceID());
    data->WriteByteSeq(player_guid[1]);
    data->WriteByteSeq(bg_guid[1]);
    data->WriteByteSeq(unkguid[7]);
    *data << uint32(result);
    data->WriteByteSeq(unkguid[6]);
    data->WriteByteSeq(bg_guid[0]);
    data->WriteByteSeq(bg_guid[6]);
    data->WriteByteSeq(bg_guid[5]);
    data->WriteByteSeq(player_guid[6]);
    data->WriteByteSeq(bg_guid[4]);
    data->WriteByteSeq(player_guid[0]);
    data->WriteByteSeq(unkguid[3]);
    data->WriteByteSeq(player_guid[5]);
    data->WriteByteSeq(unkguid[1]);
}

void BattlegroundMgr::BuildUpdateWorldStatePacket(WorldPacket* data, uint32 field, uint32 value)
{
    data->Initialize(SMSG_UPDATE_WORLD_STATE, 4+4+1);
    data->WriteBit(0);
    *data << uint32(value);
    *data << uint32(field);
}

void BattlegroundMgr::BuildPlayerLeftBattlegroundPacket(WorldPacket* data, uint64 guid)
{
    ObjectGuid guidBytes = guid;

    data->Initialize(SMSG_BATTLEGROUND_PLAYER_LEFT, 8);
    
    uint8 bitOrder[8] = { 4, 6, 3, 0, 7, 5, 1, 2 };
    data->WriteBitInOrder(guidBytes, bitOrder);
    
    uint8 byteOrder[8] = { 4, 7, 0, 6, 5, 2, 1, 3 };
    data->WriteBytesSeq(guidBytes, byteOrder);
}

void BattlegroundMgr::BuildPlayerJoinedBattlegroundPacket(WorldPacket* data, uint64 guid)
{
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_JOINED, 8);
    ObjectGuid playerGuid = guid;

    uint8 bits[8] = { 1, 3, 0, 5, 7, 4, 6, 2 };
    data->WriteBitInOrder(playerGuid, bits);

    uint8 bytes[8] = { 4, 1, 7, 3, 0, 2, 5, 6 };
    data->WriteBytesSeq(playerGuid, bytes);
}

Battleground* BattlegroundMgr::GetBattlegroundThroughClientInstance(uint32 instanceId, BattlegroundTypeId bgTypeId)
{
    //cause at HandleBattlegroundJoinOpcode the clients sends the instanceid he gets from
    //SMSG_BATTLEFIELD_LIST we need to find the battleground with this clientinstance-id
    Battleground* bg = GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return NULL;

    if (bg->isArena())
        return GetBattleground(instanceId, bgTypeId);

    for (BattlegroundSet::iterator itr = m_Battlegrounds[bgTypeId].begin(); itr != m_Battlegrounds[bgTypeId].end(); ++itr)
    {
        if (itr->second->GetClientInstanceID() == instanceId)
            return itr->second;
    }
    return NULL;
}

BattlegroundSet BattlegroundMgr::GetBattlegroundsByType(BattlegroundTypeId bgTypeId)
{
    return m_Battlegrounds[bgTypeId];
}

Battleground* BattlegroundMgr::GetBattleground(uint32 InstanceID, BattlegroundTypeId bgTypeId)
{
    if (!InstanceID)
        return NULL;
    //search if needed
    BattlegroundSet::iterator itr;
    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        for (auto& bg : m_Battlegrounds)
        {
            itr = bg.second.find(InstanceID);
            if (itr != bg.second.end())
                return itr->second;
        }
        return NULL;
    }
    itr = m_Battlegrounds[bgTypeId].find(InstanceID);
    return ((itr != m_Battlegrounds[bgTypeId].end()) ? itr->second : NULL);
}

Battleground* BattlegroundMgr::GetBattlegroundTemplate(BattlegroundTypeId bgTypeId)
{
    //map is sorted and we can be sure that lowest instance id has only BG template
    return m_Battlegrounds[bgTypeId].empty() ? NULL : m_Battlegrounds[bgTypeId].begin()->second;
}

uint32 BattlegroundMgr::CreateClientVisibleInstanceId(BattlegroundTypeId bgTypeId, BattlegroundBracketId bracket_id)
{
    if (IsArenaType(bgTypeId))
        return 0;                                           //arenas don't have client-instanceids

    return uint32(bgTypeId);

    // we create here an instanceid, which is just for
    // displaying this to the client and without any other use..
    // the client-instanceIds are unique for each battleground-type
    // the instance-id just needs to be as low as possible, beginning with 1
    // the following works, because std::set is default ordered with "<"
    // the optimalization would be to use as bitmask std::vector<uint32> - but that would only make code unreadable
    uint32 lastId = 0;
    for (std::set<uint32>::iterator itr = m_ClientBattlegroundIds[bgTypeId][bracket_id].begin(); itr != m_ClientBattlegroundIds[bgTypeId][bracket_id].end();)
    {
        if ((++lastId) != *itr)                             //if there is a gap between the ids, we will break..
            break;
        lastId = *itr;
    }
    m_ClientBattlegroundIds[bgTypeId][bracket_id].insert(lastId + 1);
    return lastId + 1;
}

// create a new battleground that will really be used to play
Battleground* BattlegroundMgr::CreateNewBattleground(BattlegroundTypeId bgTypeId, PvPDifficultyEntry const* bracketEntry, uint8 arenaType, bool isRated, uint32 const* disabledBGs, bool isWargame)
{
    // get the template BG
    Battleground* bg_template = GetBattlegroundTemplate(bgTypeId);
    BattlegroundSelectionWeightMap* selectionWeights = NULL;

    if (!bg_template)
    {
        sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground: CreateNewBattleground - bg template not found for %u", bgTypeId);
        return NULL;
    }

    bool isRandom = bgTypeId == BATTLEGROUND_RB;
    bool isRatedBg = bgTypeId == BATTLEGROUND_RATED_10_VS_10;
    bool isArena = bg_template->isArena();

    if (isArena)
        selectionWeights = /*&m_ArenaSelectionWeights*/ nullptr;
    else if (isRandom)
        selectionWeights = /*&m_BGSelectionWeights*/ nullptr;
    else if (isRatedBg)
        selectionWeights = /*&m_RatedBGSelectionWeights*/ nullptr;

    if (selectionWeights)
    {
        if (selectionWeights->empty())
           return NULL;
        uint32 Weight = 0;
        uint32 selectedWeight = 0;
        bgTypeId = BATTLEGROUND_TYPE_NONE;
        // Get sum of all weights
        for (BattlegroundSelectionWeightMap::const_iterator it = selectionWeights->begin(); it != selectionWeights->end(); ++it)
            Weight += it->second;
        if (!Weight)
            return NULL;
        // Select a random value
        selectedWeight = urand(0, Weight-1);

        // Select the correct bg (if we have in DB A(10), B(20), C(10), D(15) --> [0---A---9|10---B---29|30---C---39|40---D---54])
        Weight = 0;
        for (BattlegroundSelectionWeightMap::const_iterator it = selectionWeights->begin(); it != selectionWeights->end(); ++it)
        {
            Weight += it->second;
            if (selectedWeight < Weight)
            {
                bgTypeId = it->first;
                break;
            }
        }

        bg_template = GetBattlegroundTemplate(bgTypeId);
        if (!bg_template)
        {
            sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground: CreateNewBattleground - bg template not found for %u", bgTypeId);
            return NULL;
        }
    }
    else if (isRandom)
    {
        BattlemasterListEntry const* battlemaster = sBattlemasterListStore.LookupEntry(BATTLEGROUND_RB);
        ASSERT(battlemaster != nullptr && "There is no info about BATTLEGROUND_RB (32)");

        uint32 level = bracketEntry->minLevel + ((bracketEntry->maxLevel - bracketEntry->minLevel) / 2); // Convert 85-89 to 85 + (89-85) / 2 = 87

        std::vector<uint32> randomTypes;
        for (uint8 i = 0; i < 11; ++i)
        {
            uint32 const _mapId = battlemaster->mapid[i];
            if (_mapId == 0xFFFFFFFF)
                continue;

            if (disabledBGs != nullptr)
            {
                if (_mapId == disabledBGs[0] || _mapId == disabledBGs[1])
                    continue;
            }

            PvPDifficultyEntry const* _entry = GetBattlegroundBracketByLevel(_mapId, level);
            if (!_entry)
                continue;

            for (uint32 j = 1; j < sBattlemasterListStore.GetNumRows(); ++j)
            {
                BattlemasterListEntry const* bl = sBattlemasterListStore.LookupEntry(j);
                if (!bl || bl->mapid[0] != _mapId)
                    continue;

                randomTypes.push_back(bl->id);
                break;
            }
        }

        ASSERT(!randomTypes.empty());

        bgTypeId = BATTLEGROUND_TYPE_NONE;

        uint32 totalWeight = 0;
        for (auto itr : m_BGSelectionWeights)
        {
            if (std::find(
                randomTypes.begin(),
                randomTypes.end(),
                itr.first) == randomTypes.end())
                continue;

            totalWeight += itr.second;
        }

        if (totalWeight == 0)
        {
            uint32 index = urand(0, (randomTypes.size() - 1) * 5) / randomTypes.size();
            bgTypeId = BattlegroundTypeId(randomTypes[index]);
        }
        else
        {
            // Select a random value
            uint32 selectedWeight = urand(0, totalWeight - 1);

            totalWeight = 0;
            for (auto itr : m_BGSelectionWeights)
            {
                if (std::find(
                    randomTypes.begin(),
                    randomTypes.end(),
                    itr.first) == randomTypes.end())
                    continue;

                totalWeight += itr.second;
                if (selectedWeight < totalWeight)
                {
                    bgTypeId = itr.first;
                    break;
                }
            }
        }

        bg_template = GetBattlegroundTemplate(bgTypeId);
        if (!bg_template)
        {
            sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground: CreateNewBattleground - bg template not found for %u", bgTypeId);
            return NULL;
        }
    }
    else if (isArena)
    {
        uint32 level = bracketEntry->minLevel + ((bracketEntry->maxLevel - bracketEntry->minLevel) / 2); // Convert 85-89 to 85 + (89-85) / 2 = 87

        const static uint32 arenaMaps[7] = { 559, 562, 572, 617, 618, 980, 1134 };

        std::vector<uint32> randomTypes;

        for (auto _mapId : arenaMaps)
        {
            PvPDifficultyEntry const* _entry = GetBattlegroundBracketByLevel(_mapId, level);
            if (!_entry)
                continue;

            for (uint32 j = 1; j < sBattlemasterListStore.GetNumRows(); ++j)
            {
                BattlemasterListEntry const* bl = sBattlemasterListStore.LookupEntry(j);
                if (!bl || bl->mapid[0] != _mapId)
                    continue;

                randomTypes.push_back(bl->id);
                break;
            }
        }

        ASSERT(!randomTypes.empty());

        bgTypeId = BATTLEGROUND_TYPE_NONE;

        uint32 totalWeight = 0;
        for (auto itr : m_ArenaSelectionWeights)
        {
            if (std::find(
                randomTypes.begin(),
                randomTypes.end(),
                itr.first) == randomTypes.end())
                continue;

            totalWeight += itr.second;
        }

        if (totalWeight == 0)
        {
            uint32 index = urand(0, (randomTypes.size() - 1) * 5) / randomTypes.size();
            bgTypeId = BattlegroundTypeId(randomTypes[index]);
        }
        else
        {
            // Select a random value
            uint32 selectedWeight = urand(0, totalWeight - 1);

            totalWeight = 0;
            for (auto itr : m_ArenaSelectionWeights)
            {
                if (std::find(
                    randomTypes.begin(),
                    randomTypes.end(),
                    itr.first) == randomTypes.end())
                    continue;

                totalWeight += itr.second;
                if (selectedWeight < totalWeight)
                {
                    bgTypeId = itr.first;
                    break;
                }
            }
        }

        bg_template = GetBattlegroundTemplate(bgTypeId);
        if (!bg_template)
        {
            sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground: CreateNewBattleground - bg template not found for %u", bgTypeId);
            return NULL;
        }
    }
    else if (isRatedBg)
    {
        BattlemasterListEntry const* battlemaster = sBattlemasterListStore.LookupEntry(BATTLEGROUND_RATED_10_VS_10);
        ASSERT(battlemaster != nullptr && "There is no info about BATTLEGROUND_RATED_10_VS_10 (100)");
        
        uint32 level = bracketEntry->minLevel + ((bracketEntry->maxLevel - bracketEntry->minLevel) / 2); // Convert 85-89 to 85 + (89-85) / 2 = 87

        std::vector<uint32> randomTypes;
        for (uint8 i = 0; i < 11; ++i)
        {
            uint32 const _mapId = battlemaster->mapid[i];
            if (_mapId == 0xFFFFFFFF)
                continue;
            
            PvPDifficultyEntry const* _entry = GetBattlegroundBracketByLevel(_mapId, level);
            if (!_entry)
                continue;

            for (uint32 j = 1; j < sBattlemasterListStore.GetNumRows(); ++j)
            {
                BattlemasterListEntry const* bl = sBattlemasterListStore.LookupEntry(j);
                if (!bl || bl->mapid[0] != _mapId)
                    continue;

                randomTypes.push_back(bl->id);
                break;
            }
        }

        ASSERT(!randomTypes.empty());

        bgTypeId = BATTLEGROUND_TYPE_NONE;

        uint32 totalWeight = 0;
        for (auto itr : m_RatedBGSelectionWeights)
        {
            if (std::find(
                randomTypes.begin(),
                randomTypes.end(),
                itr.first) == randomTypes.end())
                continue;

            totalWeight += itr.second;
        }

        if (totalWeight == 0)
        {
            uint32 index = urand(0, (randomTypes.size() - 1) * 5) / randomTypes.size();
            bgTypeId = BattlegroundTypeId(randomTypes[index]);
        }
        else
        {
            // Select a random value
            uint32 selectedWeight = urand(0, totalWeight - 1);

            totalWeight = 0;
            for (auto itr : m_RatedBGSelectionWeights)
            {
                if (std::find(
                    randomTypes.begin(),
                    randomTypes.end(),
                    itr.first) == randomTypes.end())
                    continue;

                totalWeight += itr.second;
                if (selectedWeight < totalWeight)
                {
                    bgTypeId = itr.first;
                    break;
                }
            }
        }

        bg_template = GetBattlegroundTemplate(bgTypeId);
        if (!bg_template)
        {
            sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground: CreateNewBattleground - bg template not found for %u", bgTypeId);
            return NULL;
        }
    }

    Battleground* bg = NULL;
    // create a copy of the BG template
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV:
            bg = new BattlegroundAV(*(BattlegroundAV*)bg_template);
            break;
        case BATTLEGROUND_WS:
            bg = new BattlegroundWS(*(BattlegroundWS*)bg_template);
            break;
        case BATTLEGROUND_AB:
            bg = new BattlegroundAB(*(BattlegroundAB*)bg_template);
            break;
        case BATTLEGROUND_NA:
            bg = new BattlegroundNA(*(BattlegroundNA*)bg_template);
            break;
        case BATTLEGROUND_BE:
            bg = new BattlegroundBE(*(BattlegroundBE*)bg_template);
            break;
        case BATTLEGROUND_AA:
            bg = new BattlegroundAA(*(BattlegroundAA*)bg_template);
            break;
        case BATTLEGROUND_EY:
        case BATTLEGROUND_EYR:
            bg = new BattlegroundEY(*(BattlegroundEY*)bg_template);
            break;
        case BATTLEGROUND_RL:
            bg = new BattlegroundRL(*(BattlegroundRL*)bg_template);
            break;
        case BATTLEGROUND_SA:
            bg = new BattlegroundSA(*(BattlegroundSA*)bg_template);
            break;
        case BATTLEGROUND_DS:
            bg = new BattlegroundDS(*(BattlegroundDS*)bg_template);
            break;
        case BATTLEGROUND_RV:
            bg = new BattlegroundRV(*(BattlegroundRV*)bg_template);
            break;
        case BATTLEGROUND_IC:
            bg = new BattlegroundIC(*(BattlegroundIC*)bg_template);
            break;
        case BATTLEGROUND_TV:
            bg = new BattlegroundTV(*(BattlegroundTV*)bg_template);
            break;
        case BATTLEGROUND_TTP:
            bg = new BattlegroundTTP(*(BattlegroundTTP*)bg_template);
            break;
        case BATTLEGROUND_TP:
            bg = new BattlegroundTP(*(BattlegroundTP*)bg_template);
            break;
        case BATTLEGROUND_BFG:
            bg = new BattlegroundBFG(*(BattlegroundBFG*)bg_template);
            break;
        case BATTLEGROUND_RB:
            bg = new BattlegroundRB(*(BattlegroundRB*)bg_template);
            break;
        case BATTLEGROUND_KT:
            bg = new BattlegroundKT(*(BattlegroundKT*)bg_template);
            break;
        case BATTLEGROUND_SSM:
            bg = new BattlegroundSM(*(BattlegroundSM*)bg_template);
            break;
        case BATTLEGROUND_DG:
            bg = new BattlegroundDG(*(BattlegroundDG*)bg_template);
            break;
        case BATTLEGROUND_RATED_10_VS_10:
            bg = new BattlegroundRBG(*(BattlegroundRBG*)bg_template);
            break;
        default:
            //error, but it is handled few lines above
            return 0;
    }

    // set battelground difficulty before initialization
    bg->SetBracket(bracketEntry);

    // generate a new instance id
    bg->SetInstanceID(sMapMgr->GenerateInstanceId()); // set instance id
    bg->SetClientInstanceID(CreateClientVisibleInstanceId(isRandom ? BATTLEGROUND_RB : (isRatedBg ? BATTLEGROUND_RATED_10_VS_10 : bgTypeId), bracketEntry->GetBracketId()));

    // reset the new bg (set status to status_wait_queue from status_none)
    bg->Reset();

    // start the joining of the bg
    bg->SetStatus(STATUS_WAIT_JOIN);
    bg->SetArenaType(arenaType);
    bg->SetRated(isRated);
    bg->SetRatedBG(isRatedBg);
    bg->SetRandom(isRandom);
    bg->SetWargame(isWargame);
    bg->SetTypeID(isRandom ? BATTLEGROUND_RB : (isRatedBg ? BATTLEGROUND_RATED_10_VS_10 : bgTypeId));
    bg->SetRandomTypeID(bgTypeId);
    bg->InitGUID();

    return bg;
}

// used to create the BG templates
uint32 BattlegroundMgr::CreateBattleground(CreateBattlegroundData& data)
{
    // Create the BG
    Battleground* bg = NULL;
    switch (data.bgTypeId)
    {
        case BATTLEGROUND_AV: bg = new BattlegroundAV; break;
        case BATTLEGROUND_WS: bg = new BattlegroundWS; break;
        case BATTLEGROUND_AB: bg = new BattlegroundAB; break;
        case BATTLEGROUND_NA: bg = new BattlegroundNA; break;
        case BATTLEGROUND_BE: bg = new BattlegroundBE; break;
        case BATTLEGROUND_AA: bg = new BattlegroundAA; break;
        case BATTLEGROUND_EY:
        case BATTLEGROUND_EYR:
            bg = new BattlegroundEY;
            break;
        case BATTLEGROUND_RL: bg = new BattlegroundRL; break;
        case BATTLEGROUND_SA: bg = new BattlegroundSA; break;
        case BATTLEGROUND_DS: bg = new BattlegroundDS; break;
        case BATTLEGROUND_RV: bg = new BattlegroundRV; break;
        case BATTLEGROUND_IC: bg = new BattlegroundIC; break;
        case BATTLEGROUND_TV: bg = new BattlegroundTV; break;
        case BATTLEGROUND_TTP: bg = new BattlegroundTTP; break;
        case BATTLEGROUND_TP: bg = new BattlegroundTP; break;
        case BATTLEGROUND_BFG: bg = new BattlegroundBFG; break;
        case BATTLEGROUND_RB: bg = new BattlegroundRB; break;
        case BATTLEGROUND_KT: bg = new BattlegroundKT; break;
        case BATTLEGROUND_SSM: bg = new BattlegroundSM; break;
        case BATTLEGROUND_DG: bg = new BattlegroundDG; break;
        case BATTLEGROUND_RATED_10_VS_10: bg = new BattlegroundRBG; break;
        default:
            bg = new Battleground;
            break;
    }

    bg->SetMapId(data.MapID);
    bg->SetTypeID(data.bgTypeId);
    bg->InitGUID();
    bg->SetInstanceID(0);
    bg->SetArenaorBGType(data.IsArena);
    bg->SetRatedBG(data.bgTypeId == BATTLEGROUND_RATED_10_VS_10);
    bg->SetMinPlayersPerTeam(data.MinPlayersPerTeam);
    bg->SetMaxPlayersPerTeam(data.MaxPlayersPerTeam);
    bg->SetMinPlayers(data.MinPlayersPerTeam* 2);
    bg->SetMaxPlayers(data.MaxPlayersPerTeam* 2);
    bg->SetName(data.BattlegroundName);
    bg->SetTeamStartLoc(ALLIANCE, data.Team1StartLocX, data.Team1StartLocY, data.Team1StartLocZ, data.Team1StartLocO);
    bg->SetTeamStartLoc(HORDE,    data.Team2StartLocX, data.Team2StartLocY, data.Team2StartLocZ, data.Team2StartLocO);
    bg->SetStartMaxDist(data.StartMaxDist);
    bg->SetLevelRange(data.LevelMin, data.LevelMax);
    bg->SetHolidayId(data.holiday);
    bg->SetScriptId(data.scriptId);
    bg->SetClientInstanceID(uint32(data.bgTypeId));

    // add bg to update list
    AddBattleground(bg->GetInstanceID(), bg->GetTypeID(), bg);

    // return some not-null value, bgTypeId is good enough for me
    return data.bgTypeId;
}

void BattlegroundMgr::CreateInitialBattlegrounds()
{
    uint32 oldMSTime = getMSTime();

    uint8 selectionWeight;
    BattlemasterListEntry const* bl;
    BattlemasterListEntry const* rated_bl = sBattlemasterListStore.LookupEntry(BATTLEGROUND_RATED_10_VS_10);

    //                                               0   1                  2                  3       4       5                 6               7              8            9             10      11       12
    QueryResult result = WorldDatabase.Query("SELECT id, MinPlayersPerTeam, MaxPlayersPerTeam, MinLvl, MaxLvl, AllianceStartLoc, AllianceStartO, HordeStartLoc, HordeStartO, StartMaxDist, Weight, holiday, ScriptName, name_loc1, name_loc2, name_loc3, name_loc4, name_loc5, name_loc6, name_loc7, name_loc8, name_loc9, name_loc10, name_loc11 FROM battleground_template");

    if (!result)
    {
        sLog->outError(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 battlegrounds. DB table `battleground_template` is empty.");
        return;
    }

    uint32 count = 0, startId;

    do
    {
        Field* fields = result->Fetch();

        uint32 bgTypeID_ = fields[0].GetUInt32();
        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, bgTypeID_, NULL))
            continue;

        // can be overwrite by values from DB
        bl = sBattlemasterListStore.LookupEntry(bgTypeID_);
        if (!bl)
        {
            sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground ID %u not found in BattlemasterList.dbc. Battleground not created.", bgTypeID_);
            continue;
        }

        CreateBattlegroundData data;
        data.bgTypeId = BattlegroundTypeId(bgTypeID_);
        data.IsArena = (bl->type == TYPE_ARENA);
        data.MinPlayersPerTeam = fields[1].GetUInt16();
        data.MaxPlayersPerTeam = fields[2].GetUInt16();
        data.LevelMin = fields[3].GetUInt8();
        data.LevelMax = fields[4].GetUInt8();

        // check values from DB
        if (data.MaxPlayersPerTeam == 0 || data.MinPlayersPerTeam > data.MaxPlayersPerTeam)
        {
            sLog->outError(LOG_FILTER_SQL, "Table `battleground_template` for id %u has bad values for MinPlayersPerTeam (%u) and MaxPlayersPerTeam(%u)",
                data.bgTypeId, data.MinPlayersPerTeam, data.MaxPlayersPerTeam);
            continue;
        }

        if (data.LevelMin == 0 || data.LevelMax == 0 || data.LevelMin > data.LevelMax)
        {
            sLog->outError(LOG_FILTER_SQL, "Table `battleground_template` for id %u has bad values for LevelMin (%u) and LevelMax(%u)",
                data.bgTypeId, data.LevelMin, data.LevelMax);
            continue;
        }

        startId = fields[5].GetUInt32();
        if (WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry(startId))
        {
            data.Team1StartLocX = start->x;
            data.Team1StartLocY = start->y;
            data.Team1StartLocZ = start->z;
            data.Team1StartLocO = fields[6].GetFloat();
        }
        else if (data.bgTypeId == BATTLEGROUND_AA || data.bgTypeId == BATTLEGROUND_RB || data.bgTypeId == BATTLEGROUND_RATED_10_VS_10)
        {
            data.Team1StartLocX = 0;
            data.Team1StartLocY = 0;
            data.Team1StartLocZ = 0;
            data.Team1StartLocO = fields[6].GetFloat();
        }
        else
        {
            sLog->outError(LOG_FILTER_SQL, "Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `AllianceStartLoc`. BG not created.", data.bgTypeId, startId);
            continue;
        }

        startId = fields[7].GetUInt32();
        if (WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry(startId))
        {
            data.Team2StartLocX = start->x;
            data.Team2StartLocY = start->y;
            data.Team2StartLocZ = start->z;
            data.Team2StartLocO = fields[8].GetFloat();
        }
        else if (data.bgTypeId == BATTLEGROUND_AA || data.bgTypeId == BATTLEGROUND_RB || data.bgTypeId == BATTLEGROUND_RATED_10_VS_10)
        {
            data.Team2StartLocX = 0;
            data.Team2StartLocY = 0;
            data.Team2StartLocZ = 0;
            data.Team2StartLocO = fields[8].GetFloat();
        }
        else
        {
            sLog->outError(LOG_FILTER_SQL, "Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `HordeStartLoc`. BG not created.", data.bgTypeId, startId);
            continue;
        }

        data.StartMaxDist = fields[9].GetFloat();

        selectionWeight = fields[10].GetUInt8();
        data.holiday = fields[11].GetUInt32();
        data.scriptId = sObjectMgr->GetScriptId(fields[12].GetCString());

        data.BattlegroundName = fields[13 + sWorld->GetDefaultDbcLocale()].GetString();
        data.MapID = bl->mapid[0];

        if (!CreateBattleground(data))
            continue;

        if (data.IsArena)
        {
            if (data.bgTypeId != BATTLEGROUND_AA)
                m_ArenaSelectionWeights[data.bgTypeId] = selectionWeight;
        }
        else if (data.bgTypeId != BATTLEGROUND_RB)
        {
            if (data.bgTypeId != BATTLEGROUND_RATED_10_VS_10)
                m_BGSelectionWeights[data.bgTypeId] = selectionWeight;
        }

        for (int i = 0; i < 11; ++i)
            if (rated_bl->mapid[i] == bl->mapid[0] && bl->mapid[1] == -1)
                m_RatedBGSelectionWeights[data.bgTypeId] = selectionWeight;

        ++count;
    }
    while (result->NextRow());

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u battlegrounds in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void BattlegroundMgr::BuildBattlegroundListPacket(WorldPacket* data, ObjectGuid guid, Player* player, BattlegroundTypeId bgTypeId)
{
    if (!player)
        return;

    uint32 winner_conquest = (player->GetRandomWinner() ? BG_REWARD_WINNER_CONQUEST_FIRST : BG_REWARD_WINNER_CONQUEST_LAST) / 100;
    uint32 winner_honor = (player->GetRandomWinner() ? BG_REWARD_WINNER_HONOR_FIRST : BG_REWARD_WINNER_HONOR_LAST) / 100;

    ByteBuffer dataBuffer(32);

    uint32 count = 0;
    if (bgTypeId == BATTLEGROUND_AA)                         // arena
    {
        count = 0;
    }
    else                                                    // battleground
    {
        if (Battleground* bgTemplate = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId))
        {
            // expected bracket entry
            if (PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bgTemplate->GetMapId(), player->getLevel()))
            {
                BattlegroundBracketId bracketId = bracketEntry->GetBracketId();
                for (std::set<uint32>::iterator itr = m_ClientBattlegroundIds[bgTypeId][bracketId].begin(); itr != m_ClientBattlegroundIds[bgTypeId][bracketId].end();++itr)
                {
                    dataBuffer << uint32(*itr);
                    ++count;
                }
            }
        }
    }

    data->Initialize(SMSG_BATTLEFIELD_LIST, 1 + 1 + 8 + 1 + 1 + 28 + count*4);
    data->WriteBit(1);
    data->WriteBit(guid[2]);
    data->WriteBit(guid[3]);
    data->WriteBit(guid[0]);
    data->WriteBit(0); // byte50
    data->WriteBit(guid[4]);
    data->WriteBit(guid[7]);
    data->WriteBit(guid[5]);
    data->WriteBit(1); // byte50
    data->WriteBit(guid[6]);
    data->WriteBit(guid[1]);
    data->WriteBits(count, 22);
    data->WriteBit(0);

    *data << uint32(150);
    *data << uint8(90);

    data->WriteByteSeq(guid[1]);
    data->WriteByteSeq(guid[4]);
    data->append(dataBuffer);
    *data << uint8(90);
    *data << uint32(45);
    data->WriteByteSeq(guid[7]);
    data->WriteByteSeq(guid[0]);
    *data << uint32(32);
    data->WriteByteSeq(guid[6]);
    data->WriteByteSeq(guid[5]);
    data->WriteByteSeq(guid[3]);
    *data << uint32(270); // 270
    *data << uint32(270); // 270
    data->WriteByteSeq(guid[2]);
    *data << uint32(150);
    *data << uint32(45);
    
    /**data << uint32(winner_conquest); // dword28
    data->WriteByteSeq(guid[4]);
    *data << uint32(45); // dword20
    data->WriteByteSeq(guid[2]);
    data->WriteByteSeq(guid[7]);
    data->WriteByteSeq(guid[1]);
    data->WriteByteSeq(guid[5]);
    data->WriteByteSeq(guid[0]);
    data->WriteByteSeq(guid[3]);
    *data << uint32(winner_honor); // dword4C
    *data << uint32(winner_conquest); // dword1C
    data->append(dataBuffer);
    data->WriteByteSeq(guid[6]);
    *data << uint32(winner_honor); // dword44*/
}

void BattlegroundMgr::SendToBattleground(Player* player, uint32 instanceId, BattlegroundTypeId bgTypeId)
{
    Battleground* bg = GetBattleground(instanceId, bgTypeId);
    if (bg)
    {
        uint32 mapid = bg->GetMapId();
        float x, y, z, O;
        uint32 team = player->GetBGTeam();
        if (team == 0)
            team = player->GetTeam();
        bg->GetTeamStartLoc(team, x, y, z, O);

        /*if (x == 0 && y == 0 && z == 0)
        {
            sLog->outInfo(LOG_FILTER_SERVER_LOADING, "Incorrect coordinats for bgTypeId: %u, bg->GetTypeId: %u instanceId %u", bgTypeId, bg->GetTypeID(bg->IsRandom()), instanceId);
        }

        sLog->outInfo(LOG_FILTER_SERVER_LOADING, "BATTLEGROUND: Sending %s to map %u, X %f, Y %f, Z %f, O %f", player->GetName(), mapid, x, y, z, O);*/

        player->TeleportTo(mapid, x, y, z, O);
    }
    else
    {
        sLog->outError(LOG_FILTER_BATTLEGROUND, "player %u is trying to port to non-existent bg instance %u", player->GetGUIDLow(), instanceId);
    }
}

void BattlegroundMgr::SendAreaSpiritHealerQueryOpcode(Player* player, Battleground* bg, uint64 guid)
{
    WorldPacket data(SMSG_AREA_SPIRIT_HEALER_TIME, 12);
    ObjectGuid npcGuid = guid;

    uint32 time_ = 30000 - bg->GetLastResurrectTime();      // resurrect every 30 seconds
    if (time_ == uint32(-1))
        time_ = 0;

    uint8 bitsOrder[8] = { 5, 1, 6, 7, 4, 0, 2, 3 };
    data.WriteBitInOrder(npcGuid, bitsOrder);

    data.WriteByteSeq(npcGuid[5]);
    data.WriteByteSeq(npcGuid[0]);
    data.WriteByteSeq(npcGuid[7]);
    data << uint32(time_);
    data.WriteByteSeq(npcGuid[2]);
    data.WriteByteSeq(npcGuid[1]);
    data.WriteByteSeq(npcGuid[4]);
    data.WriteByteSeq(npcGuid[3]);
    data.WriteByteSeq(npcGuid[6]);

    player->GetSession()->SendPacket(&data);
}

void BattlegroundMgr::SendCheckWargameEntry(Player* p_Requester, Player* p_Target, uint64 p_QueueID)
{
    uint32 l_Timeout           = 60;
    ObjectGuid l_QueueID       = p_QueueID;
    ObjectGuid l_RequesterGUID = p_Requester->GetGUID();

    WorldPacket l_Data(SMSG_WARGAME_CHECK_ENTRY);
    l_Data.WriteBit(l_QueueID[6]);
    l_Data.WriteBit(l_QueueID[1]);
    l_Data.WriteBit(l_RequesterGUID[0]);
    l_Data.WriteBit(l_RequesterGUID[3]);
    l_Data.WriteBit(l_RequesterGUID[4]);
    l_Data.WriteBit(l_RequesterGUID[7]);
    l_Data.WriteBit(l_QueueID[2]);
    l_Data.WriteBit(l_RequesterGUID[2]);
    l_Data.WriteBit(l_QueueID[4]);
    l_Data.WriteBit(l_RequesterGUID[5]);
    l_Data.WriteBit(l_QueueID[7]);
    l_Data.WriteBit(l_QueueID[3]);
    l_Data.WriteBit(l_QueueID[0]);
    l_Data.WriteBit(l_QueueID[5]);
    l_Data.WriteBit(l_RequesterGUID[1]);
    l_Data.WriteBit(l_RequesterGUID[6]);

    l_Data.WriteByteSeq(l_QueueID[7]);
    l_Data.WriteByteSeq(l_RequesterGUID[1]);
    l_Data.WriteByteSeq(l_RequesterGUID[5]);
    l_Data << uint32(l_Timeout);
    l_Data.WriteByteSeq(l_QueueID[1]);
    l_Data.WriteByteSeq(l_QueueID[5]);
    l_Data.WriteByteSeq(l_RequesterGUID[0]);

    l_Data << uint32(realmID);
    l_Data << uint16(0);
    l_Data << uint8(0);

    l_Data.WriteByteSeq(l_QueueID[0]);
    l_Data.WriteByteSeq(l_RequesterGUID[4]);
    l_Data.WriteByteSeq(l_QueueID[6]);
    l_Data.WriteByteSeq(l_RequesterGUID[2]);
    l_Data.WriteByteSeq(l_QueueID[3]);
    l_Data.WriteByteSeq(l_RequesterGUID[3]);
    l_Data.WriteByteSeq(l_QueueID[4]);
    l_Data.WriteByteSeq(l_QueueID[2]);
    l_Data.WriteByteSeq(l_RequesterGUID[6]);
    l_Data << uint64(0);    ///< bnet id
    l_Data.WriteByteSeq(l_RequesterGUID[7]);

    p_Target->GetSession()->SendPacket(&l_Data);
}

bool BattlegroundMgr::IsArenaType(BattlegroundTypeId bgTypeId)
{
    return (bgTypeId == BATTLEGROUND_AA ||
        bgTypeId == BATTLEGROUND_BE ||
        bgTypeId == BATTLEGROUND_NA ||
        bgTypeId == BATTLEGROUND_DS ||
        bgTypeId == BATTLEGROUND_RV ||
        bgTypeId == BATTLEGROUND_RL ||
        bgTypeId == BATTLEGROUND_TV ||
        bgTypeId == BATTLEGROUND_TTP);
}

BattlegroundQueueTypeId BattlegroundMgr::BGQueueTypeId(BattlegroundTypeId bgTypeId, uint8 arenaType)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_WS:
            return BATTLEGROUND_QUEUE_WS;
        case BATTLEGROUND_AB:
            return BATTLEGROUND_QUEUE_AB;
        case BATTLEGROUND_AV:
            return BATTLEGROUND_QUEUE_AV;
        case BATTLEGROUND_EY:
            return BATTLEGROUND_QUEUE_EY;
        case BATTLEGROUND_SA:
            return BATTLEGROUND_QUEUE_SA;
        case BATTLEGROUND_IC:
            return BATTLEGROUND_QUEUE_IC;
        case BATTLEGROUND_TP:
            return BATTLEGROUND_QUEUE_TP;
        case BATTLEGROUND_BFG:
            return BATTLEGROUND_QUEUE_BFG;
        case BATTLEGROUND_RB:
            return BATTLEGROUND_QUEUE_RB;
        case BATTLEGROUND_KT:
            return BATTLEGROUND_QUEUE_KT;
        case BATTLEGROUND_CTF3:
            return BATTLEGROUND_QUEUE_CTF3;
        case BATTLEGROUND_SSM:
            return BATTLEGROUND_QUEUE_SSM;
        case BATTLEGROUND_DG:
            return BATTLEGROUND_QUEUE_DG;
        case BATTLEGROUND_AA:
        case BATTLEGROUND_NA:
        case BATTLEGROUND_RL:
        case BATTLEGROUND_BE:
        case BATTLEGROUND_DS:
        case BATTLEGROUND_RV:
        case BATTLEGROUND_TV:
        case BATTLEGROUND_TTP:
            switch (arenaType)
            {
                case ARENA_TYPE_2v2:
                    return BATTLEGROUND_QUEUE_2v2;
                case ARENA_TYPE_3v3:
                    return BATTLEGROUND_QUEUE_3v3;
                case ARENA_TYPE_5v5:
                    return BATTLEGROUND_QUEUE_5v5;
                default:
                    return BATTLEGROUND_QUEUE_NONE;
            }
        case BATTLEGROUND_RATED_10_VS_10:
            return BATTLEGROUND_QUEUE_RATED_10_VS_10;
        case BATTLEGROUND_RATED_15_VS_15:
            return BATTLEGROUND_QUEUE_RATED_15_VS_15;
        case BATTLEGROUND_RATED_25_VS_25:
            return BATTLEGROUND_QUEUE_RATED_25_VS_25;
        default:
            return BATTLEGROUND_QUEUE_NONE;
    }
}

BattlegroundTypeId BattlegroundMgr::BGTemplateId(BattlegroundQueueTypeId bgQueueTypeId)
{
    switch (bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_WS:
            return BATTLEGROUND_WS;
        case BATTLEGROUND_QUEUE_AB:
            return BATTLEGROUND_AB;
        case BATTLEGROUND_QUEUE_AV:
            return BATTLEGROUND_AV;
        case BATTLEGROUND_QUEUE_EY:
            return BATTLEGROUND_EY;
        case BATTLEGROUND_QUEUE_SA:
            return BATTLEGROUND_SA;
        case BATTLEGROUND_QUEUE_IC:
            return BATTLEGROUND_IC;
        case BATTLEGROUND_QUEUE_TP:
            return BATTLEGROUND_TP;
        case BATTLEGROUND_QUEUE_BFG:
            return BATTLEGROUND_BFG;
        case BATTLEGROUND_QUEUE_RB:
            return BATTLEGROUND_RB;
        case BATTLEGROUND_QUEUE_KT:
            return BATTLEGROUND_KT;
        case BATTLEGROUND_QUEUE_CTF3:
            return BATTLEGROUND_CTF3;
        case BATTLEGROUND_QUEUE_SSM:
            return BATTLEGROUND_SSM;
        case BATTLEGROUND_QUEUE_DG:
            return BATTLEGROUND_DG;
        case BATTLEGROUND_QUEUE_2v2:
        case BATTLEGROUND_QUEUE_3v3:
        case BATTLEGROUND_QUEUE_5v5:
            return BATTLEGROUND_AA;
        case BATTLEGROUND_QUEUE_RATED_10_VS_10:
            return BATTLEGROUND_RATED_10_VS_10;
        case BATTLEGROUND_QUEUE_RATED_15_VS_15:
            return BATTLEGROUND_RATED_15_VS_15;
        case BATTLEGROUND_QUEUE_RATED_25_VS_25:
            return BATTLEGROUND_RATED_25_VS_25;
        default:
            return BattlegroundTypeId(0);                   // used for unknown template (it existed and do nothing)
    }
}

uint8 BattlegroundMgr::BGArenaType(BattlegroundQueueTypeId bgQueueTypeId)
{
    switch (bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_2v2:
            return ARENA_TYPE_2v2;
        case BATTLEGROUND_QUEUE_3v3:
            return ARENA_TYPE_3v3;
        case BATTLEGROUND_QUEUE_5v5:
            return ARENA_TYPE_5v5;
        default:
            return 0;
    }
}

void BattlegroundMgr::ToggleTesting()
{
    m_Testing = !m_Testing;
    if (m_Testing)
        sWorld->SendWorldText(LANG_DEBUG_BG_ON);
    else
        sWorld->SendWorldText(LANG_DEBUG_BG_OFF);
}

void BattlegroundMgr::ToggleArenaTesting()
{
    m_ArenaTesting = !m_ArenaTesting;
    if (m_ArenaTesting)
        sWorld->SendWorldText(LANG_DEBUG_ARENA_ON);
    else
        sWorld->SendWorldText(LANG_DEBUG_ARENA_OFF);
}

void BattlegroundMgr::SetHolidayWeekends(std::list<uint32> activeHolidayId)
{
    for (auto bgtype : battlegroundTypes)
    {
        if (Battleground* bg = GetBattlegroundTemplate(BattlegroundTypeId(bgtype)))
        {
            bool holidayActivate = false;

            if (uint32 holidayId = bg->GetHolidayId())
                for (auto activeId: activeHolidayId)
                    if (holidayId == activeId)
                        holidayActivate = true;

            bg->SetHoliday(holidayActivate);
        }
    }
}

void BattlegroundMgr::ScheduleQueueUpdate(uint32 arenaMatchmakerRating, uint8 arenaType, BattlegroundQueueTypeId bgQueueTypeId, BattlegroundTypeId bgTypeId, BattlegroundBracketId bracket_id, uint32 const* disabledBGs)
{
    //This method must be atomic, TODO add mutex
    //we will use only 1 number created of bgTypeId and bracket_id
    QueueSchedulerItem* schedule_id = new QueueSchedulerItem(arenaMatchmakerRating, arenaType, bgQueueTypeId, bgTypeId, bracket_id, disabledBGs);
    bool found = false;
    for (uint8 i = 0; i < m_QueueUpdateScheduler.size(); i++)
    {
        if (m_QueueUpdateScheduler[i]->_arenaMMRating == arenaMatchmakerRating
            && m_QueueUpdateScheduler[i]->_arenaType == arenaType
            && m_QueueUpdateScheduler[i]->_bgQueueTypeId == bgQueueTypeId
            && m_QueueUpdateScheduler[i]->_bgTypeId == bgTypeId
            && m_QueueUpdateScheduler[i]->_bracket_id == bracket_id)
        {
            found = true;
            break;
        }
    }
    if (!found)
        m_QueueUpdateScheduler.push_back(schedule_id);
    else
        delete schedule_id;
}

uint32 BattlegroundMgr::GetMaxRatingDifference() const
{
    // this is for stupid people who can't use brain and set max rating difference to 0
    uint32 diff = sWorld->getIntConfig(CONFIG_ARENA_MAX_RATING_DIFFERENCE);
    if (diff == 0)
        diff = 5000;
    return diff;
}

uint32 BattlegroundMgr::GetRatingDiscardTimer() const
{
    return sWorld->getIntConfig(CONFIG_ARENA_RATING_DISCARD_TIMER);
}

uint32 BattlegroundMgr::GetPrematureFinishTime() const
{
    return sWorld->getIntConfig(CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER);
}

void BattlegroundMgr::LoadBattleMastersEntry()
{
    uint32 oldMSTime = getMSTime();

    mBattleMastersMap.clear();                                  // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry, bg_template FROM battlemaster_entry");

    if (!result)
    {
        sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 battlemaster entries. DB table `battlemaster_entry` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        ++count;

        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId  = fields[1].GetUInt32();
        if (!sBattlemasterListStore.LookupEntry(bgTypeId))
        {
            sLog->outError(LOG_FILTER_SQL, "Table `battlemaster_entry` contain entry %u for not existed battleground type %u, ignored.", entry, bgTypeId);
            continue;
        }

        mBattleMastersMap[entry] = BattlegroundTypeId(bgTypeId);
    }
    while (result->NextRow());

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u battlemaster entries in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

HolidayIds BattlegroundMgr::BGTypeToWeekendHolidayId(BattlegroundTypeId bgTypeId)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_AV: return HOLIDAY_CALL_TO_ARMS_AV;
        case BATTLEGROUND_EY: return HOLIDAY_CALL_TO_ARMS_EY;
        case BATTLEGROUND_WS: return HOLIDAY_CALL_TO_ARMS_WS;
        case BATTLEGROUND_SA: return HOLIDAY_CALL_TO_ARMS_SA;
        case BATTLEGROUND_AB: return HOLIDAY_CALL_TO_ARMS_AB;
        case BATTLEGROUND_IC: return HOLIDAY_CALL_TO_ARMS_IC;
        case BATTLEGROUND_TP: return HOLIDAY_CALL_TO_ARMS_TP;
        case BATTLEGROUND_BFG: return HOLIDAY_CALL_TO_ARMS_BFG;
        case BATTLEGROUND_SSM: return HOLIDAY_CALL_TO_ARMS_SM;
        case BATTLEGROUND_DG: return HOLIDAY_CALL_TO_ARMS_DG;
        default: return HOLIDAY_NONE;
    }
}

BattlegroundTypeId BattlegroundMgr::WeekendHolidayIdToBGType(HolidayIds holiday)
{
    switch (holiday)
    {
        case HOLIDAY_CALL_TO_ARMS_AV: return BATTLEGROUND_AV;
        case HOLIDAY_CALL_TO_ARMS_EY: return BATTLEGROUND_EY;
        case HOLIDAY_CALL_TO_ARMS_WS: return BATTLEGROUND_WS;
        case HOLIDAY_CALL_TO_ARMS_SA: return BATTLEGROUND_SA;
        case HOLIDAY_CALL_TO_ARMS_AB: return BATTLEGROUND_AB;
        case HOLIDAY_CALL_TO_ARMS_IC: return BATTLEGROUND_IC;
        case HOLIDAY_CALL_TO_ARMS_TP: return BATTLEGROUND_TP;
        case HOLIDAY_CALL_TO_ARMS_BFG: return BATTLEGROUND_BFG;
        case HOLIDAY_CALL_TO_ARMS_SM: return BATTLEGROUND_SSM;
        case HOLIDAY_CALL_TO_ARMS_DG: return BATTLEGROUND_DG;
        default: return BATTLEGROUND_TYPE_NONE;
    }
}

bool BattlegroundMgr::IsBGWeekend(BattlegroundTypeId bgTypeId)
{
    return IsHolidayActive(BGTypeToWeekendHolidayId(bgTypeId));
}

bool BattlegroundMgr::IsBattlegroundHoliday(BattlegroundTypeId bgTypeId) const
{
    BattlegroundHolidayContainer::const_iterator itr = m_BattlegroundHolidays.find(bgTypeId);
    if (itr != m_BattlegroundHolidays.end())
    {
        return itr->second;
    }

    return false;
}

void BattlegroundMgr::InitBattlegroundQueues()
{
    for (uint8 i = 0; i < MAX_BATTLEGROUND_QUEUE_TYPES; ++i)
    {
        m_BattlegroundQueues[i].SetQueueTypeId((BattlegroundQueueTypeId)i);
    }
}

void BattlegroundMgr::InitWargame(Group* p_First, Group* p_Second, BattlegroundTypeId p_BgTypeId)
{
    if (p_First == nullptr
        || p_Second == nullptr)
        return;

    uint8 l_ArenaType = 0;

    if (BattlegroundMgr::IsArenaType(p_BgTypeId))
    {
        if (p_First->GetMembersCount() != p_Second->GetMembersCount())
            return;

        switch (p_First->GetMembersCount())
        {
            case 2:
                l_ArenaType = ARENA_TYPE_2v2;
                break;
            case 3:
                l_ArenaType = ARENA_TYPE_3v3;
                break;
            case 5:
                l_ArenaType = ARENA_TYPE_5v5;
                break;
            default:
                return;
        }
    }

    Battleground* l_BattlegroundTemplate = sBattlegroundMgr->GetBattlegroundTemplate(p_BgTypeId);
    if (l_BattlegroundTemplate == nullptr)
        return;

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, p_BgTypeId, nullptr))
        return;

    BattlegroundQueueTypeId l_BGQueueTypeID  = BattlegroundMgr::BGQueueTypeId(p_BgTypeId, l_ArenaType);
    PvPDifficultyEntry const* l_BracketEntry = GetBattlegroundBracketByLevel(l_BattlegroundTemplate->GetMapId(), DEFAULT_MAX_LEVEL);
    if (!l_BracketEntry)
        return;

    BattlegroundQueue& l_BGQueue = sBattlegroundMgr->m_BattlegroundQueues[l_BGQueueTypeID];

    /// Create the new battleground.
    Battleground* l_BattlegroundInstance = sBattlegroundMgr->CreateNewBattleground(p_BgTypeId, l_BracketEntry, l_ArenaType, false, nullptr, true);
    if (l_BattlegroundInstance == nullptr)
        return;

    auto l_PrepareGroupToWargame = [this, &l_BGQueue, l_BGQueueTypeID, l_ArenaType, l_BattlegroundTemplate, l_BattlegroundInstance, l_BracketEntry](Group* p_Group, uint32 p_Team) -> void
    {
        Player* l_Leader = sObjectAccessor->FindPlayer(p_Group->GetLeaderGUID());

        auto l_GroupQueueInfo = l_BGQueue.AddGroup(l_Leader, p_Group, l_BattlegroundTemplate->GetTypeID(), l_BracketEntry, l_ArenaType, false, false, 0, 0);
        uint32 l_AvgTime      = l_BGQueue.GetAverageQueueWaitTime(l_GroupQueueInfo, l_BracketEntry->GetBracketId());

        for (GroupReference * l_It = p_Group->GetFirstMember(); l_It != NULL; l_It = l_It->next())
        {
            Player* l_Member = l_It->getSource();

            if (!l_Member)
                continue;   // this should never happen

            // add to queue
            uint32 l_QueueSlot = l_Member->AddBattlegroundQueueId(l_BGQueueTypeID);

            // add joined time data
            l_Member->AddBattlegroundQueueJoinTime(l_BGQueueTypeID, l_GroupQueueInfo->JoinTime);

            l_Member->SetWargameInvite(true);

            WorldPacket l_Data; // send status packet (in queue)
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&l_Data, l_BattlegroundInstance, l_Member, l_QueueSlot, STATUS_WAIT_QUEUE, l_AvgTime, l_GroupQueueInfo->JoinTime, l_GroupQueueInfo->ArenaType);
            l_Member->GetSession()->SendPacket(&l_Data);
        }

        l_BGQueue.InviteGroupToBG(l_GroupQueueInfo, l_BattlegroundInstance, p_Team);
    };

    /// Add groups to the battleground and remove them from waiting groups list.
    l_PrepareGroupToWargame(p_First,  HORDE);
    l_PrepareGroupToWargame(p_Second, ALLIANCE);

    l_BattlegroundInstance->StartBattleground();
}