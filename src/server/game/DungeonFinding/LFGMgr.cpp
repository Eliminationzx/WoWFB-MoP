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

#include "Common.h"
#include "SharedDefines.h"
#include "DBCStores.h"
#include "Containers.h"
#include "DisableMgr.h"
#include "ObjectMgr.h"
#include "SocialMgr.h"
#include "LFGMgr.h"
#include "GroupMgr.h"
#include "GameEventMgr.h"
#include "LFGScripts.h"
#include "LFGGroupData.h"
#include "LFGPlayerData.h"

#include "Group.h"
#include "Player.h"

LFGMgr::LFGMgr(): m_update(true), m_QueueTimer(0), m_Testing(false), m_lfgProposalId(1),
m_WaitTimeAvg(-1), m_WaitTimeTank(-1), m_WaitTimeHealer(-1), m_WaitTimeDps(-1),
m_NumWaitTimeAvg(0), m_NumWaitTimeTank(0), m_NumWaitTimeHealer(0), m_NumWaitTimeDps(0)
{
    m_update = sWorld->getBoolConfig(CONFIG_DUNGEON_FINDER_ENABLE);
    if (m_update)
    {
        new LFGPlayerScript();
        new LFGGroupScript();

        // Initialize dungeon cache
        for (uint32 i = 0; i < sLFGDungeonStore.GetNumRows(); ++i)
        {
            LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(i);
            if (dungeon)
            {
                if (dungeon->type != TYPEID_RANDOM_DUNGEON)
                    m_CachedDungeonMap[dungeon->grouptype].insert(dungeon->ID);
                m_CachedDungeonMap[0].insert(dungeon->ID);
            }
        }
    }
}

LFGMgr::~LFGMgr()
{
    for (LfgRewardMap::iterator itr = m_RewardMap.begin(); itr != m_RewardMap.end(); ++itr)
        delete itr->second;

    for (LfgQueueInfoMap::iterator it = m_QueueInfoMap.begin(); it != m_QueueInfoMap.end(); ++it)
        delete it->second;

    for (LfgProposalMap::iterator it = m_Proposals.begin(); it != m_Proposals.end(); ++it)
        delete it->second;

    for (LfgPlayerBootMap::iterator it = m_Boots.begin(); it != m_Boots.end(); ++it)
        delete it->second;

    for (LfgRoleCheckMap::iterator it = m_RoleChecks.begin(); it != m_RoleChecks.end(); ++it)
        delete it->second;
}

void LFGMgr::_LoadFromDB(Field* fields, uint64 guid)
{
    if (!fields)
        return;

    if (!IS_GROUP(guid))
        return;

    uint32 dungeon = fields[16].GetUInt32();

    uint8 state = fields[17].GetUInt8();

    if (!dungeon || !state)
        return;

    SetDungeon(guid, dungeon);

    switch (state)
    {
        case LFG_STATE_DUNGEON:
        case LFG_STATE_FINISHED_DUNGEON:
            SetState(guid, (LfgState)state);
            break;
        default:
            break;
    }
}

void LFGMgr::_SaveToDB(uint64 guid, uint32 db_guid)
{
    if (!IS_GROUP(guid))
        return;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_LFG_DATA);

    stmt->setUInt32(0, db_guid);

    CharacterDatabase.Execute(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_LFG_DATA);
    stmt->setUInt32(0, db_guid);

    stmt->setUInt32(1, GetDungeon(guid));
    stmt->setUInt32(2, GetState(guid));

    CharacterDatabase.Execute(stmt);
}

