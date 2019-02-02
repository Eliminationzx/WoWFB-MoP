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

#include "Chat.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "Arena.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "Creature.h"
#include "Formulas.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Language.h"
#include "MapManager.h"
#include "Object.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "Util.h"
#include "Guild.h"
#include "GuildMgr.h"

namespace JadeCore
{
    class BattlegroundChatBuilder
    {
        public:
            BattlegroundChatBuilder(ChatMsg msgtype, int32 textId, Player const* source, va_list* args = NULL)
                : _msgtype(msgtype), _textId(textId), _source(source), _args(args) { }

            void operator()(WorldPacket& data, LocaleConstant loc_idx)
            {
                char const* text = sObjectMgr->GetTrinityString(_textId, loc_idx);
                if (_args)
                {
                    // we need copy va_list before use or original va_list will corrupted
                    va_list ap;
                    va_copy(ap, *_args);

                    char str[2048];
                    vsnprintf(str, 2048, text, ap);
                    va_end(ap);

                    do_helper(data, &str[0]);
                }
                else
                    do_helper(data, text);
            }

        private:
            void do_helper(WorldPacket& data, char const* text)
            {
                ChatHandler::FillMessageData(&data, _source ? _source->GetSession() : NULL, _msgtype, LANG_UNIVERSAL, NULL, _source ? _source->GetGUID() : 0, text, NULL, NULL);
            }

            ChatMsg _msgtype;
            int32 _textId;
            Player const* _source;
            va_list* _args;
    };

    class Battleground2ChatBuilder
    {
        public:
            Battleground2ChatBuilder(ChatMsg msgtype, int32 textId, Player const* source, int32 arg1, int32 arg2)
                : _msgtype(msgtype), _textId(textId), _source(source), _arg1(arg1), _arg2(arg2) {}

            void operator()(WorldPacket& data, LocaleConstant loc_idx)
            {
                char const* text = sObjectMgr->GetTrinityString(_textId, loc_idx);
                char const* arg1str = _arg1 ? sObjectMgr->GetTrinityString(_arg1, loc_idx) : "";
                char const* arg2str = _arg2 ? sObjectMgr->GetTrinityString(_arg2, loc_idx) : "";

                char str[2048];
                snprintf(str, 2048, text, arg1str, arg2str);

                ChatHandler::FillMessageData(&data, _source ? _source->GetSession() : NULL, _msgtype, LANG_UNIVERSAL, NULL, _source ? _source->GetGUID() : 0, str, NULL, NULL);
            }

        private:
            ChatMsg _msgtype;
            int32 _textId;
            Player const* _source;
            int32 _arg1;
            int32 _arg2;
    };
}                                                           // namespace JadeCore

template<class Do>
void Battleground::BroadcastWorker(Do& _do)
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HighGuid::Player)))
            _do(player);
}

