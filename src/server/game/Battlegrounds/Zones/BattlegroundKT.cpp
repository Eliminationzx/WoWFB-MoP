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

#include "Object.h"
#include "Player.h"
#include "Battleground.h"
#include "BattlegroundKT.h"
#include "Creature.h"
#include "GameObject.h"
#include "ObjectMgr.h"
#include "BattlegroundMgr.h"
#include "WorldPacket.h"
#include "Language.h"
#include "MapManager.h"

BattlegroundKT::BattlegroundKT()
{
    StartMessageIds[BG_STARTING_EVENT_FIRST]  = 0;
    StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_KT_START_ONE_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_KT_START_HALF_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_KT_HAS_BEGUN;

    m_ReputationCapture = 0;
    m_HonorWinKills = 0;
    m_HonorEndKills = 0;

    m_EndTimer = 0;
    m_LastCapturedOrbTeam = TEAM_NONE;

    for (uint8 i = 0; i < MAX_ORBS; ++i)
    {
        m_UpdatePointsTimer[i] = -1;
        m_OrbKeepers[i] = 0;
    }
}

BattlegroundKT::~BattlegroundKT()
{
}

void BattlegroundKT::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() == STATUS_WAIT_JOIN)
    {
        m_CheatersCheckTimer -= diff;
        if (m_CheatersCheckTimer <= 0)
        {
            for (auto itr : GetPlayers())
            {
                Player * plr = ObjectAccessor::FindPlayer(itr.first);
                if (!plr || !plr->IsInWorld())
                    continue;

                if (plr->GetPositionZ() < 24)
                {
                    if (plr->GetBGTeam() == HORDE)
                        plr->TeleportTo(998, 1781.31f, 1597.76f, 33.61f, plr->GetOrientation(), 0);
                    else
                        plr->TeleportTo(998, 1784.42f, 1072.73f, 29.88f, plr->GetOrientation(), 0);
                }
            }
            m_CheatersCheckTimer = 4000;
        }
    }
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (m_EndTimer <= diff)
        {
            uint32 allianceScore = GetTeamScore(ALLIANCE);
            uint32 hordeScore    = GetTeamScore(HORDE);

            if (allianceScore > hordeScore)
                EndBattleground(ALLIANCE);
            else if (allianceScore < hordeScore)
                EndBattleground(HORDE);
            else
            {
                // if 0 => tie
                EndBattleground(m_LastCapturedOrbTeam);
            }
        }
        else
        {
            uint32 minutesLeftPrev = GetRemainingTimeInMinutes();
            m_EndTimer -= diff;
            uint32 minutesLeft = GetRemainingTimeInMinutes();

            if (minutesLeft != minutesLeftPrev)
                UpdateWorldState(BG_KT_TIME_REMAINING, minutesLeft);
        }

        for (uint8 i = 0; i < MAX_ORBS; ++i)
        {
            if (m_UpdatePointsTimer[i] == -1)
                continue;

            if (uint32(m_UpdatePointsTimer[i]) <= diff)
            {
                if (uint64 guid = m_OrbKeepers[i])
                {
                    if (m_playersZone.find(guid) != m_playersZone.end())
                    {
                        if (Player* player = ObjectAccessor::FindPlayer(guid))
                        {
                            AccumulateScore(player->GetBGTeam() == ALLIANCE ? BG_TEAM_ALLIANCE : BG_TEAM_HORDE, m_playersZone[guid]);
                            UpdatePlayerScore(player, SCORE_ORB_SCORE, m_playersZone[guid]);
                        }
                    }
                }

                m_UpdatePointsTimer[i] = BG_KT_POINTS_UPDATE_TIME;
            }
            else
                m_UpdatePointsTimer[i] -= diff;
        }
    }
}

void BattlegroundKT::StartingEventCloseDoors()
{
    SpawnBGObject(BG_KT_OBJECT_A_DOOR, RESPAWN_IMMEDIATELY);
    SpawnBGObject(BG_KT_OBJECT_H_DOOR, RESPAWN_IMMEDIATELY);

    DoorClose(BG_KT_OBJECT_A_DOOR);
    DoorClose(BG_KT_OBJECT_H_DOOR);

    for (uint8 i = 0; i < MAX_ORBS; ++i)
        SpawnBGObject(BG_KT_OBJECT_ORB_1 + i, RESPAWN_ONE_DAY);

    // Spawn Buffs
    for (uint32 l_Iter = 0; l_Iter < MAX_BUFFS_KT; l_Iter++)
        SpawnBGObject(BG_KT_OBJECT_BUFF_NORTH + l_Iter, RESPAWN_IMMEDIATELY);
}