/// Load rewards for completing dungeons
void LFGMgr::LoadRewards()
{
    uint32 oldMSTime = getMSTime();

    for (LfgRewardMap::iterator itr = m_RewardMap.begin(); itr != m_RewardMap.end(); ++itr)
        delete itr->second;
    m_RewardMap.clear();

    // ORDER BY is very important for GetRandomDungeonReward!
    QueryResult result = WorldDatabase.Query("SELECT dungeonId, maxLevel, firstQuestId, firstMoneyVar, firstXPVar, otherQuestId, otherMoneyVar, otherXPVar FROM lfg_dungeon_rewards ORDER BY dungeonId, maxLevel ASC");

    if (!result)
    {
        sLog->outError(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 lfg dungeon rewards. DB table `lfg_dungeon_rewards` is empty!");
        return;
    }

    uint32 count = 0;

    Field* fields = NULL;
    do
    {
        fields = result->Fetch();
        uint32 dungeonId = fields[0].GetUInt32();
        uint32 maxLevel = fields[1].GetUInt8();
        uint32 firstQuestId = fields[2].GetUInt32();
        uint32 firstMoneyVar = fields[3].GetUInt32();
        uint32 firstXPVar = fields[4].GetUInt32();
        uint32 otherQuestId = fields[5].GetUInt32();
        uint32 otherMoneyVar = fields[6].GetUInt32();
        uint32 otherXPVar = fields[7].GetUInt32();

        if (!sLFGDungeonStore.LookupEntry(dungeonId))
        {
            sLog->outError(LOG_FILTER_SQL, "Dungeon %u specified in table `lfg_dungeon_rewards` does not exist!", dungeonId);
            continue;
        }

        if (!maxLevel || maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            sLog->outError(LOG_FILTER_SQL, "Level %u specified for dungeon %u in table `lfg_dungeon_rewards` can never be reached!", maxLevel, dungeonId);
            maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
        }

        if (firstQuestId && !sObjectMgr->GetQuestTemplate(firstQuestId))
        {
            sLog->outError(LOG_FILTER_SQL, "First quest %u specified for dungeon %u in table `lfg_dungeon_rewards` does not exist!", firstQuestId, dungeonId);
            firstQuestId = 0;
        }

        if (otherQuestId && !sObjectMgr->GetQuestTemplate(otherQuestId))
        {
            sLog->outError(LOG_FILTER_SQL, "Other quest %u specified for dungeon %u in table `lfg_dungeon_rewards` does not exist!", otherQuestId, dungeonId);
            otherQuestId = 0;
        }

        m_RewardMap.insert(LfgRewardMap::value_type(dungeonId, new LfgReward(maxLevel, firstQuestId, firstMoneyVar, firstXPVar, otherQuestId, otherMoneyVar, otherXPVar)));
        ++count;
    }
    while (result->NextRow());

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u lfg dungeon rewards in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}
void LFGMgr::LoadEntrancePositions()
{
    uint32 oldMSTime = getMSTime();
    m_entrancePositions.clear();

    QueryResult result = WorldDatabase.Query("SELECT dungeonId, position_x, position_y, position_z, orientation FROM lfg_entrances");

    if (!result)
    {
        sLog->outError(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 lfg entrance positions. DB table `lfg_entrances` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 dungeonId = fields[0].GetUInt32();
        Position pos;
        pos.m_positionX = fields[1].GetFloat();
        pos.m_positionY = fields[2].GetFloat();
        pos.m_positionZ = fields[3].GetFloat();
        pos.m_orientation = fields[4].GetFloat();
        m_entrancePositions[dungeonId] = pos;
        ++count;
    }
    while (result->NextRow());

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u lfg entrance positions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void LFGMgr::Update(uint32 diff)
{
    if (!m_update)
        return;

    m_update = false;
    time_t currTime = time(NULL);

    // Remove obsolete role checks
    for (LfgRoleCheckMap::iterator it = m_RoleChecks.begin(); it != m_RoleChecks.end();)
    {
        LfgRoleCheckMap::iterator itRoleCheck = it++;
        LfgRoleCheck* roleCheck = itRoleCheck->second;
        if (currTime < roleCheck->cancelTime)
            continue;
        roleCheck->state = LFG_ROLECHECK_MISSING_ROLE;

        for (LfgRolesMap::const_iterator itRoles = roleCheck->roles.begin(); itRoles != roleCheck->roles.end(); ++itRoles)
        {
            uint64 guid = itRoles->first;
            ClearState(guid);
            if (Player* player = ObjectAccessor::FindPlayer(guid))
            {
                player->GetSession()->SendLfgRoleCheckUpdate(roleCheck);

                if (itRoles->first == roleCheck->leader)
                    player->GetSession()->SendLfgJoinResult(roleCheck->leader, LfgJoinResultData(LFG_JOIN_FAILED, LFG_ROLECHECK_MISSING_ROLE));
            }
        }
        delete roleCheck;
        m_RoleChecks.erase(itRoleCheck);
    }

    // Remove obsolete proposals
    for (LfgProposalMap::iterator it = m_Proposals.begin(); it != m_Proposals.end();)
    {
        LfgProposalMap::iterator itRemove = it++;
        if (itRemove->second->cancelTime < currTime)
            RemoveProposal(itRemove, LFG_UPDATETYPE_PROPOSAL_FAILED);
    }

    // Remove obsolete kicks
    for (LfgPlayerBootMap::iterator it = m_Boots.begin(); it != m_Boots.end();)
    {
        LfgPlayerBootMap::iterator itBoot = it++;
        LfgPlayerBoot* pBoot = itBoot->second;
        if (pBoot->cancelTime < currTime)
        {
            pBoot->inProgress = false;
            for (LfgAnswerMap::const_iterator itVotes = pBoot->votes.begin(); itVotes != pBoot->votes.end(); ++itVotes)
                if (Player* plrg = ObjectAccessor::FindPlayer(itVotes->first))
                    if (plrg->GetGUID() != pBoot->victim)
                        plrg->GetSession()->SendLfgBootPlayer(pBoot);
            delete pBoot;
            m_Boots.erase(itBoot);
        }
    }

    uint32 l_DiffTime = getMSTime();
    bool l_Exit = false;

    // Check if a proposal can be formed with the new groups being added
    for (LfgGuidListMap::iterator it = m_newToQueue.begin(); it != m_newToQueue.end(); ++it)
    {
        uint8 queueId = it->first;
        LfgGuidList& newToQueue = it->second;
        LfgGuidList& currentQueue = m_currentQueue[queueId];
        LfgGuidList firstNew;
        while (!newToQueue.empty())
        {
            /// Currently, LFG update can take more than 10 secs
            if ((getMSTime() - l_DiffTime) > 100)
            {
                l_Exit = true;
                break;
            }

            uint64 frontguid = newToQueue.front();
            sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::Update: QueueId %u: checking [" UI64FMTD "] newToQueue(%u), currentQueue(%u)", queueId, frontguid, uint32(newToQueue.size()), uint32(currentQueue.size()));
            firstNew.push_back(frontguid);
            newToQueue.pop_front();
            uint8 alreadyInQueue = 0;
            LfgGuidList temporalList = currentQueue;
            if (LfgProposal* pProposal = FindNewGroups(firstNew, temporalList, LFG_CATEGORIE_DUNGEON)) // Group found!
            {
                // Remove groups in the proposal from new and current queues (not from queue map)
                for (LfgGuidList::const_iterator itQueue = pProposal->queues.begin(); itQueue != pProposal->queues.end(); ++itQueue)
                {
                    currentQueue.remove(*itQueue);
                    newToQueue.remove(*itQueue);
                }
                m_Proposals[++m_lfgProposalId] = pProposal;

                uint64 guid = 0;
                for (LfgProposalPlayerMap::const_iterator itPlayers = pProposal->players.begin(); itPlayers != pProposal->players.end(); ++itPlayers)
                {
                    guid = itPlayers->first;
                    SetState(guid, LFG_STATE_PROPOSAL);
                    if (Player* player = ObjectAccessor::FindPlayer(itPlayers->first))
                    {
                        Group* grp = player->GetGroup();
                        if (grp)
                         {
                            uint64 gguid = grp->GetGUID();
                            SetState(gguid, LFG_STATE_PROPOSAL);
                            sLFGMgr->SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        }
                        else
                            sLFGMgr->SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        player->GetSession()->SendLfgUpdateProposal(m_lfgProposalId, pProposal);
                    }
                }

                if (pProposal->state == LFG_PROPOSAL_SUCCESS)
                    UpdateProposal(m_lfgProposalId, guid, true);
            }
            else
            {
                if (std::find(currentQueue.begin(), currentQueue.end(), frontguid) == currentQueue.end()) //already in queue?
                    ++alreadyInQueue; //currentQueue.push_back(frontguid);         // Lfg group not found, add this group to the queue.
                temporalList = currentQueue;
                m_CompatibleMap.clear();
            }

            if (LfgProposal* pProposal = FindNewGroups(firstNew, temporalList, LFG_CATEGORIE_RAID)) // Group found!
            {
                // Remove groups in the proposal from new and current queues (not from queue map)
                for (LfgGuidList::const_iterator itQueue = pProposal->queues.begin(); itQueue != pProposal->queues.end(); ++itQueue)
                {
                    currentQueue.remove(*itQueue);
                    newToQueue.remove(*itQueue);
                }
                m_Proposals[++m_lfgProposalId] = pProposal;

                uint64 guid = 0;
                for (LfgProposalPlayerMap::const_iterator itPlayers = pProposal->players.begin(); itPlayers != pProposal->players.end(); ++itPlayers)
                {
                    guid = itPlayers->first;
                    SetState(guid, LFG_STATE_PROPOSAL);
                    if (Player* player = ObjectAccessor::FindPlayer(itPlayers->first))
                    {
                        Group* grp = player->GetGroup();
                        if (grp)
                         {
                            uint64 gguid = grp->GetGUID();
                            SetState(gguid, LFG_STATE_PROPOSAL);
                            sLFGMgr->SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        }
                        else
                            sLFGMgr->SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        player->GetSession()->SendLfgUpdateProposal(m_lfgProposalId, pProposal);
                    }
                }

                if (pProposal->state == LFG_PROPOSAL_SUCCESS)
                    UpdateProposal(m_lfgProposalId, guid, true);
            }
            else
            {
                if (std::find(currentQueue.begin(), currentQueue.end(), frontguid) == currentQueue.end()) //already in queue?
                    ++alreadyInQueue; //currentQueue.push_back(frontguid);         // Lfg group not found, add this group to the queue.
                temporalList = currentQueue;
                m_CompatibleMap.clear();
            }

            if (LfgProposal* pProposal = FindNewGroups(firstNew, temporalList, LFG_CATEGORIE_SCENARIO)) // Group found!
            {
                // Remove groups in the proposal from new and current queues (not from queue map)
                for (LfgGuidList::const_iterator itQueue = pProposal->queues.begin(); itQueue != pProposal->queues.end(); ++itQueue)
                {
                    currentQueue.remove(*itQueue);
                    newToQueue.remove(*itQueue);
                }
                m_Proposals[++m_lfgProposalId] = pProposal;

                uint64 guid = 0;
                for (LfgProposalPlayerMap::const_iterator itPlayers = pProposal->players.begin(); itPlayers != pProposal->players.end(); ++itPlayers)
                {
                    guid = itPlayers->first;
                    SetState(guid, LFG_STATE_PROPOSAL);
                    if (Player* player = ObjectAccessor::FindPlayer(itPlayers->first))
                    {
                        Group* grp = player->GetGroup();
                        if (grp)
                         {
                            uint64 gguid = grp->GetGUID();
                            SetState(gguid, LFG_STATE_PROPOSAL);
                            sLFGMgr->SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        }
                        else
                            sLFGMgr->SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        player->GetSession()->SendLfgUpdateProposal(m_lfgProposalId, pProposal);
                    }
                }

                if (pProposal->state == LFG_PROPOSAL_SUCCESS)
                    UpdateProposal(m_lfgProposalId, guid, true);
            }
            else
            {
                if (std::find(currentQueue.begin(), currentQueue.end(), frontguid) == currentQueue.end()) //already in queue?
                    ++alreadyInQueue; //currentQueue.push_back(frontguid);         // Lfg group not found, add this group to the queue.
                temporalList = currentQueue;
            }

            if (LfgProposal* pProposal = CheckForSingle(firstNew)) // Group found!
            {
                // Remove groups in the proposal from new and current queues (not from queue map)
                for (LfgGuidList::const_iterator itQueue = pProposal->queues.begin(); itQueue != pProposal->queues.end(); ++itQueue)
                {
                    currentQueue.remove(*itQueue);
                    newToQueue.remove(*itQueue);
                }
                m_Proposals[++m_lfgProposalId] = pProposal;

                uint64 guid = 0;
                for (LfgProposalPlayerMap::const_iterator itPlayers = pProposal->players.begin(); itPlayers != pProposal->players.end(); ++itPlayers)
                {
                    guid = itPlayers->first;
                    SetState(guid, LFG_STATE_PROPOSAL);
                    if (Player* player = ObjectAccessor::FindPlayer(itPlayers->first))
                    {
                        Group* grp = player->GetGroup();
                        if (grp)
                         {
                            uint64 gguid = grp->GetGUID();
                            SetState(gguid, LFG_STATE_PROPOSAL);
                            sLFGMgr->SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        }
                        else
                            sLFGMgr->SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        player->GetSession()->SendLfgUpdateProposal(m_lfgProposalId, pProposal);
                    }
                }

                if (pProposal->state == LFG_PROPOSAL_SUCCESS)
                    UpdateProposal(m_lfgProposalId, guid, true);
            }
            else
            {
                if (std::find(currentQueue.begin(), currentQueue.end(), frontguid) == currentQueue.end()) //already in queue?
                    ++alreadyInQueue; 
                temporalList = currentQueue;
            }
            // if alreadyInQueue == 4 then new players have not found any group
            if (alreadyInQueue == 4 && std::find(currentQueue.begin(), currentQueue.end(), frontguid) == currentQueue.end())
                currentQueue.push_back(frontguid);

            firstNew.clear();
        }

        if (l_Exit)
           break;
    }

    // Update all players status queue info
    if (m_QueueTimer > LFG_QUEUEUPDATE_INTERVAL)
    {
        m_QueueTimer = 0;
        currTime = time(NULL);
        for (LfgQueueInfoMap::const_iterator itQueue = m_QueueInfoMap.begin(); itQueue != m_QueueInfoMap.end(); ++itQueue)
        {
            LfgQueueInfo* queue = itQueue->second;
            if (!queue)
            {
                sLog->outError(LOG_FILTER_LFG, "LFGMgr::Update: [" UI64FMTD "] queued with null queue info!", itQueue->first);
                continue;
            }
            uint32 dungeonId = (*queue->dungeons.begin());
            uint32 queuedTime = uint32(currTime - queue->joinTime);
            uint8 role = ROLE_NONE;
            for (LfgRolesMap::const_iterator itPlayer = queue->roles.begin(); itPlayer != queue->roles.end(); ++itPlayer)
                role |= itPlayer->second;
            role &= ~ROLE_LEADER;

            int32 waitTime = -1;
            switch (role)
            {
                case ROLE_NONE:                             // Should not happen - just in case
                    waitTime = -1;
                    break;
                case ROLE_TANK:
                    waitTime = m_WaitTimeTank;
                    break;
                case ROLE_HEALER:
                    waitTime = m_WaitTimeHealer;
                    break;
                case ROLE_DAMAGE:
                    waitTime = m_WaitTimeDps;
                    break;
                default:
                    waitTime = m_WaitTimeAvg;
                    break;
            }

            for (LfgRolesMap::const_iterator itPlayer = queue->roles.begin(); itPlayer != queue->roles.end(); ++itPlayer)
                if (Player* player = ObjectAccessor::FindPlayer(itPlayer->first))
                {
                    player->GetSession()->SendLfgQueueStatus(dungeonId, waitTime, m_WaitTimeAvg, m_WaitTimeTank, m_WaitTimeHealer, m_WaitTimeDps, queuedTime, queue->tanks, queue->healers, queue->dps);
                }
        }
    }
    else
        m_QueueTimer += diff;
    m_update = true;
}

/**
   Add a guid to the queue of guids to be added to main queue. It guid its already
   in queue does nothing. If this function is called guid is not in the main queue
   (No need to check it here)

   @param[in]     guid Player or group guid to add to queue
   @param[in]     queueId Queue Id to add player/group to
*/
void LFGMgr::AddToQueue(uint64 guid, uint8 queueId)
{
    if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP))
        queueId = 0;

    LfgGuidList& list = m_newToQueue[queueId];
    if (std::find(list.begin(), list.end(), guid) != list.end())
        sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::AddToQueue: [" UI64FMTD "] already in new queue. ignoring", guid);
    else
    {
        list.push_back(guid);
        sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::AddToQueue: [" UI64FMTD "] added to m_newToQueue (size: %u)", guid, uint32(list.size()));
    }
}

/**
   Removes a guid from the main and new queues.

   @param[in]     guid Player or group guid to add to queue
   @return true if guid was found in main queue.
*/
bool LFGMgr::RemoveFromQueue(uint64 guid)
{
    for (LfgGuidListMap::iterator it = m_currentQueue.begin(); it != m_currentQueue.end(); ++it)
        it->second.remove(guid);

    for (LfgGuidListMap::iterator it = m_newToQueue.begin(); it != m_newToQueue.end(); ++it)
        it->second.remove(guid);

    RemoveFromCompatibles(guid);

    LfgQueueInfoMap::iterator it = m_QueueInfoMap.find(guid);
    if (it != m_QueueInfoMap.end())
    {
        delete it->second;
        m_QueueInfoMap.erase(it);
        sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::RemoveFromQueue: [" UI64FMTD "] removed", guid);
        return true;
    }
    else
    {
        sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::RemoveFromQueue: [" UI64FMTD "] not in queue", guid);
        return false;
    }

}

/**
    Generate the dungeon lock map for a given player

   @param[in]     player Player we need to initialize the lock status map
*/
void LFGMgr::InitializeLockedDungeons(Player* p_Player)
{
    uint64 l_Guid = p_Player->GetGUID();
    uint8 l_Level = p_Player->getLevel();
    uint8 l_Expansion = p_Player->GetSession()->Expansion();
    LfgDungeonSet l_Dungeons = GetDungeonsByRandom(0);
    LfgLockMap l_Lock;

    for (LfgDungeonSet::const_iterator l_Iter = l_Dungeons.begin(); l_Iter != l_Dungeons.end(); ++l_Iter)
    {
        LFGDungeonEntry const* l_Dungeon = sLFGDungeonStore.LookupEntry(*l_Iter);
        if (!l_Dungeon) ///< Should never happen - We provide a list from sLFGDungeonStore
             continue;

        LfgLockStatus l_LockData;
        l_LockData.lockstatus = LFG_LOCKSTATUS_OK;

        AccessRequirement const* l_AccessReq = sObjectMgr->GetAccessRequirement(l_Dungeon->map, Difficulty(l_Dungeon->difficulty));
        LFRAccessRequirement const* l_LFRAccessReq = sObjectMgr->GetLFRAccessRequirement(l_Dungeon->ID);

        uint8 l_LevelMin = 0;
        uint8 l_LevelMax = 0;

        if (l_AccessReq)
        {
            if (l_AccessReq->levelMin && l_Level < l_AccessReq->levelMin)
                l_LevelMin = l_AccessReq->levelMin;
            if (l_AccessReq->levelMax && l_Level > l_AccessReq->levelMax)
                l_LevelMax = l_AccessReq->levelMax;
        }

        if (l_LFRAccessReq != nullptr)
        {
            if (l_LFRAccessReq->LevelMin && l_Level < l_LFRAccessReq->LevelMin)
                l_LevelMin = l_LFRAccessReq->LevelMin;
            if (l_LFRAccessReq->LevelMax && l_Level > l_LFRAccessReq->LevelMax)
                l_LevelMax = l_LFRAccessReq->LevelMax;
        }

        if (l_Dungeon->expansion > l_Expansion)
            l_LockData.lockstatus = LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
        else if (DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, l_Dungeon->map, p_Player))
            l_LockData.lockstatus = LFG_LOCKSTATUS_RAID_LOCKED;
        else if (l_Dungeon->difficulty > REGULAR_DIFFICULTY && p_Player->GetBoundInstance(l_Dungeon->map, Difficulty(l_Dungeon->difficulty)))
            l_LockData.lockstatus = LFG_LOCKSTATUS_RAID_LOCKED;
        else if (l_Dungeon->minlevel > l_Level || l_LevelMin)
            l_LockData.lockstatus = LFG_LOCKSTATUS_TOO_LOW_LEVEL;
        else if (l_Dungeon->maxlevel < l_Level || l_LevelMax)
            l_LockData.lockstatus = LFG_LOCKSTATUS_TOO_HIGH_LEVEL;
        else if (l_Dungeon->flags & LFG_FLAG_SEASONAL)
        {
            if (HolidayIds l_Holiday = sLFGMgr->GetDungeonSeason(l_Dungeon->ID))
            {
                if (l_Holiday == HOLIDAY_WOTLK_LAUNCH || !IsHolidayActive(l_Holiday))
                    l_LockData.lockstatus = LFG_LOCKSTATUS_NOT_IN_SEASON;
            }
        }
        else if (l_LockData.lockstatus == LFG_LOCKSTATUS_OK && l_AccessReq && l_LFRAccessReq == nullptr)
        {
            if (l_AccessReq->achievement && !p_Player->GetAchievementMgr().HasAchieved(l_AccessReq->achievement))
                l_LockData.lockstatus = LFG_LOCKSTATUS_ACHIEVEMENT_NOT_COMPLITED;
            else if (p_Player->GetTeam() == ALLIANCE && l_AccessReq->quest_A && !p_Player->GetQuestRewardStatus(l_AccessReq->quest_A))
                l_LockData.lockstatus = LFG_LOCKSTATUS_QUEST_NOT_COMPLETED;
            else if (p_Player->GetTeam() == HORDE && l_AccessReq->quest_H && !p_Player->GetQuestRewardStatus(l_AccessReq->quest_H))
                l_LockData.lockstatus = LFG_LOCKSTATUS_QUEST_NOT_COMPLETED;
            else if (l_AccessReq->item)
            {
                if (!p_Player->HasItemCount(l_AccessReq->item) && (!l_AccessReq->item2 || !p_Player->HasItemCount(l_AccessReq->item2)))
                    l_LockData.lockstatus = LFG_LOCKSTATUS_MISSING_ITEM;
            }
            else if (l_AccessReq->item2 && !p_Player->HasItemCount(l_AccessReq->item2))
                l_LockData.lockstatus = LFG_LOCKSTATUS_MISSING_ITEM;
            else
            {
                uint32 l_AvgItemLevel = (uint32)p_Player->GetAverageItemLevel();
                if (l_AccessReq->itemlevelMin && l_AccessReq->itemlevelMin > l_AvgItemLevel)
                {
                    l_LockData.itemLevel = l_AccessReq->itemlevelMin;
                    l_LockData.lockstatus = LFG_LOCKSTATUS_TOO_LOW_GEAR_SCORE;
                }
                if (l_AccessReq->itemlevelMax && l_AccessReq->itemlevelMax < l_AvgItemLevel)
                {
                    l_LockData.itemLevel = l_AccessReq->itemlevelMax;
                    l_LockData.lockstatus = LFG_LOCKSTATUS_TOO_HIGH_GEAR_SCORE;
                }
            }
        }
        else if (l_LockData.lockstatus == LFG_LOCKSTATUS_OK && l_LFRAccessReq)
        {
            if (l_LFRAccessReq->Achievement && !p_Player->GetAchievementMgr().HasAchieved(l_LFRAccessReq->Achievement))
               l_LockData.lockstatus = LFG_LOCKSTATUS_ACHIEVEMENT_NOT_COMPLITED;
            else if (p_Player->GetTeam() == ALLIANCE && l_LFRAccessReq->QuestA && !p_Player->GetQuestRewardStatus(l_LFRAccessReq->QuestA))
                l_LockData.lockstatus = LFG_LOCKSTATUS_QUEST_NOT_COMPLETED;
            else if (p_Player->GetTeam() == HORDE && l_LFRAccessReq->QuestH && !p_Player->GetQuestRewardStatus(l_LFRAccessReq->QuestH))
                l_LockData.lockstatus = LFG_LOCKSTATUS_QUEST_NOT_COMPLETED;
            else if (l_LFRAccessReq->Item)
            {
                if (!p_Player->HasItemCount(l_LFRAccessReq->Item) && (!l_LFRAccessReq->Item2 || !p_Player->HasItemCount(l_LFRAccessReq->Item2)))
                    l_LockData.lockstatus = LFG_LOCKSTATUS_MISSING_ITEM;
            }
            else if (l_LFRAccessReq->Item2 && !p_Player->HasItemCount(l_LFRAccessReq->Item2))
                l_LockData.lockstatus = LFG_LOCKSTATUS_MISSING_ITEM;
            else
            {
                uint32 l_AvgItemLevel = p_Player->GetAverageItemLevel();
                if (l_LFRAccessReq->ItemLevelMin && l_LFRAccessReq->ItemLevelMin > l_AvgItemLevel)
                    l_LockData.lockstatus = LFG_LOCKSTATUS_TOO_LOW_GEAR_SCORE;
                if (l_LFRAccessReq->ItemLevelMax && l_LFRAccessReq->ItemLevelMax < l_AvgItemLevel)
                    l_LockData.lockstatus = LFG_LOCKSTATUS_TOO_HIGH_GEAR_SCORE;
            }
        }

        // Some scenarios are alliance or horde only
        // Cannot skip scenario lock data
        // Because client always shows them
        if (l_Dungeon->faction != -1)
        {
            if (l_Dungeon->faction == 1 && p_Player->GetTeamId() != TEAM_ALLIANCE)
            {
                l_LockData.itemLevel = 999;
                l_LockData.lockstatus = LFG_LOCKSTATUS_TOO_LOW_GEAR_SCORE;
            }

            if (l_Dungeon->faction == 0 && p_Player->GetTeamId() != TEAM_HORDE)
            {
                l_LockData.itemLevel = 999;
                l_LockData.lockstatus = LFG_LOCKSTATUS_TOO_LOW_GEAR_SCORE;
            }
        }

        if (l_Dungeon->type != TYPEID_RANDOM_DUNGEON)
        {
            if (l_Dungeon->map > 0)
            {
                LfgEntrancePositionMap::const_iterator l_Itr = m_entrancePositions.find(l_Dungeon->ID);
                if (l_Itr == m_entrancePositions.end() && !sObjectMgr->GetMapEntranceTrigger(l_Dungeon->map))
                {
                    l_LockData.itemLevel = 999;
                    l_LockData.lockstatus = LFG_LOCKSTATUS_TOO_LOW_GEAR_SCORE;
                }
            }
        }

        if (l_LockData.lockstatus != LFG_LOCKSTATUS_OK)
            l_Lock[l_Dungeon->Entry()] = l_LockData;
    }

    SetLockedDungeons(l_Guid, l_Lock);
}