Battleground::Battleground()
{
    m_TypeID            = BATTLEGROUND_TYPE_NONE;
    m_RandomTypeID      = BATTLEGROUND_TYPE_NONE;
    m_Guid              = MAKE_NEW_GUID(m_TypeID, 0, HighGuid::TypeBattleGround);
    m_InstanceID        = 0;
    m_Status            = STATUS_NONE;
    m_ClientInstanceID  = 0;
    m_EndTime           = 0;
    m_LastResurrectTime = 0;
    m_BracketId         = BG_BRACKET_ID_FIRST;
    m_InvitedAlliance   = 0;
    m_InvitedHorde      = 0;
    m_ArenaType         = 0;
    m_IsArena           = false;
    m_Winner            = 2;
    m_StartTime         = 0;
    m_CountdownTimer    = 0;
    m_ResetStatTimer    = 0;
    m_ValidStartPositionTimer = 0;
    m_Events            = 0;
    m_IsRated           = false;
    m_IsRatedBg         = false;
    m_BuffChange        = false;
    m_IsRandom          = false;
    m_Name              = "";
    m_LevelMin          = 0;
    m_LevelMax          = 0;
    m_InBGFreeSlotQueue = false;
    m_SetDeleteThis     = false;
    m_canChangeRate     = false;

    m_PlayersPositionsTimer = 500;
    m_BattleFatigueUpdateTimer = 0;

    m_MaxPlayersPerTeam = 0;
    m_MaxPlayers        = 0;
    m_MinPlayersPerTeam = 0;
    m_MinPlayers        = 0;

    m_MapId             = 0;
    m_Map               = NULL;

    m_TeamStartLocX[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocX[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocY[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocY[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocZ[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocZ[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocO[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocO[BG_TEAM_HORDE]      = 0;

    m_StartMaxDist = 0.0f;
    m_holiday = 0;

    m_ArenaTeamRatingChanges[BG_TEAM_ALLIANCE]   = 0;
    m_ArenaTeamRatingChanges[BG_TEAM_HORDE]      = 0;

    m_ArenaTeamMMR[BG_TEAM_ALLIANCE]   = 0;
    m_ArenaTeamMMR[BG_TEAM_HORDE]      = 0;

    m_BgRaids[BG_TEAM_ALLIANCE]         = NULL;
    m_BgRaids[BG_TEAM_HORDE]            = NULL;

    m_PlayersCount[BG_TEAM_ALLIANCE]    = 0;
    m_PlayersCount[BG_TEAM_HORDE]       = 0;

    m_TeamScores[BG_TEAM_ALLIANCE]      = 0;
    m_TeamScores[BG_TEAM_HORDE]         = 0;

    m_PrematureCountDown = false;

    m_HonorMode = BG_NORMAL;

    StartDelayTimes[BG_STARTING_EVENT_FIRST]  = BG_START_DELAY_2M;
    StartDelayTimes[BG_STARTING_EVENT_SECOND] = BG_START_DELAY_1M;
    StartDelayTimes[BG_STARTING_EVENT_THIRD]  = BG_START_DELAY_30S;
    StartDelayTimes[BG_STARTING_EVENT_FOURTH] = BG_START_DELAY_NONE;
    //we must set to some default existing values
    StartMessageIds[BG_STARTING_EVENT_FIRST]  = LANG_BG_WS_START_TWO_MINUTES;
    StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_WS_START_ONE_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_WS_START_HALF_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_WS_HAS_BEGUN;

    m_CrowdChosed = false;

    m_dampeningIsCasted = false;
    m_FrameIsFixed1 = false;
    ScriptId = 0;

    for (uint8 i = 0; i < BG_TEAMS_COUNT; ++i)
        _arenaTeamScores[i].Reset();
}

Battleground::~Battleground()
{
    // remove objects and creatures
    // (this is done automatically in mapmanager update, when the instance is reset after the reset time)
    uint32 size = uint32(BgCreatures.size());
    for (uint32 i = 0; i < size; ++i)
        DelCreature(i);

    size = uint32(BgObjects.size());
    for (uint32 i = 0; i < size; ++i)
        DelObject(i);

    sBattlegroundMgr->RemoveBattleground(GetInstanceID(), GetTypeID());
    // unload map
    if (m_Map)
    {
        m_Map->SetUnload();
        //unlink to prevent crash, always unlink all pointer reference before destruction
        m_Map->SetBG(NULL);
        m_Map = NULL;
    }
    // remove from bg free slot queue
    RemoveFromBGFreeSlotQueue();

    for (BattlegroundScoreMap::const_iterator itr = PlayerScores.begin(); itr != PlayerScores.end(); ++itr)
        delete itr->second;
}

void Battleground::Update(uint32 diff)
{
    if (!PreUpdateImpl(diff))
        return;

    if (!GetPlayersSize())
    {
        //BG is empty
        // if there are no players invited, delete BG
        // this will delete arena or bg object, where any player entered
        // [[   but if you use battleground object again (more battles possible to be played on 1 instance)
        //      then this condition should be removed and code:
        //      if (!GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE))
        //          this->AddToFreeBGObjectsQueue(); // not yet implemented
        //      should be used instead of current
        // ]]
        // Battleground Template instance cannot be updated, because it would be deleted
        if (!GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE))
            m_SetDeleteThis = true;
        return;
    }

    switch (GetStatus())
    {
        case STATUS_WAIT_JOIN:
            if (GetPlayersSize())
            {
                _ProcessJoin(diff);
                //_CheckSafePositions(diff); disable, use check for every bg in PostUpdateImpl
            }
            break;
        case STATUS_IN_PROGRESS:
            _ProcessOfflineQueue();
            // after 20 minutes without one team losing, the arena closes with no winner and no rating change
            if (isArena())
            {
                if (GetElapsedTime() >= 20 * MINUTE * IN_MILLISECONDS)
                {
                    UpdateArenaWorldState();
                    CheckArenaAfterTimerConditions();
                    return;
                }

                if (!m_dampeningIsCasted)
                {
                    uint32 time = (GetArenaType() == ARENA_TEAM_2v2 ? 5 : 10) * MINUTE * IN_MILLISECONDS;
                    if (GetElapsedTime() >= time)
                    {
                        CastDampeningToAllPlayers();
                        m_dampeningIsCasted = true;
                    }
                }

                if (GetElapsedTime() >= 61 * IN_MILLISECONDS)
                {
                    if (!m_FrameIsFixed1)
                    {
                        for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
                        {
                            if (Player* player = _GetPlayer(itr, "Fix frame"))
                            {
                                bool hasFlag = player->m_stealth.HasFlag(STEALTH_GENERAL);
                                if (!hasFlag)
                                    player->m_stealth.AddFlag(STEALTH_GENERAL);
                                
                                player->UpdateObjectVisibility();
                                player->m_stealth.DelFlag(STEALTH_GENERAL);
                                player->UpdateObjectVisibility();
                                
                                if (hasFlag) // Restore flag if present
                                {
                                    player->m_stealth.AddFlag(STEALTH_GENERAL);
                                    player->UpdateObjectVisibility();
                                }
                            }
                        }

                        m_FrameIsFixed1 = true;
                    }
                }
                
                if (m_BattleFatigueUpdateTimer <= diff)
                {
                    UpdateBattleFatigue();
                    m_BattleFatigueUpdateTimer = 5000;
                }
                else
                    m_BattleFatigueUpdateTimer += diff;
            }
            else
            {
                // Since 5.0.X, client doesn't send request to know players flags positions
                // We need to handle it server-side and send new position every 5 s
                if (m_PlayersPositionsTimer <= diff)
                {
                    SendFlagsPositions();
                    m_PlayersPositionsTimer = 5000;
                }
                else
                    m_PlayersPositionsTimer -= diff;

                _ProcessRessurect(diff);
                if (sBattlegroundMgr->GetPrematureFinishTime() && (GetPlayersCountByTeam(ALLIANCE) < GetMinPlayersPerTeam() || GetPlayersCountByTeam(HORDE) < GetMinPlayersPerTeam()))
                    _ProcessProgress(diff);
                else if (m_PrematureCountDown)
                    m_PrematureCountDown = false;
            }
            break;
        case STATUS_WAIT_LEAVE:
            _ProcessLeave(diff);
            break;
        default:
            break;
    }

    // Update start time and reset stats timer
    m_StartTime += diff;
    m_ResetStatTimer += diff;

    PostUpdateImpl(diff);
}

inline void Battleground::_CheckSafePositions(uint32 diff)
{
    float maxDist = GetStartMaxDist();
    if (!maxDist)
        return;

    m_ValidStartPositionTimer += diff;
    if (m_ValidStartPositionTimer >= CHECK_PLAYER_POSITION_INVERVAL)
    {
        m_ValidStartPositionTimer = 0;

        Position pos;
        float x, y, z, o;
        for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
            if (Player* player = ObjectAccessor::FindPlayer(itr->first))
            {
                player->GetPosition(&pos);
                GetTeamStartLoc(player->GetBGTeam(), x, y, z, o);
                if (pos.GetExactDistSq(x, y, z) > maxDist)
                {
                    sLog->outDebug(LOG_FILTER_BATTLEGROUND, "BATTLEGROUND: Sending %s back to start location (map: %u) (possible exploit)", player->GetName(), GetMapId());
                    player->TeleportTo(GetMapId(), x, y, z, o);
                }
            }
    }
}

inline void Battleground::_ProcessOfflineQueue()
{
    // remove offline players from bg after 5 minutes
    if (!m_OfflineQueue.empty())
    {
        BattlegroundPlayerMap::iterator itr = m_Players.find(*(m_OfflineQueue.begin()));
        if (itr != m_Players.end())
        {
            if (itr->second.OfflineRemoveTime <= sWorld->GetGameTime())
            {
                RemovePlayerAtLeave(itr->first, true, true);// remove player from BG
                m_OfflineQueue.pop_front();                 // remove from offline queue
                //do not use itr for anything, because it is erased in RemovePlayerAtLeave()
            }
        }
    }

}

inline void Battleground::_ProcessRessurect(uint32 diff)
{
    // *********************************************************
    // ***        BATTLEGROUND RESSURECTION SYSTEM           ***
    // *********************************************************
    // this should be handled by spell system
    m_LastResurrectTime += diff;
    if (m_LastResurrectTime >= RESURRECTION_INTERVAL)
    {
        if (GetReviveQueueSize())
        {
            for (std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
            {
                Creature* sh = NULL;
                for (std::vector<uint64>::const_iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
                {
                    Player* player = ObjectAccessor::FindPlayer(*itr2);
                    if (!player)
                        continue;

                    if (!sh && player->IsInWorld())
                    {
                        sh = player->GetMap()->GetCreature(itr->first);
                        // only for visual effect
                        if (sh)
                            // Spirit Heal, effect 117
                            sh->CastSpell(sh, SPELL_SPIRIT_HEAL, true);
                    }

                    // Resurrection visual
                    player->CastSpell(player, SPELL_RESURRECTION_VISUAL, true);
                    m_ResurrectQueue.push_back(*itr2);
                }
                (itr->second).clear();
            }

            m_ReviveQueue.clear();
            m_LastResurrectTime = 0;
        }
        else
            // queue is clear and time passed, just update last resurrection time
            m_LastResurrectTime = 0;
    }
    else if (m_LastResurrectTime > 500)    // Resurrect players only half a second later, to see spirit heal effect on NPC
    {
        for (std::vector<uint64>::const_iterator itr = m_ResurrectQueue.begin(); itr != m_ResurrectQueue.end(); ++itr)
        {
            Player* player = ObjectAccessor::FindPlayer(*itr);
            if (!player)
                continue;
            player->ResurrectPlayer(1.0f);
            player->CastSpell(player, 6962, true);
            player->CastSpell(player, SPELL_SPIRIT_HEAL_MANA, true);
            player->SpawnCorpseBones(false);
        }
        m_ResurrectQueue.clear();
    }
}

inline void Battleground::_ProcessProgress(uint32 diff)
{
    // *********************************************************
    // ***           BATTLEGROUND BALLANCE SYSTEM            ***
    // *********************************************************
    // if less then minimum players are in on one side, then start premature finish timer
    if (!m_PrematureCountDown)
    {
        m_PrematureCountDown = true;
        m_PrematureCountDownTimer = sBattlegroundMgr->GetPrematureFinishTime();
    }
    else if (m_PrematureCountDownTimer < diff)
    {
        // time's up!
        uint32 winner = 0;
        if (GetPlayersCountByTeam(ALLIANCE) >= GetMinPlayersPerTeam())
            winner = ALLIANCE;
        else if (GetPlayersCountByTeam(HORDE) >= GetMinPlayersPerTeam())
            winner = HORDE;

        EndBattleground(winner);
        m_PrematureCountDown = false;
    }
    else if (!sBattlegroundMgr->isTesting())
    {
        uint32 newtime = m_PrematureCountDownTimer - diff;
        // announce every minute
        if (newtime > (MINUTE * IN_MILLISECONDS))
        {
            if (newtime / (MINUTE * IN_MILLISECONDS) != m_PrematureCountDownTimer / (MINUTE * IN_MILLISECONDS))
                PSendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING, CHAT_MSG_SYSTEM, NULL, (uint32)(m_PrematureCountDownTimer / (MINUTE * IN_MILLISECONDS)));
        }
        else
        {
            //announce every 15 seconds
            if (newtime / (15 * IN_MILLISECONDS) != m_PrematureCountDownTimer / (15 * IN_MILLISECONDS))
                PSendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING_SECS, CHAT_MSG_SYSTEM, NULL, (uint32)(m_PrematureCountDownTimer / IN_MILLISECONDS));
        }
        m_PrematureCountDownTimer = newtime;
    }
}

inline void Battleground::_ProcessJoin(uint32 diff)
{
    // *********************************************************
    // ***           BATTLEGROUND STARTING SYSTEM            ***
    // *********************************************************
    ModifyStartDelayTime(diff);
    ModifyCountdownTimer(diff);

    // I know it's a too big but it's the value sent in packet, I get it from retail sniff.
    // I think it's link to the countdown when bgs start
    SetRemainingTime(300000);

    if (m_ResetStatTimer > 5000)
    {
        m_ResetStatTimer = 0;
        for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
            if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                player->ResetAllPowers();
    }

    if (!(m_Events & BG_STARTING_EVENT_1))
    {
        m_Events |= BG_STARTING_EVENT_1;

        if (!FindBgMap())
        {
            sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::_ProcessJoin: map (map id: %u, instance id: %u) is not created!", m_MapId, m_InstanceID);
            EndNow();
            return;
        }

        // Setup here, only when at least one player has ported to the map
        if (!SetupBattleground() && !sBattlegroundMgr->isTesting())
        {
            EndNow();
            return;
        }

        StartingEventCloseDoors();
        SetStartDelayTime(StartDelayTimes[BG_STARTING_EVENT_FIRST]);
        SetCountdownTimer(StartDelayTimes[BG_STARTING_EVENT_FIRST]);
        // First start warning - 2 or 1 minute
        SendMessageToAll(StartMessageIds[BG_STARTING_EVENT_FIRST], CHAT_MSG_BG_SYSTEM_NEUTRAL);
    }
    // After 1 minute or 30 seconds, warning is signaled
    else if (GetStartDelayTime() <= StartDelayTimes[BG_STARTING_EVENT_SECOND] && !(m_Events & BG_STARTING_EVENT_2))
    {
        m_Events |= BG_STARTING_EVENT_2;
        SendMessageToAll(StartMessageIds[BG_STARTING_EVENT_SECOND], CHAT_MSG_BG_SYSTEM_NEUTRAL);
        SendCountdownTimer();
    }
    // After 30 or 15 seconds, warning is signaled
    else if (GetStartDelayTime() <= StartDelayTimes[BG_STARTING_EVENT_THIRD] && !(m_Events & BG_STARTING_EVENT_3))
    {
        m_Events |= BG_STARTING_EVENT_3;
        SendMessageToAll(StartMessageIds[BG_STARTING_EVENT_THIRD], CHAT_MSG_BG_SYSTEM_NEUTRAL);
    }
    // Delay expired (after 2 or 1 minute)
    else if (GetStartDelayTime() <= 0 && !(m_Events & BG_STARTING_EVENT_4))
    {
        m_Events |= BG_STARTING_EVENT_4;

        StartingEventOpenDoors();
        DespawnCrystals();

        /// Remove some auras when doors open
        for (BattlegroundPlayerMap::const_iterator l_Itr = m_Players.begin(); l_Itr != m_Players.end(); ++l_Itr)
        {
            Player* l_Player = _GetPlayer(l_Itr, "RemoveAura");
            if (l_Player)
            {
                l_Player->RemoveAura(115867); ///< Mana Tea
                l_Player->RemoveAura(125195); ///< Tigereye Brew
                l_Player->RemoveAura(128939); ///< Elusive Brew
            }
        }

        SendWarningToAll(StartMessageIds[BG_STARTING_EVENT_FOURTH]);
        SetStatus(STATUS_IN_PROGRESS);
        SetStartDelayTime(StartDelayTimes[BG_STARTING_EVENT_FOURTH]);

        // Remove preparation
        if (isArena())
        {
            // TODO : add arena sound PlaySoundToAll(SOUND_ARENA_START);
            for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
            {
                if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                {
                    // BG Status packet
                    WorldPacket status;
                    BattlegroundQueueTypeId bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(m_TypeID, GetArenaType());
                    uint32 queueSlot = player->GetBattlegroundQueueIndex(bgQueueTypeId);
                    sBattlegroundMgr->BuildBattlegroundStatusPacket(&status, this, player, queueSlot, STATUS_IN_PROGRESS, player->GetBattlegroundQueueJoinTime(BATTLEGROUND_AA), GetElapsedTime(), GetArenaType());
                    player->GetSession()->SendPacket(&status);

                    player->RemoveAurasDueToSpell(SPELL_ARENA_PREPARATION);
                    player->ResetAllPowers();

                    // remove auras with duration lower than 30s
                    Unit::AuraApplicationMap & auraMap = player->GetAppliedAuras();
                    for (Unit::AuraApplicationMap::iterator iter = auraMap.begin(); iter != auraMap.end();)
                    {
                        AuraApplication * aurApp = iter->second;
                        Aura* aura = aurApp->GetBase();
                        if (!aura->IsPermanent()
                            && aura->GetDuration() <= 30*IN_MILLISECONDS
                            && aurApp->IsPositive()
                            && (!(aura->GetSpellInfo()->Attributes & SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY))
                            && (!aura->HasEffectType(SPELL_AURA_MOD_INVISIBILITY)))
                            player->RemoveAura(iter);
                        else
                            ++iter;
                    }
                }
            }

            CheckArenaWinConditions();
            m_canChangeRate = true;
        }
        else
        {
            PlaySoundToAll(SOUND_BG_START);

            for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
            {
                if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                {
                    player->RemoveAurasDueToSpell(SPELL_PREPARATION);
                    player->RemoveAurasDueToSpell(SPELL_ARENA_PREPARATION);
                    player->ResetAllPowers();

                    if (IsRatedBG())
                    {
                        // remove auras with duration lower than 30s
                        Unit::AuraApplicationMap & auraMap = player->GetAppliedAuras();
                        for (Unit::AuraApplicationMap::iterator iter = auraMap.begin(); iter != auraMap.end();)
                        {
                            AuraApplication * aurApp = iter->second;
                            Aura* aura = aurApp->GetBase();
                            if (!aura->IsPermanent()
                                && aura->GetDuration() <= 30 * IN_MILLISECONDS
                                && aurApp->IsPositive()
                                && (!(aura->GetSpellInfo()->Attributes & SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY))
                                && (!aura->HasEffectType(SPELL_AURA_MOD_INVISIBILITY)))
                                player->RemoveAura(iter);
                            else
                                ++iter;
                        }
                    }
                }
            }
            std::string const& bgName = GetName();
            // Announce BG starting
            if (sWorld->getBoolConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_ENABLE))
            {
                if (IsRatedBG())
                {
                    sWorld->SendWorldText(LANG_RBG_STARTED_ANNOUNCE_WORLD, bgName.c_str(), GetMinLevel(), GetMaxLevel());
                }
                else
                {
                    sWorld->SendWorldText(LANG_BG_STARTED_ANNOUNCE_WORLD, bgName.c_str(), GetMinLevel(), GetMaxLevel());
                }
            }
        }
    }

    if (m_EndTime > 0 && (m_EndTime -= diff) > 0)
        m_EndTime -= diff;
}

inline void Battleground::_ProcessLeave(uint32 diff)
{
    // *********************************************************
    // ***           BATTLEGROUND ENDING SYSTEM              ***
    // *********************************************************
    // remove all players from battleground after 2 minutes
    m_EndTime -= diff;
    if (m_EndTime <= 0)
    {
        m_EndTime = 0;
        BattlegroundPlayerMap::iterator itr, next;
        for (itr = m_Players.begin(); itr != m_Players.end(); itr = next)
        {
            next = itr;
            ++next;
            //itr is erased here!
            RemovePlayerAtLeave(itr->first, true, true);// remove player from BG
            // do not change any battleground's private variables
        }
    }
}

inline Player* Battleground::_GetPlayer(uint64 guid, bool offlineRemove, const char* context) const
{
    Player* player = NULL;
    if (!offlineRemove)
    {
        player = ObjectAccessor::FindPlayer(guid);
        if (!player)
            sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::%s: player (GUID: %u) not found for BG (map: %u, instance id: %u)!",
                context, GUID_LOPART(guid), m_MapId, m_InstanceID);
    }
    return player;
}

inline Player* Battleground::_GetPlayer(BattlegroundPlayerMap::iterator itr, const char* context)
{
    return _GetPlayer(itr->first, itr->second.OfflineRemoveTime, context);
}

inline Player* Battleground::_GetPlayer(BattlegroundPlayerMap::const_iterator itr, const char* context) const
{
    return _GetPlayer(itr->first, itr->second.OfflineRemoveTime, context);
}

inline Player* Battleground::_GetPlayerForTeam(uint32 teamId, BattlegroundPlayerMap::const_iterator itr, const char* context) const
{
    Player* player = _GetPlayer(itr, context);
    if (player)
    {
        uint32 team = itr->second.Team;
        if (!team)
            team = player->GetTeam();
        if (team != teamId)
            player = NULL;
    }
    return player;
}

void Battleground::SetTeamStartLoc(uint32 TeamID, float X, float Y, float Z, float O)
{
    BattlegroundTeamId idx = GetTeamIndexByTeamId(TeamID);
    m_TeamStartLocX[idx] = X;
    m_TeamStartLocY[idx] = Y;
    m_TeamStartLocZ[idx] = Z;
    m_TeamStartLocO[idx] = O;
}

void Battleground::SendPacketToAll(WorldPacket* packet)
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = _GetPlayer(itr, "SendPacketToAll"))
            player->GetSession()->SendPacket(packet);
}

void Battleground::SendPacketToTeam(uint32 TeamID, WorldPacket* packet, Player* sender, bool self)
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = _GetPlayerForTeam(TeamID, itr, "SendPacketToTeam"))
            if (self || sender != player)
                player->GetSession()->SendPacket(packet);
}