void BattlegroundKT::StartingEventOpenDoors()
{
    DoorOpen(BG_KT_OBJECT_A_DOOR);
    DoorOpen(BG_KT_OBJECT_H_DOOR);

    for (uint8 i = 0; i < MAX_ORBS; ++i)
        SpawnBGObject(BG_KT_OBJECT_ORB_1 + i, RESPAWN_IMMEDIATELY);

    // Players that join battleground after start are not eligible to get achievement.
    StartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, BG_KT_EVENT_START_BATTLE);
}

void BattlegroundKT::AddPlayer(Player *plr)
{
    Battleground::AddPlayer(plr);
    //create score and add it to map, default values are set in constructor
    BattleGroundKTScore* sc = new BattleGroundKTScore;

    PlayerScores[plr->GetGUID()] = sc;
    m_playersZone[plr->GetGUID()] = KT_ZONE_OUT;
}

void BattlegroundKT::EventPlayerClickedOnOrb(Player* source, GameObject* target_obj)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (!source->IsWithinDistInMap(target_obj, 10))
        return;

    uint32 index = target_obj->GetEntry() - BG_KT_OBJECT_ORB_1_ENTRY;

    // If this orb is already keeped by a player, there is a problem
    if (index >= MAX_ORBS || m_OrbKeepers[index] != 0)
        return;

    // Check if the player already have an orb
    if (HaveOrb(source->GetGUID()))
        return;

    SetOrb(source->GetGUID(), index);
    source->CastSpell(source, BG_KT_ORBS_SPELLS[index], true);
}

void BattlegroundKT::EventPlayerDroppedOrb(Player* source)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 index = GetOrbIndex(source->GetGUID());
    if (index == MAX_ORBS)
        return;

    source->RemoveAurasDueToSpell(BG_KT_ORBS_SPELLS[index]);
}

void BattlegroundKT::TakeOrb(Player* player, uint8 const index)
{
    auto const team = player->GetBGTeam() == ALLIANCE;

    PlaySoundToAll(team ? BG_KT_SOUND_A_ORB_PICKED_UP : BG_KT_SOUND_H_ORB_PICKED_UP);

    player->CastSpell(player, team ? BG_KT_ALLIANCE_INSIGNIA : BG_KT_HORDE_INSIGNIA, true);

    UpdatePlayerScore(player, SCORE_ORB_HANDLES, 1);

    UpdateWorldState(BG_KT_ICON_A, 1);
    SpawnBGObject(BG_KT_OBJECT_ORB_1 + index, RESPAWN_ONE_DAY);

    if (Creature* aura = GetBGCreature(BG_KT_CREATURE_ORB_AURA_1 + index))
        aura->RemoveAllAuras();

    SendMessageToAll(LANG_BG_KT_PICKEDUP, team ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, player);
    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
}

void BattlegroundKT::ReleaseOrb(Player* player, uint8 const index)
{
    auto const team = player->GetBGTeam() == ALLIANCE;

    PlaySoundToAll(team ? BG_KT_SOUND_A_ORB_PICKED_UP : BG_KT_SOUND_H_ORB_PICKED_UP);
    player->RemoveAurasDueToSpell(BG_KT_ALLIANCE_INSIGNIA);
    player->RemoveAurasDueToSpell(BG_KT_HORDE_INSIGNIA);

    SetOrb(0, index);

    SpawnBGObject(BG_KT_OBJECT_ORB_1 + index, RESPAWN_IMMEDIATELY);

    if (Creature* aura = GetBGCreature(BG_KT_CREATURE_ORB_AURA_1 + index))
        aura->AddAura(BG_KT_ORBS_AURA[index], aura);

    UpdateWorldState(BG_KT_ICON_A, 0);
    SendMessageToAll(LANG_BG_KT_DROPPED, team ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, player);
    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

    AccumulateScoreByValue(team ? BG_TEAM_HORDE : BG_TEAM_ALLIANCE, BG_KT_ORB_OWNER_DIED_POINTS);
}