/**
    Adds the player/group to lfg queue. If player is in a group then it is the leader
    of the group tying to join the group. Join conditions are checked before adding
    to the new queue.

   @param[in]     player Player trying to join (or leader of group trying to join)
   @param[in]     roles Player selected roles
   @param[in]     dungeons Dungeons the player/group is applying for
   @param[in]     comment Player selected comment
*/
void LFGMgr::Join(Player* player, uint8 roles, const LfgDungeonSet& selectedDungeons, const std::string& comment)
{
    if (!player || !player->GetSession() || selectedDungeons.empty())
        return;

    Group* grp = player->GetGroup();
    uint64 guid = player->GetGUID();
    uint64 gguid = grp ? grp->GetGUID() : guid;
    LfgJoinResultData joinData;
    PlayerSet players;
    uint32 rDungeonId = 0;
    bool isContinue = grp && grp->isLFGGroup() && GetState(gguid) != LFG_STATE_FINISHED_DUNGEON;
    LfgDungeonSet dungeons = selectedDungeons;

    // Do not allow to change dungeon in the middle of a current dungeon
    if (isContinue)
    {
        dungeons.clear();
        dungeons.insert(GetDungeon(gguid));
    }

    // Already in queue?
    LfgQueueInfoMap::iterator itQueue = m_QueueInfoMap.find(gguid);
    if (itQueue != m_QueueInfoMap.end())
    {
        LfgDungeonSet playerDungeons = GetSelectedDungeons(guid);
        if (playerDungeons == dungeons)                     // Joining the same dungeons -- Send OK
         {
            LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, dungeons, comment);
            player->GetSession()->SendLfgJoinResult(player->GetGUID(), joinData); // Default value of joinData.result = LFG_JOIN_OK
            if (grp)
            {
                for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    if (itr->getSource() && itr->getSource()->GetSession())
                    {
                        SendUpdateStatus(itr->getSource(), updateData);
                    }
                }
            }
            return;
        }
        else // Remove from queue and rejoin
        {
            SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE, playerDungeons, GetComment(player->GetGUID())));
            RemoveFromQueue(gguid);
        }
    }

    const LFGDungeonEntry* entry = sLFGDungeonStore.LookupEntry(*dungeons.begin() & 0xFFFFFF);
    uint8 l_Category = LFG_CATEGORIE_NONE;
    uint8 maxGroupSize = 5;
    if (entry != NULL)
        l_Category = entry->category;
    if (l_Category == LFG_CATEGORIE_RAID)
        maxGroupSize = 25;
    if (l_Category == LFG_CATEGORIE_SCENARIO || (entry != nullptr && entry->isScenario()))
        maxGroupSize = 3;
    if (entry && entry->isScenarioSingle())
    {
        maxGroupSize = 1;
    }

    if (IsTest())
        maxGroupSize = 2;

    // Check player or group member restrictions
    if (player->InBattleground() || player->InArena() || player->InBattlegroundQueue())
        joinData.result = LFG_JOIN_USING_BG_SYSTEM;
    else if (player->HasAura(LFG_SPELL_DUNGEON_DESERTER))
        joinData.result = LFG_JOIN_DESERTER;
    else if (player->HasAura(LFG_SPELL_DUNGEON_COOLDOWN))
        joinData.result = LFG_JOIN_RANDOM_COOLDOWN;
    else if (dungeons.empty())
        joinData.result = LFG_JOIN_NOT_MEET_REQS;
    else if (grp)
    {
        if (grp->GetMembersCount() > maxGroupSize)
            joinData.result = LFG_JOIN_TOO_MUCH_MEMBERS;
        else
        {
            uint8 memberCount = 0;
            for (GroupReference* itr = grp->GetFirstMember(); itr != NULL && joinData.result == LFG_JOIN_OK; itr = itr->next())
            {
                if (Player* plrg = itr->getSource())
                {
                    if (plrg->HasAura(LFG_SPELL_DUNGEON_DESERTER))
                        joinData.result = LFG_JOIN_PARTY_DESERTER;
                    else if (plrg->HasAura(LFG_SPELL_DUNGEON_COOLDOWN))
                        joinData.result = LFG_JOIN_PARTY_RANDOM_COOLDOWN;
                    else if (plrg->InBattleground() || plrg->InArena() || plrg->InBattlegroundQueue())
                        joinData.result = LFG_JOIN_USING_BG_SYSTEM;
                    ++memberCount;
                    players.insert(plrg);
                }
            }
            if (memberCount != grp->GetMembersCount())
                joinData.result = LFG_JOIN_DISCONNECTED;
        }
    }
    else
        players.insert(player);

    // Check if all dungeons are valid
    bool isRaid = false;
    if (joinData.result == LFG_JOIN_OK)
    {
        bool isDungeon = false;
        for (LfgDungeonSet::const_iterator it = dungeons.begin(); it != dungeons.end() && joinData.result == LFG_JOIN_OK; ++it)
        {
            LfgType l_LfgType           = GetDungeonType(*it & 0x00FFFFFF);
            LfgCategory l_LfgCategorie  = GetLfgCategorie(*it & 0x00FFFFFF);

            if (l_LfgType == TYPEID_RANDOM_DUNGEON)
            {
                if (dungeons.size() > 1)               // Only allow 1 random dungeon
                    joinData.result = LFG_JOIN_DUNGEON_INVALID;
                else
                    rDungeonId = (*dungeons.begin());
            }

            switch (l_LfgCategorie)
            {
                case LFG_CATEGORIE_DUNGEON:
                    if (isRaid)
                        joinData.result = LFG_JOIN_MIXED_RAID_DUNGEON;
                    isDungeon = true;
                    break;
                case LFG_CATEGORIE_RAID:
                    if (isDungeon)
                        joinData.result = LFG_JOIN_MIXED_RAID_DUNGEON;
                    isRaid = true;
                    break;
                case LFG_CATEGORIE_SCENARIO:
                    isDungeon = true;
                    break;
                default:
                    joinData.result = LFG_JOIN_DUNGEON_INVALID;
                    break;
            }
        }

        // it could be changed
        if (joinData.result == LFG_JOIN_OK)
        {
            // Expand random dungeons and check restrictions
            if (rDungeonId)
                dungeons = GetDungeonsByRandom(rDungeonId, true);

            // if we have lockmap then there are no compatible dungeons
            GetCompatibleDungeons(dungeons, players, joinData.lockmap);
            if (dungeons.empty())
                joinData.result = grp ? LFG_JOIN_PARTY_NOT_MEET_REQS : LFG_JOIN_NOT_MEET_REQS;
        }
    }

    // Can't join. Send result
    if (joinData.result != LFG_JOIN_OK)
    {
        sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::Join: [" UI64FMTD "] joining with %u members. result: %u", guid, grp ? grp->GetMembersCount() : 1, joinData.result);
        if (!dungeons.empty())                             // Only should show lockmap when have no dungeons available
            joinData.lockmap.clear();
        player->GetSession()->SendLfgJoinResult(player->GetGUID(), joinData);
        return;
    }

    SetComment(guid, comment);

    if (grp)                                               // Begin rolecheck
    {
        // Create new rolecheck
        LfgRoleCheck* roleCheck = new LfgRoleCheck();
        roleCheck->cancelTime = time_t(time(NULL)) + LFG_TIME_ROLECHECK;
        roleCheck->state = LFG_ROLECHECK_INITIALITING;
        roleCheck->leader = guid;
        roleCheck->dungeons = dungeons;
        roleCheck->rDungeonId = rDungeonId;
        m_RoleChecks[gguid] = roleCheck;

        if (rDungeonId)
        {
            dungeons.clear();
            dungeons.insert(rDungeonId);
        }

        SetState(gguid, LFG_STATE_ROLECHECK);
        // Send update to player
        LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_JOIN_QUEUE, dungeons, comment);
        for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            if (Player* plrg = itr->getSource())
            {
                uint64 pguid = plrg->GetGUID();
                SetState(pguid, LFG_STATE_ROLECHECK);
                if (!isContinue)
                    SetSelectedDungeons(pguid, dungeons);
                roleCheck->roles[pguid] = 0;
            }
        }
        // Update leader role
        UpdateRoleCheck(gguid, guid, roles);
    }
    else                                                   // Add player to queue
    {
        // Queue player
        LfgQueueInfo* pqInfo = new LfgQueueInfo();
        pqInfo->joinTime = time_t(time(NULL));
        pqInfo->roles[player->GetGUID()] = roles;
        pqInfo->dungeons = dungeons;
        const LFGDungeonEntry* entry = sLFGDungeonStore.LookupEntry(*dungeons.begin() & 0xFFFFFF);
        if (entry != NULL)
        {
            pqInfo->type = entry->type;
            pqInfo->dps = entry->dpsNeeded;
            pqInfo->healers = entry->healerNeeded;
            pqInfo->tanks = entry->tankNeeded;
            pqInfo->category = entry->category;
        }
        if (roles & ROLE_TANK)
            --pqInfo->tanks;
        else if (roles & ROLE_HEALER)
            --pqInfo->healers;
        else
            --pqInfo->dps;

        m_QueueInfoMap[guid] = pqInfo;

        // Send update to player
        SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, dungeons, comment));
        SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_JOIN_QUEUE, dungeons, comment));
        player->GetSession()->SendLfgJoinResult(player->GetGUID(), joinData);
        SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_JOIN_QUEUE, dungeons, comment));
        SetState(gguid, LFG_STATE_QUEUED);
        SetRoles(guid, roles);
        if (!isContinue)
        {
            if (rDungeonId)
            {
                dungeons.clear();
                dungeons.insert(rDungeonId);
            }
            SetSelectedDungeons(guid, dungeons);
        }
        AddToQueue(guid, uint8(player->GetTeam()));
    }
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::Join: [" UI64FMTD "] joined with %u members. dungeons: %u", guid, grp ? grp->GetMembersCount() : 1, uint8(dungeons.size()));
}