void Battleground::PlaySoundToAll(uint32 SoundID)
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (Player* player = _GetPlayer(itr, "SendPacketToAll"))
            player->SendPlaySound(SoundID, true);
    }
}

void Battleground::PlaySoundToTeam(uint32 SoundID, uint32 TeamID)
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = _GetPlayerForTeam(TeamID, itr, "PlaySoundToTeam"))
        {
            player->SendPlaySound(SoundID, true);
        }
}

void Battleground::CastSpellOnTeam(uint32 SpellID, uint32 TeamID)
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = _GetPlayerForTeam(TeamID, itr, "CastSpellOnTeam"))
            player->CastSpell(player, SpellID, true);
}

void Battleground::RemoveAuraOnTeam(uint32 SpellID, uint32 TeamID)
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = _GetPlayerForTeam(TeamID, itr, "RemoveAuraOnTeam"))
            player->RemoveAura(SpellID);
}

void Battleground::YellToAll(Creature* creature, const char* text, uint32 language)
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = _GetPlayer(itr, "YellToAll"))
        {
            WorldPacket data;
            creature->BuildMonsterChat(&data, CHAT_MSG_MONSTER_YELL, text, language, creature->GetName(), itr->first);
            player->GetSession()->SendPacket(&data);
        }
}

void Battleground::RewardHonorToTeam(uint32 Honor, uint32 TeamID)
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = _GetPlayerForTeam(TeamID, itr, "RewardHonorToTeam"))
            UpdatePlayerScore(player, NULL, SCORE_BONUS_HONOR, Honor);
}

void Battleground::RewardReputationToTeam(uint32 faction_id, uint32 Reputation, uint32 TeamID)
{
    if (IsRatedBG())
        return;

    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id))
        for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            if (Player* player = _GetPlayerForTeam(TeamID, itr, "RewardReputationToTeam"))
                player->GetReputationMgr().ModifyReputation(factionEntry, Reputation);
}

void Battleground::UpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data;
    sBattlegroundMgr->BuildUpdateWorldStatePacket(&data, Field, Value);
    SendPacketToAll(&data);
}

void Battleground::UpdateWorldStateForPlayer(uint32 Field, uint32 Value, Player* Source)
{
    WorldPacket data;
    sBattlegroundMgr->BuildUpdateWorldStatePacket(&data, Field, Value);
    Source->GetSession()->SendPacket(&data);
}