void BattlegroundKT::RemovePlayer(Player* player, uint64 /*guid*/, uint32 /*team*/)
{
    if (!player)
        return;

    EventPlayerDroppedOrb(player);
    m_playersZone.erase(player->GetGUID());
}

void BattlegroundKT::UpdateOrbState(Team team, uint32 value)
{
    if (team == ALLIANCE)
        UpdateWorldState(BG_KT_ICON_A, value);
    else
        UpdateWorldState(BG_KT_ICON_H, value);
}

void BattlegroundKT::UpdateTeamScore(Team team)
{
    if (team == ALLIANCE)
        UpdateWorldState(BG_KT_ORB_POINTS_A, GetTeamScore(team));
    else
        UpdateWorldState(BG_KT_ORB_POINTS_H, GetTeamScore(team));
}

void BattlegroundKT::HandleAreaTrigger(Player* source, uint32 trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint64 sourceGuid = source->GetGUID();
    switch(trigger)
    {
        case 7734: // Out-In trigger
        {
            if (m_playersZone.find(sourceGuid) == m_playersZone.end())
                return;

            if (m_playersZone[sourceGuid] == KT_ZONE_OUT)
                m_playersZone[sourceGuid] = KT_ZONE_IN;
            else
                m_playersZone[sourceGuid] = KT_ZONE_OUT;
            break;
        }
        case 7735: // Middle-In trigger
        {
            if (m_playersZone.find(sourceGuid) == m_playersZone.end())
                return;

            if (m_playersZone[sourceGuid] == KT_ZONE_IN)
                m_playersZone[sourceGuid] = KT_ZONE_MIDDLE;
            else
                m_playersZone[sourceGuid] = KT_ZONE_IN;
            break;
        }
        default:
            break;
    }
}

bool BattlegroundKT::SetupBattleground()
{
    // Doors
    if (   !AddObject(BG_KT_OBJECT_A_DOOR, BG_KT_OBJECT_DOOR_ENTRY, BG_KT_DoorPositions[0][0], BG_KT_DoorPositions[0][1], BG_KT_DoorPositions[0][2], BG_KT_DoorPositions[0][3], 0, 0, sin(BG_KT_DoorPositions[0][3]/2), cos(BG_KT_DoorPositions[0][3]/2), RESPAWN_IMMEDIATELY)
        || !AddObject(BG_KT_OBJECT_H_DOOR, BG_KT_OBJECT_DOOR_ENTRY, BG_KT_DoorPositions[1][0], BG_KT_DoorPositions[1][1], BG_KT_DoorPositions[1][2], BG_KT_DoorPositions[1][3], 0, 0, sin(BG_KT_DoorPositions[1][3]/2), cos(BG_KT_DoorPositions[1][3]/2), RESPAWN_IMMEDIATELY))
        return false;

    if (   !AddSpiritGuide(BG_KT_CREATURE_SPIRIT_1, BG_KT_SpiritPositions[0][0], BG_KT_SpiritPositions[0][1], BG_KT_SpiritPositions[0][2], BG_KT_SpiritPositions[0][3], ALLIANCE)
        || !AddSpiritGuide(BG_KT_CREATURE_SPIRIT_2, BG_KT_SpiritPositions[1][0], BG_KT_SpiritPositions[1][1], BG_KT_SpiritPositions[1][2], BG_KT_SpiritPositions[1][3], HORDE))
        return false;

    // Buffs
    for (int i = 0; i < MAX_BUFFS_KT; i++)
    {
        if (!AddObject(BG_KT_OBJECT_BUFF_NORTH + i, Buff_Entries[2], BG_KT_BuffPositions[i][0], BG_KT_BuffPositions[i][1], BG_KT_BuffPositions[i][2], BG_KT_BuffPositions[i][3], 0, 0, std::sin(BG_KT_BuffPositions[i][3] / 2), std::cos(BG_KT_BuffPositions[i][3] / 2), BUFF_RESPAWN_TIME))
        {
            sLog->outError(LOG_FILTER_BATTLEGROUND, "BatteGroundKT: Failed to spawn buff object. Battleground not created!");
            return false;
        }
    }
    // Orbs
    for (uint8 i = 0; i < MAX_ORBS; ++i)
    {
        if (!AddObject(BG_KT_OBJECT_ORB_1 + i, BG_KT_OBJECT_ORB_1_ENTRY + i, BG_KT_OrbPositions[i][0], BG_KT_OrbPositions[i][1], BG_KT_OrbPositions[i][2], BG_KT_OrbPositions[i][3], 0, 0, sin(BG_KT_OrbPositions[i][3]/2), cos(BG_KT_OrbPositions[i][3]/2), RESPAWN_ONE_DAY))
            return false;

        if (Creature* trigger = AddCreature(WORLD_TRIGGER, BG_KT_CREATURE_ORB_AURA_1 + i, TEAM_NEUTRAL, BG_KT_OrbPositions[i][0], BG_KT_OrbPositions[i][1], BG_KT_OrbPositions[i][2], BG_KT_OrbPositions[i][3], RESPAWN_IMMEDIATELY))
            trigger->AddAura(BG_KT_ORBS_AURA[i], trigger);
    }

    return true;
}