/**
    Leaves Dungeon System. Player/Group is removed from queue, rolechecks, proposals
    or votekicks. Player or group needs to be not NULL and using Dungeon System

   @param[in]     player Player trying to leave (can be NULL)
   @param[in]     grp Group trying to leave (default NULL)
*/
void LFGMgr::Leave(Player* player, Group* grp /* = NULL*/)
{
    if (!player && !grp)
        return;

    uint64 guid = grp ? grp->GetGUID() : player->GetGUID();
    LfgState state = GetState(guid);

    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::Leave: [" UI64FMTD "]", guid);
    switch (state)
    {
        case LFG_STATE_QUEUED:
            {
                uint8 tankAdded = 0;
                uint8 healerAdded = 0;
                uint8 dpsAdded = 0;
                LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE);

                if (grp)
                {
                    RestoreState(guid);
                    for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
                    {
                        if (Player* plrg = itr->getSource())
                        {
                            SendUpdateStatus(plrg, updateData);
                            uint64 pguid = plrg->GetGUID();
                            ClearState(pguid);
                        }
                    }
                }
                else
                {
                    SendUpdateStatus(player, updateData);
                    ClearState(guid);
                }
                RemoveFromQueue(guid);
            }
            break;
        case LFG_STATE_ROLECHECK:
            if (grp)
                UpdateRoleCheck(guid);                     // No player to update role = LFG_ROLECHECK_ABORTED
            break;
        case LFG_STATE_PROPOSAL:
        {
            // Remove from Proposals
            LfgProposalMap::iterator it = m_Proposals.begin();
            while (it != m_Proposals.end())
            {
                LfgProposalPlayerMap::iterator itPlayer = it->second->players.find(player ? player->GetGUID() : grp->GetLeaderGUID());
                if (itPlayer != it->second->players.end())
                {
                    // Mark the player/leader of group who left as didn't accept the proposal
                    itPlayer->second->accept = LFG_ANSWER_DENY;
                    break;
                }
                ++it;
            }

            // Remove from queue - if proposal is found, RemoveProposal will call RemoveFromQueue
            if (it != m_Proposals.end())
                RemoveProposal(it, LFG_UPDATETYPE_PROPOSAL_DECLINED);
            break;
        }
        default:
            break;
    }
}

/**
   Sends the leader of a group the offer to continue popup

   @param[in]     grp Group to send offer to
*/
void LFGMgr::OfferContinue(Group* grp)
{
    if (grp)
    {
        uint64 gguid = grp->GetGUID();
        if (Player* leader = ObjectAccessor::FindPlayer(grp->GetLeaderGUID()))
            leader->GetSession()->SendLfgOfferContinue(GetDungeon(gguid, false));
    }
}

/**
   Checks que main queue to try to form a Lfg group. Returns first match found (if any)

   @param[in]     check List of guids trying to match with other groups
   @param[in]     all List of all other guids in main queue to match against
   @return Pointer to proposal, if match is found
*/
LfgProposal* LFGMgr::FindNewGroups(LfgGuidList& check, LfgGuidList& all, LfgCategory p_Category)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::FindNewGroup: (%s) - all(%s)", ConcatenateGuids(check).c_str(), ConcatenateGuids(all).c_str());

    uint8 maxGroupSize = 5;
    if (p_Category == LFG_CATEGORIE_RAID)
        maxGroupSize = 25;
    if (p_Category == LFG_CATEGORIE_SCENARIO)
        maxGroupSize = 3;

    if (IsTest())
        maxGroupSize = 2;

    LfgProposal* pProposal = NULL;
    if (check.empty() || check.size() > maxGroupSize || !CheckCompatibility(check, pProposal, p_Category))
        return NULL;

    // Try to match with queued groups
    while (!pProposal && !all.empty())
    {
        check.push_back(all.front());
        all.pop_front();
        pProposal = FindNewGroups(check, all, p_Category);
        check.pop_back();
    }
    return pProposal;
}

/**
   Check compatibilities between groups

   @param[in]     check List of guids to check compatibilities
   @param[out]    pProposal Proposal found if groups are compatibles and Match
   @return true if group are compatibles
*/
bool LFGMgr::CheckCompatibility(LfgGuidList check, LfgProposal*& pProposal, LfgCategory p_Category)
{
    if (pProposal)                                         // Do not check anything if we already have a proposal
        return false;

    uint8 maxGroupSize = 5;
    if (p_Category == LFG_CATEGORIE_RAID)
        maxGroupSize = 25;
    if (p_Category == LFG_CATEGORIE_SCENARIO)
        maxGroupSize = 3;

    if (IsTest())
        maxGroupSize = 2;

    std::string strGuids = ConcatenateGuids(check);

    if (check.size() > maxGroupSize || check.empty())
    {
        sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::CheckCompatibility: (%s): Size wrong - Not compatibles", strGuids.c_str());
        return false;
    }

    if (check.size() == 1 && IS_PLAYER_GUID(check.front())) // Player joining dungeon... compatible
        return true;

    // Previously cached?
    LfgAnswer answer = GetCompatibles(strGuids);
    if (answer != LFG_ANSWER_PENDING)
    {
        sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::CheckCompatibility: (%s) compatibles (cached): %d", strGuids.c_str(), answer);
        return bool(answer);
    }

    // Check all but new compatiblitity
    if (check.size() > 2)
    {
        uint64 frontGuid = check.front();
        check.pop_front();

        // Check all-but-new compatibilities (New, A, B, C, D) --> check(A, B, C, D)
        if (!CheckCompatibility(check, pProposal, p_Category))          // Group not compatible
        {
            sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::CheckCompatibility: (%s) not compatibles (%s not compatibles)", strGuids.c_str(), ConcatenateGuids(check).c_str());
            SetCompatibles(strGuids, false);
            return false;
        }
        check.push_front(frontGuid);
        // all-but-new compatibles, now check with new
    }

    uint8 numPlayers = 0;
    uint8 numLfgGroups = 0;
    uint32 groupLowGuid = 0;
    LfgQueueInfoMap pqInfoMap;
    for (LfgGuidList::const_iterator it = check.begin(); it != check.end() && numLfgGroups < 2 && numPlayers <= maxGroupSize; ++it)
    {
        uint64 guid = (*it);
        LfgQueueInfoMap::iterator itQueue = m_QueueInfoMap.find(guid);
        if (itQueue == m_QueueInfoMap.end() || GetState(guid) != LFG_STATE_QUEUED)
        {
            if (Player* plr = ObjectAccessor::FindPlayer(guid))
                if (itQueue != m_QueueInfoMap.end())
                    SendUpdateStatus(plr, LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE, itQueue->second->dungeons, GetComment(guid))); // TODO CHECK

            RemoveFromQueue(guid);
            return false;
        }

        pqInfoMap[guid] = itQueue->second;
        numPlayers += itQueue->second->roles.size();

        if (IS_GROUP(guid))
        {
            uint32 lowGuid = GUID_LOPART(guid);
            if (Group* grp = sGroupMgr->GetGroupByGUID(lowGuid))
                if (grp->isLFGGroup())
                {
                    if (!numLfgGroups)
                        groupLowGuid = lowGuid;
                    ++numLfgGroups;
                }
        }
    }

    if (check.size() == 1 && numPlayers != maxGroupSize)   // Single group with less than MAXGROUPSIZE - Compatibles
        return true;

    // Do not match - groups already in a lfgDungeon or too much players
    if (numLfgGroups > 1 || numPlayers > maxGroupSize)
    {
        SetCompatibles(strGuids, false);
        if (numLfgGroups > 1)
            sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::CheckCompatibility: (%s) More than one Lfggroup (%u)", strGuids.c_str(), numLfgGroups);
        else
            sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::CheckCompatibility: (%s) Too much players (%u)", strGuids.c_str(), numPlayers);
        return false;
    }

    // ----- Player checks -----
    LfgRolesMap rolesMap;
    uint64 leader = 0;
    for (LfgQueueInfoMap::const_iterator it = pqInfoMap.begin(); it != pqInfoMap.end(); ++it)
    {
        for (LfgRolesMap::const_iterator itRoles = it->second->roles.begin(); itRoles != it->second->roles.end(); ++itRoles)
        {
            // Assign new leader
            if (itRoles->second & ROLE_LEADER && (!leader || urand(0, 1)))
                leader = itRoles->first;

            rolesMap[itRoles->first] = itRoles->second;
        }
    }

    if (rolesMap.size() != numPlayers)                     // Player in multiples queues!
        return false;

    PlayerSet players;
    for (LfgRolesMap::const_iterator it = rolesMap.begin(); it != rolesMap.end(); ++it)
    {
        Player* player = ObjectAccessor::FindPlayer(it->first);
        if (!player)
            sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::CheckCompatibility: (%s) Warning! [" UI64FMTD "] offline! Marking as not compatibles!", strGuids.c_str(), it->first);
        else
        {
            for (PlayerSet::const_iterator itPlayer = players.begin(); itPlayer != players.end() && player; ++itPlayer)
            {
                // Do not form a group with ignoring candidates
                if ((player->GetSocial() && player->GetSocial()->HasIgnore((*itPlayer)->GetGUIDLow())) || ((*itPlayer)->GetSocial() && (*itPlayer)->GetSocial()->HasIgnore(player->GetGUIDLow())))
                {
                    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::CheckCompatibility: (%s) Players [" UI64FMTD "] and [" UI64FMTD "] ignoring", strGuids.c_str(), (*itPlayer)->GetGUID(), player->GetGUID());
                    player = NULL;
                }
            }
            if (player)
                players.insert(player);
        }
    }

    // if we dont have the same ammount of players then we have self ignoring candidates or different faction groups
    // otherwise check if roles are compatible
    if (players.size() != numPlayers || !CheckGroupRoles(rolesMap, p_Category))
    {
        if (players.size() == numPlayers)
            sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::CheckCompatibility: (%s) Roles not compatible", strGuids.c_str());
        SetCompatibles(strGuids, false);
        return false;
    }

    // ----- Selected Dungeon checks -----
    // Check if there are any compatible dungeon from the selected dungeons
    LfgDungeonSet compatibleDungeons;

    LfgQueueInfoMap::const_iterator itFirst = pqInfoMap.begin();
    for (LfgDungeonSet::const_iterator itDungeon = itFirst->second->dungeons.begin(); itDungeon != itFirst->second->dungeons.end(); ++itDungeon)
    {
        LfgQueueInfoMap::const_iterator itOther = itFirst;
        if (itOther != pqInfoMap.end() && itOther->second->category != p_Category )
            continue;
        ++itOther;
        while (itOther != pqInfoMap.end() && itOther->second->dungeons.find(*itDungeon) != itOther->second->dungeons.end())
            ++itOther;

        if (itOther == pqInfoMap.end())
            compatibleDungeons.insert(*itDungeon);
    }
    LfgLockPartyMap lockMap;
    GetCompatibleDungeons(compatibleDungeons, players, lockMap);

    if (compatibleDungeons.empty())
    {
        SetCompatibles(strGuids, false);
        return false;
    }
    SetCompatibles(strGuids, true);

    // ----- Group is compatible, if we have MAXGROUPSIZE members then match is found
    if (numPlayers != maxGroupSize)
    {
        sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::CheckCompatibility: (%s) Compatibles but not match. Players(%u)", strGuids.c_str(), numPlayers);
        uint8 Tanks_Needed = LFG_TANKS_NEEDED;
        uint8 Healers_Needed = LFG_HEALERS_NEEDED;
        uint8 Dps_Needed = LFG_DPS_NEEDED;

        switch (p_Category)
        {
        case LFG_CATEGORIE_DUNGEON:
            Dps_Needed = 3;
            Healers_Needed = 1;
            Tanks_Needed = 1;
            break;
        case LFG_CATEGORIE_RAID:
            Dps_Needed = 17;
            Healers_Needed = 6;
            Tanks_Needed = 2;
            break;
        case LFG_CATEGORIE_SCENARIO:
            Dps_Needed = 3;
            Healers_Needed = 0;
            Tanks_Needed = 0;
            break;
        default:
            Dps_Needed = 3;
            Healers_Needed = 1;
            Tanks_Needed = 1;
            break;
        }

        if (IsTest())
        {
            Dps_Needed     = 2;
            Healers_Needed = 0;
            Tanks_Needed   = 0;
        }

        for (LfgQueueInfoMap::const_iterator itQueue = pqInfoMap.begin(); itQueue != pqInfoMap.end(); ++itQueue)
        {
            LfgQueueInfo* queue = itQueue->second;
            for (LfgRolesMap::const_iterator itPlayer = queue->roles.begin(); itPlayer != queue->roles.end(); ++itPlayer)
            {
                uint8 roles = itPlayer->second;
                if ((roles & ROLE_TANK) && Tanks_Needed > 0)
                    --Tanks_Needed;
                else if ((roles & ROLE_HEALER) && Healers_Needed > 0)
                    --Healers_Needed;
                else if ((roles & ROLE_DAMAGE) && Dps_Needed > 0)
                    --Dps_Needed;
            }
        }
        for (PlayerSet::const_iterator itPlayers = players.begin(); itPlayers != players.end(); ++itPlayers)
        {
            for (LfgQueueInfoMap::const_iterator itQueue = pqInfoMap.begin(); itQueue != pqInfoMap.end(); ++itQueue)
            {
                LfgQueueInfo* queue = itQueue->second;
                if (!queue)
                    continue;

                for (LfgRolesMap::const_iterator itPlayer = queue->roles.begin(); itPlayer != queue->roles.end(); ++itPlayer)
                {
                    if (*itPlayers == ObjectAccessor::FindPlayer(itPlayer->first))
                    {
                        queue->tanks = Tanks_Needed;
                        queue->healers = Healers_Needed;
                        queue->dps = Dps_Needed;
                    }
                }
            }
        }
        return true;
    }
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::CheckCompatibility: (%s) MATCH! Group formed", strGuids.c_str());

    // GROUP FORMED!
    // TODO - Improve algorithm to select proper group based on Item Level
    // Do not match bad tank and bad healer on same group

    // Select a random dungeon from the compatible list
    // TODO - Select the dungeon based on group item Level, not just random
    // Create a new proposal
    pProposal = new LfgProposal(JadeCore::Containers::SelectRandomContainerElement(compatibleDungeons));
    pProposal->cancelTime = time_t(time(NULL)) + LFG_TIME_PROPOSAL;
    pProposal->state = LFG_PROPOSAL_INITIATING;
    pProposal->queues = check;
    pProposal->groupLowGuid = groupLowGuid;

    // Assign new roles to players and assign new leader
    PlayerSet::const_iterator itPlayers = players.begin();
    if (!leader)
    {
        uint8 pos = uint8(urand(0, players.size() - 1));
        for (uint8 i = 0; i < pos; ++i)
            ++itPlayers;
        leader = (*itPlayers)->GetGUID();
    }
    pProposal->leader = leader;

    uint8 numAccept = 0;
    for (itPlayers = players.begin(); itPlayers != players.end(); ++itPlayers)
    {
        uint64 guid = (*itPlayers)->GetGUID();
        LfgProposalPlayer* ppPlayer = new LfgProposalPlayer();
        if (Group* grp = (*itPlayers)->GetGroup())
        {
            ppPlayer->groupLowGuid = grp->GetLowGUID();
            if (grp->isLFGGroup()) // Player from existing group, autoaccept
            {
                ppPlayer->accept = LFG_ANSWER_AGREE;
                ++numAccept;
            }
        }
        ppPlayer->role = rolesMap[guid];
        pProposal->players[guid] = ppPlayer;
    }
    if (numAccept == maxGroupSize)
        pProposal->state = LFG_PROPOSAL_SUCCESS;

    return true;
}