void Battleground::EndBattleground(uint32 p_Winner)
{
    RemoveFromBGFreeSlotQueue();

    Group* winner_team = NULL;
    Group* loser_team = NULL;
    uint32 loser_team_rating = 0;
    uint32 loser_matchmaker_rating = 0;
    int32  loser_change = 0;
    int32  loser_matchmaker_change = 0;
    uint32 winner_team_rating = 0;
    uint32 winner_matchmaker_rating = 0;
    int32  winner_change = 0;
    int32  winner_matchmaker_change = 0;
    WorldPacket data;
    int32 winmsg_id = 0;

    if (p_Winner == ALLIANCE)
    {
        winmsg_id = isBattleground() ? LANG_BG_A_WINS : LANG_ARENA_GOLD_WINS;

        PlaySoundToAll(SOUND_ALLIANCE_WINS);                // alliance wins sound

        SetWinner(WINNER_ALLIANCE);
    }
    else if (p_Winner == HORDE)
    {
        winmsg_id = isBattleground() ? LANG_BG_H_WINS : LANG_ARENA_GREEN_WINS;

        PlaySoundToAll(SOUND_HORDE_WINS);                   // horde wins sound

        SetWinner(WINNER_HORDE);
    }
    else
        SetWinner(3);

    SetStatus(STATUS_WAIT_LEAVE);
    //we must set it this way, because end time is sent in packet!
    SetRemainingTime(TIME_AUTOCLOSE_BATTLEGROUND);

    loser_team = GetBgRaid(GetOtherTeam(p_Winner));
    winner_team = GetBgRaid(p_Winner);

    if (IsRatedBG())
    {
        if (winner_team && loser_team && winner_team != loser_team && GetWinner() != 3)
        {
            loser_matchmaker_rating = GetArenaMatchmakerRating(GetOtherTeam(p_Winner), SLOT_RBG);
            winner_matchmaker_rating = GetArenaMatchmakerRating(p_Winner, SLOT_RBG);
        }

        /// Handle guild challenges
        for (BattlegroundPlayerMap::iterator l_Itr = m_Players.begin(); l_Itr != m_Players.end(); l_Itr++)
        {
            if (isArena())
                break;

            Player* l_Player = _GetPlayer(l_Itr, "EndBattleground");
            if (!l_Player)
                continue;

            if (l_Player->GetGroup() && l_Player->GetGroup()->IsGuildGroup(0, true, true))
            {
                if (Guild* l_Guild = l_Player->GetGuild())
                {
                    l_Guild->CompleteGuildChallenge(GuildChallengeType::ChallengeRatedBG);
                    break;
                }
            }
        }
    }

    // arena rating calculation
    if (isArena() && isRated() && !IsWargame())
    {
        uint8 slot = Arena::GetSlotByType(GetArenaType());

        if (winner_team && loser_team && winner_team != loser_team && GetWinner() != 3)
        {
            winner_matchmaker_rating = GetArenaMatchmakerRating(p_Winner, slot);
            winner_team_rating = winner_team->GetRating(slot);
            loser_team_rating = loser_team->GetRating(slot);
            loser_matchmaker_rating = GetArenaMatchmakerRating(GetOtherTeam(p_Winner), slot);
            loser_team->LostAgainst(loser_matchmaker_rating, winner_matchmaker_rating, loser_change, slot);

            if (m_canChangeRate)
            {
                winner_team->WonAgainst(winner_matchmaker_rating, loser_matchmaker_rating, winner_change, slot);
                SetArenaMatchmakerRating(p_Winner, winner_matchmaker_rating + winner_matchmaker_change);
                SetArenaTeamRatingChangeForTeam(p_Winner, winner_change);
            }

            SetArenaMatchmakerRating(GetOtherTeam(p_Winner), loser_matchmaker_rating + loser_matchmaker_change);
            SetArenaTeamRatingChangeForTeam(GetOtherTeam(p_Winner), loser_change);

            // bg team that the client expects is different to TeamId
            // alliance 1, horde 0
            uint8 winnerTeam = p_Winner == ALLIANCE ? BG_TEAM_ALLIANCE : BG_TEAM_HORDE;
            uint8 loserTeam = p_Winner == ALLIANCE ? BG_TEAM_HORDE : BG_TEAM_ALLIANCE;

            _arenaTeamScores[winnerTeam].Assign(winner_team_rating, winner_team_rating + winner_change, winner_matchmaker_rating);
            _arenaTeamScores[loserTeam].Assign(loser_team_rating, loser_team_rating + loser_change, loser_matchmaker_rating);


            if (sWorld->getBoolConfig(CONFIG_ARENA_LOG_EXTENDED_INFO))
                for (Battleground::BattlegroundScoreMap::const_iterator itr = GetPlayerScoresBegin(); itr != GetPlayerScoresEnd(); ++itr)
                    if (Player* l_Player = ObjectAccessor::FindPlayer(itr->first))
                        sLog->outArena("Statistics match Type: %u for %s (GUID: " UI64FMTD ", IP: %s): %u damage, %u healing, %u killing blows", m_ArenaType, l_Player->GetName(), itr->first, l_Player->GetSession()->GetRemoteAddress().c_str(), itr->second->DamageDone, itr->second->HealingDone, itr->second->KillingBlows);
        }
        // Deduct 12 points from each teams arena-rating if there are no winners after 45+2 minutes
        else if (GetWinner() == 3)
        {
            SetArenaTeamRatingChangeForTeam(ALLIANCE, ARENA_TIMELIMIT_POINTS_LOSS);
            SetArenaTeamRatingChangeForTeam(HORDE, ARENA_TIMELIMIT_POINTS_LOSS);
            if (winner_team)
                winner_team->FinishGame(ARENA_TIMELIMIT_POINTS_LOSS, slot);
            if (loser_team)
                loser_team->FinishGame(ARENA_TIMELIMIT_POINTS_LOSS, slot);
        }
    }

    bool guildAwarded = false;
    uint8 aliveWinners = GetAlivePlayersCountByTeam(p_Winner);
    for (BattlegroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        uint32 l_Team = itr->second.Team;

        if (itr->second.OfflineRemoveTime)
        {
            //if rated arena match - make member lost!
            if (isArena() && isRated() && winner_team && loser_team && winner_team != loser_team && GetWinner() != 3)
            {
                if (l_Team == p_Winner)
                    winner_team->OfflineMemberLost(itr->first, loser_matchmaker_rating, Arena::GetSlotByType(GetArenaType()), winner_matchmaker_change);
                else
                    loser_team->OfflineMemberLost(itr->first, winner_matchmaker_rating, Arena::GetSlotByType(GetArenaType()), loser_matchmaker_change);
            }
            continue;
        }

        Player* l_Player = _GetPlayer(itr, "EndBattleground");
        if (!l_Player)
            continue;

        /// No hook there, only way to send killed credit to player for those quests
        if (l_Team == p_Winner)
        {
            switch (GetTypeID(true))
            {
            case BATTLEGROUND_KT:
            {
                switch (l_Player->GetTeamId())
                {
                case TEAM_ALLIANCE:
                    if (l_Player->GetQuestStatus(32389) == QUEST_STATUS_INCOMPLETE)
                        l_Player->KilledMonsterCredit(68931);
                    break;
                case TEAM_HORDE:
                    if (l_Player->GetQuestStatus(32431) == QUEST_STATUS_INCOMPLETE)
                        l_Player->KilledMonsterCredit(68931);
                    break;
                default:
                    break;
                }
                break;
            }
            case BATTLEGROUND_SSM:
            {
                switch (l_Player->GetTeamId())
                {
                case TEAM_ALLIANCE:
                    if (l_Player->GetQuestStatus(32389) == QUEST_STATUS_INCOMPLETE)
                        l_Player->KilledMonsterCredit(68932);
                    break;
                case TEAM_HORDE:
                    if (l_Player->GetQuestStatus(32431) == QUEST_STATUS_INCOMPLETE)
                        l_Player->KilledMonsterCredit(68932);
                    break;
                default:
                    break;
                }
                break;
            }
            default:
                break;
            }

        }

        // should remove spirit of redemption
        if (l_Player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            l_Player->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

        // Last standing - Rated 5v5 arena & be solely alive player
        if (l_Team == p_Winner && isArena() && isRated() && !IsWargame() && GetArenaType() == ARENA_TYPE_5v5 && aliveWinners == 1 && l_Player->isAlive())
            l_Player->CastSpell(l_Player, SPELL_THE_LAST_STANDING, true);

        if (!l_Player->isAlive())
        {
            l_Player->ResurrectPlayer(1.0f);
            l_Player->SpawnCorpseBones();
        }
        else
        {
            //needed cause else in av some creatures will kill the players at the end
            l_Player->CombatStop();
            l_Player->getHostileRefManager().deleteReferences();
        }

        //this line is obsolete - team is set ALWAYS
        //if (!team) team = player->GetTeam();

        // per player calculation
        if (isArena() && isRated() && !IsWargame() && winner_team && loser_team && winner_team != loser_team && GetWinner() != 3)
        {
            uint8 slot = Arena::GetSlotByType(GetArenaType());
            if (l_Team == p_Winner)
            {
                // update achievement BEFORE personal rating update
                uint32 rating = l_Player->GetArenaPersonalRating(slot);
                l_Player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA, rating ? rating : 1);
                l_Player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA, GetMapId());
                if (GetArenaType() != ARENA_TYPE_5v5)
                    l_Player->ModifyCurrency(CURRENCY_TYPE_CONQUEST_META_ARENA, sWorld->getIntConfig(CONFIG_CURRENCY_CONQUEST_POINTS_ARENA_REWARD));
            }
            else
            {
                // Member lost
                // Update personal rating
                /*int32 mod = Arena::GetRatingMod(player->GetArenaPersonalRating(slot), winner_matchmaker_rating, false);
                player->SetArenaPersonalRating(player->GetArenaPersonalRating(slot) + mod, slot);

                // Update matchmaker rating
                player->SetArenaMatchMakerRating(slot, player->GetArenaMatchMakerRating(slot) + loser_matchmaker_change);

                // Update personal played stats
                player->IncrementWeekGames(slot);
                player->IncrementSeasonGames(slot);*/

                // Arena lost => reset the win_rated_arena having the "no_lose" condition
                l_Player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA, ACHIEVEMENT_CRITERIA_CONDITION_NO_LOSE);
            }
        }

        if (IsRatedBG() && winner_team && loser_team && winner_team != loser_team && GetWinner() != 3)
        {
            if (l_Player->GetArenaPersonalRating(SLOT_RBG) < l_Player->GetArenaMatchMakerRating(SLOT_RBG))
            {
                int32 rating_change = Arena::GetRatingMod(l_Player->GetArenaPersonalRating(SLOT_RBG), l_Player->GetBGTeam() == p_Winner ? loser_matchmaker_rating : winner_matchmaker_rating, true);

                if (l_Player->GetArenaPersonalRating(SLOT_RBG) < 1000)
                    rating_change = 192;

                l_Player->SetArenaPersonalRating(SLOT_RBG, l_Player->GetArenaPersonalRating(SLOT_RBG) + rating_change);
            }

            if (l_Player->GetBGTeam() == p_Winner)
            {
                l_Player->ModifyCurrency(CURRENCY_TYPE_CONQUEST_META_RBG, sWorld->getIntConfig(CONFIG_CURRENCY_CONQUEST_POINTS_RATED_BG_REWARD));

                int32 MMRating_mod = Arena::GetMatchmakerRatingMod(winner_matchmaker_rating, loser_matchmaker_rating, true);
                l_Player->SetArenaMatchMakerRating(SLOT_RBG, l_Player->GetArenaMatchMakerRating(SLOT_RBG) + MMRating_mod);

                l_Player->IncrementWeekWins(SLOT_RBG);
                l_Player->IncrementSeasonWins(SLOT_RBG);
                l_Player->IncrementWeekGames(SLOT_RBG);
                l_Player->IncrementSeasonGames(SLOT_RBG);

                l_Player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_BG_RATING, l_Player->GetArenaPersonalRating(SLOT_RBG));
                l_Player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND, 1);
            }
            else
            {
                int32 MMRating_mod = Arena::GetMatchmakerRatingMod(loser_matchmaker_rating, winner_matchmaker_rating, false);
                l_Player->SetArenaMatchMakerRating(SLOT_RBG, l_Player->GetArenaMatchMakerRating(SLOT_RBG) + MMRating_mod);
                l_Player->IncrementWeekGames(SLOT_RBG);
                l_Player->IncrementSeasonGames(SLOT_RBG);
            }
        }

        uint32 winner_bonus = l_Player->GetRandomWinner() ? BG_REWARD_WINNER_HONOR_LAST : BG_REWARD_WINNER_HONOR_FIRST;
        uint32 loser_bonus = l_Player->GetRandomWinner() ? BG_REWARD_LOSER_HONOR_LAST : BG_REWARD_LOSER_HONOR_FIRST;

        // remove temporary currency bonus auras before rewarding l_Player
        l_Player->RemoveAura(SPELL_HONORABLE_DEFENDER_25Y);
        l_Player->RemoveAura(SPELL_HONORABLE_DEFENDER_60Y);

        // Reward winner team
        if (l_Team == p_Winner)
        {
            if (!isArena() && !IsRatedBG())
            {
                if ((IsRandom() || BattlegroundMgr::IsBGWeekend(GetTypeID())) && !IsWargame())
                {
                    UpdatePlayerScore(l_Player, NULL, SCORE_BONUS_HONOR, winner_bonus);
                    if (!l_Player->GetRandomWinner())
                    {
                        // 100cp awarded for the first rated battleground won each day 
                        l_Player->ModifyCurrency(CURRENCY_TYPE_CONQUEST_META_RANDOM_BG, BG_REWARD_WINNER_CONQUEST_FIRST);
                        l_Player->SetRandomWinner(true);
                    }
                    else // 50cp awarded for each non-rated battleground won 
                        l_Player->ModifyCurrency(CURRENCY_TYPE_CONQUEST_META_RANDOM_BG, BG_REWARD_WINNER_CONQUEST_LAST);
                }
            }

            l_Player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_BG, 1);
            if (!guildAwarded)
            {
                guildAwarded = true;
                if (uint32 guildId = GetBgMap()->GetOwnerGuildId(l_Player->GetTeam()))
                    if (Guild* guild = sGuildMgr->GetGuildById(guildId))
                    {
                        guild->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_BG, 1, 0, 0, NULL, l_Player);
                        if (isArena() && isRated() && winner_team && loser_team && winner_team != loser_team)
                            guild->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA, std::max<uint32>(winner_team->GetRating(Arena::GetSlotByType(GetArenaType())), 1), 0, 0, NULL, l_Player);
                    }
            }
        }
        else
        {
            if (IsRandom() || BattlegroundMgr::IsBGWeekend(GetTypeID()))
                UpdatePlayerScore(l_Player, NULL, SCORE_BONUS_HONOR, loser_bonus, !IsWargame());
        }

        l_Player->ResetAllPowers();
        l_Player->CombatStopWithPets(true);
        l_Player->BG_RestoreTalentInfo();
        BlockMovement(l_Player);

        sBattlegroundMgr->BuildPvpLogDataPacket(&data, this);
        l_Player->GetSession()->SendPacket(&data);

        BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());
        if (isArena())
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, l_Player, l_Player->GetBattlegroundQueueIndex(bgQueueTypeId), STATUS_WAIT_LEAVE, l_Player->GetBattlegroundQueueJoinTime(BATTLEGROUND_AA), GetElapsedTime(), GetArenaType());
        else
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, l_Player, l_Player->GetBattlegroundQueueIndex(bgQueueTypeId), STATUS_WAIT_LEAVE, l_Player->GetBattlegroundQueueJoinTime(GetTypeID()), GetElapsedTime(), GetArenaType());
        l_Player->GetSession()->SendPacket(&data);

        if (!IsWargame())
            l_Player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND, 1);
    }

    if (winmsg_id)
        SendMessageToAll(winmsg_id, CHAT_MSG_BG_SYSTEM_NEUTRAL);
}

uint32 Battleground::GetBonusHonorFromKill(uint32 kills) const
{
    //variable kills means how many honorable kills you scored (so we need kills * honor_for_one_kill)
    uint32 maxLevel = std::min(GetMaxLevel(), 90U);
    return JadeCore::Honor::hk_honor_at_level(maxLevel, float(kills));
}

void Battleground::BlockMovement(Player* player)
{
    player->SetClientControl(player, 0);                          // movement disabled NOTE: the effect will be automatically removed by client when the player is teleported from the battleground, so no need to send with uint8(1) in RemovePlayerAtLeave()
}