void BattlegroundKT::Reset()
{
    //call parent's class reset
    Battleground::Reset();
    BgObjects.resize(BG_KT_OBJECT_MAX);
    BgCreatures.resize(BG_KT_CREATURE_MAX);

    for (uint32 i = 0; i < MAX_ORBS; ++i)
    {
        m_UpdatePointsTimer[i] = -1;
        m_OrbKeepers[i] = 0;
    }

    bool isBGWeekend = BattlegroundMgr::IsBGWeekend(GetTypeID());
    m_ReputationCapture = (isBGWeekend) ? 45 : 35;
    m_HonorWinKills = (isBGWeekend) ? 3 : 1;
    m_HonorEndKills = (isBGWeekend) ? 4 : 2;
    m_CheatersCheckTimer = 0;

    m_EndTimer = BG_KT_TIME_LIMIT;
    m_LastCapturedOrbTeam = TEAM_NONE;
}

void BattlegroundKT::EndBattleground(uint32 winner)
{
    //win reward
    if (winner == ALLIANCE)
        RewardHonorToTeam(GetBonusHonorFromKill(m_HonorWinKills), ALLIANCE);
    if (winner == HORDE)
        RewardHonorToTeam(GetBonusHonorFromKill(m_HonorWinKills), HORDE);
    //complete map_end rewards (even if no team wins)
    RewardHonorToTeam(GetBonusHonorFromKill(m_HonorEndKills), ALLIANCE);
    RewardHonorToTeam(GetBonusHonorFromKill(m_HonorEndKills), HORDE);

    Battleground::EndBattleground(winner);
}

void BattlegroundKT::HandleKillPlayer(Player *player, Player *killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    EventPlayerDroppedOrb(player);

    Battleground::HandleKillPlayer(player, killer);
}

void BattlegroundKT::UpdatePlayerScore(Player* Source, uint32 type, uint32 value, bool doAddHonor)
{
    BattlegroundScoreMap::iterator itr = PlayerScores.find(Source->GetObjectGuid());
    if (itr == PlayerScores.end())                         // player not found
        return;

    switch(type)
    {
        case SCORE_ORB_HANDLES:                           // orb handles
            ((BattleGroundKTScore*)itr->second)->OrbHandles += value;
            break;
        case SCORE_ORB_SCORE:
            ((BattleGroundKTScore*)itr->second)->Score += value;
            break;
        default:
            Battleground::UpdatePlayerScore(Source, NULL, type, value, doAddHonor);
            break;
    }
}

WorldSafeLocsEntry const* BattlegroundKT::GetClosestGraveYard(Player* player)
{
    //if status in progress, it returns main graveyards with spiritguides
    //else it will return the graveyard in the flagroom - this is especially good
    //if a player dies in preparation phase - then the player can't cheat
    //and teleport to the graveyard outside the flagroom
    //and start running around, while the doors are still closed
    if (player->GetBGTeam() == ALLIANCE)
    {
        if (GetStatus() == STATUS_IN_PROGRESS)
            return sWorldSafeLocsStore.LookupEntry(KT_GRAVEYARD_RECTANGLEA1);
        else
            return sWorldSafeLocsStore.LookupEntry(KT_GRAVEYARD_RECTANGLEA2);
    }
    else
    {
        if (GetStatus() == STATUS_IN_PROGRESS)
            return sWorldSafeLocsStore.LookupEntry(KT_GRAVEYARD_RECTANGLEH1);
        else
            return sWorldSafeLocsStore.LookupEntry(KT_GRAVEYARD_RECTANGLEH2);
    }
}