/**
   Update the Role check info with the player selected role.

   @param[in]     grp Group guid to update rolecheck
   @param[in]     guid Player guid (0 = rolecheck failed)
   @param[in]     roles Player selected roles
*/
void LFGMgr::UpdateRoleCheck(uint64 gguid, uint64 guid /* = 0 */, uint8 roles /* = ROLE_NONE */)
{
    if (!gguid)
        return;

    LfgRolesMap check_roles;
    LfgRoleCheckMap::iterator itRoleCheck = m_RoleChecks.find(gguid);
    if (itRoleCheck == m_RoleChecks.end())
        return;

    LfgRoleCheck* roleCheck = itRoleCheck->second;
    bool sendRoleChosen = roleCheck->state != LFG_ROLECHECK_DEFAULT && guid;

    LfgDungeonSet dungeons;
    if (roleCheck->rDungeonId)
        dungeons.insert(roleCheck->rDungeonId);
    else
        dungeons = roleCheck->dungeons;

    if (!guid)
        roleCheck->state = LFG_ROLECHECK_ABORTED;
    else if (roles < ROLE_TANK)                            // Player selected no role.
        roleCheck->state = LFG_ROLECHECK_NO_ROLE;
    else
    {
        roleCheck->roles[guid] = roles;

        // Check if all players have selected a role
        LfgRolesMap::const_iterator itRoles = roleCheck->roles.begin();
        while (itRoles != roleCheck->roles.end() && itRoles->second != ROLE_NONE)
            ++itRoles;

        if (itRoles == roleCheck->roles.end())
        {
            // use temporal var to check roles, CheckGroupRoles modifies the roles
            check_roles = roleCheck->roles;
            const LFGDungeonEntry* entry = sLFGDungeonStore.LookupEntry(*roleCheck->dungeons.begin() & 0xFFFFFF);
            if (entry != NULL)
                roleCheck->state = CheckGroupRoles(check_roles, (LfgCategory)entry->category)
                    ? LFG_ROLECHECK_FINISHED : LFG_ROLECHECK_WRONG_ROLES;
        }
    }

    uint8 team = 0;

    LfgJoinResultData joinData = LfgJoinResultData(LFG_JOIN_FAILED, roleCheck->state);
    for (LfgRolesMap::const_iterator it = roleCheck->roles.begin(); it != roleCheck->roles.end(); ++it)
    {
        uint64 pguid = it->first;
        Player* plrg = ObjectAccessor::FindPlayer(pguid);
        if (!plrg)
        {
            if (roleCheck->state == LFG_ROLECHECK_FINISHED)
                SetState(pguid, LFG_STATE_QUEUED);
            else if (roleCheck->state != LFG_ROLECHECK_INITIALITING)
                ClearState(pguid);
            continue;
        }

        team = uint8(plrg->GetTeam());
        if (!sendRoleChosen)
            plrg->GetSession()->SendLfgRoleChosen(guid, roles);

        plrg->GetSession()->SendLfgRoleCheckUpdate(roleCheck);
        switch (roleCheck->state)
        {
            case LFG_ROLECHECK_INITIALITING:
                continue;
            case LFG_ROLECHECK_FINISHED:
                SetState(pguid, LFG_STATE_QUEUED);
                SetRoles(pguid, it->second);
                //SendUpdateStatus(plrg, LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE, dungeons, GetComment(pguid)));
                SendUpdateStatus(plrg, LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, dungeons, GetComment(pguid)));
                SendUpdateStatus(plrg, LfgUpdateData(LFG_UPDATETYPE_JOIN_QUEUE, dungeons, GetComment(pguid)));
                break;
            default:
                if (roleCheck->leader == pguid)
                    plrg->GetSession()->SendLfgJoinResult(plrg->GetGUID(), joinData);
                SendUpdateStatus(plrg, LfgUpdateData(LFG_UPDATETYPE_ROLECHECK_FAILED));
                ClearState(pguid);
                break;
        }
    }

    if (roleCheck->state == LFG_ROLECHECK_FINISHED)
    {
        SetState(gguid, LFG_STATE_QUEUED);
        LfgQueueInfo* pqInfo = new LfgQueueInfo();
        pqInfo->joinTime = time_t(time(NULL));
        pqInfo->roles = roleCheck->roles;
        pqInfo->dungeons = roleCheck->dungeons;

        const LFGDungeonEntry* entry = sLFGDungeonStore.LookupEntry(*roleCheck->dungeons.begin() & 0xFFFFFF);
        if (entry != NULL)
        {
            pqInfo->type = entry->type;
            pqInfo->dps = entry->dpsNeeded;
            pqInfo->healers = entry->healerNeeded;
            pqInfo->tanks = entry->tankNeeded;
            pqInfo->category = entry->category;
        }

        // Set queue roles needed - As we are using check_roles will not have more that 1 tank, 1 healer, 3 dps
        for (LfgRolesMap::const_iterator it = check_roles.begin(); it != check_roles.end(); ++it)
        {
            uint8 roles2 = it->second;
            if (roles2 & ROLE_TANK)
                --pqInfo->tanks;
            else if (roles2 & ROLE_HEALER)
                --pqInfo->healers;
            else
                --pqInfo->dps;
        }

        m_QueueInfoMap[gguid] = pqInfo;
        if (GetState(gguid) != LFG_STATE_NONE)
        {
            LfgGuidList& currentQueue = m_currentQueue[team];
            currentQueue.push_front(gguid);
        }
        for (LfgRolesMap::const_iterator it = check_roles.begin(); it != check_roles.end(); ++it)
        {
            Player* plrg = ObjectAccessor::FindPlayer(it->first);
            if (!plrg)
                continue;

            SendUpdateStatus(plrg, LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, dungeons, GetComment(plrg->GetGUID())));
            SendUpdateStatus(plrg, LfgUpdateData(LFG_UPDATETYPE_JOIN_QUEUE, dungeons, GetComment(plrg->GetGUID())));
        }
        AddToQueue(gguid, team);
    }

    if (roleCheck->state != LFG_ROLECHECK_INITIALITING)
    {
        if (roleCheck->state != LFG_ROLECHECK_FINISHED)
            RestoreState(gguid);
        delete roleCheck;
        m_RoleChecks.erase(itRoleCheck);
    }
}

/**
   Remove from cached compatible dungeons any entry that contains the given guid

   @param[in]     guid Guid to remove from compatible cache
*/
void LFGMgr::RemoveFromCompatibles(uint64 guid)
{
    std::stringstream out;
    out << guid;
    std::string strGuid = out.str();

    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::RemoveFromCompatibles: Removing [" UI64FMTD "]", guid);
    for (LfgCompatibleMap::iterator itNext = m_CompatibleMap.begin(); itNext != m_CompatibleMap.end();)
    {
        LfgCompatibleMap::iterator it = itNext++;
        if (it->first.find(strGuid) != std::string::npos)  // Found, remove it
            m_CompatibleMap.erase(it);
    }
}

/**
   Stores the compatibility of a list of guids

   @param[in]     key String concatenation of guids (| used as separator)
   @param[in]     compatibles Compatibles or not
*/
void LFGMgr::SetCompatibles(std::string const& key, bool compatibles)
{
    m_CompatibleMap[key] = LfgAnswer(compatibles);
}

/**
   Get the compatibility of a group of guids

   @param[in]     key String concatenation of guids (| used as separator)
   @return 1 (Compatibles), 0 (Not compatibles), -1 (Not set)
*/
LfgAnswer LFGMgr::GetCompatibles(std::string const& key)
{
    LfgAnswer answer = LFG_ANSWER_PENDING;
    LfgCompatibleMap::iterator it = m_CompatibleMap.find(key);
    if (it != m_CompatibleMap.end())
        answer = it->second;

    return answer;
}

/**
   Given a list of dungeons remove the dungeons players have restrictions.

   @param[in, out] dungeons Dungeons to check restrictions
   @param[in]     players Set of players to check their dungeon restrictions
   @param[out]    lockMap Map of players Lock status info of given dungeons (Empty if dungeons is not empty)
*/
void LFGMgr::GetCompatibleDungeons(LfgDungeonSet& dungeons, const PlayerSet& players, LfgLockPartyMap& lockMap)
{
    lockMap.clear();
    for (PlayerSet::const_iterator it = players.begin(); it != players.end() && !dungeons.empty(); ++it)
    {
        uint64 guid = (*it)->GetGUID();
        LfgLockMap cachedLockMap = GetLockedDungeons(guid);
        for (LfgLockMap::const_iterator it2 = cachedLockMap.begin(); it2 != cachedLockMap.end() && !dungeons.empty(); ++it2)
        {
            uint32 dungeonId = (it2->first & 0x00FFFFFF); // Compare dungeon ids
            LfgDungeonSet::iterator itDungeon = dungeons.find(dungeonId);
            if (itDungeon != dungeons.end())
            {
                dungeons.erase(itDungeon);
                lockMap[guid][dungeonId] = it2->second;
            }
        }
    }
    if (!dungeons.empty())
        lockMap.clear();
}

/**
   Check if a group can be formed with the given group roles

   @param[in]     groles Map of roles to check
   @param[in]     removeLeaderFlag Determines if we have to remove leader flag (only used first call, Default = true)
   @return True if roles are compatible
*/
bool LFGMgr::CheckGroupRoles(LfgRolesMap& groles, LfgCategory p_Category, bool removeLeaderFlag /*= true*/)
{
    if (groles.empty())
        return false;

    uint8 damage = 0;
    uint8 tank = 0;
    uint8 healer = 0;

    uint8 dpsNeeded = 0;
    uint8 healerNeeded = 0;
    uint8 tankNeeded = 0;

    switch (p_Category)
    {
    case LFG_CATEGORIE_DUNGEON:
        dpsNeeded = 3;
        healerNeeded = 1;
        tankNeeded = 1;
        break;
    case LFG_CATEGORIE_RAID:
        dpsNeeded = 17;
        healerNeeded = 6;
        tankNeeded = 2;
        break;
    case LFG_CATEGORIE_SCENARIO:
        dpsNeeded = 3;
        healerNeeded = 0;
        tankNeeded = 0;
        break;
    default:
        dpsNeeded = 3;
        healerNeeded = 1;
        tankNeeded = 1;
        break;
    }

    if (IsTest())
    {
        dpsNeeded    = 2;
        healerNeeded = 0;
        tankNeeded   = 0;
    }

    if (removeLeaderFlag)
        for (LfgRolesMap::iterator it = groles.begin(); it != groles.end(); ++it)
            it->second &= ~ROLE_LEADER;

    for (LfgRolesMap::iterator it = groles.begin(); it != groles.end(); ++it)
    {
        if (it->second == ROLE_NONE)
            return false;

        if (it->second & ROLE_TANK)
        {
            if (it->second != ROLE_TANK)
            {
                it->second -= ROLE_TANK;
                if (CheckGroupRoles(groles, p_Category, false))
                    return true;
                it->second += ROLE_TANK;
            }
            else if (tank == tankNeeded)
                return false;
            else
                tank++;
        }

        if (it->second & ROLE_HEALER)
        {
            if (it->second != ROLE_HEALER)
            {
                it->second -= ROLE_HEALER;
                if (CheckGroupRoles(groles, p_Category, false))
                    return true;
                it->second += ROLE_HEALER;
            }
            else if (healer == healerNeeded)
                return false;
            else
                healer++;
        }

        if (it->second & ROLE_DAMAGE)
        {
            if (it->second != ROLE_DAMAGE)
            {
                it->second -= ROLE_DAMAGE;
                if (CheckGroupRoles(groles, p_Category, false))
                    return true;
                it->second += ROLE_DAMAGE;
            }
            else if (damage == dpsNeeded)
                return false;
            else
                damage++;
        }
    }
    return (tank + healer + damage) == uint8(groles.size());
}

LfgCategory LFGMgr::GetLfgCategorie(uint32 dungeonId)
{
    LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(dungeonId);
    if (!dungeon)
        return LFG_CATEGORIE_NONE;

    return LfgCategory(dungeon->category);
}