void Battleground::RemovePlayerAtLeave(uint64 guid, bool Transport, bool SendPacket)
{
    uint32 team = GetPlayerTeam(guid);
    bool participant = false;
    // Remove from lists/maps
    BattlegroundPlayerMap::iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
    {
        UpdatePlayersCountByTeam(team, true);               // -1 player
        m_Players.erase(itr);
        // check if the player was a participant of the match, or only entered through gm command (goname)
        participant = true;
    }

    BattlegroundScoreMap::iterator itr2 = PlayerScores.find(guid);
    if (itr2 != PlayerScores.end())
    {
        delete itr2->second;                                // delete player's score
        PlayerScores.erase(itr2);
    }

    RemovePlayerFromResurrectQueue(guid);

    Player* player = ObjectAccessor::FindPlayer(guid);

    // should remove spirit of redemption
    if (player)
    {
        if (player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            player->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

        if (!player->isAlive())                              // resurrect on exit
        {
            player->ResurrectPlayer(1.0f);
            player->SpawnCorpseBones();
        }

        player->BG_RestoreTalentInfo();
    }

    RemovePlayer(player, guid, team);                           // BG subclass specific code

    BattlegroundTypeId bgTypeId = GetTypeID();
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());

    if (participant) // if the player was a match participant, remove auras, calc rating, update queue
    {
        if (player)
        {
            player->ClearAfkReports();

            if (!team) team = player->GetTeam();

            // if arena, remove the specific arena auras
            if (isArena())
            {
                bgTypeId=BATTLEGROUND_AA;                   // set the bg type to all arenas (it will be used for queue refreshing)

                // unsummon current and summon old pet if there was one and there isn't a current pet
                if (Pet* pet = player->GetPet())
                    player->RemovePet(pet, PET_SLOT_ACTUAL_PET_SLOT, false, pet->m_Stampeded);
                else
                    player->RemovePet(NULL, PET_SLOT_ACTUAL_PET_SLOT, false, true);

                player->ResummonPetTemporaryUnSummonedIfAny();

                if (isRated() && GetStatus() == STATUS_IN_PROGRESS)
                {
                    //left a rated match while the encounter was in progress, consider as loser
                    Group* winner_group = GetBgRaid(GetOtherTeam(team));
                    Group* loser_group = GetBgRaid(team);

                    if (winner_group && loser_group && winner_group != loser_group)
                    {
                        uint8 slot = Arena::GetSlotByType(GetArenaType());

                        // Update personal rating
                        int32 mod = Arena::GetRatingMod(player->GetArenaPersonalRating(slot), GetArenaMatchmakerRating(GetOtherTeam(team), slot), false);
                        player->SetArenaPersonalRating(slot, player->GetArenaPersonalRating(slot) + mod);

                        // Update matchmaker rating
                        player->SetArenaMatchMakerRating(slot, player->GetArenaMatchMakerRating(slot) -12);

                        // Update personal played stats
                        player->IncrementWeekGames(slot);
                        player->IncrementSeasonGames(slot);
                    }
                }
            }
            else if (IsRatedBG())
            {
                if (GetStatus() == STATUS_IN_PROGRESS)
                {
                    //left a rated match while the encounter was in progress, consider as loser
                    Group* winner_group = GetBgRaid(GetOtherTeam(team));
                    Group* loser_group = GetBgRaid(team);

                    if (winner_group && loser_group && winner_group != loser_group)
                    {
                        uint8 slot = SLOT_RBG;

                        // Update personal rating
                        int32 mod = Arena::GetRatingMod(player->GetArenaPersonalRating(slot), GetArenaMatchmakerRating(GetOtherTeam(team), slot), false);
                        player->SetArenaPersonalRating(slot, player->GetArenaPersonalRating(slot) + mod);

                        // Update matchmaker rating
                        player->SetArenaMatchMakerRating(slot, player->GetArenaMatchMakerRating(slot) -12);

                        // Update personal played stats
                        player->IncrementWeekGames(slot);
                        player->IncrementSeasonGames(slot);
                    }
                }
            }
            if (SendPacket)
            {
                WorldPacket data;
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, player, player->GetBattlegroundQueueIndex(bgQueueTypeId), STATUS_NONE, player->GetBattlegroundQueueJoinTime(bgTypeId), 0, 0);
                player->GetSession()->SendPacket(&data);
            }

            // this call is important, because player, when joins to battleground, this method is not called, so it must be called when leaving bg
            player->RemoveBattlegroundQueueId(bgQueueTypeId);
        }
        else
        // removing offline participant
        {
            if (isRated() && GetStatus() == STATUS_IN_PROGRESS)
            {
                //left a rated match while the encounter was in progress, consider as loser
                Group* others_group = GetBgRaid(GetOtherTeam(team));
                Group* players_group = GetBgRaid(team);
                if (others_group && players_group)
                    players_group->OfflineMemberLost(guid, GetArenaMatchmakerRating(GetOtherTeam(team), Arena::GetSlotByType(GetArenaType())), Arena::GetSlotByType(GetArenaType()));
            }
        }

        // remove from raid group if player is member
        if (Group* group = GetBgRaid(team))
        {
            if (!group->RemoveMember(guid))                // group was disbanded
            {
                SetBgRaid(team, NULL);
            }
        }
        DecreaseInvitedCount(team);
        //we should update battleground queue, but only if bg isn't ending
        if (isBattleground() && GetStatus() < STATUS_WAIT_LEAVE)
        {
            // a player has left the battleground, so there are free slots -> add to queue
            AddToBGFreeSlotQueue();
            sBattlegroundMgr->ScheduleQueueUpdate(0, 0, bgQueueTypeId, bgTypeId, GetBracketId());
        }
        // Let others know
        WorldPacket data;
        sBattlegroundMgr->BuildPlayerLeftBattlegroundPacket(&data, guid);
        SendPacketToTeam(team, &data, player, false);
    }

    if (player)
    {
        // Do next only if found in battleground
        player->SetBattlegroundId(0, BATTLEGROUND_TYPE_NONE);  // We're not in BG.
        // reset destination bg team
        player->SetBGTeam(0);
        if (IsRatedBG())
        {
            player->RemoveAura(81748);
            player->RemoveAura(81744);
        }
        player->SetByteValue(PLAYER_BYTES_3, 3, 0);
        player->RemoveBattlegroundQueueJoinTime(bgTypeId);

        if (Transport)
            player->TeleportToBGEntryPoint();

        sLog->outInfo(LOG_FILTER_BATTLEGROUND, "BATTLEGROUND: Removed player %s from Battleground.", player->GetName());
    }

    //battleground object will be deleted next Battleground::Update() call
}

// this method is called when no players remains in battleground
void Battleground::Reset()
{
    SetWinner(WINNER_NONE);
    SetStatus(STATUS_WAIT_QUEUE);
    SetElapsedTime(0);
    SetRemainingTime(0);
    SetLastResurrectTime(0);
    SetArenaType(0);
    SetRated(false);

    m_Events = 0;

    if (m_InvitedAlliance > 0 || m_InvitedHorde > 0)
        sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::Reset: one of the counters is not 0 (alliance: %u, horde: %u) for BG (map: %u, instance id: %u)!",
            m_InvitedAlliance, m_InvitedHorde, m_MapId, m_InstanceID);

    m_InvitedAlliance = 0;
    m_InvitedHorde = 0;
    m_InBGFreeSlotQueue = false;

    m_Players.clear();

    for (BattlegroundScoreMap::const_iterator itr = PlayerScores.begin(); itr != PlayerScores.end(); ++itr)
        delete itr->second;
    PlayerScores.clear();

    for (uint8 i = 0; i < BG_TEAMS_COUNT; ++i)
        _arenaTeamScores[i].Reset();

    ResetBGSubclass();
}

void Battleground::StartBattleground()
{
    SetElapsedTime(0);
    SetLastResurrectTime(0);
    // add BG to free slot queue
    AddToBGFreeSlotQueue();

    // add bg to update list
    // This must be done here, because we need to have already invited some players when first BG::Update() method is executed
    // and it doesn't matter if we call StartBattleground() more times, because m_Battlegrounds is a map and instance id never changes
    sBattlegroundMgr->AddBattleground(GetInstanceID(), GetTypeID(), this);
    if (m_IsRated)
        sLog->outArena("Arena match type: %u for Team1Id: %u - Team2Id: %u started.", m_ArenaType, m_ArenaTeamIds[BG_TEAM_ALLIANCE], m_ArenaTeamIds[BG_TEAM_HORDE]);
}

uint32 Battleground::BuildArenaOpponentSpecializations(WorldPacket* data, uint32 team)
{
    uint32 opponent_count = 0;
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (_GetPlayerForTeam(team, itr, "BuildArenaOpponentSpecializations"))
            opponent_count++;

    if (!opponent_count)
        return 0;

    ByteBuffer dataBuffer(opponent_count*12);
    data->WriteBits(opponent_count, 21);

    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (Player* pl = _GetPlayerForTeam(team, itr, "BuildArenaOpponentSpecializations"))
        {
            ObjectGuid guid = pl->GetGUID();
            uint8 bitOrder[8] = { 0, 2, 1, 6, 7, 3, 5, 4 };
            data->WriteBitInOrder(guid, bitOrder);

            dataBuffer.WriteByteSeq(guid[5]);
            dataBuffer.WriteByteSeq(guid[1]);
            dataBuffer.WriteByteSeq(guid[7]);
            dataBuffer << int32(pl->GetSpecializationId(pl->GetActiveSpec()));
            dataBuffer.WriteByteSeq(guid[6]);
            dataBuffer.WriteByteSeq(guid[3]);
            dataBuffer.WriteByteSeq(guid[0]);
            dataBuffer.WriteByteSeq(guid[2]);
            dataBuffer.WriteByteSeq(guid[4]);  
        }
    }

    if (dataBuffer.size())
        data->append(dataBuffer);

    return opponent_count;
}

void Battleground::CastDampeningToAllPlayers()
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (Player* player = _GetPlayer(itr, "CastDampeningToAllPlayers"))
        {
            player->CastSpell(player, 110310, true);
            if (Pet* pet = player->GetPet())
                pet->CastSpell(pet, 110310, true);
        }
    }
}

void Battleground::UpdateBattleFatigue()
{
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (Player* player = _GetPlayer(itr, "UpdateBattleFatigue"))
        {
            const uint32 c_BattleFatigueEffect = 134735;

            if (auto aura = player->GetAura(c_BattleFatigueEffect))
                aura->RefreshDuration();
            else
                player->AddAura(c_BattleFatigueEffect, player);
        }
    }
}