void BattlegroundKT::AccumulateScore(uint32 team, BG_KT_ZONE zone)
{
    if (zone >= KT_ZONE_MAX)
        return;

    if (team >= TEAM_NEUTRAL)
        return;

    if (GetStatus() == STATUS_WAIT_LEAVE)
        return;

    AccumulateScoreByValue(team, BG_KT_TickPoints[zone]);
}

void BattlegroundKT::AccumulateScoreByValue(uint32 team, uint32 value)
{
    if (GetStatus() == STATUS_WAIT_LEAVE)
        return;

    if (team >= TEAM_NEUTRAL)
        return;

    m_TeamScores[team] += value;

    if (m_TeamScores[team] > BG_KT_MAX_TEAM_SCORE)
        m_TeamScores[team] = BG_KT_MAX_TEAM_SCORE;

    if (team == BG_TEAM_ALLIANCE)
        UpdateWorldState(BG_KT_ORB_POINTS_A, m_TeamScores[team]);
    if (team == BG_TEAM_HORDE)
        UpdateWorldState(BG_KT_ORB_POINTS_H, m_TeamScores[team]);

    // Test win condition
    if (m_TeamScores[BG_TEAM_ALLIANCE] >= BG_KT_MAX_TEAM_SCORE)
        EndBattleground(ALLIANCE);
    else if (m_TeamScores[BG_TEAM_HORDE] >= BG_KT_MAX_TEAM_SCORE)
        EndBattleground(HORDE);
}

void BattlegroundKT::FillInitialWorldStates(WorldPacket& data)
{
    Player::AppendWorldState(data, uint32(BG_KT_ORB_POINTS_A), GetTeamScore(ALLIANCE));
    Player::AppendWorldState(data, uint32(BG_KT_ORB_POINTS_H), GetTeamScore(HORDE));

    /*if (m_OrbState[BG_TEAM_ALLIANCE] == BG_KT_ORB_STATE_ON_GROUND)
        Player::AppendWorldState(data, BG_KT_ICON_A, -1);
    else if (m_OrbState[BG_TEAM_ALLIANCE] == BG_KT_ORB_STATE_ON_PLAYER)
        Player::AppendWorldState(data, BG_KT_ICON_A, 1);
    else
        Player::AppendWorldState(data, BG_KT_ICON_A, 0);

    if (m_OrbState[BG_TEAM_HORDE] == BG_KT_ORB_STATE_ON_GROUND)
        Player::AppendWorldState(data, BG_KT_ICON_H, -1);
    else if (m_OrbState[BG_TEAM_HORDE] == BG_KT_ORB_STATE_ON_PLAYER)
        Player::AppendWorldState(data, BG_KT_ICON_H, 1);
    else
        Player::AppendWorldState(data, BG_KT_ICON_H, 0);*/

    Player::AppendWorldState(data, BG_KT_ORB_POINTS_MAX, BG_KT_MAX_TEAM_SCORE);

    /*if (m_OrbState[BG_TEAM_HORDE] == BG_KT_ORB_STATE_ON_PLAYER)
        Player::AppendWorldState(data, BG_KT_ORB_STATE, 2);
    else
        Player::AppendWorldState(data, BG_KT_ORB_STATE, 1);

    if (m_OrbState[BG_TEAM_ALLIANCE] == BG_KT_ORB_STATE_ON_PLAYER)
        Player::AppendWorldState(data, BG_KT_ORB_STATE, 2);
    else
        Player::AppendWorldState(data, BG_KT_ORB_STATE, 1);*/

    Player::AppendWorldState(data, BG_KT_TIME_ENABLED, 1);
    Player::AppendWorldState(data, BG_KT_TIME_REMAINING, GetRemainingTimeInMinutes());
}