/**
   Update Proposal info with player answer

   @param[in]     proposalId Proposal id to be updated
   @param[in]     guid Player guid to update answer
   @param[in]     accept Player answer
*/
void LFGMgr::UpdateProposal(uint32 proposalId, uint64 guid, bool accept)
{
    // Check if the proposal exists
    LfgProposalMap::iterator itProposal = m_Proposals.find(proposalId);
    if (itProposal == m_Proposals.end())
        return;
    LfgProposal* pProposal = itProposal->second;

    // Check if proposal have the current player
    LfgProposalPlayerMap::iterator itProposalPlayer = pProposal->players.find(guid);
    if (itProposalPlayer == pProposal->players.end())
        return;
    LfgProposalPlayer* ppPlayer = itProposalPlayer->second;

    ppPlayer->accept = LfgAnswer(accept);
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::UpdateProposal: Player [" UI64FMTD "] of proposal %u selected: %u", guid, proposalId, accept);
    if (!accept)
    {
        RemoveProposal(itProposal, LFG_UPDATETYPE_PROPOSAL_DECLINED);
        return;
    }

    LfgPlayerList players;
    LfgPlayerList playersToTeleport;

    // check if all have answered and reorder players (leader first)
    bool allAnswered = true;
    for (LfgProposalPlayerMap::const_iterator itPlayers = pProposal->players.begin(); itPlayers != pProposal->players.end(); ++itPlayers)
    {
        if (Player* player = ObjectAccessor::FindPlayer(itPlayers->first))
        {
            if (itPlayers->first == pProposal->leader)
                players.push_front(player);
            else
                players.push_back(player);

            // Only teleport new players
            //Group* grp = player->GetGroup();
            //uint64 gguid = grp ? grp->GetGUID() : 0;
            //if (!gguid || !grp->isLFGGroup() || GetState(gguid) == LFG_STATE_FINISHED_DUNGEON)
            //    playersToTeleport.push_back(player);
        }

        if (itPlayers->second->accept != LFG_ANSWER_AGREE)   // No answer (-1) or not accepted (0)
            allAnswered = false;
    }

    if (!allAnswered)
    {
        for (LfgPlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
            (*it)->GetSession()->SendLfgUpdateProposal(proposalId, pProposal);
    }
    else
    {
        bool sendUpdate = pProposal->state != LFG_PROPOSAL_SUCCESS;
        for (LfgPlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
            (*it)->GetSession()->SendLfgUpdateProposal(proposalId, pProposal);

        pProposal->state = LFG_PROPOSAL_SUCCESS;
        time_t joinTime = time_t(time(NULL));
        std::map<uint64, int32> waitTimesMap;
        // Save wait times before redoing groups
        for (LfgPlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            LfgProposalPlayer* player = pProposal->players[(*it)->GetGUID()];
            uint32 lowgroupguid = (*it)->GetGroup() ? (*it)->GetGroup()->GetLowGUID() : 0;
            if (player->groupLowGuid != lowgroupguid)
                sLog->outError(LOG_FILTER_LFG, "LFGMgr::UpdateProposal: [" UI64FMTD "] group mismatch: actual (%u) - queued (%u)", (*it)->GetGUID(), lowgroupguid, player->groupLowGuid);

            uint64 guid2 = player->groupLowGuid ? MAKE_NEW_GUID(player->groupLowGuid, 0, HighGuid::Group) : (*it)->GetGUID();
            LfgQueueInfoMap::iterator itQueue = m_QueueInfoMap.find(guid2);
            if (itQueue == m_QueueInfoMap.end())
            {
                sLog->outError(LOG_FILTER_LFG, "LFGMgr::UpdateProposal: Queue info for guid [" UI64FMTD "] not found!", guid);
                waitTimesMap[(*it)->GetGUID()] = -1;
            }
            else
                waitTimesMap[(*it)->GetGUID()] = int32(joinTime - itQueue->second->joinTime);
        }

        // Set the dungeon difficulty
        LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(pProposal->dungeonId & 0xFFFF);
        ASSERT(dungeon);

        // Create a new group (if needed)
        LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_GROUP_FOUND);
        Group* grp = pProposal->groupLowGuid ? sGroupMgr->GetGroupByGUID(pProposal->groupLowGuid) : NULL;
        
        if (dungeon->difficulty == RAID_TOOL_DIFFICULTY && grp != nullptr && !grp->isRaidGroup())
            grp->ConvertToRaid();

        for (LfgPlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = (*it);
            uint64 pguid = player->GetGUID();
            Group* group = player->GetGroup();
            if (sendUpdate)
                player->GetSession()->SendLfgUpdateProposal(proposalId, pProposal);

            if (group)
            {
                SendUpdateStatus(player, updateData);
                if (group != grp)
                    player->RemoveFromGroup();
            }
            else
                SendUpdateStatus(player, updateData);

            if (!grp)
            {
                grp = new Group();
                grp->Create(player);

                if (dungeon->difficulty == RAID_TOOL_DIFFICULTY)
                    grp->ConvertToRaid();

                grp->ConvertToLFG();
                uint64 gguid = grp->GetGUID();
                SetState(gguid, LFG_STATE_PROPOSAL);
                sGroupMgr->AddGroup(grp);
            }
            else if (group != grp)
                grp->AddMember(player);

            // Update timers
            uint8 role = GetRoles(pguid);
            role &= ~ROLE_LEADER;
            switch (role)
            {
                case ROLE_DAMAGE:
                {
                    uint32 old_number = m_NumWaitTimeDps++;
                    m_WaitTimeDps = int32((m_WaitTimeDps * old_number + waitTimesMap[player->GetGUID()]) / m_NumWaitTimeDps);
                    break;
                }
                case ROLE_HEALER:
                {
                    uint32 old_number = m_NumWaitTimeHealer++;
                    m_WaitTimeHealer = int32((m_WaitTimeHealer * old_number + waitTimesMap[player->GetGUID()]) / m_NumWaitTimeHealer);
                    break;
                }
                case ROLE_TANK:
                {
                    uint32 old_number = m_NumWaitTimeTank++;
                    m_WaitTimeTank = int32((m_WaitTimeTank * old_number + waitTimesMap[player->GetGUID()]) / m_NumWaitTimeTank);
                    break;
                }
                default:
                {
                    uint32 old_number = m_NumWaitTimeAvg++;
                    m_WaitTimeAvg = int32((m_WaitTimeAvg * old_number + waitTimesMap[player->GetGUID()]) / m_NumWaitTimeAvg);
                    break;
                }
            }

            playersToTeleport.push_back(player);
            
            m_teleport.push_back(pguid);
            grp->SetLfgRoles(pguid, pProposal->players[pguid]->role);
            SetState(pguid, LFG_STATE_DUNGEON);

            // Add the cooldown spell if queued for a random dungeon
            if (dungeon->type == TYPEID_RANDOM_DUNGEON)
                player->CastSpell(player, LFG_SPELL_DUNGEON_COOLDOWN, false);
        }

        if (dungeon->difficulty == RAID_TOOL_DIFFICULTY)
            grp->SetRaidDifficulty(Difficulty(dungeon->difficulty));
        grp->SetDungeonDifficulty(Difficulty(dungeon->difficulty));

        uint64 gguid = grp->GetGUID();
        SetDungeon(gguid, dungeon->Entry());
        SetState(gguid, LFG_STATE_DUNGEON);
        _SaveToDB(gguid, grp->GetDbStoreId());

        // Remove players/groups from Queue
        for (LfgGuidList::const_iterator it = pProposal->queues.begin(); it != pProposal->queues.end(); ++it)
            RemoveFromQueue(*it);

        uint8 maxPlayersToTeleport = 0;
        uint8 playersTeleported = 0;

        if (!grp->isRaidGroup())
            maxPlayersToTeleport = 5;
        else
        {
            switch (grp->GetRaidDifficulty())
            {
                case MAN10_DIFFICULTY:
                case MAN10_HEROIC_DIFFICULTY:
                    maxPlayersToTeleport = 10;
                    break;
                case MAN25_DIFFICULTY:
                case MAN25_HEROIC_DIFFICULTY:
                case RAID_TOOL_DIFFICULTY:
                    maxPlayersToTeleport = 25;
                    break;
                case MAN40_DIFFICULTY:
                    maxPlayersToTeleport = 40;
                    break;
            }
        }

        if (dungeon->difficulty == RAID_TOOL_DIFFICULTY)
            maxPlayersToTeleport = 25;

        // Teleport players
        for (LfgPlayerList::const_iterator it = playersToTeleport.begin(); it != playersToTeleport.end(); ++it)
        {
            if (playersTeleported >= maxPlayersToTeleport)
                break;

            TeleportPlayer(*it, false);

            playersTeleported++;
        }

        // Update group info
        grp->SendUpdate();

        delete pProposal;
        m_Proposals.erase(itProposal);
    }
}

/**
   Remove a proposal from the pool, remove the group that didn't accept (if needed) and readd the other members to the queue

   @param[in]     itProposal Iterator to the proposal to remove
   @param[in]     type Type of removal (LFG_UPDATETYPE_PROPOSAL_FAILED, LFG_UPDATETYPE_PROPOSAL_DECLINED)
*/
void LFGMgr::RemoveProposal(LfgProposalMap::iterator itProposal, LfgUpdateType type)
{
    LfgProposal* pProposal = itProposal->second;
    pProposal->state = LFG_PROPOSAL_FAILED;

    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::RemoveProposal: Proposal %u, state FAILED, UpdateType %u", itProposal->first, type);
    // Mark all people that didn't answered as no accept
    if (type == LFG_UPDATETYPE_PROPOSAL_FAILED)
        for (LfgProposalPlayerMap::const_iterator it = pProposal->players.begin(); it != pProposal->players.end(); ++it)
            if (it->second->accept == LFG_ANSWER_PENDING)
                it->second->accept = LFG_ANSWER_DENY;

    // Mark players/groups to be removed
    LfgGuidSet toRemove;
    for (LfgProposalPlayerMap::const_iterator it = pProposal->players.begin(); it != pProposal->players.end(); ++it)
    {
        if (it->second->accept == LFG_ANSWER_AGREE)
            continue;

        uint64 guid = it->second->groupLowGuid ? MAKE_NEW_GUID(it->second->groupLowGuid, 0, HighGuid::Group) : it->first;
        // Player didn't accept or still pending when no secs left
        if (it->second->accept == LFG_ANSWER_DENY || type == LFG_UPDATETYPE_PROPOSAL_FAILED)
        {
            it->second->accept = LFG_ANSWER_DENY;
            toRemove.insert(guid);
        }
    }

    uint8 team = 0;
    // Notify players
    for (LfgProposalPlayerMap::const_iterator it = pProposal->players.begin(); it != pProposal->players.end(); ++it)
    {
        Player* player = ObjectAccessor::FindPlayer(it->first);
        if (!player)
            continue;

        team = uint8(player->GetTeam());
        player->GetSession()->SendLfgUpdateProposal(itProposal->first, pProposal);

        Group* grp = player->GetGroup();
        uint64 guid = player->GetGUID();
        uint64 gguid = it->second->groupLowGuid ? MAKE_NEW_GUID(it->second->groupLowGuid, 0, HighGuid::Group) : guid;

        if (toRemove.find(gguid) != toRemove.end())         // Didn't accept or in same group that someone that didn't accept
        {
            LfgUpdateData updateData;
            if (it->second->accept == LFG_ANSWER_DENY)
            {
                updateData.updateType = type;
                sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::RemoveProposal: [" UI64FMTD "] didn't accept. Removing from queue and compatible cache", guid);
            }
            else
            {
                updateData.updateType = LFG_UPDATETYPE_REMOVED_FROM_QUEUE;
                sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::RemoveProposal: [" UI64FMTD "] in same group that someone that didn't accept. Removing from queue and compatible cache", guid);
            }
            ClearState(guid);
            if (grp)
            {
                RestoreState(gguid);
                SendUpdateStatus(player, updateData);
            }
            else
                SendUpdateStatus(player, updateData);
        }
        else
        {
            sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::RemoveProposal: Readding [" UI64FMTD "] to queue.", guid);
            SetState(guid, LFG_STATE_QUEUED);
            if (grp)
            {
                SetState(gguid, LFG_STATE_QUEUED);
                SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, GetSelectedDungeons(guid), GetComment(guid)));
            }
            else
                SendUpdateStatus(player, LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, GetSelectedDungeons(guid), GetComment(guid)));
        }
    }

    // Remove players/groups from queue
    for (LfgGuidSet::const_iterator it = toRemove.begin(); it != toRemove.end(); ++it)
    {
        uint64 guid = *it;
        RemoveFromQueue(guid);
        pProposal->queues.remove(guid);
    }

    // Readd to queue
    for (LfgGuidList::const_iterator it = pProposal->queues.begin(); it != pProposal->queues.end(); ++it)
    {
        uint64 guid = *it;
        LfgGuidList& currentQueue = m_currentQueue[team];
        currentQueue.push_front(guid);         //Add GUID for high priority
        AddToQueue(guid, team);                //We have to add each GUID in newQueue to check for a new groups
    }

    delete pProposal;
    m_Proposals.erase(itProposal);
}

/**
   Initialize a boot kick vote

   @param[in]     grp Group the vote kicks belongs to
   @param[in]     kicker Kicker guid
   @param[in]     victim Victim guid
   @param[in]     reason Kick reason
*/
void LFGMgr::InitBoot(Group* grp, uint64 kicker, uint64 victim, std::string reason)
{
    if (!grp)
        return;

    uint64 gguid = grp->GetGUID();
    SetState(gguid, LFG_STATE_BOOT);

    LfgPlayerBoot* pBoot = new LfgPlayerBoot();
    pBoot->inProgress = true;
    pBoot->cancelTime = time_t(time(NULL)) + LFG_TIME_BOOT;
    pBoot->reason = reason;
    pBoot->victim = victim;
    pBoot->votedNeeded = GetVotesNeeded(gguid);

    // Set votes
    for (GroupReference* itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        if (Player* plrg = itr->getSource())
        {
            uint64 guid = plrg->GetGUID();
            SetState(guid, LFG_STATE_BOOT);
            if (guid == victim)
                pBoot->votes[victim] = LFG_ANSWER_DENY;    // Victim auto vote NO
            else if (guid == kicker)
                pBoot->votes[kicker] = LFG_ANSWER_AGREE;   // Kicker auto vote YES
            else
            {
                pBoot->votes[guid] = LFG_ANSWER_PENDING;   // Other members need to vote
                plrg->GetSession()->SendLfgBootPlayer(pBoot);
            }
        }
    }

    m_Boots[grp->GetLowGUID()] = pBoot;
}