void Battleground::AddPlayer(Player* player)
{
    // remove afk from player
    if (player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK))
        player->ToggleAFK();

    // score struct must be created in inherited class

    uint64 guid = player->GetGUID();
    uint32 team = player->GetBGTeam();

    BattlegroundPlayer bp;
    bp.OfflineRemoveTime = 0;
    bp.Team = team;

    // Add to list/maps
    m_Players[guid] = bp;

    UpdatePlayersCountByTeam(team, false);                  // +1 player

    WorldPacket data;
    sBattlegroundMgr->BuildPlayerJoinedBattlegroundPacket(&data, guid);
    SendPacketToTeam(team, &data, player, false);

    // BG Status packet
    BattlegroundQueueTypeId bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(m_TypeID, GetArenaType());
    uint32 queueSlot = player->GetBattlegroundQueueIndex(bgQueueTypeId);

    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (isArena())
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, player, queueSlot, STATUS_IN_PROGRESS, player->GetBattlegroundQueueJoinTime(BATTLEGROUND_AA), GetElapsedTime(), GetArenaType());
        else
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, player, queueSlot, STATUS_IN_PROGRESS, player->GetBattlegroundQueueJoinTime(m_TypeID), GetElapsedTime(), GetArenaType());
        player->GetSession()->SendPacket(&data);
    }

    player->Dismount();
    player->RemoveAurasByType(SPELL_AURA_MOUNTED);
    player->RemoveAurasByType(SPELL_AURA_FLY);

    // add arena specific auras
    if (isArena())
    {
        player->ResummonPetTemporaryUnSummonedIfAny();

        // Removing pet's buffs and debuffs which are not permanent on Arena enter
        if (Pet* pet = player->GetPet())
        {
            if (!pet->isAlive())
                pet->setDeathState(ALIVE);

            // Set pet at full health
            pet->SetHealth(pet->GetMaxHealth());

            pet->RemoveAllAuras();
            pet->CastPetAuras(true);

            player->RemoveArenaSpellCooldowns(true);
        }

        player->RemoveArenaEnchantments(TEMP_ENCHANTMENT_SLOT);
        if (team == ALLIANCE)                                // gold
        {
            if (player->GetTeam() == HORDE)
                player->CastSpell(player, SPELL_HORDE_GOLD_FLAG, true);
            else
                player->CastSpell(player, SPELL_ALLIANCE_GOLD_FLAG, true);
        }
        else                                                // green
        {
            if (player->GetTeam() == HORDE)
                player->CastSpell(player, SPELL_HORDE_GREEN_FLAG, true);
            else
                player->CastSpell(player, SPELL_ALLIANCE_GREEN_FLAG, true);
        }

        player->DestroyConjuredItems(true);
        player->UnsummonPetTemporaryIfAny();
        player->GetPetJournal()->UnSummonCurrentBattlePet(false);

        if (GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
        {
            player->CastSpell(player, SPELL_ARENA_PREPARATION, true);
            player->BG_DumpTalentInfo();
            player->ResetAllPowers();
            SendCountdownTimer();
        }

        /* send to enemy team info about as*/
        if (!player->isSpectator())
        {
            // Set arena faction client-side to display arena unit frame
            player->SetByteValue(PLAYER_BYTES_3, 3, player->GetBGTeam() == HORDE ? 0 : 1);

            WorldPacket team_packet(SMSG_ARENA_OPPONENT_SPECIALIZATIONS);
            if (uint32 count = BuildArenaOpponentSpecializations(&team_packet, player->GetBGTeam()))
                SendPacketToTeam(GetOtherTeam(player->GetBGTeam()), &team_packet);

            /* send to my team info about enemy team*/
            team_packet.Initialize(SMSG_ARENA_OPPONENT_SPECIALIZATIONS);
            if (uint32 count = BuildArenaOpponentSpecializations(&team_packet, GetOtherTeam(player->GetBGTeam())))
                SendPacketToTeam(player->GetBGTeam(), &team_packet);
         }
    }
    else
    {
        if (GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
        {
            player->CastSpell(player, SPELL_PREPARATION, true);   // reduces all mana cost of spells.
            player->CastSpell(player, SPELL_ARENA_PREPARATION, true);
            player->BG_DumpTalentInfo();
            SendCountdownTimer();
        }
    }

    player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);
    player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_BG, ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);
    player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE, ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);
    player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);
    player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL, ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);
    player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);
    player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA, ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);
    player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL, ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);
    player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE, ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);
    player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS, ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);
    player->GetAchievementMgr().ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL, ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP, GetMapId(), true);

    // setup BG group membership
    PlayerAddedToBGCheckIfBGIsRunning(player);
    AddOrSetPlayerToCorrectBgGroup(player, team);

    if (IsRatedBG())
    {
        if (player->GetTeam() != player->GetBGTeam())
            player->AddAura(player->GetBGTeam() == ALLIANCE ? 81748 : 81744, player);
    }

    // Log
    sLog->outInfo(LOG_FILTER_BATTLEGROUND, "BATTLEGROUND: Player %s joined the battle.", player->GetName());
}

// this method adds player to his team's bg group, or sets his correct group if player is already in bg group
void Battleground::AddOrSetPlayerToCorrectBgGroup(Player* player, uint32 team)
{
    uint64 playerGuid = player->GetGUID();
    Group* group = GetBgRaid(team);
    if (!group)                                      // first player joined
    {
        group = new Group;
        SetBgRaid(team, group);
        group->Create(player);
    }
    else                                            // raid already exist
    {
        if (group->IsMember(playerGuid))
        {
            uint8 subgroup = group->GetMemberGroup(playerGuid);
            player->SetBattlegroundOrBattlefieldRaid(group, subgroup);
        }
        else
        {
            group->AddMember(player);
            if (Group* originalGroup = player->GetOriginalGroup())
                if (originalGroup->IsLeader(playerGuid))
                {
                    group->ChangeLeader(playerGuid);
                    group->SendUpdate();
                }
        }
    }
}

// This method should be called when player logs into running battleground
void Battleground::EventPlayerLoggedIn(Player* player)
{
    if (IsRatedBG())
    {
        if (player->GetTeam() != player->GetBGTeam())
            player->AddAura(player->GetBGTeam() == ALLIANCE ? 81748 : 81744, player);
    }

    uint64 guid = player->GetGUID();
    // player is correct pointer
    for (std::deque<uint64>::iterator itr = m_OfflineQueue.begin(); itr != m_OfflineQueue.end(); ++itr)
    {
        if (*itr == guid)
        {
            m_OfflineQueue.erase(itr);
            break;
        }
    }
    m_Players[guid].OfflineRemoveTime = 0;
    PlayerAddedToBGCheckIfBGIsRunning(player);
    // if battleground is starting, then add preparation aura
    // we don't have to do that, because preparation aura isn't removed when player logs out
}

// This method should be called when player logs out from running battleground
void Battleground::EventPlayerLoggedOut(Player* player)
{
    uint64 guid = player->GetGUID();
    if (!IsPlayerInBattleground(guid))  // Check if this player really is in battleground (might be a GM who teleported inside)
        return;

    // player is correct pointer, it is checked in WorldSession::LogoutPlayer()
    m_OfflineQueue.push_back(player->GetGUID());
    m_Players[guid].OfflineRemoveTime = sWorld->GetGameTime() + MAX_OFFLINE_TIME;
    if (GetStatus() == STATUS_IN_PROGRESS || (GetStatus() == STATUS_WAIT_JOIN && isArena()))
    {
        if (!player->isSpectator())
        {
            uint32 playerTeam = GetPlayerTeam(guid);
            uint32 otherTeam = GetOtherTeam(playerTeam);

            // drop flag and handle other cleanups
            RemovePlayer(player, guid, playerTeam);

            if (player && IsRatedBG())
                player->setFactionForRace(player->getRace());

            // 1 player is logging out, if it is the last, then end arena!
            if (isArena())
                if (GetAlivePlayersCountByTeam(playerTeam) <= 1 && GetPlayersCountByTeam(otherTeam))
                    EndBattleground(otherTeam);
         }
    }
    
    player->BG_RestoreTalentInfo();
}

// This method should be called only once ... it adds pointer to queue
void Battleground::AddToBGFreeSlotQueue()
{
    // make sure to add only once
    if (!m_InBGFreeSlotQueue && isBattleground())
    {
        sBattlegroundMgr->BGFreeSlotQueue[m_TypeID].push_front(this);
        m_InBGFreeSlotQueue = true;
    }
}

// This method removes this battleground from free queue - it must be called when deleting battleground - not used now
void Battleground::RemoveFromBGFreeSlotQueue()
{
    // set to be able to re-add if needed
    m_InBGFreeSlotQueue = false;
    // uncomment this code when battlegrounds will work like instances
    for (BGFreeSlotQueueType::iterator itr = sBattlegroundMgr->BGFreeSlotQueue[m_TypeID].begin(); itr != sBattlegroundMgr->BGFreeSlotQueue[m_TypeID].end(); ++itr)
    {
        if ((*itr)->GetInstanceID() == m_InstanceID)
        {
            sBattlegroundMgr->BGFreeSlotQueue[m_TypeID].erase(itr);
            return;
        }
    }
}