/**
   Update Boot info with player answer

   @param[in]     player Player who has answered
   @param[in]     accept player answer
*/
void LFGMgr::UpdateBoot(Player* player, bool accept)
{
    Group* grp = player ? player->GetGroup() : NULL;
    if (!grp)
        return;

    uint32 bootId = grp->GetLowGUID();
    uint64 guid = player->GetGUID();

    LfgPlayerBootMap::iterator itBoot = m_Boots.find(bootId);
    if (itBoot == m_Boots.end())
        return;

    LfgPlayerBoot* pBoot = itBoot->second;
    if (!pBoot)
        return;

    if (pBoot->votes[guid] != LFG_ANSWER_PENDING)          // Cheat check: Player can't vote twice
        return;

    pBoot->votes[guid] = LfgAnswer(accept);

    uint8 votesNum = 0;
    uint8 agreeNum = 0;
    for (LfgAnswerMap::const_iterator itVotes = pBoot->votes.begin(); itVotes != pBoot->votes.end(); ++itVotes)
    {
        if (itVotes->second != LFG_ANSWER_PENDING)
        {
            ++votesNum;
            if (itVotes->second == LFG_ANSWER_AGREE)
                ++agreeNum;
        }
    }

    if (votesNum == pBoot->votes.size()) // Vote didnt passed
    {
        // Send update info to all players
        pBoot->inProgress = false;
        for (LfgAnswerMap::const_iterator itVotes = pBoot->votes.begin(); itVotes != pBoot->votes.end(); ++itVotes)
        {
            uint64 pguid = itVotes->first;
            if (pguid != pBoot->victim)
            {
                SetState(pguid, LFG_STATE_DUNGEON);
                if (Player* plrg = ObjectAccessor::FindPlayer(pguid))
                    plrg->GetSession()->SendLfgBootPlayer(pBoot);
            }
        }

        uint64 gguid = grp->GetGUID();
        SetState(gguid, LFG_STATE_DUNGEON);
        if (agreeNum >= pBoot->votedNeeded )                // Vote passed - Kick player
        {
            if (Player* victim = ObjectAccessor::FindPlayer(pBoot->victim))
            {
                TeleportPlayer(victim, true, false);
                SetState(pBoot->victim, LFG_STATE_NONE);
            }
            Player::RemoveFromGroup(grp, pBoot->victim);
            OfferContinue(grp);
            DecreaseKicksLeft(gguid);
        }
        delete pBoot;
        m_Boots.erase(itBoot);
    }
}

/**
   Teleports the player in or out the dungeon

   @param[in]     player Player to teleport
   @param[in]     out Teleport out (true) or in (false)
   @param[in]     fromOpcode Function called from opcode handlers? (Default false)
*/
void LFGMgr::TeleportPlayer(Player* player, bool out, bool fromOpcode /*= false*/)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::TeleportPlayer: [" UI64FMTD "] is being teleported %s", player->GetGUID(), out ? "out" : "in");

    LfgTeleportError error = LFG_TELEPORTERROR_OK;
    Group* grp = player->GetGroup();

    if (!grp || !grp->isLFGGroup())                        // should never happen, but just in case...
        error = LFG_TELEPORTERROR_INVALID_LOCATION;
    else if (!player->isAlive())
        error = LFG_TELEPORTERROR_PLAYER_DEAD;
    else if (player->IsMirrorTimerActive(FATIGUE_TIMER))
        error = LFG_TELEPORTERROR_FATIGUE;
    else
    {
        LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(GetDungeon(grp->GetGUID()));

        if (out)
        {
            // Player needs to be inside the LFG dungeon to be able to teleport out
            if (dungeon && player->GetMapId() == uint32(dungeon->map))
            {
                player->RemoveAurasDueToSpell(LFG_SPELL_LUCK_OF_THE_DRAW);
                player->TeleportToBGEntryPoint();
            }
            else
                player->GetSession()->SendLfgTeleportError(LFG_TELEPORTERROR_DONT_REPORT); // Not sure which error message to send

            return;
        }

        if (!dungeon)
            error = LFG_TELEPORTERROR_INVALID_LOCATION;
        else if (player->GetMapId() != uint32(dungeon->map))  // Do not teleport players in dungeon to the entrance
        {
            uint32 mapid = 0;
            float x = 0;
            float y = 0;
            float z = 0;
            float orientation = 0;

            if (!fromOpcode)
            {
                // Select a player inside to be teleported to
                for (GroupReference* itr = grp->GetFirstMember(); itr != NULL && !mapid; itr = itr->next())
                {
                    Player* plrg = itr->getSource();
                    if (plrg && plrg != player && plrg->GetMapId() == uint32(dungeon->map))
                    {
                        mapid = plrg->GetMapId();
                        x = plrg->GetPositionX();
                        y = plrg->GetPositionY();
                        z = plrg->GetPositionZ();
                        orientation = plrg->GetOrientation();
                    }
                }
            }

            if (!mapid)
            {
                LfgEntrancePositionMap::const_iterator itr = m_entrancePositions.find(dungeon->ID);
                if (itr != m_entrancePositions.end())
                {
                    mapid = dungeon->map;
                    x = itr->second.GetPositionX();
                    y = itr->second.GetPositionY();
                    z = itr->second.GetPositionZ();
                    orientation = itr->second.GetOrientation();
                }
                else if (AreaTriggerStruct const* at = sObjectMgr->GetMapEntranceTrigger(dungeon->map))
                {
                    mapid = at->target_mapId;
                    x = at->target_X;
                    y = at->target_Y;
                    z = at->target_Z;
                    orientation = at->target_Orientation;
                }
                else
                {
                    sLog->outError(LOG_FILTER_LFG, "LfgMgr::TeleportPlayer: Failed to teleport [" UI64FMTD "]: No areatrigger found for map: %u difficulty: %u", player->GetGUID(), dungeon->map, dungeon->difficulty);
                    error = LFG_TELEPORTERROR_INVALID_LOCATION;
                }
            }

            if (error == LFG_TELEPORTERROR_OK)
            {
                if (!player->GetMap()->IsDungeon())
                    player->SetBattlegroundEntryPoint();

                if (player->isInFlight())
                {
                    player->GetMotionMaster()->MovementExpired();
                    player->CleanupAfterTaxiFlight();
                }

                //player->SetUnitMovementFlags(0);
                //player->ClearMovementData();
                if (player->TeleportTo(mapid, x, y, z, orientation))
                    // FIXME - HACK - this should be done by teleport, when teleporting far
                    player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                else
                {
                    error = LFG_TELEPORTERROR_INVALID_LOCATION;
                    sLog->outError(LOG_FILTER_LFG, "LfgMgr::TeleportPlayer: Failed to teleport [" UI64FMTD "] to map %u: ", player->GetGUID(), mapid);
                }
            }
        }
    }

    if (error != LFG_TELEPORTERROR_OK)
        player->GetSession()->SendLfgTeleportError(uint8(error));
}

void LFGMgr::SendUpdateStatus(Player* player, const LfgUpdateData& updateData)
{
    LfgQueueInfo* info = NULL;

    ObjectGuid guid = player->GetGUID();

    LfgQueueInfoMap::iterator itr = m_QueueInfoMap.find(guid);
    if (itr == m_QueueInfoMap.end() && player->GetGroup())
        itr = m_QueueInfoMap.find(player->GetGroup()->GetGUID());

    info = itr != m_QueueInfoMap.end() ? itr->second : NULL;
    if (!info)
        return;

    bool queued = false;
    bool unk1 = true; // looks like this bit is always 'true'
    bool unk2 = false; // true in some case, probably related to teleport_to_dungeon icon
    bool join = false;
    bool quit = true;
    
    /*
    first  - LFG_UPDATETYPE_ADDED_TO_QUEUE
    second - LFG_UPDATETYPE_JOIN_QUEUE
    */

    switch (updateData.updateType)
    {
        case LFG_UPDATETYPE_JOIN_QUEUE:
            queued = true;
            // without breakpoint
        case LFG_UPDATETYPE_ADDED_TO_QUEUE:
            join = true;
            break;
        case LFG_UPDATETYPE_UPDATE_STATUS:
            join = true; // todo check
            queued = true; // todo check
            unk2 = true;
            break;
        case LFG_UPDATETYPE_PROPOSAL_BEGIN:
            quit = false;
            break;
        case LFG_UPDATETYPE_GROUP_DISBAND_UNK16:
        case LFG_UPDATETYPE_GROUP_FOUND:
            quit = true;
            unk2 = true;
            break;
        case LFG_UPDATETYPE_GROUP_UNK15:
        case LFG_UPDATETYPE_REMOVED_FROM_QUEUE:
            quit = false;
            unk2 = true;
            break;
        case LFG_UPDATETYPE_GROUP_UNK14:
            join = true;
            break;
        default:
            break;
    }

    WorldPacket data(SMSG_LFG_UPDATE_STATUS, 48);

    data.WriteBits(updateData.dungeons.size(), 22);
    data.WriteBit(guid[7]);
    data.WriteBit(queued);     // queued
    data.WriteBit(unk1);       // unk1
    data.WriteBit(guid[5]);
    data.WriteBit(guid[3]);
    data.WriteBits(0, 24);     // party members guids
    data.WriteBit(guid[6]);
    data.WriteBit(unk2);       // unk2
    data.WriteBit(guid[2]);
    data.WriteBit(guid[1]);
    // for unkcount
    // data.writebit in order 1 3 6 7 4 5 2 0
    data.WriteBit(guid[0]);
    data.WriteBit(join);       // join - if true - show "lfg eye"
    data.WriteBit(guid[4]);
    data.WriteBits(updateData.comment.size(), 8);
    data.WriteBit(quit);       // quit

    // for unkcount
    // data.writebyteseq in order 6 5 2 0 1 7 4 3
    data << uint32(info->roles[player->GetGUID()]); 
    data << uint32(info->joinTime);
    data.WriteByteSeq(guid[1]);
    for (int i = 0; i < 3; ++i)
        data << uint8(0);

    data.WriteByteSeq(guid[7]);
    for (auto i = updateData.dungeons.begin(); i != updateData.dungeons.end(); ++i)
    {
        LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(*i);
        data << uint32(dungeon ? dungeon->Entry() : 0); // Dungeon
    }
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[5]);
    data.WriteString(updateData.comment);
    data << uint8(updateData.updateType);           // Update type
    data << uint32(0);                              // QueueId
    data << uint8(info->category);
    data << uint32(3);                              // Flags
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[0]);

    player->GetSession()->SendPacket(&data);
}

/**
   Give completion reward to player

   @param[in]     dungeonId Id of the dungeon finished
   @param[in]     player Player to reward
*/
void LFGMgr::RewardDungeonDoneFor(uint32 const p_DungeonID, Player* p_Player)
{
    Group* l_Group = p_Player->GetGroup();

    /// Player not in group or not a LFG group, ignore it
    if (!l_Group || !l_Group->isLFGGroup())
         return;

    uint64 l_Guid = p_Player->GetGUID();
    uint64 l_GroupGuid = p_Player->GetGroup()->GetGUID();
    uint32 l_GroupDungeonID = GetDungeon(l_GroupGuid);

    /// Player has finished dungeon but group queued for another one, ignore it
    if (l_GroupDungeonID != p_DungeonID)
        return;

    /// Player already awarded, ignore it
    if (GetState(l_Guid) == LfgState::LFG_STATE_FINISHED_DUNGEON)
        return;

    /// Mark dungeon as finished
    SetState(l_GroupGuid, LfgState::LFG_STATE_FINISHED_DUNGEON);

    /// Clear player related LFG stuff
    uint32 l_RandDungeonID = (*GetSelectedDungeons(l_Guid).begin());
    if (GetSelectedDungeons(l_Guid).empty())
        l_RandDungeonID = p_DungeonID;

    ClearState(l_Guid);
    SetState(l_Guid, LfgState::LFG_STATE_FINISHED_DUNGEON);
    
    /// Give rewards only for compatible dungeons
    LFGDungeonEntry const* l_Dungeon = sLFGDungeonStore.LookupEntry(l_RandDungeonID);
    if (!l_Dungeon || (l_Dungeon->type != LfgType::TYPEID_RANDOM_DUNGEON && l_Dungeon->type != LfgType::TYPEID_DUNGEON && !(l_Dungeon->flags & LfgFlags::LFG_FLAG_SEASONAL)))
         return;

    /// Update achievements
    if (l_Dungeon->difficulty == Difficulty::HEROIC_DIFFICULTY)
        p_Player->UpdateAchievementCriteria(AchievementCriteriaTypes::ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS, 1);

    /// This dungeon has no LFG reward registered, ignore it
    LfgReward const* l_Rewards = GetRandomDungeonReward(l_RandDungeonID, p_Player->getLevel());
    if (!l_Rewards)
        return;

    uint8 l_Index = 0;
    Quest const* l_QuestReward = sObjectMgr->GetQuestTemplate(l_Rewards->reward[l_Index].questId);
    if (!l_QuestReward)
         return;

    /// If we can take the quest, means that we haven't done this kind of "run", IE: First Heroic Random of Day.
    if (p_Player->CanRewardQuest(l_QuestReward, false))
        p_Player->RewardQuest(l_QuestReward, 0, NULL, false);
    else
    {
        l_Index = 1;
        l_QuestReward = sObjectMgr->GetQuestTemplate(l_Rewards->reward[l_Index].questId);
        if (!l_QuestReward)
            return;

        /// We give reward without informing client (retail does this)
        p_Player->RewardQuest(l_QuestReward, 0, NULL, false);
    }

    time_t current = time(nullptr);
    
    if ((current - p_Player->m_bonusFactionLastTime) >= DAY)
    {
        if (uint32 bonusFactionId = p_Player->GetUInt32Value(PLAYER_FIELD_LFG_BONUS_FACTION_ID))
        {
            if (FactionEntry const* _faction = sFactionStore.LookupEntry(bonusFactionId))
            {
                int32 amount = 300;
                if (auto modifier = p_Player->GetTotalAuraModifier(SPELL_AURA_MOD_REPUTATION_GAIN))
                    AddPct(amount, modifier);

                if (int32 modifier = p_Player->GetSession()->GetReputationModifier(bonusFactionId))
                    AddPct(amount, modifier);

                p_Player->GetReputationMgr().ModifyReputation(_faction, amount);
            }

            p_Player->m_bonusFactionLastTime = current;
        }
    }

    // Give rewards
    p_Player->GetSession()->SendLfgPlayerReward(l_Dungeon->Entry(), GetDungeon(l_GroupGuid, false), l_Index, l_Rewards, l_QuestReward);
}

// --------------------------------------------------------------------------//
// Auxiliar Functions
// --------------------------------------------------------------------------//

/**
   Get the dungeon list that can be done given a random dungeon entry.

   @param[in]     randomdungeon Random dungeon id (if value = 0 will return all dungeons)
   @returns Set of dungeons that can be done.
*/
const LfgDungeonSet& LFGMgr::GetDungeonsByRandom(uint32 randomdungeon, bool check)
{
    LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(randomdungeon);
    uint32 groupType = dungeon ? dungeon->grouptype : 0;

    if (!check)
        return m_CachedDungeonMap[groupType];

    LfgDungeonSet& cachedDungeon = m_CachedDungeonMap[groupType];
    for (LfgDungeonSet::const_iterator it = cachedDungeon.begin(); it != cachedDungeon.end();)
    {
        LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(*it);
        if (dungeon && dungeon->map > 0)
        {
            LfgEntrancePositionMap::const_iterator itr = m_entrancePositions.find(dungeon->ID);
            if (itr == m_entrancePositions.end() && !sObjectMgr->GetMapEntranceTrigger(dungeon->map))
            {
                cachedDungeon.erase(it++);
                continue;
            }

            if (AccessRequirement const* ar = sObjectMgr->GetAccessRequirement(dungeon->map, Difficulty(dungeon->difficulty)))
            {
                if (ar->levelMin > 90 || ar->levelMax > 90)
                {
                    cachedDungeon.erase(it++);
                    continue;
                }
            }
        }

        ++it;
    }
    return cachedDungeon;
}

/**
   Get the reward of a given random dungeon at a certain level

   @param[in]     dungeon dungeon id
   @param[in]     level Player level
   @returns Reward
*/
LfgReward const* LFGMgr::GetRandomDungeonReward(uint32 p_DungeonID, uint8 p_Level)
{
    LfgReward const* l_Reward = nullptr;
    LfgRewardMap::const_iterator l_Itr = m_RewardMap.find(p_DungeonID & 0x00FFFFFF);
    if (l_Itr != m_RewardMap.end())
        l_Reward = l_Itr->second;

    return l_Reward;
}

/**
   Given a Dungeon id returns the dungeon Type

   @param[in]     dungeon dungeon id
   @returns Dungeon type
*/
LfgType LFGMgr::GetDungeonType(uint32 dungeonId)
{
    LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(dungeonId);
    if (!dungeon)
        return LFG_TYPE_NONE;

    return LfgType(dungeon->type);
}

/**
   Given a list of guids returns the concatenation using | as delimiter

   @param[in]     check list of guids
   @returns Concatenated string
*/
std::string LFGMgr::ConcatenateGuids(LfgGuidList const& check)
{
    if (check.empty())
        return "";

    std::ostringstream o;
    LfgGuidList::const_iterator it = check.begin();
    o << (*it);
    for (++it; it != check.end(); ++it)
        o << '|' << (*it);
    return o.str();
}

HolidayIds LFGMgr::GetDungeonSeason(uint32 dungeonId)
{
    HolidayIds holiday = HOLIDAY_NONE;

    switch (dungeonId)
    {
    case 285:
        holiday = HOLIDAY_HALLOWS_END;
        break;
    case 286:
        holiday = HOLIDAY_FIRE_FESTIVAL;
        break;
    case 287:
        holiday = HOLIDAY_BREWFEST;
        break;
    case 288:
        holiday = HOLIDAY_LOVE_IS_IN_THE_AIR;
        break;
    case 296:
    case 297:
    case 298:
    case 299:
    case 306:
    case 308:
    case 309:
    case 310:
        holiday = HOLIDAY_WOTLK_LAUNCH; // Actualy this is cataclysm_launch_dungeon, but we didn't have a holiday for it
        break;
    default:
        break;
    }

    return holiday;
}

LfgState LFGMgr::GetState(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::GetState: [" UI64FMTD "]", guid);
    if (IS_GROUP(guid))
        return m_Groups[guid].GetState();
    else
        return m_Players[guid].GetState();
}

uint32 LFGMgr::GetDungeon(uint64 guid, bool asId /*= true*/)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::GetDungeon: [" UI64FMTD "] asId: %u", guid, asId);
    return m_Groups[guid].GetDungeon(asId);
}

uint8 LFGMgr::GetRoles(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::GetRoles: [" UI64FMTD "]", guid);
    return m_Players[guid].GetRoles();
}

const std::string& LFGMgr::GetComment(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::GetComment: [" UI64FMTD "]", guid);
    return m_Players[guid].GetComment();
}

bool LFGMgr::IsTeleported(uint64 pguid)
{
    if (std::find(m_teleport.begin(), m_teleport.end(), pguid) != m_teleport.end())
    {
        m_teleport.remove(pguid);
        return true;
    }
    return false;
}

const LfgDungeonSet& LFGMgr::GetSelectedDungeons(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::GetSelectedDungeons: [" UI64FMTD "]", guid);
    return m_Players[guid].GetSelectedDungeons();
}

const LfgLockMap& LFGMgr::GetLockedDungeons(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::GetLockedDungeons: [" UI64FMTD "]", guid);
    return m_Players[guid].GetLockedDungeons();
}

uint8 LFGMgr::GetKicksLeft(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::GetKicksLeft: [" UI64FMTD "]", guid);
    return m_Groups[guid].GetKicksLeft();
}

uint8 LFGMgr::GetVotesNeeded(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::GetVotesNeeded: [" UI64FMTD "]", guid);
    return m_Groups[guid].GetVotesNeeded();
}

void LFGMgr::RestoreState(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::RestoreState: [" UI64FMTD "]", guid);
    m_Groups[guid].RestoreState();
}

void LFGMgr::ClearState(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::ClearState: [" UI64FMTD "]", guid);
    m_Players[guid].ClearState();
}

void LFGMgr::SetState(uint64 guid, LfgState state)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::SetState: [" UI64FMTD "] state %u", guid, state);
    if (IS_GROUP(guid))
        m_Groups[guid].SetState(state);
    else
        m_Players[guid].SetState(state);
}

void LFGMgr::SetDungeon(uint64 guid, uint32 dungeon)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::SetDungeon: [" UI64FMTD "] dungeon %u", guid, dungeon);
    m_Groups[guid].SetDungeon(dungeon);
}

void LFGMgr::SetRoles(uint64 guid, uint8 roles)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::SetRoles: [" UI64FMTD "] roles: %u", guid, roles);
    m_Players[guid].SetRoles(roles);
}

void LFGMgr::SetComment(uint64 guid, const std::string& comment)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::SetComment: [" UI64FMTD "] comment: %s", guid, comment.c_str());
    m_Players[guid].SetComment(comment);
}

void LFGMgr::SetSelectedDungeons(uint64 guid, const LfgDungeonSet& dungeons)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::SetSelectedDungeons: [" UI64FMTD "]", guid);
    m_Players[guid].SetSelectedDungeons(dungeons);
}

void LFGMgr::SetLockedDungeons(uint64 guid, const LfgLockMap& lock)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::SetLockedDungeons: [" UI64FMTD "]", guid);
    m_Players[guid].SetLockedDungeons(lock);
}

void LFGMgr::DecreaseKicksLeft(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::DecreaseKicksLeft: [" UI64FMTD "]", guid);
    m_Groups[guid].DecreaseKicksLeft();
}

void LFGMgr::RemovePlayerData(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::RemovePlayerData: [" UI64FMTD "]", guid);
    LfgPlayerDataMap::iterator it = m_Players.find(guid);
    if (it != m_Players.end())
        m_Players.erase(it);
}

void LFGMgr::RemoveGroupData(uint64 guid)
{
    sLog->outDebug(LOG_FILTER_LFG, "LFGMgr::RemoveGroupData: [" UI64FMTD "]", guid);
    LfgGroupDataMap::iterator it = m_Groups.find(guid);
    if (it != m_Groups.end())
        m_Groups.erase(it);
}

LfgUpdateData LFGMgr::GetLfgStatus(uint64 guid)
{
    LfgPlayerData& playerData = m_Players[guid];
    return LfgUpdateData(LFG_UPDATETYPE_UPDATE_STATUS,  playerData.GetSelectedDungeons(), playerData.GetComment());
}

void LFGMgr::AutomaticLootAssignation(Creature* p_Creature, Group* p_Group)
{
    float l_DropChance = sWorld->getFloatConfig(CONFIG_LFR_DROP_CHANCE);

    LootTemplate const* l_LootTemplate = LootTemplates_Creature.GetLootFor(p_Creature->GetCreatureTemplate()->lootid);
    if (l_LootTemplate == nullptr)
        return;

    std::list< ItemTemplate const*> l_LootTable;
    l_LootTemplate->FillAutoAssignationLoot(l_LootTable);

    for (GroupReference* l_GroupRef = p_Group->GetFirstMember(); l_GroupRef != nullptr; l_GroupRef = l_GroupRef->next())
    {
        Player* l_Member = l_GroupRef->getSource();
        if (l_Member == nullptr || l_Member->BossAlreadyLooted(p_Creature))
            continue;

        l_Member->AddBossLooted(p_Creature);

        uint32 l_SpecializationId = l_Member->GetLootSpecId() ? l_Member->GetLootSpecId() : l_Member->GetSpecializationId(l_Member->GetActiveSpec());
        std::vector<uint32> l_Items;

        for (ItemTemplate const* l_ItemTemplate : l_LootTable)
        {
            for (SpecIndex l_ItemSpecializationId : l_ItemTemplate->specs)
            {
                if (l_ItemSpecializationId == l_SpecializationId)
                    l_Items.push_back(l_ItemTemplate->ItemId);
            }
        }

        if (!roll_chance_f(l_DropChance) || l_Items.empty())
        {
            ItemTemplate const* l_DefaultRewardItem = GetDefaultAutomaticLootItem(p_Creature);
            if (!l_DefaultRewardItem)
                continue;

            l_Member->AddItem(l_DefaultRewardItem->ItemId, 1);
            l_Member->SendDisplayToast(l_DefaultRewardItem->ItemId, 1, TOAST_TYPE_NEW_ITEM, false, false);
            continue;
        }

        std::random_shuffle(l_Items.begin(), l_Items.end());
        l_Member->AddItem(l_Items[0], 1);
        l_Member->SendDisplayToast(l_Items[0], 1, TOAST_TYPE_NEW_ITEM, false, false);
    }
}

/// Add default reward from LFR raid here
ItemTemplate const* LFGMgr::GetDefaultAutomaticLootItem(Creature* p_Creature)
{
    uint32 l_ItemId = 0;
    uint32 l_MapId  = p_Creature->GetMapId();

    switch (l_MapId)
    {
        case 996:  ///< Terrace of Endless Spring
            l_ItemId = 95617;   ///< Dividends of the Everlasting Spring
            break;
        case 1008: ///< Mogu'shan Vaults
            l_ItemId = 95618;   ///< Cache of Mogu Riches
            break;
        case 1009: ///< Heart of Fear
            l_ItemId = 95619;   ///< Amber Encased Treasure Pouch
            break;
        case 1098: ///< Throne of Thunder
            l_ItemId = 95343;   ///< Treasures of the Thunder King
            break;
        case 1136: ///< Siege of Orgrimmar
        {
            switch (p_Creature->GetEntry())
            {
                case 71543: ///< Immerseus
                case 71475: ///< Rook Stonetoe
                case 71479: ///< He Softfoot
                case 71480: ///< Sun Thunderheart
                case 71965: ///< Norushen
                case 72276: ///< Amalgam of Corruption
                case 71734: ///< Sha of Pride
                    l_ItemId = 104271;
                    break;
                default:
                    l_ItemId = 104275;
                    break;
            }

            break;
        }
        default:
            break;
    }

    return sObjectMgr->GetItemTemplate(l_ItemId);
}

LfgProposal* LFGMgr::CheckForSingle(LfgGuidList& check)
{
    for (uint64 &guid : check)
    {
        uint64 playerGuid = guid;

        LfgQueueInfoMap::const_iterator itr = m_QueueInfoMap.find(playerGuid);
        if (itr == m_QueueInfoMap.end())
            continue;

        if (itr->second->dungeons.size() != 1)
            continue;

        uint32 dungeonId  = (*(itr->second->dungeons.begin()));
        LFGDungeonEntry const* dungeonEntry = sLFGDungeonStore.LookupEntry(dungeonId);
        if (!dungeonEntry)
            continue;

        if (!dungeonEntry->isScenarioSingle())
            continue;

        LfgGuidList groupGuids;
        groupGuids.push_back(playerGuid);

        LfgProposalPlayer* ppPlayer = new LfgProposalPlayer();
        
        ppPlayer->role = ROLE_DAMAGE;

        LfgProposal* pProposal = new LfgProposal(dungeonId);
        pProposal->cancelTime = time_t(time(NULL)) + LFG_TIME_PROPOSAL;
        pProposal->state = LFG_PROPOSAL_INITIATING;
        pProposal->queues = groupGuids;
        pProposal->groupLowGuid = 0;
        pProposal->leader = playerGuid;
        pProposal->players[playerGuid] = ppPlayer;

        return pProposal;
    }

    return nullptr;
}