// get the number of free slots for team
// returns the number how many players can join battleground to MaxPlayersPerTeam
uint32 Battleground::GetFreeSlotsForTeam(uint32 Team) const
{
    // if BG is starting ... invite anyone
    if (GetStatus() == STATUS_WAIT_JOIN)
        return (GetInvitedCount(Team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(Team) : 0;
    // if BG is already started .. do not allow to join too much players of one faction
    uint32 otherTeam;
    uint32 otherIn;
    if (Team == ALLIANCE)
    {
        otherTeam = GetInvitedCount(HORDE);
        otherIn = GetPlayersCountByTeam(HORDE);
    }
    else
    {
        otherTeam = GetInvitedCount(ALLIANCE);
        otherIn = GetPlayersCountByTeam(ALLIANCE);
    }
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        // difference based on ppl invited (not necessarily entered battle)
        // default: allow 0
        uint32 diff = 0;
        // allow join one person if the sides are equal (to fill up bg to minplayersperteam)
        if (otherTeam == GetInvitedCount(Team))
            diff = 1;
        // allow join more ppl if the other side has more players
        else if (otherTeam > GetInvitedCount(Team))
            diff = otherTeam - GetInvitedCount(Team);

        // difference based on max players per team (don't allow inviting more)
        uint32 diff2 = (GetInvitedCount(Team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(Team) : 0;
        // difference based on players who already entered
        // default: allow 0
        uint32 diff3 = 0;
        // allow join one person if the sides are equal (to fill up bg minplayersperteam)
        if (otherIn == GetPlayersCountByTeam(Team))
            diff3 = 1;
        // allow join more ppl if the other side has more players
        else if (otherIn > GetPlayersCountByTeam(Team))
            diff3 = otherIn - GetPlayersCountByTeam(Team);
        // or other side has less than minPlayersPerTeam
        else if (GetInvitedCount(Team) <= GetMinPlayersPerTeam())
            diff3 = GetMinPlayersPerTeam() - GetInvitedCount(Team) + 1;

        // return the minimum of the 3 differences

        // min of diff and diff 2
        diff = std::min(diff, diff2);
        // min of diff, diff2 and diff3
        return std::min(diff, diff3);
    }
    return 0;
}

bool Battleground::HasFreeSlots() const
{
    return GetPlayersSize() < GetMaxPlayers();
}

void Battleground::UpdatePlayerScore(Player* Source, Player* victim, uint32 type, uint32 value, bool doAddHonor)
{
    //this procedure is called from virtual function implemented in bg subclass
    BattlegroundScoreMap::const_iterator itr = PlayerScores.find(Source->GetGUID());
    if (itr == PlayerScores.end())                         // player not found...
        return;

    switch (type)
    {
        case SCORE_KILLING_BLOWS:                           // Killing blows
            itr->second->KillingBlows += value;
            break;
        case SCORE_DEATHS:                                  // Deaths
            itr->second->Deaths += value;
            break;
        case SCORE_HONORABLE_KILLS:                         // Honorable kills
            itr->second->HonorableKills += value;
            break;
        case SCORE_BONUS_HONOR:                             // Honor bonus
            // do not add honor in arenas
            if (isBattleground())
            {
                // reward honor instantly
                if (doAddHonor)
                    Source->RewardHonor(NULL, 1, value);    // RewardHonor calls UpdatePlayerScore with doAddHonor = false
                else
                    itr->second->BonusHonor += value;
            }
            break;
            // used only in EY, but in MSG_PVP_LOG_DATA opcode
        case SCORE_DAMAGE_DONE:                             // Damage Done
        {
            itr->second->DamageDone += value;
            if (victim)
            {
                if (victim->GetHealth() < m_minHealth)
                {
                    m_minHealth = victim->GetHealth();
                    m_teamDealMaxDamage = Source->GetBGTeam();
                }
            }
            break;
        }
        case SCORE_HEALING_DONE:                            // Healing Done
            itr->second->HealingDone += value;
            break;
        default:
            sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::UpdatePlayerScore: unknown score type (%u) for BG (map: %u, instance id: %u)!",
                type, m_MapId, m_InstanceID);
            break;
    }
}

void Battleground::AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid)
{
    m_ReviveQueue[npc_guid].push_back(player_guid);

    Player* player = ObjectAccessor::FindPlayer(player_guid);
    if (!player)
        return;

    player->CastSpell(player, SPELL_WAITING_FOR_RESURRECT, true);
}

void Battleground::SendSpectateAddonsMsg(SpectatorAddonMsg& msg)
{
    if (!HaveSpectators())
        return;

    for (SpectatorList::iterator itr = m_Spectators.begin(); itr != m_Spectators.end(); ++itr)
        msg.SendPacket(*itr);
}

void Battleground::RemovePlayerFromResurrectQueue(uint64 player_guid)
{
    for (std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
    {
        for (std::vector<uint64>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
        {
            if (*itr2 == player_guid)
            {
                (itr->second).erase(itr2);
                if (Player* player = ObjectAccessor::FindPlayer(player_guid))
                    player->RemoveAurasDueToSpell(SPELL_WAITING_FOR_RESURRECT);
                return;
            }
        }
    }
}

bool Battleground::AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 /*respawnTime*/)
{
    // If the assert is called, means that BgObjects must be resized!
    ASSERT(type < BgObjects.size());

    Map* map = FindBgMap();
    if (!map)
        return false;
    // Must be created this way, adding to godatamap would add it to the base map of the instance
    // and when loading it (in go::LoadFromDB()), a new guid would be assigned to the object, and a new object would be created
    // So we must create it specific for this instance
    GameObject* go = new GameObject;
    if (!go->Create(GetBgMap()->GenerateLowGuid<HighGuid::GameObject>(), entry, GetBgMap(),
        PHASEMASK_NORMAL, x, y, z, o, rotation0, rotation1, rotation2, rotation3, 100, GO_STATE_READY))
    {
        sLog->outError(LOG_FILTER_SQL, "Battleground::AddObject: cannot create gameobject (entry: %u) for BG (map: %u, instance id: %u)!",
                entry, m_MapId, m_InstanceID);
        sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::AddObject: cannot create gameobject (entry: %u) for BG (map: %u, instance id: %u)!",
                entry, m_MapId, m_InstanceID);
        delete go;
        return false;
    }
/*
    uint32 guid = go->GetGUIDLow();

    // without this, UseButtonOrDoor caused the crash, since it tried to get go info from godata
    // iirc that was changed, so adding to go data map is no longer required if that was the only function using godata from GameObject without checking if it existed
    GameObjectData& data = sObjectMgr->NewGOData(guid);

    data.id             = entry;
    data.mapid          = GetMapId();
    data.posX           = x;
    data.posY           = y;
    data.posZ           = z;
    data.orientation    = o;
    data.rotation0      = rotation0;
    data.rotation1      = rotation1;
    data.rotation2      = rotation2;
    data.rotation3      = rotation3;
    data.spawntimesecs  = respawnTime;
    data.spawnMask      = 1;
    data.animprogress   = 100;
    data.go_state       = 1;
*/
    // Add to world, so it can be later looked up from HashMapHolder
    if (!map->AddToMap(go))
    {
        delete go;
        return false;
    }
    BgObjects[type] = go->GetGUID();
    return true;
}

// Some doors aren't despawned so we cannot handle their closing in gameobject::update()
// It would be nice to correctly implement GO_ACTIVATED state and open/close doors in gameobject code
void Battleground::DoorClose(uint32 type)
{
    if (GameObject* obj = GetBgMap()->GetGameObject(BgObjects[type]))
    {
        // If doors are open, close it
        if (obj->getLootState() == GO_ACTIVATED && obj->GetGoState() != GO_STATE_READY)
        {
            obj->SetLootState(GO_READY);
            obj->SetGoState(GO_STATE_READY);
        }
    }
    else
        sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::DoorClose: door gameobject (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!",
            type, GUID_LOPART(BgObjects[type]), m_MapId, m_InstanceID);
}

void Battleground::DoorOpen(uint32 type)
{
    if (GameObject* obj = GetBgMap()->GetGameObject(BgObjects[type]))
    {
        obj->SetLootState(GO_ACTIVATED);
        obj->SetGoState(GO_STATE_ACTIVE);
    }
    else
        sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::DoorOpen: door gameobject (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!",
            type, GUID_LOPART(BgObjects[type]), m_MapId, m_InstanceID);
}

GameObject* Battleground::GetBGObject(uint32 type)
{
    GameObject* obj = GetBgMap()->GetGameObject(BgObjects[type]);
    if (!obj)
        sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::GetBGObject: gameobject (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!",
            type, GUID_LOPART(BgObjects[type]), m_MapId, m_InstanceID);
    return obj;
}

Creature* Battleground::GetBGCreature(uint32 type)
{
    Creature* creature = GetBgMap()->GetCreature(BgCreatures[type]);
    if (BgCreatures[type] > 0LL && !creature)
        sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::GetBGCreature: creature (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!",
            type, GUID_LOPART(BgCreatures[type]), m_MapId, m_InstanceID);
    return creature;
}

void Battleground::SpawnBGObject(uint32 type, uint32 respawntime)
{
    if (Map* map = FindBgMap())
        if (GameObject* obj = map->GetGameObject(BgObjects[type]))
        {
            if (respawntime)
                obj->SetLootState(GO_JUST_DEACTIVATED);
            else
                if (obj->getLootState() == GO_JUST_DEACTIVATED)
                    // Change state from GO_JUST_DEACTIVATED to GO_READY in case battleground is starting again
                    obj->SetLootState(GO_READY);
            obj->SetRespawnTime(respawntime);
            map->AddToMap(obj);
        }
}

Creature* Battleground::AddCreature(uint32 entry, uint32 type, uint32 teamval, float x, float y, float z, float o, uint32 respawntime)
{
    // If the assert is called, means that BgCreatures must be resized!
    ASSERT(type < BgCreatures.size());

    Map* map = FindBgMap();
    if (!map)
        return NULL;

    Creature* creature = new Creature;
    if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, PHASEMASK_NORMAL, entry, 0, teamval, x, y, z, o))
    {
        sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::AddCreature: cannot create creature (entry: %u) for BG (map: %u, instance id: %u)!",
            entry, m_MapId, m_InstanceID);
        delete creature;
        return NULL;
    }

    creature->SetBattleground(this);
    BgCreatures[type] = creature->GetGUID();

    creature->SetHomePosition(x, y, z, o);

    CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(entry);
    if (!cinfo)
    {
        sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::AddCreature: creature template (entry: %u) does not exist for BG (map: %u, instance id: %u)!",
            entry, m_MapId, m_InstanceID);
        delete creature;
        return NULL;
    }
    // Force using DB speeds
    creature->SetSpeed(MOVE_WALK,   cinfo->speed_walk);
    creature->SetSpeed(MOVE_RUN,    cinfo->speed_run);
    creature->SetSpeed(MOVE_FLIGHT, cinfo->speed_fly);

    if (!map->AddToMap(creature))
    {
        delete creature;
        return NULL;
    }

    if (respawntime)
        creature->SetRespawnDelay(respawntime);

    return  creature;
}

bool Battleground::DelCreature(uint32 type)
{
    if (!BgCreatures[type])
        return true;

    if (Creature* creature = GetBgMap()->GetCreature(BgCreatures[type]))
    {
        creature->AddObjectToRemoveList();
        BgCreatures[type] = 0;
        return true;
    }

    sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::DelCreature: creature (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!",
        type, GUID_LOPART(BgCreatures[type]), m_MapId, m_InstanceID);
    BgCreatures[type] = 0;
    return false;
}

bool Battleground::DelObject(uint32 type)
{
    if (!BgObjects[type])
        return true;

    if (GameObject* obj = GetBgMap()->GetGameObject(BgObjects[type]))
    {
        obj->SetRespawnTime(0);                                 // not save respawn time
        obj->Delete();
        BgObjects[type] = 0;
        return true;
    }
    sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::DelObject: gameobject (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!",
        type, GUID_LOPART(BgObjects[type]), m_MapId, m_InstanceID);
    BgObjects[type] = 0;
    return false;
}

bool Battleground::AddSpiritGuide(uint32 type, float x, float y, float z, float o, uint32 team)
{
    uint32 entry = (team == ALLIANCE) ?
        BG_CREATURE_ENTRY_A_SPIRITGUIDE :
        BG_CREATURE_ENTRY_H_SPIRITGUIDE;

    if (Creature* creature = AddCreature(entry, type, team, x, y, z, o))
    {
        creature->setDeathState(DEAD);
        creature->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, creature->GetGUID());
        // aura
        // TODO: Fix display here
        // creature->SetVisibleAura(0, SPELL_SPIRIT_HEAL_CHANNEL);
        // casting visual effect
        creature->SetUInt32Value(UNIT_CHANNEL_SPELL, SPELL_SPIRIT_HEAL_CHANNEL);
        // correct cast speed
        creature->SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);
        creature->SetFloatValue(UNIT_MOD_CAST_HASTE, 1.0f);
        //creature->CastSpell(creature, SPELL_SPIRIT_HEAL_CHANNEL, true);
        return true;
    }
    sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::AddSpiritGuide: cannot create spirit guide (type: %u, entry: %u) for BG (map: %u, instance id: %u)!",
        type, entry, m_MapId, m_InstanceID);
    EndNow();
    return false;
}

void Battleground::SendMessageToAll(int32 entry, ChatMsg type, Player const* source)
{
    if (!entry)
        return;

    JadeCore::BattlegroundChatBuilder bg_builder(type, entry, source);
    JadeCore::LocalizedPacketDo<JadeCore::BattlegroundChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

void Battleground::PSendMessageToAll(int32 entry, ChatMsg type, Player const* source, ...)
{
    if (!entry)
        return;

    va_list ap;
    va_start(ap, source);

    JadeCore::BattlegroundChatBuilder bg_builder(type, entry, source, &ap);
    JadeCore::LocalizedPacketDo<JadeCore::BattlegroundChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);

    va_end(ap);
}

void Battleground::SendWarningToAll(int32 entry, ...)
{
    if (!entry)
        return;

    const char *format = sObjectMgr->GetTrinityStringForDBCLocale(entry);

    char str[1024];
    va_list ap;
    va_start(ap, entry);
    vsnprintf(str, 1024, format, ap);
    va_end(ap);
    std::string msg(str);

    WorldPacket data;
    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, NULL, 0, msg.c_str(), NULL);
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(itr->first, 0, HighGuid::Player)))
            if (player->GetSession())
                player->GetSession()->SendPacket(&data);
}

void Battleground::SendMessage2ToAll(int32 entry, ChatMsg type, Player const* source, int32 arg1, int32 arg2)
{
    JadeCore::Battleground2ChatBuilder bg_builder(type, entry, source, arg1, arg2);
    JadeCore::LocalizedPacketDo<JadeCore::Battleground2ChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

void Battleground::SendCountdownTimer()
{
    int countdownSec = (GetMaxCountdownTimer() - ceil(float(GetElapsedTime()) / 1000));

    for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayer(itr->first))
            player->SendStartTimer(countdownSec, GetMaxCountdownTimer(), PVP_TIMER);
}

void Battleground::EndNow()
{
    RemoveFromBGFreeSlotQueue();
    SetStatus(STATUS_WAIT_LEAVE);
    SetRemainingTime(0);
}

// To be removed
const char* Battleground::GetTrinityString(int32 entry)
{
    // FIXME: now we have different DBC locales and need localized message for each target client
    return sObjectMgr->GetTrinityStringForDBCLocale(entry);
}

// IMPORTANT NOTICE:
// buffs aren't spawned/despawned when players captures anything
// buffs are in their positions when battleground starts
void Battleground::HandleTriggerBuff(uint64 go_guid)
{
    GameObject* obj = GetBgMap()->GetGameObject(go_guid);
    if (!obj || obj->GetGoType() != GAMEOBJECT_TYPE_TRAP || !obj->isSpawned())
        return;

    // Change buff type, when buff is used:
    int32 index = BgObjects.size() - 1;
    while (index >= 0 && BgObjects[index] != go_guid)
        index--;
    if (index < 0)
    {
        sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::HandleTriggerBuff: cannot find buff gameobject (GUID: %u, entry: %u, type: %u) in internal data for BG (map: %u, instance id: %u)!",
            GUID_LOPART(go_guid), obj->GetEntry(), obj->GetGoType(), m_MapId, m_InstanceID);
        return;
    }

    // Randomly select new buff
    uint8 buff = urand(0, 2);
    uint32 entry = obj->GetEntry();
    if (m_BuffChange && entry != Buff_Entries[buff])
    {
        // Despawn current buff
        SpawnBGObject(index, RESPAWN_ONE_DAY);
        // Set index for new one
        for (uint8 currBuffTypeIndex = 0; currBuffTypeIndex < 3; ++currBuffTypeIndex)
            if (entry == Buff_Entries[currBuffTypeIndex])
            {
                index -= currBuffTypeIndex;
                index += buff;
            }
    }

    SpawnBGObject(index, BUFF_RESPAWN_TIME);
}

void Battleground::HandleKillPlayer(Player* victim, Player* killer)
{
    // Keep in mind that for arena this will have to be changed a bit

    // Add +1 deaths
    UpdatePlayerScore(victim, NULL, SCORE_DEATHS, 1, !IsWargame());
    // Add +1 kills to group and +1 killing_blows to killer
    if (killer)
    {
        // Don't reward credit for killing ourselves, like fall damage of hellfire (warlock)
        if (killer == victim)
            return;

        UpdatePlayerScore(killer, NULL, SCORE_HONORABLE_KILLS, 1, !IsWargame());
        UpdatePlayerScore(killer, NULL, SCORE_KILLING_BLOWS, 1, !IsWargame());

        for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player* creditedPlayer = ObjectAccessor::FindPlayer(itr->first);
            if (!creditedPlayer || creditedPlayer == killer)
                continue;

            if (creditedPlayer->GetTeam() == killer->GetTeam() && creditedPlayer->IsAtGroupRewardDistance(victim))
                UpdatePlayerScore(creditedPlayer, NULL, SCORE_HONORABLE_KILLS, 1, !IsWargame());
        }
    }

    if (!isArena() && !IsWargame())
    {
        // To be able to remove insignia -- ONLY IN Battlegrounds
        victim->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        RewardXPAtKill(killer, victim);
    }
}

// Return the player's team based on battlegroundplayer info
// Used in same faction arena matches mainly
uint32 Battleground::GetPlayerTeam(uint64 guid) const
{
    BattlegroundPlayerMap::const_iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
        return itr->second.Team;
    return 0;
}

uint32 Battleground::GetOtherTeam(uint32 teamId) const
{
    return teamId ? ((teamId == ALLIANCE) ? HORDE : ALLIANCE) : 0;
}

bool Battleground::IsPlayerInBattleground(uint64 guid) const
{
    BattlegroundPlayerMap::const_iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
        return true;
    return false;
}

void Battleground::PlayerAddedToBGCheckIfBGIsRunning(Player* player)
{
    if (GetStatus() != STATUS_WAIT_LEAVE)
        return;

    WorldPacket data;
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());

    BlockMovement(player);

    sBattlegroundMgr->BuildPvpLogDataPacket(&data, this);
    player->GetSession()->SendPacket(&data);

    sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, this, player, player->GetBattlegroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, player->GetBattlegroundQueueJoinTime(GetTypeID()), GetElapsedTime(), GetArenaType());
    player->GetSession()->SendPacket(&data);
}

uint32 Battleground::GetAlivePlayersCountByTeam(uint32 Team) const
{
    int count = 0;
    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.Team == Team)
        {
            Player* player = ObjectAccessor::FindPlayer(itr->first);
            if (player && player->isAlive() && !player->isSpectator() && !player->HasByteFlag(UNIT_FIELD_BYTES_2, 3, FORM_SPIRITOFREDEMPTION))
                ++count;
        }
    }
    return count;
}

void Battleground::SetHoliday(bool is_holiday)
{
    m_HonorMode = is_holiday ? BG_HOLIDAY : BG_NORMAL;
}

int32 Battleground::GetObjectType(uint64 guid)
{
    for (uint32 i = 0; i < BgObjects.size(); ++i)
        if (BgObjects[i] == guid)
            return i;
    sLog->outError(LOG_FILTER_BATTLEGROUND, "Battleground::GetObjectType: player used gameobject (GUID: %u) which is not in internal data for BG (map: %u, instance id: %u), cheating?",
        GUID_LOPART(guid), m_MapId, m_InstanceID);
    return -1;
}

void Battleground::HandleKillUnit(Creature* /*creature*/, Player* /*killer*/)
{
}

void Battleground::CheckArenaAfterTimerConditions()
{
    if (GetAlivePlayersCountByTeam(ALLIANCE) == GetAlivePlayersCountByTeam(HORDE))
        EndBattleground(WINNER_NONE);
    else
        EndBattleground(GetAlivePlayersCountByTeam(ALLIANCE) > GetAlivePlayersCountByTeam(HORDE) ? ALLIANCE : HORDE);
}

void Battleground::CheckArenaWinConditions()
{
    if (!GetAlivePlayersCountByTeam(ALLIANCE) && GetPlayersCountByTeam(HORDE))
        EndBattleground(HORDE);
    else if (GetPlayersCountByTeam(ALLIANCE) && !GetAlivePlayersCountByTeam(HORDE))
        EndBattleground(ALLIANCE);
}

void Battleground::UpdateArenaWorldState()
{
    UpdateWorldState(0xe10, GetAlivePlayersCountByTeam(HORDE));
    UpdateWorldState(0xe11, GetAlivePlayersCountByTeam(ALLIANCE));
}

void Battleground::SetBgRaid(uint32 TeamID, Group* bg_raid)
{
    Group*& old_raid = TeamID == ALLIANCE ? m_BgRaids[BG_TEAM_ALLIANCE] : m_BgRaids[BG_TEAM_HORDE];
    if (old_raid)
        old_raid->SetBattlegroundGroup(NULL);
    if (bg_raid)
        bg_raid->SetBattlegroundGroup(this);
    old_raid = bg_raid;
}

WorldSafeLocsEntry const* Battleground::GetClosestGraveYard(Player* player)
{
    return sObjectMgr->GetClosestGraveYard(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), player->GetTeam());
}

bool Battleground::IsTeamScoreInRange(uint32 team, uint32 minScore, uint32 maxScore) const
{
    BattlegroundTeamId teamIndex = GetTeamIndexByTeamId(team);
    uint32 score = std::max(m_TeamScores[teamIndex], 0);
    return score >= minScore && score <= maxScore;
}

void Battleground::StartTimedAchievement(AchievementCriteriaTimedTypes type, uint32 entry)
{
    for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayer(itr->first))
            player->GetAchievementMgr().StartTimedAchievement(type, entry);
}

void Battleground::SetBracket(PvPDifficultyEntry const* bracketEntry)
{
    m_BracketId = bracketEntry->GetBracketId();
    SetLevelRange(bracketEntry->minLevel, bracketEntry->maxLevel);
}

void Battleground::RewardXPAtKill(Player* killer, Player* victim)
{
    if (IsWargame())
        return;

    if (sWorld->getBoolConfig(CONFIG_BG_XP_FOR_KILL) && killer && victim)
        killer->RewardPlayerAndGroupAtKill(victim, true);
}

void Battleground::SendFlagsPositions()
{
    uint32 count = 0;
    std::vector<Player*> players;

    if (Player* plr = ObjectAccessor::FindPlayer(GetFlagPickerGUID(TEAM_ALLIANCE)))
    {
        players.push_back(plr);
        ++count;
    }

    if (Player* plr = ObjectAccessor::FindPlayer(GetFlagPickerGUID(TEAM_HORDE)))
    {
        players.push_back(plr);
        ++count;
    }

    WorldPacket data(SMSG_BATTLEGROUND_PLAYER_POSITIONS, (20 + count)/8 + 18*count);

    data.WriteBits(count, 20);

    for (auto itr : players)
    {
        ObjectGuid guid = itr->GetGUID();

        uint8 bits[8] = { 3, 5, 2, 0, 4, 6, 1, 7};
        data.WriteBitInOrder(guid, bits); 
    }

    for (auto itr : players)
    {
        ObjectGuid guid = itr->GetGUID();

        data << uint8(itr->GetTeamId() == TEAM_ALLIANCE ? 1 : 2); // Icon
        data.WriteByteSeq(guid[6]);
        data << float(itr->GetPositionY());
        data.WriteByteSeq(guid[7]);
        data << float(itr->GetPositionX());
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[2]);
        data << uint8(itr->GetTeamId() == TEAM_ALLIANCE ? 3 : 2); // ArenaSlot
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[5]);
    }

    SendPacketToAll(&data);
}

void Battleground::SendClearFlagsPositions()
{
    WorldPacket data(SMSG_BATTLEGROUND_PLAYER_POSITIONS, 3);
    data.WriteBits(0, 20);
    SendPacketToAll(&data);
}

void Battleground::RelocateDeadPlayers(uint64 queueIndex)
{
    // Those who are waiting to resurrect at this node are taken to the closest own node's graveyard
    std::vector<uint64>& ghostList = m_ReviveQueue[queueIndex];
    if (!ghostList.empty())
    {
        WorldSafeLocsEntry const* closestGrave = NULL;
        for (std::vector<uint64>::const_iterator itr = ghostList.begin(); itr != ghostList.end(); ++itr)
        {
            Player* player = ObjectAccessor::FindPlayer(*itr);
            if (!player)
                continue;

            if (!closestGrave)
                closestGrave = GetClosestGraveYard(player);

            if (closestGrave)
                player->TeleportTo(GetMapId(), closestGrave->x, closestGrave->y, closestGrave->z, player->GetOrientation());
        }
        ghostList.clear();
    }
}

uint32 Battleground::GetArenaMatchmakerRating(uint32 Team, uint8 slot)
{
    uint32 MMR = 0;
    uint32 count = 0;

    if (Group* group = GetBgRaid(Team))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref != NULL; ref = ref->next())
        {
            if (Player* groupMember = ref->getSource())
            {
                MMR += groupMember->GetArenaMatchMakerRating(slot);
                ++count;
            }
        }
    }

    if (!count)
        count = 1;

    MMR /= count;

    return MMR;
}

uint8 Battleground::ClickFastStart(Player *player, GameObject *go)
{
    if (!isArena())
        return 0;

    std::set<uint64>::iterator pIt = m_playersWantsFastStart.find(player->GetGUID());
    if (pIt != m_playersWantsFastStart.end() || GetStartDelayTime() < BG_START_DELAY_15S)
        return m_playersWantsFastStart.size();

    m_playersWantsFastStart.insert(player->GetGUID());

    std::set<GameObject*>::iterator goIt = m_crystals.find(go);
    if (goIt == m_crystals.end())
        m_crystals.insert(go);

    uint8 playersNeeded = 0;
    switch(GetArenaType())
    {
        case ARENA_TYPE_2v2:
            playersNeeded = 4;
            break;
        case ARENA_TYPE_3v3:
            playersNeeded = 6;
            break;
        case ARENA_TYPE_5v5:
            playersNeeded = 10;
            break;
    }

    if (m_playersWantsFastStart.size() == playersNeeded)
    {
        DespawnCrystals();
        SetStartDelayTime(BG_START_DELAY_15S);
    }

    return m_playersWantsFastStart.size();
}

void Battleground::DespawnCrystals()
{
    if (m_crystals.empty())
        return;

    for (std::set<GameObject*>::iterator itr = m_crystals.begin(); itr != m_crystals.end();)
    {
        GameObject *go = *itr;
        go->Delete();
        itr = m_crystals.erase(itr);
    }
}

void Battleground::AddSpectator(Player* player)
{
    if (!player)
        return;

    m_Spectators.insert(player->GetGUID()); 
    player->SetSpectateRemoving(false);
}

void Battleground::RemoveSpectator(Player* player)
{
    m_Spectators.erase(player->GetGUID()); 
    player->SetSpectateRemoving(true);
}
