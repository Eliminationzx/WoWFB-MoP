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
#include "DBCEnums.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "DatabaseEnv.h"
#include "AchievementMgr.h"
#include "Arena.h"
#include "CellImpl.h"
#include "GameEventMgr.h"
#include "GridNotifiersImpl.h"
#include "Guild.h"
#include "Language.h"
#include "Player.h"
#include "SpellMgr.h"
#include "DisableMgr.h"
#include "ScriptMgr.h"
#include "MapManager.h"
#include "Battleground.h"
#include "BattlegroundAB.h"
#include "Map.h"
#include "InstanceScript.h"
#include "Group.h"
#include "Chat.h"
#include "MapUpdater.h"

namespace JadeCore
{
    class AchievementChatBuilder
    {
        public:
            AchievementChatBuilder(Player const& player, ChatMsg msgtype, int32 textId, uint32 ach_id)
                : i_player(player), i_msgtype(msgtype), i_textId(textId), i_achievementId(ach_id) {}
            void operator()(WorldPacket& data, LocaleConstant loc_idx)
            {
                char const* text = sObjectMgr->GetTrinityString(i_textId, loc_idx);

                ChatHandler::FillMessageData(&data, i_player.GetSession(), i_msgtype, LANG_UNIVERSAL, NULL, i_player.GetGUID(), text, NULL, NULL, i_achievementId);
            }

        private:
            Player const& i_player;
            ChatMsg i_msgtype;
            int32 i_textId;
            uint32 i_achievementId;
    };
}                                                           // namespace JadeCore

bool AchievementCriteriaData::IsValid(CriteriaEntry const* criteria)
{
    if (dataType >= MAX_ACHIEVEMENT_CRITERIA_DATA_TYPE)
    {
        sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` for criteria (Entry: %u) has wrong data type (%u), ignored.", criteria->ID, dataType);
        return false;
    }

    switch (criteria->Type)
    {
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG:
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:          // Only hardcoded list
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
        case ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE:
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST:    // Only Children's Week achievements
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:                // Only Children's Week achievements
        case ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
            break;
        default:
            if (dataType != ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` has data for non-supported criteria type (Entry: %u Type: %u), ignored.", criteria->ID, criteria->Type);
                return false;
            }
            break;
    }

    switch (dataType)
    {
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE:
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_VALUE:
        case ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT:
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_CREATURE:
            if (!creature.id || !sObjectMgr->GetCreatureTemplate(creature.id))
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_CREATURE (%u) has non-existing creature id in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, creature.id);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE:
            if (!classRace.class_id && !classRace.race_id)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE (%u) must not have 0 in either value field, ignored.",
                    criteria->ID, criteria->Type, dataType);
                return false;
            }
            if (classRace.class_id && ((1 << (classRace.class_id-1)) & CLASSMASK_ALL_PLAYABLE) == 0)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE (%u) has non-existing class in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, classRace.class_id);
                return false;
            }
            if (classRace.race_id && ((1 << (classRace.race_id-1)) & RACEMASK_ALL_PLAYABLE) == 0)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE (%u) has non-existing race in value2 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, classRace.race_id);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_LESS_HEALTH:
            if (health.percent < 1 || health.percent > 100)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_PLAYER_LESS_HEALTH (%u) has wrong percent value in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, health.percent);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA:
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA:
        {
            SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(aura.spell_id);
            if (!spellEntry)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type %s (%u) has wrong spell id in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, (dataType == ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA ? "ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA" : "ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA"), dataType, aura.spell_id);
                return false;
            }
            if (aura.effect_idx >= 3)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type %s (%u) has wrong spell effect index in value2 (%u), ignored.",
                    criteria->ID, criteria->Type, (dataType == ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA ? "ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA" : "ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA"), dataType, aura.effect_idx);
                return false;
            }
            if (!spellEntry->Effects[aura.effect_idx].ApplyAuraName)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type %s (%u) has non-aura spell effect (ID: %u Effect: %u), ignores.",
                    criteria->ID, criteria->Type, (dataType == ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA ? "ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA" : "ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA"), dataType, aura.spell_id, aura.effect_idx);
                return false;
            }
            return true;
        }
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_LEVEL:
            if (level.minlevel > STRONG_MAX_LEVEL)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_LEVEL (%u) has wrong minlevel in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, level.minlevel);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_GENDER:
            if (gender.gender > GENDER_NONE)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_GENDER (%u) has wrong gender in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, gender.gender);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT:
            if (!ScriptId)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT (%u) does not have ScriptName set, ignored.",
                    criteria->ID, criteria->Type, dataType);
                return false;
            }
            return true;
        /*
        @Todo:
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_DIFFICULTY:
        */
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_PLAYER_COUNT:
            if (map_players.maxcount <= 0)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_PLAYER_COUNT (%u) has wrong max players count in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, map_players.maxcount);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_TEAM:
            if (team.team != ALLIANCE && team.team != HORDE)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_TEAM (%u) has unknown team in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, team.team);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_DRUNK:
            if (drunk.state >= MAX_DRUNKEN)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_DRUNK (%u) has unknown drunken state in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, drunk.state);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_HOLIDAY:
            if (!sHolidaysStore.LookupEntry(holiday.id))
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_HOLIDAY (%u) has unknown holiday in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, holiday.id);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_BG_LOSS_TEAM_SCORE:
            return true;                                    // Not check correctness node indexes
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_EQUIPED_ITEM:
            if (equipped_item.item_quality >= MAX_ITEM_QUALITY)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_requirement` (Entry: %u Type: %u) for requirement ACHIEVEMENT_CRITERIA_REQUIRE_S_EQUIPED_ITEM (%u) has unknown quality state in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, equipped_item.item_quality);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE:
            if (!classRace.class_id && !classRace.race_id)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE (%u) must not have 0 in either value field, ignored.",
                    criteria->ID, criteria->Type, dataType);
                return false;
            }
            if (classRace.class_id && ((1 << (classRace.class_id-1)) & CLASSMASK_ALL_PLAYABLE) == 0)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE (%u) has non-existing class in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, classRace.class_id);
                return false;
            }
            if (classRace.race_id && ((1 << (classRace.race_id-1)) & RACEMASK_ALL_PLAYABLE) == 0)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE (%u) has non-existing race in value2 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, classRace.race_id);
                return false;
            }
            return true;
        default:
            sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) has data for non-supported data type (%u), ignored.", criteria->ID, criteria->Type, dataType);
            return false;
    }
}

bool AchievementCriteriaData::Meets(uint32 criteria_id, Player const* source, Unit const* target, uint32 miscValue1 /*= 0*/) const
{
    switch (dataType)
    {
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE:
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_CREATURE:
            if (!target || target->GetTypeId() != TYPEID_UNIT)
                return false;
            return target->GetEntry() == creature.id;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE:
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return false;
            if (classRace.class_id && classRace.class_id != target->ToPlayer()->getClass())
                return false;
            if (classRace.race_id && classRace.race_id != target->ToPlayer()->getRace())
                return false;
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE:
            if (!source || source->GetTypeId() != TYPEID_PLAYER)
                return false;
            if (classRace.class_id && classRace.class_id != source->ToPlayer()->getClass())
                return false;
            if (classRace.race_id && classRace.race_id != source->ToPlayer()->getRace())
                return false;
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_LESS_HEALTH:
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return false;
            return !target->HealthAbovePct(health.percent);
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA:
            return source->HasAuraEffect(aura.spell_id, aura.effect_idx);
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA:
            return target && target->HasAuraEffect(aura.spell_id, aura.effect_idx);
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_VALUE:
           return miscValue1 >= value.minvalue;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_LEVEL:
            if (!target)
                return false;
            return target->getLevel() >= level.minlevel;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_GENDER:
            if (!target)
                return false;
            return target->getGender() == gender.gender;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT:
            return sScriptMgr->OnCriteriaCheck(ScriptId, const_cast<Player*>(source), const_cast<Unit*>(target));
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_PLAYER_COUNT:
            return source->GetMap()->GetPlayersCountExceptGMs() <= map_players.maxcount;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_TEAM:
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return false;
            return target->ToPlayer()->GetTeam() == team.team;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_DRUNK:
            return Player::GetDrunkenstateByValue(source->GetDrunkValue()) >= DrunkenState(drunk.state);
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_HOLIDAY:
            return IsHolidayActive(HolidayIds(holiday.id));
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_BG_LOSS_TEAM_SCORE:
        {
            Battleground* bg = source->GetBattleground();
            if (!bg)
                return false;

            uint16 winnerTeamScore = 0;
            switch(bg->GetTypeID(true))
            {
                case BATTLEGROUND_WS:
                case BATTLEGROUND_TP:
                    winnerTeamScore = 3;
                    break;
                case BATTLEGROUND_AB:
                case BATTLEGROUND_EY:
                    winnerTeamScore = 1600;
                    break;
                default:
                    break;
            }
            if (winnerTeamScore > 0 && !bg->IsTeamScoreInRange(source->GetTeam(), winnerTeamScore, winnerTeamScore))
                return false;

            return bg->IsTeamScoreInRange(source->GetTeam() == ALLIANCE ? HORDE : ALLIANCE, bg_loss_team_score.min_score, bg_loss_team_score.max_score);
        }
        case ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT:
        {
            if (!source->IsInWorld())
                return false;
            Map* map = source->GetMap();
            if (!map->IsDungeon())
            {
                sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "Achievement system call ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT (%u) for achievement criteria %u for non-dungeon/non-raid map %u",
                    ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT, criteria_id, map->GetId());
                    return false;
            }
            InstanceScript* instance = ((InstanceMap*)map)->GetInstanceScript();
            if (!instance)
            {
                sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "Achievement system call ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT (%u) for achievement criteria %u for map %u but map does not have a instance script",
                    ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT, criteria_id, map->GetId());
                return false;
            }
            return instance->CheckAchievementCriteriaMeet(criteria_id, source, target, miscValue1);
        }
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_EQUIPED_ITEM:
        {
            ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(miscValue1);
            if (!pProto)
                return false;
            return pProto->ItemLevel >= equipped_item.item_level && pProto->Quality >= equipped_item.item_quality;
        }
        default:
            break;
    }
    return false;
}

bool AchievementCriteriaDataSet::Meets(Player const* source, Unit const* target, uint32 miscvalue /*= 0*/) const
{
    for (Storage::const_iterator itr = storage.begin(); itr != storage.end(); ++itr)
        if (!itr->Meets(criteria_id, source, target, miscvalue))
            return false;

    return true;
}

template<class T>
AchievementMgr<T>::AchievementMgr(T* owner): _owner(owner), _achievementPoints(0)
{
}

template<class T>
AchievementMgr<T>::~AchievementMgr()
{
}

template<class T>
void AchievementMgr<T>::SendPacket(WorldPacket* data) const
{
}

template<>
void AchievementMgr<Guild>::SendPacket(WorldPacket* data) const
{
    GetOwner()->BroadcastPacket(data);
}

template<>
void AchievementMgr<Player>::SendPacket(WorldPacket* data) const
{
    GetOwner()->GetSession()->SendPacket(data);
}

template<class T>
void AchievementMgr<T>::RemoveCriteriaProgress(const CriteriaTreeEntry* tree)
{
    CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
    if (!progressMap)
        return;

    CriteriaProgressMap::iterator criteriaProgress = progressMap->find(tree->ID);
    if (criteriaProgress == progressMap->end())
        return;

    WorldPacket data(SMSG_CRITERIA_DELETED, 4);
    data << uint32(tree->CriteriaID);
    SendPacket(&data);

    progressMap->erase(criteriaProgress);
}

template<>
void AchievementMgr<Guild>::RemoveCriteriaProgress(const CriteriaTreeEntry* tree)
{
    CriteriaProgressMap::iterator criteriaProgress = GetCriteriaProgressMap()->find(tree->ID);
    if (criteriaProgress == GetCriteriaProgressMap()->end())
        return;

    ObjectGuid guid = GetOwner()->GetGUID();

    WorldPacket data(SMSG_GUILD_CRITERIA_DELETED, 4 + 8);

    uint8 bitOrder[8] = {7, 3, 4, 2, 1, 5, 6, 0};
    data.WriteBitInOrder(guid, bitOrder);

    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[3]);
    data << uint32(tree->CriteriaID);
    data.WriteByteSeq(guid[0]);

    SendPacket(&data);

    GetCriteriaProgressMap()->erase(criteriaProgress);
}

template<class T>
void AchievementMgr<T>::ResetAchievementCriteria(AchievementCriteriaTypes type, uint64 miscValue1, uint64 miscValue2, bool evenIfCriteriaComplete)
{
    // Disable for gamemasters with GM-mode enabled
    //if (GetOwner()->isGameMaster())
        //return;

    CriteriaTreeEntryList const& list = sAchievementMgr->GetAchievementCriteriaByType(type);
    for (CriteriaTreeEntry const* tree : list)
    {
        AchievementEntry const* achievement = sAchievementMgr->GetAchievement(tree);
        if (!achievement)
            continue;

        // Don't update already completed criteria if not forced or achievement already complete
        if ((IsCompletedCriteria(tree, achievement) && !evenIfCriteriaComplete) || HasAchieved(achievement->ID))
            continue;

        CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);

        uint32 types[MAX_CRITERIA_REQUIREMENTS] = { criteria->StartEvent, criteria->FailEvent };
        uint32 values[MAX_CRITERIA_REQUIREMENTS] = { criteria->StartAsset, criteria->FailAsset };
        
        for (uint8 j = 0; j < MAX_CRITERIA_REQUIREMENTS; ++j)
        {
            if (types[j] == miscValue1 && (!values[j] || values[j] == miscValue2))
            {
                RemoveCriteriaProgress(tree);
                break;
            }
        }
    }
}

template<>
void AchievementMgr<Guild>::ResetAchievementCriteria(AchievementCriteriaTypes /*type*/, uint64 /*miscValue1*/, uint64 /*miscValue2*/, bool /*evenIfCriteriaComplete*/)
{
    // Not needed
}

template<class T>
void AchievementMgr<T>::DeleteFromDB(uint32 /*lowguid*/, uint32 /*accountId*/)
{
}

template<>
void AchievementMgr<Player>::DeleteFromDB(uint32 lowguid, uint32 accountId)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_ACHIEVEMENT);
    stmt->setUInt32(0, lowguid);
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

template<>
void AchievementMgr<Guild>::DeleteFromDB(uint32 lowguid, uint32 accountId)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ALL_GUILD_ACHIEVEMENTS);
    stmt->setUInt32(0, lowguid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ALL_GUILD_ACHIEVEMENT_CRITERIA);
    stmt->setUInt32(0, lowguid);
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

template<class T>
void AchievementMgr<T>::SaveToDB(SQLTransaction& /*trans*/)
{
}

template<>
void AchievementMgr<Player>::SaveToDB(SQLTransaction& trans)
{
    m_CompletedAchievementsLock.acquire();
    if (!m_completedAchievements.empty())
    {
        bool need_execute = false;

        std::ostringstream ssAccDel;
        std::ostringstream ssAccIns;

        std::ostringstream ssCharDel;
        std::ostringstream ssCharIns;
        std::ostringstream sscount;

        uint32 points = 0;

        for (CompletedAchievementMap::iterator itr =  m_completedAchievements.begin(); itr != m_completedAchievements.end(); ++itr)
            if (AchievementEntry const* pAchievement = sAchievementStore.LookupEntry(itr->first))
                points += pAchievement->points;

        if (points)
        {
            sscount << "REPLACE INTO character_achievement_count (guid, count) VALUES (" << GetOwner()->GetGUIDLow() << "," << points << ");";
            trans->Append(sscount.str().c_str());
        }

        for (CompletedAchievementMap::iterator iter = m_completedAchievements.begin(); iter != m_completedAchievements.end(); ++iter)
        {
            if (!iter->second.changed)
                continue;

            bool mustSaveForCharacter = iter->second.completedByThisCharacter;

            // First new/changed record prefix
            if (!need_execute)
            {
                ssAccDel << "DELETE FROM account_achievement WHERE account = " << GetOwner()->GetSession()->GetAccountId() << " AND achievement IN (";
                ssAccIns << "INSERT INTO account_achievement (account, first_guid, achievement, date) VALUES ";

                if (mustSaveForCharacter)
                {
                    ssCharDel << "DELETE FROM character_achievement WHERE guid = " << GetOwner()->GetGUIDLow() << " AND achievement IN (";
                    ssCharIns << "INSERT INTO character_achievement (guid, achievement) VALUES ";
                }
                need_execute = true;
            }
            // Next new/changed record prefix
            else
            {
                ssAccDel << ',';
                ssAccIns << ',';

                if (mustSaveForCharacter)
                {
                    ssCharDel << ',';
                    ssCharIns << ',';
                }
            }

            // New/changed record data
            ssAccDel << iter->first;
            ssAccIns << '(' << GetOwner()->GetSession()->GetAccountId() << ',' << iter->second.first_guid << ',' << iter->first << ',' << iter->second.date << ')';

            if (mustSaveForCharacter)
            {
                ssCharDel << iter->first;
                ssCharIns << '(' << GetOwner()->GetGUIDLow() << ',' << iter->first << ')';
            }

            /// Mark as saved in db
            iter->second.changed = false;
        }

        if (need_execute)
        {
            ssAccDel  << ')';
            ssCharDel << ')';
            trans->Append(ssAccDel.str().c_str());
            trans->Append(ssAccIns.str().c_str());

            trans->Append(ssCharDel.str().c_str());
            trans->Append(ssCharIns.str().c_str());
        }
    }

    m_CompletedAchievementsLock.release();

    CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
    if (!progressMap)
        return;

    if (!progressMap->empty())
    {
        // Prepare deleting and insert
        bool need_execute_del       = false;
        bool need_execute_ins       = false;
        bool need_execute_account   = false;

        bool isAccountAchievement   = false;

        bool alreadyOneCharDelLine  = false;
        bool alreadyOneAccDelLine   = false;
        bool alreadyOneCharInsLine  = false;
        bool alreadyOneAccInsLine   = false;

        std::ostringstream ssAccdel;
        std::ostringstream ssAccins;
        std::ostringstream ssChardel;
        std::ostringstream ssCharins;


        uint64 guid      = GetOwner()->GetGUIDLow();
        uint32 accountId = GetOwner()->GetSession()->GetAccountId();

        for (auto& kvp : *progressMap)
        {
            CriteriaProgress& progress = kvp.second;
            if (!progress.changed)
                continue;

            CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(kvp.first);
            if (!tree)
                continue;

            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(tree);
            if (!achievement)
                continue;

            if (achievement->flags & ACHIEVEMENT_FLAG_ACCOUNT)
            {
                isAccountAchievement = true;
                need_execute_account = true;
            }
            else
                isAccountAchievement = false;

            // Deleted data (including 0 progress state)
            {
                // First new/changed record prefix (for any counter value)
                if (!need_execute_del)
                {
                    ssAccdel  << "DELETE FROM account_achievement_criteria_progress   WHERE account = " << accountId << " AND criteria IN (";
                    ssChardel << "DELETE FROM character_achievement_criteria_progress WHERE guid    = " << guid      << " AND criteria IN (";
                    need_execute_del = true;
                }
                // Next new/changed record prefix
                else
                {
                    if (isAccountAchievement)
                    {
                        if (alreadyOneAccDelLine)
                            ssAccdel  << ',';
                    }
                    else
                    {
                        if (alreadyOneCharDelLine)
                            ssChardel << ',';
                    }
                }

                // New/changed record data
                if (isAccountAchievement)
                {
                    ssAccdel << tree->ID;
                    alreadyOneAccDelLine  = true;
                }
                else
                {
                    ssChardel << tree->ID;
                    alreadyOneCharDelLine = true;
                }
            }

            // Store data only for real progress
            if (progress.counter != 0)
            {
                /// First new/changed record prefix
                if (!need_execute_ins)
                {
                    ssAccins  << "INSERT INTO account_achievement_criteria_progress   (account, criteria, counter, date) VALUES ";
                    ssCharins << "INSERT INTO character_achievement_criteria_progress (guid,    criteria, counter, date) VALUES ";
                    need_execute_ins = true;
                }
                // Next new/changed record prefix
                else
                {
                    if (isAccountAchievement)
                    {
                        if (alreadyOneAccInsLine)
                            ssAccins  << ',';
                    }
                    else
                    {
                        if (alreadyOneCharInsLine)
                            ssCharins << ',';
                    }
                }

                // New/changed record data
                if (isAccountAchievement)
                {
                    ssAccins << '(' << accountId << ',' << tree->ID << ',' << progress.counter << ',' << progress.date << ')';
                    alreadyOneAccInsLine  = true;
                }
                else
                {
                    ssCharins << '(' << guid << ',' << tree->ID << ',' << progress.counter << ',' << progress.date << ')';
                    alreadyOneCharInsLine = true;
                }
            }

            // Mark as updated in db
            progress.changed = false;
        }

        if (need_execute_del)                                // DELETE ... IN (.... _)_
        {
            ssAccdel  << ')';
            ssChardel << ')';
        }

        if (need_execute_del || need_execute_ins)
        {
            if (need_execute_del)
            {
                if (need_execute_account && alreadyOneAccDelLine)
                    trans->Append(ssAccdel.str().c_str());

                if (alreadyOneCharDelLine)
                    trans->Append(ssChardel.str().c_str());
            }

            if (need_execute_ins)
            {
                if (need_execute_account && alreadyOneAccInsLine)
                    trans->Append(ssAccins.str().c_str());

                if (alreadyOneCharInsLine)
                    trans->Append(ssCharins.str().c_str());
            }
        }
    }
}

template<>
void AchievementMgr<Guild>::SaveToDB(SQLTransaction& trans)
{
    PreparedStatement* stmt;
    std::ostringstream guidstr;
    for (CompletedAchievementMap::iterator itr = m_completedAchievements.begin(); itr != m_completedAchievements.end(); ++itr)
    {
        if (!itr->second.changed)
            continue;

        for (std::set<uint64>::const_iterator gItr = itr->second.guids.begin(); gItr != itr->second.guids.end(); ++gItr)
            guidstr << GUID_LOPART(*gItr) << ',';

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_ACHIEVEMENT);
        stmt->setUInt32(0, GetOwner()->GetId());
        stmt->setUInt16(1, itr->first);
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_ACHIEVEMENT);
        stmt->setUInt32(0, GetOwner()->GetId());
        stmt->setUInt16(1, itr->first);
        stmt->setUInt32(2, itr->second.date);
        for (std::set<uint64>::const_iterator gItr = itr->second.guids.begin(); gItr != itr->second.guids.end(); ++gItr)
            guidstr << GUID_LOPART(*gItr) << ',';

        stmt->setString(3, guidstr.str());
        trans->Append(stmt);

        guidstr.str("");

        itr->second.changed = false;
    }

    CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
    if (!progressMap)
        return;

    for (auto& kvp : *progressMap)
    {
        CriteriaProgress& progress = kvp.second;
        if (!progress.changed)
            continue;

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_ACHIEVEMENT_CRITERIA);
        stmt->setUInt32(0, GetOwner()->GetId());
        stmt->setUInt16(1, kvp.first);
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_ACHIEVEMENT_CRITERIA);
        stmt->setUInt32(0, GetOwner()->GetId());
        stmt->setUInt16(1, kvp.first);
        stmt->setUInt64(2, progress.counter);
        stmt->setUInt32(3, progress.date);
        stmt->setUInt32(4, GUID_LOPART(progress.CompletedGUID));
        trans->Append(stmt);

        progress.changed = false;
    }
}

template<class T>
void AchievementMgr<T>::LoadFromDB(PreparedQueryResult achievementResult, PreparedQueryResult criteriaResult, PreparedQueryResult achievementAccountResult, PreparedQueryResult criteriaAccountResult)
{
}

template<class T>
void AchievementMgr<T>::LoadFromDB2(PreparedQueryResult criteriaResult, PreparedQueryResult criteriaAccountResult)
{
}

template<>
void AchievementMgr<Player>::LoadFromDB(PreparedQueryResult achievementResult, PreparedQueryResult criteriaResult, PreparedQueryResult achievementAccountResult, PreparedQueryResult criteriaAccountResult)
{
    if (achievementAccountResult)
    {
        do
        {
            Field* fields = achievementAccountResult->Fetch();
            uint32 first_guid    = fields[0].GetUInt32();
            uint32 achievementid = fields[1].GetUInt16();

            // Must not happen: cleanup at server startup in sAchievementMgr->LoadCompletedAchievements()
            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(achievementid);
            if (!achievement)
                continue;

            if (achievement->flags & ACHIEVEMENT_FLAG_GUILD)
                continue;

            m_CompletedAchievementsLock.acquire();
            CompletedAchievementData& ca = m_completedAchievements[achievementid];
            ca.date = time_t(fields[2].GetUInt32());
            ca.changed = false;
            ca.first_guid = first_guid;
            ca.completedByThisCharacter = first_guid == GetOwner()->GetGUIDLow();
            m_CompletedAchievementsLock.release();

            _achievementPoints += achievement->points;

            bool l_CanAddTitle = achievement->flags & AchievementFlags::ACHIEVEMENT_FLAG_ACCOUNT || GetOwner()->GetGUIDLow() == first_guid;

            // Title achievement rewards are retroactive
            if (l_CanAddTitle)
            {
                if (AchievementReward const* reward = sAchievementMgr->GetAchievementReward(achievement))
                {
                    if (uint32 titleId = reward->titleId[Player::TeamForRace(GetOwner()->getRace()) == ALLIANCE ? 0 : 1])
                    {
                        if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId))
                            GetOwner()->SetTitle(titleEntry);
                    }
                }
            }
        }
        while (achievementAccountResult->NextRow());
    }

    if (criteriaResult)
    {
        time_t now = time(NULL);
        do
        {
            Field* fields = criteriaResult->Fetch();
            uint32 id      = fields[0].GetUInt16();
            uint32 counter = fields[1].GetUInt32();
            time_t date    = time_t(fields[2].GetUInt32());

            uint32 criteriaTree = GetCriteriaTreeFromOldCriteria(id);
            if (criteriaTree == 0)
                continue;
            
            CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(criteriaTree);
            if (!tree)
            {
                sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "AchievementMgr<T>::LoadFromDB GetCriteriaTreeFromOldCriteria return non exist CriteriaTree record %u for character criteria id %u.", criteriaTree, id);
                continue;
            }

            CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
            if (criteria)
            {
                if (criteria->StartTimer && time_t(date + criteria->StartTimer) < now)
                    continue;
            }

            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(tree);
            if (achievement && (achievement->flags & ACHIEVEMENT_FLAG_GUILD))
                continue;

            CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
            if (!progressMap)
                continue;

            CriteriaProgress& progress = (*progressMap)[criteriaTree];
            progress.counter = counter;
            progress.date    = date;
            progress.changed = true; // Mark as true, so this criteria will be saved at next SavedToDB

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_OLD_INVALID_ACHIEV_PROGRESS_CRITERIA);
            stmt->setUInt16(0, uint16(id));
            stmt->setUInt32(1, GetOwner()->GetGUIDLow());
            CharacterDatabase.Execute(stmt);
        }
        while (criteriaResult->NextRow());
    }

    if (achievementResult)
    {
        do
        {
            Field* fields = achievementResult->Fetch();
            uint32 achievementid = fields[0].GetUInt16();

            // Must not happen: cleanup at server startup in sAchievementMgr->LoadCompletedAchievements()
            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(achievementid);
            if (!achievement)
                continue;

            m_CompletedAchievementsLock.acquire();

            // Achievement in character_achievement but not in account_achievement, there is a problem.
            if (m_completedAchievements.find(achievementid) == m_completedAchievements.end())
            {
                m_CompletedAchievementsLock.release();
                continue;
            }

            CompletedAchievementData& ca = m_completedAchievements[achievementid];
            ca.completedByThisCharacter = true;

            m_CompletedAchievementsLock.release();
            _achievementPoints += achievement->points;

            if (AchievementReward const* reward = sAchievementMgr->GetAchievementReward(achievement))
            {
                if (uint32 titleId = reward->titleId[Player::TeamForRace(GetOwner()->getRace()) == ALLIANCE ? 0 : 1])
                {
                    if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId))
                        GetOwner()->SetTitle(titleEntry);
                }
            }
        }
        while (achievementResult->NextRow());
    }

    if (criteriaAccountResult)
    {
        time_t now = time(NULL);
        do
        {
            Field* fields = criteriaAccountResult->Fetch();
            uint32 id      = fields[0].GetUInt16();
            uint32 counter = fields[1].GetUInt32();
            time_t date    = time_t(fields[2].GetUInt32());

            uint32 criteriaTree = GetCriteriaTreeFromOldCriteria(id);
            if (criteriaTree == 0)
                continue;

            CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(criteriaTree);
            if (!tree)
            {
                sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "AchievementMgr<T>::LoadFromDB GetCriteriaTreeFromOldCriteria return non exist CriteriaTree record %u for account criteria id %u.", criteriaTree, id);
                continue;
            }

            CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
            if (criteria)
            {
                if (criteria->StartTimer && time_t(date + criteria->StartTimer) < now)
                    continue;
            }

            CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
            if (!progressMap)
                continue;

            // Achievement in both account & characters achievement_progress, problem
            if (progressMap->find(criteriaTree) != progressMap->end())
            {
                //sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "Achievement '%u' in both account & characters achievement_progress", id);
                continue;
            }

            CriteriaProgress& progress = (*progressMap)[criteriaTree];
            progress.counter = counter;
            progress.date    = date;
            progress.changed = true; // Mark as true, so this criteria will be saved at next SavedToDB

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(ACC_DEL_OLD_INVALID_ACHIEV_PROGRESS_CRITERIA);
            stmt->setUInt16(0, uint16(id));
            stmt->setUInt32(1, GetOwner()->GetSession()->GetAccountId());
            CharacterDatabase.Execute(stmt);
        }
        while (criteriaAccountResult->NextRow());
    }
}

template<>
void AchievementMgr<Player>::LoadFromDB2(PreparedQueryResult criteriaResult, PreparedQueryResult criteriaAccountResult)
{
    if (criteriaResult)
    {
        time_t now = time(NULL);
        do
        {
            Field* fields = criteriaResult->Fetch();
            uint32 id      = fields[0].GetUInt16();
            uint64 counter = fields[1].GetUInt64();
            time_t date    = time_t(fields[2].GetUInt32());

            CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(id);
            if (!tree)
            {
                // We will remove not existed criteria for all characters
                sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement criteria tree %u data removed from table `character_achievement_criteria_progress`.", id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt16(0, uint16(id));
                CharacterDatabase.Execute(stmt);
                continue;
            }

            CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
            if (criteria)
            {
                if (criteria->StartTimer && time_t(date + criteria->StartTimer) < now)
                    continue;
            }

            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(tree);
            if (achievement && (achievement->flags & ACHIEVEMENT_FLAG_GUILD))
                continue;

            CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
            if (!progressMap)
                continue;

            CriteriaProgress& progress = (*progressMap)[id];
            progress.counter = counter;
            progress.date    = date;
            progress.changed = false;
        }
        while (criteriaResult->NextRow());
    }

    if (criteriaAccountResult)
    {
        time_t now = time(NULL);
        do
        {
            Field* fields = criteriaAccountResult->Fetch();
            uint32 id      = fields[0].GetUInt16();
            uint64 counter = fields[1].GetUInt64();
            time_t date    = time_t(fields[2].GetUInt32());

            CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(id);
            if (!tree)
            {
                // We will remove not existed criteria for all characters
                sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement criteria tree %u data removed from table `account_achievement_criteria_progress`.", id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(ACC_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt16(0, uint16(id));
                CharacterDatabase.Execute(stmt);
                continue;
            }

            CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
            if (criteria)
            {
                if (criteria->StartTimer && time_t(date + criteria->StartTimer) < now)
                    continue;
            }

            CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
            if (!progressMap)
                continue;

            // Achievement in both account & characters achievement_progress, problem
            if (progressMap->find(id) != progressMap->end())
            {
                //sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "Achievement '%u' in both account & characters achievement_progress", id);
                continue;
            }

            CriteriaProgress& progress = (*progressMap)[id];
            progress.counter = counter;
            progress.date    = date;
            progress.changed = false;
        }
        while (criteriaAccountResult->NextRow());
    }
}

template<>
void AchievementMgr<Guild>::LoadFromDB(PreparedQueryResult achievementResult, PreparedQueryResult criteriaResult, PreparedQueryResult achievementAccountResult, PreparedQueryResult criteriaAccountResult)
{
    if (achievementResult)
    {
        do
        {
            Field* fields = achievementResult->Fetch();
            uint32 achievementid = fields[0].GetUInt16();

            // Must not happen: cleanup at server startup in sAchievementMgr->LoadCompletedAchievements()
            AchievementEntry const* achievement = sAchievementStore.LookupEntry(achievementid);
            if (!achievement)
                continue;

            if (!(achievement->flags & ACHIEVEMENT_FLAG_GUILD))
                continue;

            CompletedAchievementData& ca = m_completedAchievements[achievementid];
            ca.date = time_t(fields[1].GetUInt32());
            Tokenizer guids(fields[2].GetString(), ' ');
            for (uint32 i = 0; i < guids.size(); ++i)
                ca.guids.insert(MAKE_NEW_GUID(atol(guids[i]), 0, HighGuid::Player));

            ca.changed = false;
            _achievementPoints += achievement->points;

        }
        while (achievementResult->NextRow());
    }

    if (criteriaResult)
    {
        time_t now = time(NULL);
        do
        {
            Field* fields = criteriaResult->Fetch();
            uint32 id      = fields[0].GetUInt16();
            uint32 counter = fields[1].GetUInt32();
            time_t date    = time_t(fields[2].GetUInt32());
            uint64 guid    = fields[3].GetUInt32();

            uint32 criteriaTree = GetCriteriaTreeFromOldCriteria(id);
            if (criteriaTree == 0)
                continue;

            CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(criteriaTree);
            if (!tree)
            {
                sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "AchievementMgr<T>::LoadFromDB GetCriteriaTreeFromOldCriteria return non exist CriteriaTree record %u for guild criteria id %u.", criteriaTree, id);
                continue;
            }

            CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
            if (criteria)
            {
                if (criteria->StartTimer && time_t(date + criteria->StartTimer) < now)
                    continue;
            }

            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(tree);
            if (achievement && !(achievement->flags & ACHIEVEMENT_FLAG_GUILD))
                continue;

            CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
            if (!progressMap)
                continue;

            CriteriaProgress& progress = (*progressMap)[criteriaTree];
            progress.counter = counter;
            progress.date    = date;
            progress.CompletedGUID = MAKE_NEW_GUID(guid, 0, HighGuid::Player);
            progress.changed = true;

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_ACHIEVEMENT_CRITERIA);
            stmt->setUInt32(0, GetOwner()->GetId());
            stmt->setUInt16(1, uint16(id));
            CharacterDatabase.Execute(stmt);
        }
        while (criteriaResult->NextRow());
    }
}

template<>
void AchievementMgr<Guild>::LoadFromDB2(PreparedQueryResult criteriaResult, PreparedQueryResult criteriaAccountResult)
{
    if (criteriaResult)
    {
        time_t now = time(NULL);
        do
        {
            Field* fields = criteriaResult->Fetch();
            uint32 id      = fields[0].GetUInt16();
            uint64 counter = fields[1].GetUInt64();
            time_t date    = time_t(fields[2].GetUInt32());
            uint64 guid    = fields[3].GetUInt32();

            CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(id);
            if (!tree)
            {
                // We will remove not existed criteria for all guilds
                sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement criteria tree %u data removed from table `guild_achievement_criteria_progress`.", id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA_GUILD);
                stmt->setUInt16(0, uint16(id));
                CharacterDatabase.Execute(stmt);
                continue;
            }

            CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
            if (criteria)
            {
                if (criteria->StartTimer && time_t(date + criteria->StartTimer) < now)
                    continue;
            }

            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(tree);
            if (achievement && !(achievement->flags & ACHIEVEMENT_FLAG_GUILD))
                continue;

            CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
            if (!progressMap)
                continue;

            CriteriaProgress& progress = (*progressMap)[id];
            progress.counter = counter;
            progress.date    = date;
            progress.CompletedGUID = MAKE_NEW_GUID(guid, 0, HighGuid::Player);
            progress.changed = false;
        }
        while (criteriaResult->NextRow());
    }
}

template<class T>
void AchievementMgr<T>::LoadAfterInterRealm(QueryResult achievementResult, QueryResult criteriaResult)
{
}

template<>
void AchievementMgr<Player>::LoadAfterInterRealm(QueryResult achievementResult, QueryResult criteriaResult)
{
    if (sWorld->GetInterRealmSession()->GetProtocolVersion() >= PROTOCOL_WITH_ACHIEVEMENT)
    {
        return;
    }

    if (criteriaResult)
    {
        time_t now = time(NULL);
        do
        {
            Field* fields = criteriaResult->Fetch();
            uint32 id      = fields[0].GetUInt16();
            uint32 counter = fields[1].GetUInt32();
            time_t date    = time_t(fields[2].GetUInt32());

            uint32 criteriaTree = GetCriteriaTreeFromOldCriteria(id);
            if (criteriaTree == 0)
                continue;
            
            CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(criteriaTree);
            if (!tree)
            {
                sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "AchievementMgr<T>::LoadFromDB GetCriteriaTreeFromOldCriteria return non exist CriteriaTree record %u for character criteria id %u.", criteriaTree, id);
                continue;
            }

            CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
            if (criteria)
            {
                if (criteria->StartTimer && time_t(date + criteria->StartTimer) < now)
                    continue;
            }

            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(tree);
            if (achievement && (achievement->flags & ACHIEVEMENT_FLAG_GUILD))
                continue;

            CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
            if (!progressMap)
                continue;

            CriteriaProgress& progress = (*progressMap)[criteriaTree];
            progress.counter = counter;
            progress.date    = date;
            progress.changed = true; // Mark as true, so this criteria will be saved at next SavedToDB

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_OLD_INVALID_ACHIEV_PROGRESS_CRITERIA);
            stmt->setUInt16(0, uint16(id));
            stmt->setUInt32(1, GetOwner()->GetGUIDLow());
            CharacterDatabase.Execute(stmt);
        }
        while (criteriaResult->NextRow());
    }

    if (achievementResult)
    {
        do
        {
            Field* fields = achievementResult->Fetch();
            uint32 achievementid = fields[0].GetUInt16();

            // Must not happen: cleanup at server startup in sAchievementMgr->LoadCompletedAchievements()
            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(achievementid);
            if (!achievement)
                continue;

            // Achievement in character_achievement but not in account_achievement, there is a problem.
            if (m_completedAchievements.find(achievementid) == m_completedAchievements.end())
            {
                //sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "Achievement '%u' in character_achievement but not in account_achievement, there is a problem.", achievementid);
                continue;
            }

            CompletedAchievementData& ca = m_completedAchievements[achievementid];
            ca.completedByThisCharacter = true;
            _achievementPoints += achievement->points;

        }
        while (achievementResult->NextRow());
    }
}

template<class T>
void AchievementMgr<T>::LoadAfterInterRealm2(QueryResult criteriaResult)
{

}

template<>
void AchievementMgr<Player>::LoadAfterInterRealm2(QueryResult criteriaResult)
{
    if (sWorld->GetInterRealmSession()->GetProtocolVersion() >= PROTOCOL_WITH_ACHIEVEMENT)
    {
        return;
    }

    if (criteriaResult)
    {
        time_t now = time(NULL);
        do
        {
            Field* fields = criteriaResult->Fetch();
            uint32 id      = fields[0].GetUInt16();
            uint64 counter = fields[1].GetUInt64();
            time_t date    = time_t(fields[2].GetUInt32());

            CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(id);
            if (!tree)
            {
                // We will remove not existed criteria for all characters
                sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement criteria tree %u data removed from table `character_achievement_criteria_progress`.", id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt16(0, uint16(id));
                CharacterDatabase.Execute(stmt);

                continue;
            }

            CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
            if (criteria)
            {
                if (criteria->StartTimer && time_t(date + criteria->StartTimer) < now)
                    continue;
            }

            AchievementEntry const* achievement = sAchievementMgr->GetAchievement(tree);
            if (achievement && (achievement->flags & ACHIEVEMENT_FLAG_GUILD))
                continue;

            CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
            if (!progressMap)
                continue;

            CriteriaProgress& progress = (*progressMap)[id];
            progress.counter = counter;
            progress.date    = date;
            progress.changed = false;
        }
        while (criteriaResult->NextRow());
    }
}

template<class T>
void AchievementMgr<T>::Reset()
{
}

template<>
void AchievementMgr<Player>::Reset()
{
    m_CompletedAchievementsLock.acquire();
    for (CompletedAchievementMap::const_iterator iter = m_completedAchievements.begin(); iter != m_completedAchievements.end(); ++iter)
    {
        WorldPacket data(SMSG_ACHIEVEMENT_DELETED, 4);
        data << uint32(iter->first);
        SendPacket(&data);
    }
    m_CompletedAchievementsLock.release();

    CriteriaProgressMap* criteriaProgress = GetCriteriaProgressMap();
    if (!criteriaProgress)
        return;

    for (auto const& kvp : *criteriaProgress)
    {
        if (CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(kvp.first))
        {
            WorldPacket data(SMSG_CRITERIA_DELETED, 4);
            data << uint32(tree->CriteriaID);
            SendPacket(&data);
        }
    }

    m_CompletedAchievementsLock.acquire();
    m_completedAchievements.clear();
    m_CompletedAchievementsLock.release();

    _achievementPoints = 0;
    criteriaProgress->clear();
    DeleteFromDB(GetOwner()->GetGUIDLow());

    // Re-fill data
    CheckAllAchievementCriteria(GetOwner(), false);
}

template<>
void AchievementMgr<Guild>::Reset()
{
    ObjectGuid guid = GetOwner()->GetGUID();
    for (CompletedAchievementMap::const_iterator iter = m_completedAchievements.begin(); iter != m_completedAchievements.end(); ++iter)
    {
        WorldPacket data(SMSG_GUILD_ACHIEVEMENT_DELETED, 4);

        uint8 bitOrder[8] = {4, 1, 2, 3, 0, 7, 5, 6};
        data.WriteBitInOrder(guid, bitOrder);

        data << uint32(iter->first);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[7]);
        data << uint32(secsToTimeBitFields(iter->second.date));
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[2]);
        SendPacket(&data);
    }

    CriteriaProgressMap* map = GetCriteriaProgressMap();
    if (!map)
        return;

    while (!map->empty())
    {
        if (CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(map->begin()->first))
            RemoveCriteriaProgress(tree);
    }

    _achievementPoints = 0;
    m_completedAchievements.clear();
    DeleteFromDB(GetOwner()->GetId());
}

template<class T>
void AchievementMgr<T>::SendAchievementEarned(AchievementEntry const* achievement) const
{
    // Don't send for achievements with ACHIEVEMENT_FLAG_HIDDEN
    if (achievement->flags & ACHIEVEMENT_FLAG_HIDDEN)
        return;

    if (Guild* guild = sGuildMgr->GetGuildById(GetOwner()->GetGuildId()))
    {
        JadeCore::AchievementChatBuilder say_builder(*GetOwner(), CHAT_MSG_GUILD_ACHIEVEMENT, LANG_ACHIEVEMENT_EARNED, achievement->ID);
        JadeCore::LocalizedPacketDo<JadeCore::AchievementChatBuilder> say_do(say_builder);
        guild->BroadcastWorker(say_do);
    }

    if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_KILL | ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_GUILD))
    {
        // Broadcast realm first reached
        WorldPacket data(SMSG_SERVER_FIRST_ACHIEVEMENT, strlen(GetOwner()->GetName()) + 1 + 8 + 4 + 4);
        data << GetOwner()->GetName();
        data << uint64(GetOwner()->GetGUID());
        data << uint32(achievement->ID);
        data << uint32(0);                                  // 1=link supplied string as player name, 0=display plain string
        sWorld->SendGlobalMessage(&data);
    }
    // If player is in world he can tell his friends about new achievement
    else if (GetOwner()->IsInWorld() && !GetOwner()->IsDuringRemoveFromWorld())
    {
        CellCoord p = JadeCore::ComputeCellCoord(GetOwner()->GetPositionX(), GetOwner()->GetPositionY());

        Cell cell(p);
        cell.SetNoCreate();

        JadeCore::AchievementChatBuilder say_builder(*GetOwner(), CHAT_MSG_ACHIEVEMENT, LANG_ACHIEVEMENT_EARNED, achievement->ID);
        JadeCore::LocalizedPacketDo<JadeCore::AchievementChatBuilder> say_do(say_builder);
        JadeCore::PlayerDistWorker<JadeCore::LocalizedPacketDo<JadeCore::AchievementChatBuilder> > say_worker(GetOwner(), sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY), say_do);
        TypeContainerVisitor<JadeCore::PlayerDistWorker<JadeCore::LocalizedPacketDo<JadeCore::AchievementChatBuilder> >, WorldTypeMapContainer > message(say_worker);
        cell.Visit(p, message, *GetOwner()->GetMap(), *GetOwner(), sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY));
    }

    WorldPacket data(SMSG_ACHIEVEMENT_EARNED);

    ObjectGuid thisPlayerGuid = GetOwner()->GetGUID();
    ObjectGuid firstPlayerOnAccountGuid = GetOwner()->GetGUID();

    if (HasAccountAchieved(achievement->ID))
        firstPlayerOnAccountGuid = GetFirstAchievedCharacterOnAccount(achievement->ID);

    data.WriteBit(thisPlayerGuid[2]);
    data.WriteBit(firstPlayerOnAccountGuid[4]);
    data.WriteBit(firstPlayerOnAccountGuid[3]);
    data.WriteBit(0);                  // unk
    data.WriteBit(thisPlayerGuid[1]);
    data.WriteBit(firstPlayerOnAccountGuid[7]);
    data.WriteBit(thisPlayerGuid[3]);
    data.WriteBit(thisPlayerGuid[7]);
    data.WriteBit(thisPlayerGuid[4]);
    data.WriteBit(thisPlayerGuid[0]);
    data.WriteBit(firstPlayerOnAccountGuid[1]);
    data.WriteBit(thisPlayerGuid[6]);
    data.WriteBit(firstPlayerOnAccountGuid[6]);
    data.WriteBit(firstPlayerOnAccountGuid[2]);
    data.WriteBit(thisPlayerGuid[5]);
    data.WriteBit(firstPlayerOnAccountGuid[5]);
    data.WriteBit(firstPlayerOnAccountGuid[0]);

    data.WriteByteSeq(thisPlayerGuid[6]);
    data.WriteByteSeq(firstPlayerOnAccountGuid[6]);
    data << uint32(0);                    // unk realm id ?
    data << uint32(achievement->ID);
    data.WriteByteSeq(thisPlayerGuid[1]);
    data.WriteByteSeq(firstPlayerOnAccountGuid[3]);
    data.WriteByteSeq(firstPlayerOnAccountGuid[7]);
    data.WriteByteSeq(firstPlayerOnAccountGuid[5]);
    data.WriteByteSeq(thisPlayerGuid[2]);
    data.WriteByteSeq(thisPlayerGuid[7]);
    data.WriteByteSeq(firstPlayerOnAccountGuid[4]);
    data << uint32(0);                    // unk realm id ?
    data << uint32(secsToTimeBitFields(time(NULL)));
    data.WriteByteSeq(thisPlayerGuid[3]);
    data.WriteByteSeq(thisPlayerGuid[4]);
    data.WriteByteSeq(thisPlayerGuid[5]);
    data.WriteByteSeq(firstPlayerOnAccountGuid[2]);
    data.WriteByteSeq(firstPlayerOnAccountGuid[0]);
    data.WriteByteSeq(firstPlayerOnAccountGuid[1]);
    data.WriteByteSeq(thisPlayerGuid[0]);

    GetOwner()->SendMessageToSetInRange(&data, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY), true);
}

template<>
void AchievementMgr<Guild>::SendAchievementEarned(AchievementEntry const* achievement) const
{
    ObjectGuid guid = GetOwner()->GetGUID();

    WorldPacket data(SMSG_GUILD_ACHIEVEMENT_EARNED, 8+4+8);

    uint8 bitOrder[8] = { 5, 4, 2, 0, 3, 6, 1, 7 };
    data.WriteBitInOrder(guid, bitOrder);

    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[4]);
    data << uint32(secsToTimeBitFields(time(NULL)));
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[1]);
    data << uint32(achievement->ID);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[7]);

    SendPacket(&data);
}

template<class T>
void AchievementMgr<T>::SendCriteriaUpdate(CriteriaEntry const* /*entry*/, CriteriaProgress const* /*progress*/, uint32 /*timeElapsed*/, bool /*timedCompleted*/) const
{
}

template<>
void AchievementMgr<Player>::SendCriteriaUpdate(CriteriaEntry const* entry, CriteriaProgress const* progress, uint32 timeElapsed, bool timedCompleted) const
{
    WorldPacket data(SMSG_CRITERIA_UPDATE, 4 + 4 + 4 + 4 + 8 + 4);
    ObjectGuid playerGuid = GetOwner()->GetGUID();

    data << uint32(secsToTimeBitFields(progress->date));
    data << uint32(timeElapsed);                        // Time elapsed in seconds
    data << uint32(timeElapsed);                        // Unk

    data << uint64(progress->counter);
    data << uint32(entry->ID);
    // This are some flags, 1 is for keeping the counter at 0 in client
    if (!entry->StartTimer)
        data << uint32(0);
    else
        data << uint32(timedCompleted ? 0 : 1);
        
    uint8 bitsOrder[8] = { 2, 4, 0, 6, 3, 7, 5, 1 };
    data.WriteBitInOrder(playerGuid, bitsOrder);

    uint8 bytesOrder[8] = { 4, 2, 6, 1, 7, 3, 0, 5 };
    data.WriteBytesSeq(playerGuid, bytesOrder);

    SendPacket(&data);
}

template<>
void AchievementMgr<Guild>::SendCriteriaUpdate(CriteriaEntry const* entry, CriteriaProgress const* progress, uint32 /*timeElapsed*/, bool /*timedCompleted*/) const
{
    // Will send response to criteria progress request
    WorldPacket data(SMSG_GUILD_CRITERIA_DATA);

    ObjectGuid counter = progress->counter; // For accessing every byte individually
    ObjectGuid guid = progress->CompletedGUID ? progress->CompletedGUID : GetOwner()->GetGUID();

    sLog->outInfo(LOG_FILTER_OPCODES, "SendCriteriaUpdate: %u %u", entry->ID, uint32(counter));
    data.WriteBits(1, 19);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[5]);
    data.WriteBit(counter[4]);
    data.WriteBit(guid[0]);
    data.WriteBit(counter[6]);
    data.WriteBit(counter[3]);
    data.WriteBit(counter[7]);
    data.WriteBit(counter[5]);
    data.WriteBit(guid[1]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[7]);
    data.WriteBit(counter[1]);
    data.WriteBit(guid[6]);
    data.WriteBit(counter[0]);
    data.WriteBit(counter[2]);
    data.WriteBit(guid[2]);

    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(counter[3]);
    data.WriteByteSeq(counter[6]);
    data.WriteByteSeq(counter[5]);
    data.WriteByteSeq(counter[0]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(counter[7]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(counter[4]);
    data.WriteByteSeq(counter[1]);
    data.WriteByteSeq(guid[4]);
    data << uint32(progress->date);      // Unknown date
    data << uint32(entry->ID);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(counter[2]);
    data << uint32(progress->date);      // Unknown date
    data.WriteByteSeq(guid[3]);
    data << uint32(progress->date);      // Unknown date
    data << uint32(progress->changed);

    SendPacket(&data);
}

template<class T>
CriteriaProgressMap* AchievementMgr<T>::GetCriteriaProgressMap()
{
    return &m_criteriaProgress;
}

/**
 * Called at player login. The player might have fulfilled some achievements when the achievement system wasn't working yet
 */
template<class T>
void AchievementMgr<T>::CheckAllAchievementCriteria(Player* p_ReferencePlayer, bool fix)
{
    for (uint32 l_AchievementCriteriaType = 0; l_AchievementCriteriaType < ACHIEVEMENT_CRITERIA_TYPE_TOTAL; ++l_AchievementCriteriaType)
    {
        AchievementCriteriaUpdateTask l_Task;
        l_Task.PlayerGUID = p_ReferencePlayer->GetGUID();
        l_Task.UnitGUID   = 0;
        l_Task.Task = [l_AchievementCriteriaType](uint64 const& p_PlayerGuid, uint64 const& p_UnitGUID) -> void
        {
            /// Task will be executed async
            /// We need to ensure the player still exist
            Player* l_Player = HashMapHolder<Player>::Find(p_PlayerGuid);
            if (l_Player == nullptr)
                return;

            l_Player->GetAchievementMgr().UpdateAchievementCriteria((AchievementCriteriaTypes)l_AchievementCriteriaType, 0, 0, 0, nullptr, l_Player, true);
        };

        sAchievementMgr->AddCriteriaUpdateTask(l_Task);
    }
    
    if (fix)
    {
        for (uint32 i = 0; i < sAchievementStore.GetNumRows(); ++i)
        {
            AchievementEntry const* achievement = sAchievementStore.LookupEntry(i);
            if (!achievement)
                continue;
    
            if (p_ReferencePlayer)
            {
                if ((achievement->requiredFaction == ACHIEVEMENT_FACTION_HORDE    && p_ReferencePlayer->GetTeam() != HORDE) ||
                    (achievement->requiredFaction == ACHIEVEMENT_FACTION_ALLIANCE && p_ReferencePlayer->GetTeam() != ALLIANCE))
                continue;
            }

            if (IsCompletedAchievement(achievement))
                CompletedAchievement(achievement, p_ReferencePlayer, true);
            
            if (AchievementEntryList const* achRefList = sAchievementMgr->GetAchievementByReferencedId(achievement->ID))
                for (AchievementEntryList::const_iterator itr = achRefList->begin(); itr != achRefList->end(); ++itr)
                    if (IsCompletedAchievement(*itr, p_ReferencePlayer))
                        CompletedAchievement(*itr, p_ReferencePlayer, true);
        }
    }
}

static const uint32 achievIdByArenaSlot[MAX_ARENA_SLOT] = {1057, 1107, 1108};
static const uint32 achievIdForDungeon[][4] =
{
    // ach_cr_id, is_dungeon, is_raid, is_heroic_dungeon
    { 321,       true,      true,   true  },
    { 916,       false,     true,   false },
    { 917,       false,     true,   false },
    { 918,       true,      false,  false },
    { 2219,      false,     false,  true  },
    { 0,         false,     false,  false }
};

// Helper function to avoid having to specialize template for a 800 line long function
template <typename T> static bool IsGuild() { return false; }
template<> bool IsGuild<Guild>() { return true; }

/**
 * This function will be called whenever the user might have done a criteria relevant action
 */
template<class T>
void AchievementMgr<T>::UpdateAchievementCriteria(AchievementCriteriaTypes type, uint64 miscValue1 /*= 0*/, uint64 miscValue2 /*= 0*/, uint64 miscValue3 /*= 0*/, Unit const* unit /*= NULL*/, Player* referencePlayer /*= NULL*/, bool p_LoginCheck)
{
    if (type >= ACHIEVEMENT_CRITERIA_TYPE_TOTAL)
        return;

    if (!IsGuild<T>() && !referencePlayer)
        return;

    // disable for gamemasters with GM-mode enabled
    /*if (referencePlayer)
        if (referencePlayer->isGameMaster())
            return;*/

     // Lua_GetGuildLevelEnabled() is checked in achievement UI to display guild tab
    if (IsGuild<T>() && !sWorld->getBoolConfig(CONFIG_GUILD_LEVELING_ENABLED))
        return;

    CriteriaTreeEntryList const& list = sAchievementMgr->GetAchievementCriteriaByType(type, IsGuild<T>());
    for (CriteriaTreeEntry const* tree : list)
    {
        AchievementEntry const* achievement = sAchievementMgr->GetAchievement(tree);
        if (!achievement)
            continue;

        if (!CanUpdateCriteria(tree, achievement, miscValue1, miscValue2, miscValue3, unit, referencePlayer))
            continue;

        // Requirements not found in the dbc
        if (AchievementCriteriaDataSet const* data = sAchievementMgr->GetCriteriaDataSet(tree->CriteriaID))
            if (!data->Meets(referencePlayer, unit, miscValue1))
                continue;

        CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
        if (!criteria)
            continue;

        CriteriaValueEntry value = { criteria->Asset, tree->Amount };

        switch (type)
        {
            // std. case: increment at 1
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
        case ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL:
        case ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION:
        case ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS:    // @TODO : for online player only currently
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED:
        case ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED:
        case ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
        case ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATHS_FROM:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
        case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
        case ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL:
        case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
        case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
        case ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS:
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA: // This also behaves like ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE_GUILD:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ARCHAEOLOGY_PROJECTS:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE_TYPE:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_SCENARIO:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_CHALLENGE_DUNGEON:
            SetCriteriaProgress(tree, 1, referencePlayer, PROGRESS_ACCUMULATE);
            break;
            // std case: increment at miscValue1
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_VENDORS:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:     //@TODO : for online player only currently
        case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_DAMAGE_RECEIVED:
        case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_HEALING_RECEIVED:
        case ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
        case ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE:
        case ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE:
        case ACHIEVEMENT_CRITERIA_TYPE_CATCH_FROM_POOL:
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS:
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS:
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_TABARD:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_GUILD:
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILLS_GUILD:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND:
            SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_ACCUMULATE);
            break;
            // std case: increment at miscValue2
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE:
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_CURRENCY:
            SetCriteriaProgress(tree, miscValue2, referencePlayer, PROGRESS_ACCUMULATE);
            break;
            // std case: high value at miscValue1
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:        //@TODO : for online player only currently
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_DEALT:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_RECEIVED:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEAL_CASTED:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED:
            SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_HIGHEST);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
            SetCriteriaProgress(tree, referencePlayer->getLevel(), referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
            if (uint32 skillvalue = referencePlayer->GetBaseSkillValue(value.reach_skill_level.skillID))
                SetCriteriaProgress(tree, skillvalue, referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            if (uint32 maxSkillvalue = referencePlayer->GetPureMaxSkillValue(value.learn_skill_level.skillID))
                SetCriteriaProgress(tree, maxSkillvalue, referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
            SetCriteriaProgress(tree, referencePlayer->GetRewardedQuestCount(), referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
        {
            time_t nextDailyResetTime = sWorld->GetNextDailyQuestsResetTime();
            CriteriaProgress *progress = GetCriteriaProgress(tree->ID);

            if (!miscValue1) // Login case.
            {
                // Reset if player missed one day.
                if (progress && progress->date < (nextDailyResetTime - 2 * DAY))
                    SetCriteriaProgress(tree, 0, referencePlayer, PROGRESS_SET);
                continue;
            }

            ProgressType progressType;
            if (!progress)
                // 1st time. Start count.
                progressType = PROGRESS_SET;
            else if (progress->date < (nextDailyResetTime - 2 * DAY))
                // Last progress is older than 2 days. Player missed 1 day => Restart count.
                progressType = PROGRESS_SET;
            else if (progress->date < (nextDailyResetTime - DAY))
                // Last progress is between 1 and 2 days. => 1st time of the day.
                progressType = PROGRESS_ACCUMULATE;
            else
                // Last progress is within the day before the reset => Already counted today.
                continue;

            SetCriteriaProgress(tree, 1, referencePlayer, progressType);
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
        {
            uint32 counter = 0;

            const RewardedQuestSet &rewQuests = referencePlayer->getRewardedQuests();
            for (RewardedQuestSet::const_iterator itr = rewQuests.begin(); itr != rewQuests.end(); ++itr)
            {
                Quest const* quest = sObjectMgr->GetQuestTemplate(*itr);
                if (quest && quest->GetZoneOrSort() >= 0 && uint32(quest->GetZoneOrSort()) == value.complete_quests_in_zone.zoneID)
                    ++counter;
            }
            SetCriteriaProgress(tree, counter, referencePlayer);
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
            // miscValue1 is the ingame fallheight*100 as stored in dbc
            SetCriteriaProgress(tree, miscValue1, referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
        case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            SetCriteriaProgress(tree, 1, referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
            SetCriteriaProgress(tree, referencePlayer->GetBankBagSlotCount(), referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
        {
            int32 reputation = referencePlayer->GetReputationMgr().GetReputation(value.gain_reputation.factionID);
            if (reputation > 0)
                SetCriteriaProgress(tree, reputation, referencePlayer);
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
            SetCriteriaProgress(tree, referencePlayer->GetReputationMgr().GetExaltedFactionCount(), referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
        {
            uint32 spellCount = 0;
            for (PlayerSpellMap::const_iterator spellIter = referencePlayer->GetSpellMap().begin();
                spellIter != referencePlayer->GetSpellMap().end();
                ++spellIter)
            {
                SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellIter->first);
                for (SkillLineAbilityMap::const_iterator skillIter = bounds.first; skillIter != bounds.second; ++skillIter)
                {
                    if (skillIter->second->SkillLine == value.learn_skillline_spell.skillLine)
                        spellCount++;
                }
            }
            SetCriteriaProgress(tree, spellCount, referencePlayer);
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
            SetCriteriaProgress(tree, referencePlayer->GetReputationMgr().GetReveredFactionCount(), referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
            SetCriteriaProgress(tree, referencePlayer->GetReputationMgr().GetHonoredFactionCount(), referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_KNOWN_FACTIONS:
            SetCriteriaProgress(tree, referencePlayer->GetReputationMgr().GetVisibleFactionCount(), referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE:
        {
            uint32 spellCount = 0;
            for (PlayerSpellMap::const_iterator spellIter = referencePlayer->GetSpellMap().begin();
                spellIter != referencePlayer->GetSpellMap().end();
                ++spellIter)
            {
                SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellIter->first);
                for (SkillLineAbilityMap::const_iterator skillIter = bounds.first; skillIter != bounds.second; ++skillIter)
                    if (skillIter->second->SkillLine == value.learn_skill_line.skillLine)
                        spellCount++;
            }
            SetCriteriaProgress(tree, spellCount, referencePlayer);
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
            SetCriteriaProgress(tree, referencePlayer->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS), referencePlayer);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
            SetCriteriaProgress(tree, referencePlayer->GetMoney(), referencePlayer, PROGRESS_HIGHEST);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
            if (!miscValue1)
                SetCriteriaProgress(tree, _achievementPoints, referencePlayer, PROGRESS_SET);
            else
                SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_ACCUMULATE);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_BG_RATING:
            SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_HIGHEST);
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING:
        {
            uint32 reqTeamType = value.highest_team_rating.teamtype;

            if (miscValue1)
            {
                if (miscValue2 != reqTeamType)
                    continue;
                SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_HIGHEST);
            }
            else // Login case
            {
                for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
                {
                    SetCriteriaProgress(tree, referencePlayer->GetArenaPersonalRating(arena_slot), referencePlayer, PROGRESS_HIGHEST);
                    break;
                }
            }
        }
        break;
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
        {
            uint32 reqTeamType = value.highest_personal_rating.teamtype;

            if (miscValue1)
            {
                if (miscValue2 != reqTeamType)
                    continue;

                SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_HIGHEST);
            }

            else // Login case
            {
                for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
                    SetCriteriaProgress(tree, referencePlayer->GetArenaPersonalRating(arena_slot), referencePlayer, PROGRESS_HIGHEST);
            }

            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_SPENT_GOLD_GUILD_REPAIRS:
        {
            if (!miscValue1)
                continue;

            SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_ACCUMULATE);
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_GUILD_LEVEL:
        {
            if (!miscValue1)
                continue;

            SetCriteriaProgress(tree, miscValue1, referencePlayer);
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_CRAFT_ITEMS_GUILD:
        {
            if (!miscValue1 || !miscValue2)
                continue;

            SetCriteriaProgress(tree, miscValue2, referencePlayer, PROGRESS_ACCUMULATE);
            break;
        }
        // FIXME: not triggered in code as result, need to implement
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_RAID:
        case ACHIEVEMENT_CRITERIA_TYPE_PLAY_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_RANK:
        case ACHIEVEMENT_CRITERIA_TYPE_EARNED_PVP_TITLE:
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE:
            break;                                   // Not implemented yet :(
        }

        if (IsCompletedCriteria(tree, achievement))
            CompletedCriteriaFor(achievement, referencePlayer, p_LoginCheck);

        // Check again the completeness for SUMM and REQ COUNT achievements,
        // as they don't depend on the completed criteria but on the sum of the progress of each individual criteria
        if (achievement->flags & ACHIEVEMENT_FLAG_SUMM)
            if (IsCompletedAchievement(achievement, referencePlayer))
                CompletedAchievement(achievement, referencePlayer, p_LoginCheck);

        if (AchievementEntryList const* achRefList = sAchievementMgr->GetAchievementByReferencedId(achievement->ID))
            for (AchievementEntryList::const_iterator itr = achRefList->begin(); itr != achRefList->end(); ++itr)
                if (IsCompletedAchievement(*itr, referencePlayer))
                    CompletedAchievement(*itr, referencePlayer, p_LoginCheck);
    }
}

template<>
bool AchievementMgr<Guild>::CanCompleteCriteria(AchievementEntry const* achievement)
{
    if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL | ACHIEVEMENT_FLAG_REALM_FIRST_GUILD))
    {
        // Someone on this realm has already completed that achievement
        if (sAchievementMgr->IsRealmCompleted(achievement))
            return false;
    }
    return true;
}

template<>
bool AchievementMgr<Player>::CanCompleteCriteria(AchievementEntry const* achievement)
{
    if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
    {
        // Someone on this realm has already completed that achievement
        if (sAchievementMgr->IsRealmCompleted(achievement))
            return false;

        if (GetOwner())
            if (GetOwner()->GetSession())
                if (GetOwner()->GetSession()->GetSecurity())
                    return false;
    }

    return true;
}

template<class T>
bool AchievementMgr<T>::IsCompletedCriteria(CriteriaTreeEntry const* tree, AchievementEntry const* achievement)
{
    // counter can never complete
    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
        return false;
 
    if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL | ACHIEVEMENT_FLAG_REALM_FIRST_GUILD))
    {
        // someone on this realm has already completed that achievement
        if (sAchievementMgr->IsRealmCompleted(achievement))
            return false;
    }

    CriteriaProgress const* progress = GetCriteriaProgress(tree->ID);
    if (!progress)
        return false;

    CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
    if (!criteria)
        return false;

    CriteriaValueEntry value = { criteria->Asset, tree->Amount };
    switch (AchievementCriteriaTypes(criteria->Type))
    {
    case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG:
        return progress->counter >= value.win_bg.winCount;
    case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
        return progress->counter >= value.kill_creature.creatureCount;
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ARCHAEOLOGY_PROJECTS:
        return progress->counter >= value.archaeology.count;
    case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
        return progress->counter >= value.reach_level.level;
    case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
        return progress->counter >= value.reach_skill_level.skillLevel;
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
    case ACHIEVEMENT_CRITERIA_TYPE_WIN_CHALLENGE_DUNGEON:
        return progress->counter >= 1;
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        return progress->counter >= value.complete_quest_count.totalQuestCount;
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
        return progress->counter >= value.complete_daily_quest_daily.numberOfDays;
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
        return progress->counter >= value.complete_quests_in_zone.questCount;
    case ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE:
    case ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE:
        return progress->counter >= value.healing_done.count;
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
        return progress->counter >= value.complete_daily_quest.questCount;
    case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
        return progress->counter >= value.fall_without_dying.fallHeight;
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
        return progress->counter >= value.complete_quest.questCount;
    case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
    case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
        return progress->counter >= value.be_spell_target.spellCount;
    case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL:
    case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2:
        return progress->counter >= value.cast_spell.castCount;
    case ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
        return progress->counter >= value.bg_objective.completeCount;
    case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
        return progress->counter >= value.honorable_kill_at_area.killCount;
    case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
        return progress->counter >= 1;
    case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL:
    case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
        return progress->counter >= value.honorable_kill.killCount;
    case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
        return progress->counter >= value.own_item.itemCount;
    case ACHIEVEMENT_CRITERIA_TYPE_REACH_BG_RATING:
        return progress->counter >= value.reach_rbg_rating.max_rating;
    case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        return progress->counter >= value.win_rated_arena.count;
    case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
        return progress->counter >= value.highest_personal_rating.PersonalRating;
    case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
        return progress->counter >= (value.learn_skill_level.skillLevel * 75);
    case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
        return progress->counter >= value.use_item.itemCount;
    case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
        return progress->counter >= value.loot_item.itemCount;
    case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
        return progress->counter >= 1;
    case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
        return progress->counter >= value.buy_bank_slot.numberOfSlots;
    case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
        return progress->counter >= value.gain_reputation.reputationAmount;
    case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        return progress->counter >= value.gain_exalted_reputation.numberOfExaltedFactions;
    case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
        return progress->counter >= value.visit_barber.numberOfVisits;
    case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
        return progress->counter >= value.equip_epic_item.count;
    case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
    case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
        return progress->counter >= value.roll_greed_on_loot.count;
    case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
        return progress->counter >= value.hk_class.count;
    case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
        return progress->counter >= value.hk_race.count;
    case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
        return progress->counter >= value.do_emote.count;
    case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
        return progress->counter >= value.equip_item.count;
    case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
        return progress->counter >= value.quest_reward_money.goldInCopper;
    case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
        return progress->counter >= value.loot_money.goldInCopper;
    case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
        return progress->counter >= value.use_gameobject.useCount;
    case ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL:
        return progress->counter >= value.special_pvp_kill.killCount;
    case ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
        return progress->counter >= value.fish_in_gameobject.lootCount;
    case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
        return progress->counter >= value.learn_skillline_spell.spellCount;
    case ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL:
        return progress->counter >= value.win_duel.duelCount;
    case ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM:
        return progress->counter >= value.loot_epic_item.lootCount;
    case ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE:
        return progress->counter >= value.loot_type.lootTypeCount;
    case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE:
        return progress->counter >= value.learn_skill_line.spellCount;
    case ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
        return progress->counter >= 9000;
    case ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
        return progress->counter >= value.use_lfg.dungeonsComplete;
    case ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS:
        return progress->counter >= value.get_killing_blow.killCount;
    case ACHIEVEMENT_CRITERIA_TYPE_CURRENCY:
        return progress->counter >= value.currencyGain.count;
    case ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA:
        return value.win_arena.count && progress->counter >= value.win_arena.count;
    case ACHIEVEMENT_CRITERIA_TYPE_SPENT_GOLD_GUILD_REPAIRS:
        return progress->counter >= value.spent_gold_guild_repairs.goldCount;
    case ACHIEVEMENT_CRITERIA_TYPE_REACH_GUILD_LEVEL:
        return progress->counter >= value.reach_guild_level.level;
    case ACHIEVEMENT_CRITERIA_TYPE_CRAFT_ITEMS_GUILD:
        return progress->counter >= value.craft_items_guild.itemsCount;
    case ACHIEVEMENT_CRITERIA_TYPE_CATCH_FROM_POOL:
        return progress->counter >= value.catch_from_pool.catchCount;
    case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS:
        return progress->counter >= value.buy_guild_bank_slots.slotsCount;
    case ACHIEVEMENT_CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS:
        return progress->counter >= value.earn_guild_achievement_points.pointsCount;
    case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND:
        return progress->counter >= value.win_rated_battleground.winCount;
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_GUILD:
        return progress->counter >= value.complete_quests_guild.questCount;
    case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_TABARD:
        return progress->counter >= 1;
    case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILLS_GUILD:
        return progress->counter >= value.honorable_kills_guild.killCount;
    case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE_GUILD:
        return progress->counter >= value.kill_creature_type_guild.count;
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE_TYPE:
        return progress->counter >= value.guild_challenge_complete_type.count;
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE:
        return progress->counter >= value.guild_challenge_complete.count;
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_SCENARIO:
        return progress->counter >= value.scenario_complete.count;

        // handle all statistic-only criteria here
    case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
    case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
    case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
    case ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON:
    case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
    case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
    case ACHIEVEMENT_CRITERIA_TYPE_DEATHS_FROM:
    case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING:
    case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_VENDORS:
    case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
    case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
    case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
    case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
    case ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL:
    case ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:
    case ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION:
    case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID:
    case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:
    case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
    case ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS:
    case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
    case ACHIEVEMENT_CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
    case ACHIEVEMENT_CRITERIA_TYPE_KNOWN_FACTIONS:
    case ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
    case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED:
    case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED:
    case ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED:
    case ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
    case ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
    default:
        break;
    }
    return false;
}

template<class T>
void AchievementMgr<T>::CompletedCriteriaFor(AchievementEntry const* achievement, Player* referencePlayer, bool p_LoginCheck)
{
    // Counter can never complete
    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
        return;

    // Already completed and stored
    if (HasAchieved(achievement->ID))
        return;

    if (IsCompletedAchievement(achievement, referencePlayer))
        CompletedAchievement(achievement, referencePlayer, p_LoginCheck);
}

template<class T>
bool AchievementMgr<T>::IsCompletedAchievement(AchievementEntry const* entry, Player* referencePlayer, uint64* completedCriterias)
{
    // Counter can never complete
    if (entry->flags & ACHIEVEMENT_FLAG_COUNTER)
        return false;

    // For achievement with referenced achievement criterias get from referenced and counter from self
    uint32 achievementForTestId = entry->SharesCriteria ? entry->SharesCriteria : entry->ID;
    uint32 achievementForTestCount = entry->MinimumCriteria;

    if (CriteriaTreeEntry const* CriteriaTree = sCriteriaTreeStore.LookupEntry(entry->CriteriaTree))
        achievementForTestCount = CriteriaTree->Amount;

    CriteriaTreeEntryList const* cList = sAchievementMgr->GetAchievementCriteriaByAchievement(achievementForTestId);
    if (!cList)
        return false;

    uint64 count = 0;

    // For SUMM achievements, we have to count the progress of each criteria of the achievement.
    // Oddly, the target count is NOT contained in the achievement, but in each individual criteria
    if (entry->flags & ACHIEVEMENT_FLAG_SUMM)
    {
        for (CriteriaTreeEntry const* tree : *cList)
        {
            CriteriaProgress const* progress = GetCriteriaProgress(tree->ID);
            if (!progress)
                continue;

            count += progress->counter;
            if (count >= tree->Amount)
                return true;
        }
        return false;
    }

    // Default case - need complete all or
    bool completed_all = true;
    for (CriteriaTreeEntry const* tree : *cList)
    {
        const int allianceMask = 1 << 10;
        const int hordeMask = 1 << 11;

        const int FactionMask = allianceMask | hordeMask;

        /* Check tree faction using flags, skip if not match*/
        if (!IsGuild<T>() && referencePlayer && (tree->Flags & FactionMask))
        {
            if ((referencePlayer->GetTeam() == HORDE && !(tree->Flags & hordeMask)) || (referencePlayer->GetTeam() == ALLIANCE && !(tree->Flags & allianceMask)))
                continue;
        }

        bool completed = IsCompletedCriteria(tree, entry);

        if (!completed)
            completed = CheckForOtherFactionCriteria(tree);

        // Found an uncompleted criteria, but DONT return false yet - there might be a completed criteria with ACHIEVEMENT_CRITERIA_COMPLETE_FLAG_ALL
        if (completed)
            ++count;
        else
            completed_all = false;

        if (completedCriterias)
            *completedCriterias = count;

        // Completed as have req. count of completed criterias
        if (achievementForTestCount > 0 && achievementForTestCount <= count)
           return true;
    }

    // All criterias completed requirement
    if (completed_all && achievementForTestCount == 0)
        return true;

    return false;
}

template<class T>
CriteriaProgress* AchievementMgr<T>::GetCriteriaProgress(uint32 entry)
{
    CriteriaProgressMap* criteriaProgressMap = GetCriteriaProgressMap();
    if (!criteriaProgressMap)
        return NULL;

    CriteriaProgressMap::iterator iter = criteriaProgressMap->find(entry);
    if (iter == criteriaProgressMap->end())
        return NULL;

    return &(iter->second);
}

template<class T>
void AchievementMgr<T>::SetCriteriaProgress(CriteriaTreeEntry const* tree, uint64 changeValue, Player* referencePlayer, ProgressType ptype)
{
    CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);

    // Don't allow to cheat - doing timed achievements without timer active
    TimedAchievementMap::iterator timedIter = m_timedAchievements.find(tree->ID);
    if (criteria->StartTimer && timedIter == m_timedAchievements.end())
        return;

    CriteriaProgress* progress = GetCriteriaProgress(tree->ID);
    if (!progress)
    {
        // Not create record for 0 counter but allow it for timed achievements
        // We will need to send 0 progress to client to start the timer
        if (changeValue == 0 && !criteria->StartTimer)
            return;

        CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
        if (!progressMap)
            return;

        progress = &(*progressMap)[tree->ID];
        progress->counter = changeValue;
    }
    else
    {
        uint64 newValue = 0;
        switch (ptype)
        {
            case PROGRESS_SET:
                newValue = changeValue;
                break;
            case PROGRESS_ACCUMULATE:
            {
                // Avoid overflow
                uint64 max_value = std::numeric_limits<uint64>::max();
                newValue = max_value - progress->counter > changeValue ? progress->counter + changeValue : max_value;
                break;
            }
            case PROGRESS_HIGHEST:
                newValue = progress->counter < changeValue ? changeValue : progress->counter;
                break;
        }

        // Not update (not mark as changed) if counter will have same value
        if (progress->counter == newValue && !criteria->StartTimer)
            return;

        progress->counter = newValue;
    }

    progress->changed = true;
    progress->date = time(NULL); // Set the date to the latest update.

    AchievementEntry const* achievement = sAchievementMgr->GetAchievement(tree);
    uint32 timeElapsed = 0;
    bool criteriaComplete = IsCompletedCriteria(tree, achievement);

    if (criteria->StartTimer)
    {
        // Client expects this in packet
        timeElapsed = criteria->StartTimer - (timedIter->second / IN_MILLISECONDS);

        // Remove the timer, we wont need it anymore
        if (criteriaComplete)
            m_timedAchievements.erase(timedIter);
    }

    if (criteriaComplete && achievement->flags & ACHIEVEMENT_FLAG_SHOW_CRITERIA_MEMBERS && !progress->CompletedGUID && referencePlayer)
        progress->CompletedGUID = referencePlayer->GetGUID();

    SendCriteriaUpdate(criteria, progress, timeElapsed, criteriaComplete);
}

template<class T>
void AchievementMgr<T>::UpdateTimedAchievements(uint32 timeDiff)
{
    if (!m_timedAchievements.empty())
    {
        for (TimedAchievementMap::iterator itr = m_timedAchievements.begin(); itr != m_timedAchievements.end();)
        {
            // Time is up, remove timer and reset progress
            if (itr->second <= timeDiff)
            {
                CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(itr->first);
                RemoveCriteriaProgress(tree);
                m_timedAchievements.erase(itr++);
            }
            else
            {
                itr->second -= timeDiff;
                ++itr;
            }
        }
    }
}

template<class T>
void AchievementMgr<T>::StartTimedAchievement(AchievementCriteriaTimedTypes /*type*/, uint32 /*entry*/, uint32 /*timeLost = 0*/)
{
}

template<>
void AchievementMgr<Player>::StartTimedAchievement(AchievementCriteriaTimedTypes type, uint32 entry, uint32 timeLost /* = 0 */)
{
    CriteriaTreeEntryList const& list = sAchievementMgr->GetTimedAchievementCriteriaByType(type);
    for (CriteriaTreeEntry const* tree : list)
    {
        CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
        if (criteria->StartAsset != entry)
            continue;

        AchievementEntry const* achievement = sAchievementMgr->GetAchievement(tree);
        if (!achievement)
            continue;

        if (m_timedAchievements.find(tree->ID) == m_timedAchievements.end() && !IsCompletedCriteria(tree, achievement))
        {
            // Start the timer
            if (criteria->StartTimer * IN_MILLISECONDS > timeLost)
            {
                m_timedAchievements[tree->ID] = criteria->StartTimer * IN_MILLISECONDS - timeLost;

                // And at client too
                SetCriteriaProgress(tree, 0, GetOwner(), PROGRESS_SET);
            }
        }
    }
}

template<class T>
void AchievementMgr<T>::RemoveTimedAchievement(AchievementCriteriaTimedTypes type, uint32 entry)
{
    CriteriaTreeEntryList const& list = sAchievementMgr->GetTimedAchievementCriteriaByType(type);
    for (CriteriaTreeEntry const* tree : list)
    {
        CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
        if (criteria->StartAsset != entry)
            continue;

        TimedAchievementMap::iterator timedIter = m_timedAchievements.find(tree->ID);
        // We don't have timer for this achievement
        if (timedIter == m_timedAchievements.end())
            continue;

        // Remove progress
        RemoveCriteriaProgress(tree);

        // Remove the timer
        m_timedAchievements.erase(timedIter);
    }
}

template<class T>
void AchievementMgr<T>::CompletedAchievement(AchievementEntry const* achievement, Player* referencePlayer, bool p_LoginCheck)
{
    // Disable for gamemasters with GM-mode enabled
    //if (GetOwner()->isGameMaster())
        //return;

    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER || HasAchieved(achievement->ID))
        return;

    if (achievement->flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_NEWS)
        if (Guild* guild = sGuildMgr->GetGuildById(referencePlayer->GetGuildId()))
            guild->GetNewsLog().AddNewEvent(GUILD_NEWS_PLAYER_ACHIEVEMENT, time(NULL), referencePlayer->GetGUID(), achievement->flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_HEADER, achievement->ID);

    if (!GetOwner()->GetSession()->PlayerLoading() && !p_LoginCheck)
        SendAchievementEarned(achievement);

    if (HasAccountAchieved(achievement->ID))
    {
        m_CompletedAchievementsLock.acquire();
        CompletedAchievementData& l_Data = m_completedAchievements[achievement->ID];
        l_Data.completedByThisCharacter = true;
        l_Data.changed = true;
        m_CompletedAchievementsLock.release();
        return;
    }

    m_CompletedAchievementsLock.acquire();
    CompletedAchievementData& l_Data = m_completedAchievements[achievement->ID];
    l_Data.completedByThisCharacter = true;
    l_Data.date = time(NULL);
    l_Data.first_guid = GetOwner()->GetGUIDLow();
    l_Data.changed = true;
    m_CompletedAchievementsLock.release();

    // Don't insert for ACHIEVEMENT_FLAG_REALM_FIRST_KILL since otherwise only the first group member would reach that achievement
    // @TODO: where do set this instead?
    if (!(achievement->flags & ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
        sAchievementMgr->SetRealmCompleted(achievement);

    _achievementPoints += achievement->points;

    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT, 0, 0, 0, NULL, referencePlayer);
    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS, achievement->points, 0, 0, NULL, referencePlayer);

    // Reward items and titles if any
    AchievementReward const* reward = sAchievementMgr->GetAchievementReward(achievement);

    // No rewards
    if (!reward)
        return;

    // Titles
    //! Currently there's only one achievement that deals with gender-specific titles.
    //! Since no common attributes were found, (not even in titleRewardFlags field)
    //! we explicitly check by ID. Maybe in the future we could move the achievement_reward
    //! condition fields to the condition system.
    if (uint32 titleId = reward->titleId[achievement->ID == 1793 ? GetOwner()->getGender() : (GetOwner()->GetTeam() == ALLIANCE ? 0 : 1)])
        if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId))
            GetOwner()->SetTitle(titleEntry);

    // Mail
    if (reward->sender)
    {
        Item* item = reward->itemId ? Item::CreateItem(reward->itemId, 1, GetOwner()) : NULL;

        int loc_idx = GetOwner()->GetSession()->GetSessionDbLocaleIndex();

        // Subject and text
        std::string subject = reward->subject;
        std::string text = reward->text;
        if (loc_idx >= 0)
        {
            if (AchievementRewardLocale const* loc = sAchievementMgr->GetAchievementRewardLocale(achievement))
            {
                ObjectMgr::GetLocaleString(loc->subject, loc_idx, subject);
                ObjectMgr::GetLocaleString(loc->text, loc_idx, text);
            }
        }

        MailDraft draft(subject, text);

        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        if (item)
        {
            // Save new item before send
            item->SaveToDB(trans);                               // Save for prevent lost at next mail load, if send fail then item will deleted

            // Item
            draft.AddItem(item);
        }

        draft.SendMailTo(trans, GetOwner(), MailSender(MAIL_CREATURE, reward->sender));
        CharacterDatabase.CommitTransaction(trans);
    }
}

template<>
void AchievementMgr<Guild>::CompletedAchievement(AchievementEntry const* achievement, Player* referencePlayer, bool p_LoginCheck)
{
    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER || HasAchieved(achievement->ID))
        return;

    if (achievement->flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_NEWS)
        if (Guild* guild = sGuildMgr->GetGuildById(referencePlayer->GetGuildId()))
            guild->GetNewsLog().AddNewEvent(GUILD_NEWS_GUILD_ACHIEVEMENT, time(NULL), 0, achievement->flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_HEADER, achievement->ID);

    SendAchievementEarned(achievement);
    CompletedAchievementData& ca = m_completedAchievements[achievement->ID];
    ca.date = time(NULL);
    ca.changed = true;

    if (achievement->flags & ACHIEVEMENT_FLAG_SHOW_GUILD_MEMBERS)
    {
        if (referencePlayer->GetGuildId() == GetOwner()->GetId())
            ca.guids.insert(referencePlayer->GetGUID());

        if (Group const* group = referencePlayer->GetGroup())
            for (GroupReference const* ref = group->GetFirstMember(); ref != NULL; ref = ref->next())
                if (Player const* groupMember = ref->getSource())
                    if (groupMember->GetGuildId() == GetOwner()->GetId())
                        ca.guids.insert(groupMember->GetGUID());
    }

    sAchievementMgr->SetRealmCompleted(achievement);

    _achievementPoints += achievement->points;

    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT, 0, 0, 0, NULL, referencePlayer);
    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS, achievement->points, 0, 0, NULL, referencePlayer);
    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS, achievement->points, 0, 0, NULL, referencePlayer);
}

struct VisibleAchievementPred
{
    bool operator()(CompletedAchievementMap::value_type const& val)
    {
        AchievementEntry const* achievement = sAchievementMgr->GetAchievement(val.first);
        return achievement && !(achievement->flags & ACHIEVEMENT_FLAG_HIDDEN);
    }
};

template<class T>
void AchievementMgr<T>::SendAllAchievementData(Player* /*receiver*/)
{
    const CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
    if (!progressMap)
        return;

    std::map<uint32, uint64> _progress;
    for (auto const& kvp : *progressMap)
    {
        uint32 current = _progress[sCriteriaTreeStore.LookupEntry(kvp.first)->CriteriaID];
        _progress[sCriteriaTreeStore.LookupEntry(kvp.first)->CriteriaID] = std::max<uint32>(kvp.second.counter, current);
    }

    VisibleAchievementPred isVisible;
    size_t numCriteria = progressMap->size();

    m_CompletedAchievementsLock.acquire();
    size_t numAchievements = std::count_if(m_completedAchievements.begin(), m_completedAchievements.end(), isVisible);
    ObjectGuid guid = GetOwner()->GetGUID();

    WorldPacket data(SMSG_ALL_ACHIEVEMENT_DATA);

    data.WriteBits(numCriteria, 19);

    for (auto const& kvp : *progressMap)
    {
        CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(kvp.first);
        if (!tree)
            continue;

        CriteriaProgress const& progress = kvp.second;
        ObjectGuid counter = ObjectGuid(_progress[tree->CriteriaID]);
        
        data.WriteBit(counter[3]);
        data.WriteBit(counter[6]);
        data.WriteBit(guid[0]);
        data.WriteBit(guid[7]);
        data.WriteBit(counter[7]);
        data.WriteBit(counter[0]);
        data.WriteBit(counter[5]);
        data.WriteBits(0, 4);
        data.WriteBit(counter[4]);
        data.WriteBit(guid[3]);
        data.WriteBit(guid[6]);
        data.WriteBit(guid[4]);
        data.WriteBit(guid[5]);
        data.WriteBit(guid[1]);
        data.WriteBit(counter[2]);
        data.WriteBit(counter[1]);
        data.WriteBit(guid[2]);
    }
    
    data.WriteBits(numAchievements, 20);

    for (CompletedAchievementMap::const_iterator itr = m_completedAchievements.begin(); itr != m_completedAchievements.end(); ++itr)
    {
        if (!isVisible(*itr))
            continue;

        ObjectGuid firstAccountGuid = (*itr).second.first_guid;

        uint8 bitOrder[8] = { 4, 7, 2, 3, 1, 5, 6, 0 };
        data.WriteBitInOrder(firstAccountGuid, bitOrder);
    }

    for (CompletedAchievementMap::const_iterator itr = m_completedAchievements.begin(); itr != m_completedAchievements.end(); ++itr)
    {
        if (!isVisible(*itr))
            continue;

        ObjectGuid firstAccountGuid = (*itr).second.first_guid;

        data.WriteByteSeq(firstAccountGuid[6]);        
        data.WriteByteSeq(firstAccountGuid[5]);
        data.WriteByteSeq(firstAccountGuid[3]);
        data.WriteByteSeq(firstAccountGuid[0]);
        data.WriteByteSeq(firstAccountGuid[2]);
        data.WriteByteSeq(firstAccountGuid[1]);
        data << uint32(itr->first);
        data << uint32(realmID);                                   // Unk timer from 5.4.0 17399, sometimes 50724907
        data << uint32(realmID);                                   // Unk timer from 5.4.0 17399, sometimes 50724907
        data.WriteByteSeq(firstAccountGuid[4]);
        data << uint32(secsToTimeBitFields((*itr).second.date));
        data.WriteByteSeq(firstAccountGuid[7]);
    }
    m_CompletedAchievementsLock.release();

    for (auto const& kvp : *progressMap)
    {
        CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(kvp.first);
        if (!tree)
            continue;

        CriteriaProgress const& progress = kvp.second;
        ObjectGuid counter = ObjectGuid(_progress[tree->CriteriaID]);
        
        data << uint32(secsToTimeBitFields(progress.date));
        data.WriteByteSeq(counter[2]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[3]);
        data << uint32(tree->CriteriaID);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(counter[4]);
        data.WriteByteSeq(counter[7]);
        data.WriteByteSeq(counter[0]);
        data.WriteByteSeq(guid[0]);
        data << uint32(0);              // Unk value from 5.4.0, always 0
        data.WriteByteSeq(counter[1]);
        data.WriteByteSeq(counter[3]);
        data.WriteByteSeq(counter[6]);
        data.WriteByteSeq(counter[5]);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[7]);
        data << uint32(0);              // Unk value from 5.4.0, always 0
    }

    SendPacket(&data);
}

template<>
void AchievementMgr<Guild>::SendAllAchievementData(Player* receiver)
{
    WorldPacket data(SMSG_GUILD_ACHIEVEMENT_DATA, m_completedAchievements.size() * (4 + 4) + 3);
    data.WriteBits(m_completedAchievements.size(), 20);
    for (CompletedAchievementMap::const_iterator itr = m_completedAchievements.begin(); itr != m_completedAchievements.end(); ++itr)
    {
        data.WriteBit(0); // 3
        data.WriteBit(0); // 5
        data.WriteBit(0); // 4
        data.WriteBit(0); // 7
        data.WriteBit(0); // 2
        data.WriteBit(0); // 1
        data.WriteBit(0); // 0
        data.WriteBit(0); // 6
    }

    for (CompletedAchievementMap::const_iterator itr = m_completedAchievements.begin(); itr != m_completedAchievements.end(); ++itr)
    {
        // data.WriteByteSeq 2
        // data.WriteByteSeq 7
        data << uint32(0);
        // data.WriteByteSeq 5
        // data.WriteByteSeq 3
        // data.WriteByteSeq 1
        data << uint32(itr->first);
        // data.WriteByteSeq 6
        data << uint32(0);
        // data.WriteByteSeq 4
        // data.WriteByteSeq 0
        data << uint32(secsToTimeBitFields(itr->second.date));
    }

    receiver->GetSession()->SendPacket(&data);
    
    CriteriaProgressMap const* progressMap = GetCriteriaProgressMap();
    if (!progressMap)
        return;

    std::map<uint32, uint64> _progress;
    for (auto const& kvp : *progressMap)
    {
        uint32 current = _progress[sCriteriaTreeStore.LookupEntry(kvp.first)->CriteriaID];
        _progress[sCriteriaTreeStore.LookupEntry(kvp.first)->CriteriaID] = std::max<uint32>(kvp.second.counter, current);
    }

    data.Initialize(SMSG_GUILD_CRITERIA_DATA);
    data.WriteBits(progressMap->size(), 19);
    for (auto const& kvp : *progressMap)
    {
        CriteriaProgress const& progress = kvp.second;
        ObjectGuid counter = ObjectGuid(_progress[sCriteriaTreeStore.LookupEntry(kvp.first)->CriteriaID]);
        ObjectGuid guid = progress.CompletedGUID ? progress.CompletedGUID : GetOwner()->GetGUID();

        data.WriteBit(guid[3]);
        data.WriteBit(guid[5]);
        data.WriteBit(counter[4]);
        data.WriteBit(guid[0]);
        data.WriteBit(counter[6]);
        data.WriteBit(counter[3]);
        data.WriteBit(counter[7]);
        data.WriteBit(counter[5]);
        data.WriteBit(guid[1]);
        data.WriteBit(guid[4]);
        data.WriteBit(guid[7]);
        data.WriteBit(counter[1]);
        data.WriteBit(guid[6]);
        data.WriteBit(counter[0]);
        data.WriteBit(counter[2]);
        data.WriteBit(guid[2]);
    }
    
    for (auto const& kvp : *progressMap)
    {
        CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(kvp.first);
        CriteriaProgress const& progress = kvp.second;
        ObjectGuid counter = ObjectGuid(_progress[tree->CriteriaID]);
        ObjectGuid guid = progress.CompletedGUID ? progress.CompletedGUID : GetOwner()->GetGUID();

        sLog->outInfo(LOG_FILTER_OPCODES, "SendAllAchievementData: %u %u", tree->CriteriaID, uint32(counter));
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(counter[3]);
        data.WriteByteSeq(counter[6]);
        data.WriteByteSeq(counter[5]);
        data.WriteByteSeq(counter[0]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(counter[7]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(counter[4]);
        data.WriteByteSeq(counter[1]);
        data.WriteByteSeq(guid[4]);
        data << uint32(progress.date);      // Unknown date
        data << uint32(tree->CriteriaID);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(counter[2]);
        data << uint32(progress.date);      // Unknown date
        data.WriteByteSeq(guid[3]);
        data << uint32(progress.date);      // Unknown date
        data << uint32(progress.changed);
    }

    receiver->GetSession()->SendPacket(&data);
}

template<class T>
void AchievementMgr<T>::SendAchievementInfo(Player* receiver, uint32 achievementId /*= 0*/)
{
}

template<>
void AchievementMgr<Player>::SendAchievementInfo(Player* receiver, uint32 /*achievementId = 0 */)
{
    CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
    if (!progressMap)
        return;

    ObjectGuid guid = GetOwner()->GetGUID();
    ObjectGuid counter;

    VisibleAchievementPred isVisible;
    size_t numCriteria = progressMap->size();

    m_CompletedAchievementsLock.acquire();
    size_t numAchievements = std::count_if(m_completedAchievements.begin(), m_completedAchievements.end(), isVisible);

    WorldPacket data(SMSG_RESPOND_INSPECT_ACHIEVEMENTS);

    data.WriteBits(numCriteria, 19);
    data.WriteBits(numAchievements, 20);

    for (auto const& kvp : *progressMap)
    {
        CriteriaProgress const& progress = kvp.second;
        counter = progress.counter;

        data.WriteBit(counter[5]);
        data.WriteBits(1, 4);
        data.WriteBit(guid[4]);
        data.WriteBit(counter[6]);
        data.WriteBit(guid[6]);
        data.WriteBit(guid[1]);
        data.WriteBit(counter[0]);
        data.WriteBit(guid[5]);
        data.WriteBit(counter[7]);
        data.WriteBit(guid[2]);
        data.WriteBit(counter[1]);
        data.WriteBit(guid[7]);
        data.WriteBit(counter[2]);
        data.WriteBit(counter[3]);
        data.WriteBit(guid[0]);
        data.WriteBit(counter[4]);
        data.WriteBit(guid[3]);
    }

    data.WriteBit(guid[1]);
    data.WriteBit(guid[7]);

    for (CompletedAchievementMap::const_iterator itr = m_completedAchievements.begin(); itr != m_completedAchievements.end(); ++itr)
    {
        if (!isVisible(*itr))
            continue;

        ObjectGuid firstGuid = (*itr).second.first_guid;

        uint8 bitsOrder[8] = { 4, 3, 0, 6, 2, 5, 1, 7 };
        data.WriteBitInOrder(firstGuid, bitsOrder);
    }

    data.WriteBit(guid[3]);
    data.WriteBit(guid[5]);
    data.WriteBit(guid[6]);
    data.WriteBit(guid[0]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[2]);

    data.WriteByteSeq(guid[2]);

    for (CompletedAchievementMap::const_iterator itr = m_completedAchievements.begin(); itr != m_completedAchievements.end(); ++itr)
    {
        if (!isVisible(*itr))
            continue;

        ObjectGuid firstGuid = (*itr).second.first_guid;

        data.WriteByteSeq(firstGuid[3]);
        data.WriteByteSeq(firstGuid[0]);
        data << uint32(50397223);
        data << uint32(50397223);
        data.WriteByteSeq(firstGuid[4]);
        data << uint32((*itr).first);
        data << uint32(secsToTimeBitFields((*itr).second.date));
        data.WriteByteSeq(firstGuid[7]);
        data.WriteByteSeq(firstGuid[5]);
        data.WriteByteSeq(firstGuid[1]);
        data.WriteByteSeq(firstGuid[6]);
        data.WriteByteSeq(firstGuid[2]);
    }
    m_CompletedAchievementsLock.release();

    for (auto const& kvp : *progressMap)
    {
        CriteriaTreeEntry const* tree = sCriteriaTreeStore.LookupEntry(kvp.first);
        CriteriaProgress const& progress = kvp.second;
        counter = progress.counter;

        data << uint32(227545947);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(counter[5]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(counter[4]);
        data.WriteByteSeq(counter[7]);
        data << uint32(tree->CriteriaID);
        data.WriteByteSeq(counter[0]);
        data.WriteByteSeq(counter[1]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[1]);
        data << uint32(227545947);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(counter[2]);
        data << uint32(secsToTimeBitFields(progress.date));
        data.WriteByteSeq(counter[6]);
        data.WriteByteSeq(counter[3]);
    }

    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[7]);

    receiver->GetSession()->SendPacket(&data);
}

template<>
void AchievementMgr<Guild>::SendAchievementInfo(Player* receiver, uint32 achievementId /*= 0*/)
{
    // Will send response to criteria progress request
    CriteriaTreeEntryList const* list = sAchievementMgr->GetAchievementCriteriaByAchievement(achievementId);
    if (!list)
    {
        // Send empty packet
        WorldPacket data(SMSG_GUILD_CRITERIA_DATA, 3);
        data.WriteBits(0, 19);
        receiver->GetSession()->SendPacket(&data);
        return;
    }

    uint32 numCriteria = 0;

    CriteriaProgressMap* progressMap = GetCriteriaProgressMap();
    if (!progressMap)
        return;

    for (CriteriaTreeEntry const* tree : *list)
    {
        CriteriaProgress* progress = GetCriteriaProgress(tree->ID);
        if (!progress)
            continue;

        ++numCriteria;
    }

    WorldPacket data(SMSG_GUILD_CRITERIA_DATA, numCriteria * (1 + 36));
    data.WriteBits(numCriteria, 19);

    for (CriteriaTreeEntry const* tree : *list)
    {
        CriteriaProgress* progress = GetCriteriaProgress(tree->ID);
        if (!progress)
            continue;

        ObjectGuid counter = progress->counter;
        ObjectGuid guid = progress->CompletedGUID ? progress->CompletedGUID : GetOwner()->GetGUID();

        data.WriteBit(guid[3]);
        data.WriteBit(guid[5]);
        data.WriteBit(counter[4]);
        data.WriteBit(guid[0]);
        data.WriteBit(counter[6]);
        data.WriteBit(counter[3]);
        data.WriteBit(counter[7]);
        data.WriteBit(counter[5]);
        data.WriteBit(guid[1]);
        data.WriteBit(guid[4]);
        data.WriteBit(guid[7]);
        data.WriteBit(counter[1]);
        data.WriteBit(guid[6]);
        data.WriteBit(counter[0]);
        data.WriteBit(counter[2]);
        data.WriteBit(guid[2]);
    }

    for (CriteriaTreeEntry const* tree : *list)
    {
        CriteriaProgress* progress = GetCriteriaProgress(tree->ID);
        if (!progress)
            continue;

        ObjectGuid counter = progress->counter;
        ObjectGuid guid = progress->CompletedGUID ? progress->CompletedGUID : GetOwner()->GetGUID();

        sLog->outInfo(LOG_FILTER_OPCODES, "SendAchievementInfo: %u %u", tree->CriteriaID, uint32(counter));
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(counter[3]);
        data.WriteByteSeq(counter[6]);
        data.WriteByteSeq(counter[5]);
        data.WriteByteSeq(counter[0]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(counter[7]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(counter[4]);
        data.WriteByteSeq(counter[1]);
        data.WriteByteSeq(guid[4]);
        data << uint32(progress->date);      // Unknown date
        data << uint32(tree->CriteriaID);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(counter[2]);
        data << uint32(progress->date);      // Unknown date
        data.WriteByteSeq(guid[3]);
        data << uint32(progress->date);      // Unknown date
        data << uint32(progress->changed);
    }

    receiver->GetSession()->SendPacket(&data);
}

template<class T>
time_t AchievementMgr<T>::GetAchievementCompletedDate(AchievementEntry const* p_Entry) const
{

    auto l_Itr = m_completedAchievements.find(p_Entry->ID);
    if (l_Itr == m_completedAchievements.end())
        return 0;

    return (*l_Itr).second.date;
}

template<>
time_t AchievementMgr<Player>::GetAchievementCompletedDate(AchievementEntry const* p_Entry) const
{

    auto l_Itr = m_completedAchievements.find(p_Entry->ID);
    if (l_Itr == m_completedAchievements.end())
        return 0;

    if ((*l_Itr).second.completedByThisCharacter)
        return (*l_Itr).second.date;

    return 0;
}


template<class T>
bool AchievementMgr<T>::HasAchieved(uint32 achievementId) const
{
    m_CompletedAchievementsLock.acquire();
    bool l_Result = m_completedAchievements.find(achievementId) != m_completedAchievements.end();
    m_CompletedAchievementsLock.release();
    
    return l_Result;
}

template<>
bool AchievementMgr<Player>::HasAchieved(uint32 achievementId) const
{
    m_CompletedAchievementsLock.acquire();
    CompletedAchievementMap::const_iterator itr = m_completedAchievements.find(achievementId);

    if (itr == m_completedAchievements.end())
    {
        m_CompletedAchievementsLock.release();
        return false;
    } 
    m_CompletedAchievementsLock.release();

    return (*itr).second.completedByThisCharacter;
}

template<class T>
bool AchievementMgr<T>::HasAccountAchieved(uint32 achievementId) const
{
    m_CompletedAchievementsLock.acquire();
    bool l_Result = m_completedAchievements.find(achievementId) != m_completedAchievements.end();
    m_CompletedAchievementsLock.release();
    
    return l_Result;
}

template<class T>
uint64 AchievementMgr<T>::GetFirstAchievedCharacterOnAccount(uint32 achievementId) const
{
    m_CompletedAchievementsLock.acquire();
    CompletedAchievementMap::const_iterator itr = m_completedAchievements.find(achievementId);

    if (itr == m_completedAchievements.end())
    {
        m_CompletedAchievementsLock.release();
        return 0LL;
    }
        
    m_CompletedAchievementsLock.release();

    return (*itr).second.first_guid;
}

template<class T>
bool AchievementMgr<T>::CanUpdateCriteria(CriteriaTreeEntry const* tree, AchievementEntry const* achievement, uint64 miscValue1, uint64 miscValue2, uint64 miscValue3, Unit const* unit, Player* referencePlayer)
{
    /* Temp disable
    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_ACHIEVEMENT_CRITERIA, criteria->ID, NULL))
    {
        sLog->outTrace(LOG_FILTER_ACHIEVEMENTSYS, "CanUpdateCriteria: %s (Id: %u Type %s) Disabled",
            criteria->name, criteria->ID, AchievementGlobalMgr::GetCriteriaTypeString(criteria->type));
        return false;
    }*/

    switch (tree->CriteriaID)
    {
        case 8839:  // arena honnorable kills
        case 9161:  // arena honnorable kills
        case 9162:  // arena honnorable kills
        case 9163:  // arena honnorable kills
        case 9164:  // arena honnorable kills
        case 4961:  // arena honnorable kills 5 vs 5
        case 4960:  // arena honnorable kills 3 vs 3
        case 4959:  // arena honnorable kills 2 vs 2
        //case 15427: // 1st 25lvl guild
        // In next achievements(with flag 0x300) add to all players, not for first, temp disable it
        case 5224: // Realm First! Obsidian Slayer
        case 5223: // Realm First! Magic Seeker
        case 12818: // Realm First! Fall of the Lich King
        case 12350: // Realm First! Grand Crusader
        case 10698: // Realm First! Celestial Defender
        case 10279: // Realm First! Death's Demise
        case 5227: // Realm First! Conqueror of Naxxramas
        // lei shen
        case 22797: // A Complete Circuit
        case 22996: // A Complete Circuit
        case 22997: // A Complete Circuit
        case 22998: // A Complete Circuit
        //

            return false;
         break;
    }
    
    if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL | ACHIEVEMENT_FLAG_REALM_FIRST_GUILD))
        return false;

    if (referencePlayer) 
        if (achievement->mapID != -1 && referencePlayer->GetMapId() != uint32(achievement->mapID))
            return false;

    if (referencePlayer) 
        if ((achievement->requiredFaction == ACHIEVEMENT_FACTION_HORDE    && referencePlayer->GetTeam() != HORDE) ||
            (achievement->requiredFaction == ACHIEVEMENT_FACTION_ALLIANCE && referencePlayer->GetTeam() != ALLIANCE))
            return false;

    if (IsCompletedCriteria(tree, achievement))
        return false;

    if (!RequirementsSatisfied(tree, miscValue1, miscValue2, miscValue3, unit, referencePlayer))
        return false;

    if (!AdditionalRequirementsSatisfied(tree, miscValue1, miscValue2, unit, referencePlayer))
        return false;

    CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
    if (!ConditionsSatisfied(criteria, referencePlayer))
        return false;

    return true;
}

template<class T>
bool AchievementMgr<T>::ConditionsSatisfied(CriteriaEntry const *criteria, Player* referencePlayer) const
{
    uint32 types[MAX_CRITERIA_REQUIREMENTS] = { criteria->StartEvent, criteria->FailEvent };
    uint32 values[MAX_CRITERIA_REQUIREMENTS] = { criteria->StartAsset, criteria->FailAsset };

    for (uint32 i = 0; i < MAX_CRITERIA_REQUIREMENTS; ++i)
    {
        if (!types[i])
            continue;

        switch (types[i])
        {
            case ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP:
                if (referencePlayer->GetMapId() != values[i])
                    return false;
                break;
            case ACHIEVEMENT_CRITERIA_CONDITION_NOT_IN_GROUP:
                if (referencePlayer->GetGroup())
                    return false;
                break;
            default:
                break;
        }
    }

    return true;
}

template<class T>
bool AchievementMgr<T>::RequirementsSatisfied(CriteriaTreeEntry const* tree, uint64 p_MiscValue1, uint64 miscValue2, uint64 miscValue3, Unit const *unit, Player* referencePlayer) const
{
    uint32 achievement = sAchievementMgr->GetAchievement(tree)->ID;

    CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
    if (!criteria)
        return true;

    CriteriaValueEntry value = { criteria->Asset, tree->Amount };
    switch (AchievementCriteriaTypes(criteria->Type))
    {
        case ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION:
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
        case ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
        case ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEAL_CASTED:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_DEALT:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_RECEIVED:
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
        case ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL:
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_VENDORS:
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
        case ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_GUILD_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED:
        case ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_DAMAGE_RECEIVED:
        case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_HEALING_RECEIVED:
        case ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
        case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS:
            if (!p_MiscValue1)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        case ACHIEVEMENT_CRITERIA_TYPE_EARNED_PVP_TITLE:
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING:
        case ACHIEVEMENT_CRITERIA_TYPE_KNOWN_FACTIONS:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            m_CompletedAchievementsLock.acquire();
            if (m_completedAchievements.find(value.complete_achievement.linkedAchievement) == m_completedAchievements.end())
            {
                m_CompletedAchievementsLock.release();
                return false;
            }
            m_CompletedAchievementsLock.release();
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG:
            if (!p_MiscValue1 || !referencePlayer || value.win_bg.bgMapID != referencePlayer->GetMapId())
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
            if (!p_MiscValue1 || value.kill_creature.creatureID != p_MiscValue1)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
            // update at loading or specific skill update
            if (p_MiscValue1 && p_MiscValue1 != value.reach_skill_level.skillID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            // update at loading or specific skill update
            if (p_MiscValue1 && p_MiscValue1 != value.learn_skill_level.skillID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            if (p_MiscValue1 && p_MiscValue1 != value.complete_quests_in_zone.zoneID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
            if (!p_MiscValue1 || !referencePlayer || referencePlayer->GetMapId() != value.complete_battleground.mapID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
            if (!p_MiscValue1 || !referencePlayer || referencePlayer->GetMapId() != value.death_at_map.mapID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
        {
            if (!p_MiscValue1)
                return false;
            if (!referencePlayer)
                return false;
            // skip wrong arena achievements, if not achievIdByArenaSlot then normal total death counter
            bool notfit = false;
            for (int j = 0; j < MAX_ARENA_SLOT; ++j)
            {
                if (achievIdByArenaSlot[j] == achievement)
                {
                    Battleground* bg = referencePlayer->GetBattleground();
                    if (!bg || !bg->isArena() || Arena::GetSlotByType(bg->GetArenaType()) != j)
                        notfit = true;
                    break;
                }
            }
            if (notfit)
                return false;
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON:
        {
            if (!p_MiscValue1)
                return false;

            if (!referencePlayer)
                return false;

            Map const* map = referencePlayer->IsInWorld() ? referencePlayer->GetMap() : sMapMgr->FindMap(referencePlayer->GetMapId(), referencePlayer->GetInstanceId());
            if (!map || !map->IsDungeon())
                return false;

            // search case
            bool found = false;
            for (int j = 0; achievIdForDungeon[j][0]; ++j)
            {
                if (achievIdForDungeon[j][0] == achievement)
                {
                    if (map->IsRaid())
                    {
                        // if raid accepted (ignore difficulty)
                        if (!achievIdForDungeon[j][2])
                            break;                      // for
                    }
                    else if (referencePlayer->GetDungeonDifficulty() == REGULAR_DIFFICULTY)
                    {
                        // dungeon in normal mode accepted
                        if (!achievIdForDungeon[j][1])
                            break;                      // for
                    }
                    else
                    {
                        // dungeon in heroic mode accepted
                        if (!achievIdForDungeon[j][3])
                            break;                      // for
                    }

                    found = true;
                    break;                              // for
                }
            }
            if (!found)
                return false;

            //FIXME: work only for instances where max == min for players
            if (((InstanceMap*)map)->GetMaxPlayers() != value.death_in_dungeon.manLimit)
                return false;
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
            if (!p_MiscValue1 || p_MiscValue1 != value.killed_by_creature.creatureEntry)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
            if (!p_MiscValue1 || !unit || unit->GetTypeId() != TYPEID_PLAYER)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_DEATHS_FROM:
            if (!p_MiscValue1 || miscValue2 != value.death_from.type)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
        {
            // if miscValues != 0, it contains the questID.
            if (p_MiscValue1)
            {
                if (p_MiscValue1 != value.complete_quest.questID)
                    return false;
            }
            else
            {
                // login case.
                if (!referencePlayer || !referencePlayer->GetQuestRewardStatus(value.complete_quest.questID))
                    return false;
            }

            if (AchievementCriteriaDataSet const* data = sAchievementMgr->GetCriteriaDataSet(criteria->ID))
                if (!data->Meets(referencePlayer, unit))
                    return false;
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
            if (!p_MiscValue1 || p_MiscValue1 != value.be_spell_target.spellID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2:
            if (!p_MiscValue1 || p_MiscValue1 != value.cast_spell.spellID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
            if (p_MiscValue1 && p_MiscValue1 != value.learn_spell.spellID)
                return false;

            if (miscValue2 == 0)
            {
                if (!referencePlayer || !referencePlayer->HasSpell(value.learn_spell.spellID))
                    return false;
            }
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE:
            // miscValue1 = itemId - miscValue2 = count of item loot
            // miscValue3 = loot_type (note: 0 = LOOT_CORPSE and then it ignored)
            if (!p_MiscValue1 || !miscValue2 || !miscValue3 || miscValue3 != value.loot_type.lootType)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
            if (p_MiscValue1 && value.own_item.itemID != p_MiscValue1)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
            if (!p_MiscValue1 || value.use_item.itemID != p_MiscValue1)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
            if (!p_MiscValue1 || p_MiscValue1 != value.own_item.itemID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
        {
            if (!referencePlayer)
                return false;

            WorldMapOverlayEntry const* worldOverlayEntry = sWorldMapOverlayStore.LookupEntry(value.explore_area.areaReference);
            if (!worldOverlayEntry)
                break;

            bool matchFound = false;
            for (int j = 0; j < MAX_WORLD_MAP_OVERLAY_AREA_IDX; ++j)
            {
                uint32 area_id = worldOverlayEntry->areatableID[j];
                if (!area_id)                            // array have 0 only in empty tail
                    break;

                int32 exploreFlag = GetAreaFlagByAreaID(area_id);

                // Hack: Explore Southern Barrens
                if (value.explore_area.areaReference == 3009) exploreFlag = 515;

                if (exploreFlag < 0)
                    continue;

                uint32 playerIndexOffset = uint32(exploreFlag) / 32;
                uint32 mask = 1 << (uint32(exploreFlag) % 32);

                if (referencePlayer->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + playerIndexOffset) & mask)
                {
                    matchFound = true;
                    break;
                }
            }

            if (!matchFound)
                return false;
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
            if (p_MiscValue1 && p_MiscValue1 != value.gain_reputation.factionID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
            // miscValue1 = itemid miscValue2 = itemSlot
            if (!p_MiscValue1 || miscValue2 != value.equip_epic_item.itemSlot)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
        {
            // miscValue1 = itemid miscValue2 = diced value
            if (!p_MiscValue1 || miscValue2 != value.roll_greed_on_loot.rollValue)
                return false;

            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(uint32(p_MiscValue1));
            if (!proto)
                return false;
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
            if (!p_MiscValue1 || p_MiscValue1 != value.do_emote.emoteID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE:
        case ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE:
            if (!p_MiscValue1)
                return false;

            if (!referencePlayer)
                return false;

            if (criteria->StartEvent == ACHIEVEMENT_CRITERIA_CONDITION_BG_MAP)
            {
                if (referencePlayer->GetMapId() != criteria->StartAsset)
                    return false;

                // map specific case (BG in fact) expected player targeted damage/heal
                if (!unit || unit->GetTypeId() != TYPEID_PLAYER)
                    return false;
            }
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
            // miscValue1 = item_id
            if (!p_MiscValue1 || p_MiscValue1 != value.equip_item.itemID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
            if (!p_MiscValue1 || p_MiscValue1 != value.use_gameobject.goEntry)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
            if (!p_MiscValue1 || p_MiscValue1 != value.fish_in_gameobject.goEntry)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
            if (p_MiscValue1 && p_MiscValue1 != value.learn_skillline_spell.skillLine)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
        {
            if (!p_MiscValue1)
                return false;
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(uint32(p_MiscValue1));
            if (!proto)
                return false;
            break;
        }
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE:
            if (p_MiscValue1 && p_MiscValue1 != value.learn_skill_line.skillLine)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
            if (!p_MiscValue1 || p_MiscValue1 != value.hk_class.classID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
            if (!p_MiscValue1 || p_MiscValue1 != value.hk_race.raceID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
            if (!p_MiscValue1 || p_MiscValue1 != value.bg_objective.objectiveId)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            if (!p_MiscValue1 || p_MiscValue1 != value.honorable_kill_at_area.areaID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_CURRENCY:
            if (!p_MiscValue1 || !miscValue2 || int64(miscValue2) < 0
                || p_MiscValue1 != value.currencyGain.currency)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA:
            if (p_MiscValue1 != value.win_arena.mapID)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE_TYPE:
            if (p_MiscValue1 != value.guild_challenge_complete_type.challenge_type)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_SCENARIO:
            if (p_MiscValue1 != value.scenario_complete.scenarioType)
                return false;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_CHALLENGE_DUNGEON:
            if (!p_MiscValue1 || p_MiscValue1 != value.ChallengeDungeon.MapID)
                return false;
            break;
        default:
            break;
    }
    return true;
}

template<class T>
bool AchievementMgr<T>::AdditionalRequirementsSatisfied(CriteriaTreeEntry const* tree, uint64 p_MiscValue1, uint64 p_MiscValue2, Unit const* unit, Player* referencePlayer) const
{
    CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
    std::vector<ModifierTreeEntry const*> const* modifiers = sAchievementMgr->GetModifiers(criteria->ModifierTreeId);
    if (modifiers)
    {
        uint32 achievement = sAchievementMgr->GetAchievement(tree)->ID;
        for (ModifierTreeEntry const* modifier : *modifiers)
        {
            uint32 l_ReqValue = modifier->Asset;
            switch (AchievementCriteriaAdditionalCondition(modifier->Type))
            {
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_DRUNK_VALUE: // 1
                {
                    if (!referencePlayer || (referencePlayer->GetDrunkValue() < l_ReqValue))
                        return false;
                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ITEM_LEVEL: // 2
                {
                    ItemTemplate const* pItem = sObjectMgr->GetItemTemplate(p_MiscValue1);
                    if (!pItem)
                        return false;

                    if (pItem->ItemLevel < l_ReqValue)
                        return false;
                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_CREATURE_ENTRY: // 4
                    if (!unit || !unit->IsInWorld())
                        return false;

                    // Hack for Bros. Before Ho Ho Ho's
                    if (achievement == 1685 || achievement == 1686)
                        break;

                    if (unit->GetEntry() != l_ReqValue)
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_MUST_BE_PLAYER: // 5
                    if (!unit || !unit->IsInWorld() || unit->GetTypeId() != TYPEID_PLAYER)
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_MUST_BE_DEAD: // 6
                    if (!unit || !unit->IsInWorld() || unit->isAlive())
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_MUST_BE_ENEMY: // 7
                    if (!referencePlayer)
                        return false;
                    if (!unit || !unit->IsInWorld() || !referencePlayer->IsHostileTo(unit))
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_HAS_AURA: // 8

                    // Hack for Fa-la-la-la-Ogri'la, there are wrong auras in dbc
                    if (achievement == 1282)
                    {
                        if (referencePlayer->HasAura(62061))
                            break;
                    }
                    else if (achievement == 233) // Take a Chill Pill
                    {
                        if (!referencePlayer)
                            return false;
                        if (!referencePlayer->HasAura(23505) && !referencePlayer->HasAura(l_ReqValue))
                            return false;
                        break;
                    }
                    if (!referencePlayer || !referencePlayer->HasAura(l_ReqValue))
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_HAS_AURA: // 10

                    if (achievement == 1258) // Bloodthirsty Berserker
                    {
                        if (!unit || !unit->IsInWorld())
                            return false;
                        if (!unit->HasAura(23505) && !unit->HasAura(l_ReqValue))
                            return false;
                        break;
                    }
                    if (!unit || !unit->IsInWorld() || !unit->HasAura(l_ReqValue))
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_MUST_BE_MOUNTED: // 11
                    if (!unit || !unit->IsMounted())
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ITEM_QUALITY_MIN: // 14
                {
                    // miscValue1 is itemid
                    ItemTemplate const * const item = sObjectMgr->GetItemTemplate(uint32(p_MiscValue1));
                    if (!item || item->Quality < l_ReqValue)
                        return false;
                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ITEM_QUALITY_EQUALS: // 15
                {
                    // miscValue1 is itemid
                    ItemTemplate const * const item = sObjectMgr->GetItemTemplate(uint32(p_MiscValue1));
                    if (!item || item->Quality < l_ReqValue)
                        return false;
                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_AREA_OR_ZONE: // 17
                {
                    if (!referencePlayer)
                        return false;

                    uint32 zoneId, areaId;
                    referencePlayer->GetZoneAndAreaId(zoneId, areaId);
                    if (zoneId != l_ReqValue && areaId != l_ReqValue)
                        return false;
                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_AREA_OR_ZONE: // 18
                {
                    if (!unit)
                        return false;
                    uint32 zoneId, areaId;
                    unit->GetZoneAndAreaId(zoneId, areaId);
                    if (zoneId != l_ReqValue && areaId != l_ReqValue)
                        return false;
                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_MAP_DIFFICULTY: // 68
                {
                    if (!referencePlayer)
                        return false;

                    Map* map = referencePlayer->GetMap();
                    if (!map)
                        return false;

                    return map->GetDifficulty() == Difficulty(l_ReqValue);
                }
                /*case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ARENA_TYPE:
                {
                    if (!referencePlayer)
                        return false;

                    if (criteria->Type == ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA)
                        return referencePlayer->GetSeasonWins(Arena::GetSlotByType(l_ReqValue)) >= tree->Amount;

                    return false;
                }*/
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_RACE: // 25
                    if (!referencePlayer || (referencePlayer->getRace() != l_ReqValue))
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_CLASS: // 26
                    if (!referencePlayer || (referencePlayer->getClass() != l_ReqValue))
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_RACE: // 27
                    if (!unit || !unit->IsInWorld() || unit->GetTypeId() != TYPEID_PLAYER || unit->getRace() != l_ReqValue)
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_CLASS: // 28
                    if (!unit || !unit->IsInWorld() || unit->GetTypeId() != TYPEID_PLAYER || unit->getClass() != l_ReqValue)
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_MAX_GROUP_MEMBERS: // 29
                    if (!referencePlayer || (referencePlayer->GetGroup() && referencePlayer->GetGroup()->GetMembersCount() >= l_ReqValue))
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_CREATURE_TYPE: // 30
                {
                    if (!unit)
                        return false;
                    Creature const * const creature = unit->ToCreature();
                    if (!creature || creature->GetCreatureType() != l_ReqValue)
                        return false;
                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_MAP: // 32
                    if (!referencePlayer || (referencePlayer->GetMapId() != l_ReqValue))
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ITEM_CLASS: // 33
                {
                    ItemTemplate const* pItem = sObjectMgr->GetItemTemplate(p_MiscValue1);
                    if (!pItem)
                        return false;

                    if (pItem->Class != l_ReqValue)
                        return false;
                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_ITEM_SUBCLASS: // 34
                {
                    ItemTemplate const* pItem = sObjectMgr->GetItemTemplate(p_MiscValue1);
                    if (!pItem)
                        return false;

                    if (pItem->SubClass != l_ReqValue)
                        return false;
                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_MIN_PERSONAL_RATING: // 37
                {
                    if (p_MiscValue1 < l_ReqValue)
                        return false;
                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TITLE_BIT_INDEX: // 38
                    // miscValue1 is title's bit index
                    if (p_MiscValue1 != l_ReqValue)
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_SOURCE_LEVEL: // 39
                    if (!referencePlayer || (referencePlayer->getLevel() != l_ReqValue))
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_LEVEL: // 40
                    if (!unit || !unit->IsInWorld() || unit->getLevel() != l_ReqValue)
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_ZONE: // 41
                    if (!referencePlayer || !referencePlayer->IsInWorld() || referencePlayer->GetZoneId() != l_ReqValue)
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_TARGET_HEALTH_PERCENT_BELOW: // 46
                    if (!unit || !unit->IsInWorld() || unit->GetHealthPct() >= l_ReqValue)
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_MIN_ACHIEVEMENT_POINTS: // 56
                    if (!referencePlayer || (referencePlayer->GetAchievementMgr().GetAchievementPoints() < l_ReqValue))
                        return false;
                    break;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_REQUIRES_GUILD_GROUP: // 61
                {
                    if (!referencePlayer)
                        return false;

                    Group* pGroup = referencePlayer->GetGroup();
                    if (!pGroup)
                        return false;

                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_GUILD_REPUTATION: // 62
                {
                    if (!referencePlayer)
                        return false;

                    if (uint32(referencePlayer->GetReputationMgr().GetReputation(1168)) < l_ReqValue) // 1168 = Guild faction
                        return false;
                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_RATED_BATTLEGROUND: // 63
                {
                    if (!referencePlayer)
                        return false;

                    if (!referencePlayer->GetBattleground())
                        return false;

                    if (!referencePlayer->GetBattleground()->IsRatedBG())
                        return false;

                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_PROJECT_RARITY: // 65
                {
                    if (!p_MiscValue1)
                        return false;

                    bool ok = false;
                    for (auto const& kvp : sResearchProjectMap)
                    {
                        for (auto project : kvp.second)
                        {
                            if (project->ID == p_MiscValue1)
                            {
                                ok = (project->rare == l_ReqValue);
                                break;
                            }
                        }
                    }

                    if (!ok)
                        return false;

                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_PROJECT_RACE: // 66
                {
                    if (!p_MiscValue1)
                        return false;
                    bool ok = false;


                    auto itr = sResearchProjectMap.find(l_ReqValue);
                    if (itr != sResearchProjectMap.end())
                    {
                        for (auto project : itr->second)
                        {
                            if (project->ID == p_MiscValue1)
                            {
                                ok = true;
                                break;
                            }
                        }
                    }

                    if (!ok)
                        return false;

                    break;
                }
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_PLAYER_CONDITION:
                    if (!referencePlayer)
                        return false;

                    if (PlayerConditionEntry const* cond = sPlayerConditionStore.LookupEntry(l_ReqValue))
                        return referencePlayer->Match(cond);

                    return false;
                case ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_NEED_CHALLENGE_MEDAL:
                    if (p_MiscValue2 >= l_ReqValue)
                        return true;
                    return false;
                default:
                    break;
            }
        }
    }
    return true;
}

template<class T>
bool AchievementMgr<T>::CheckForOtherFactionCriteria(CriteriaTreeEntry const* tree)
{
    uint32 achievementId = sAchievementMgr->GetAchievement(tree)->ID;

    switch (achievementId)
    {
        /* TODO: verifie this data*/
        case 3478: // Pilgrim
        {
            switch (tree->CriteriaID)
            {
                // Now We're Cookin'
                case 11258: return HasAchieved(3577);
                // Pilgrim's Paunch
                case 11259: return HasAchieved(3557);
                // Pilgrim's Peril
                case 11260: return HasAchieved(3581);
                // Pilgrim's Progress
                case 11261: return HasAchieved(3597);

                default: break;
            }
            break;
        }
        case 1693: // Fool For Love
            switch (tree->CriteriaID)
            {
                // Flirt With Disaster
                case 6353: return HasAchieved(1280);
                // Nation of Adoration
                case 6358: return HasAchieved(1698);
                default: break;
            }
            break;
    }

    return false;
}

char const* AchievementGlobalMgr::GetCriteriaTypeString(uint32 type)
{
    return GetCriteriaTypeString(AchievementCriteriaTypes(type));
}

char const* AchievementGlobalMgr::GetCriteriaTypeString(AchievementCriteriaTypes type)
{
    switch (type)
    {
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
            return "KILL_CREATURE";
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_BG:
            return "TYPE_WIN_BG";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ARCHAEOLOGY_PROJECTS:
            return "COMPLETE_RESEARCH";
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
            return "REACH_LEVEL";
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
            return "REACH_SKILL_LEVEL";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            return "COMPLETE_ACHIEVEMENT";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
            return "COMPLETE_QUEST_COUNT";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
            return "COMPLETE_DAILY_QUEST_DAILY";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            return "COMPLETE_QUESTS_IN_ZONE";
        case ACHIEVEMENT_CRITERIA_TYPE_CURRENCY:
            return "CURRENCY";
        case ACHIEVEMENT_CRITERIA_TYPE_DAMAGE_DONE:
            return "DAMAGE_DONE";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
            return "COMPLETE_DAILY_QUEST";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
            return "COMPLETE_BATTLEGROUND";
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
            return "DEATH_AT_MAP";
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
            return "DEATH";
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON:
            return "DEATH_IN_DUNGEON";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_RAID:
            return "COMPLETE_RAID";
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
            return "KILLED_BY_CREATURE";
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
            return "KILLED_BY_PLAYER";
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
            return "FALL_WITHOUT_DYING";
        case ACHIEVEMENT_CRITERIA_TYPE_DEATHS_FROM:
            return "DEATHS_FROM";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
            return "COMPLETE_QUEST";
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
            return "BE_SPELL_TARGET";
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL:
            return "CAST_SPELL";
        case ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
            return "BG_OBJECTIVE_CAPTURE";
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            return "HONORABLE_KILL_AT_AREA";
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA:
            return "WIN_ARENA";
        case ACHIEVEMENT_CRITERIA_TYPE_PLAY_ARENA:
            return "PLAY_ARENA";
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
            return "LEARN_SPELL";
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL:
            return "HONORABLE_KILL";
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
            return "OWN_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
            return "WIN_RATED_ARENA";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING:
            return "HIGHEST_TEAM_RATING";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
            return "HIGHEST_PERSONAL_RATING";
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            return "LEARN_SKILL_LEVEL";
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
            return "USE_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
            return "LOOT_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
            return "EXPLORE_AREA";
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_RANK:
            return "OWN_RANK";
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
            return "BUY_BANK_SLOT";
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
            return "GAIN_REPUTATION";
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
            return "GAIN_EXALTED_REPUTATION";
        case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
            return "VISIT_BARBER_SHOP";
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
            return "EQUIP_EPIC_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
            return "ROLL_NEED_ON_LOOT";
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
            return "GREED_ON_LOOT";
        case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
            return "HK_CLASS";
        case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
            return "HK_RACE";
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
            return "DO_EMOTE";
        case ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE:
            return "HEALING_DONE";
        case ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS:
            return "GET_KILLING_BLOWS";
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
            return "EQUIP_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_VENDORS:
            return "MONEY_FROM_VENDORS";
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
            return "GOLD_SPENT_FOR_TALENTS";
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
            return "NUMBER_OF_TALENT_RESETS";
        case ACHIEVEMENT_CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
            return "MONEY_FROM_QUEST_REWARD";
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING:
            return "GOLD_SPENT_FOR_TRAVELLING";
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
            return "GOLD_SPENT_AT_BARBER";
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
            return "GOLD_SPENT_FOR_MAIL";
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
            return "LOOT_MONEY";
        case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
            return "USE_GAMEOBJECT";
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
            return "BE_SPELL_TARGET2";
        case ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL:
            return "SPECIAL_PVP_KILL";
        case ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
            return "FISH_IN_GAMEOBJECT";
        case ACHIEVEMENT_CRITERIA_TYPE_EARNED_PVP_TITLE:
            return "EARNED_PVP_TITLE";
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
            return "LEARN_SKILLLINE_SPELLS";
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL:
            return "WIN_DUEL";
        case ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL:
            return "LOSE_DUEL";
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE:
            return "KILL_CREATURE_TYPE";
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:
            return "GOLD_EARNED_BY_AUCTIONS";
        case ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION:
            return "CREATE_AUCTION";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID:
            return "HIGHEST_AUCTION_BID";
        case ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS:
            return "WON_AUCTIONS";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:
            return "HIGHEST_AUCTION_SOLD";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
            return "HIGHEST_GOLD_VALUE_OWNED";
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
            return "GAIN_REVERED_REPUTATION";
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
            return "GAIN_HONORED_REPUTATION";
        case ACHIEVEMENT_CRITERIA_TYPE_KNOWN_FACTIONS:
            return "KNOWN_FACTIONS";
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM:
            return "LOOT_EPIC_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
            return "RECEIVE_EPIC_ITEM";
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED:
            return "ROLL_NEED";
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED:
            return "ROLL_GREED";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_DEALT:
            return "HIT_DEALT";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_RECEIVED:
            return "HIT_RECEIVED";
        case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_DAMAGE_RECEIVED:
            return "TOTAL_DAMAGE_RECEIVED";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEAL_CASTED:
            return "HIGHEST_HEAL_CASTED";
        case ACHIEVEMENT_CRITERIA_TYPE_TOTAL_HEALING_RECEIVED:
            return "TOTAL_HEALING_RECEIVED";
        case ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED:
            return "HIGHEST_HEALING_RECEIVED";
        case ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED:
            return "QUEST_ABANDONED";
        case ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
            return "FLIGHT_PATHS_TAKEN";
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE:
            return "LOOT_TYPE";
        case ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2:
            return "CAST_SPELL2";
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE:
            return "LEARN_SKILL_LINE";
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
            return "EARN_HONORABLE_KILL";
        case ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
            return "ACCEPTED_SUMMONINGS";
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
            return "EARN_ACHIEVEMENT_POINTS";
        case ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
            return "USE_LFD_TO_GROUP_WITH_PLAYERS";
        case ACHIEVEMENT_CRITERIA_TYPE_SPENT_GOLD_GUILD_REPAIRS:
            return "SPENT_GOLD_GUILD_REPAIRS";
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_GUILD_LEVEL:
            return "REACH_GUILD_LEVEL";
        case ACHIEVEMENT_CRITERIA_TYPE_CRAFT_ITEMS_GUILD:
            return "CRAFT_ITEMS_GUILD";
        case ACHIEVEMENT_CRITERIA_TYPE_CATCH_FROM_POOL:
            return "CATCH_FROM_POOL";
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS:
            return "BUY_GUILD_BANK_SLOTS";
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS:
            return "EARN_GUILD_ACHIEVEMENT_POINTS";
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND:
            return "WIN_RATED_BATTLEGROUND";
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_BG_RATING:
            return "REACH_BG_RATING";
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_TABARD:
            return "BUY_GUILD_TABARD";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_GUILD:
            return "COMPLETE_QUESTS_GUILD";
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILLS_GUILD:
            return "HONORABLE_KILLS_GUILD";
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE_GUILD:
            return "KILL_CREATURE_TYPE_GUILD";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE_TYPE:
            return "GUILD_CHALLENGE_TYPE";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE:
            return "GUILD_CHALLENGE";
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_SCENARIO:
            return "COMPLETE_SCENARIO";
    }
    return "MISSING_TYPE";
}

template class AchievementMgr<Guild>;
template class AchievementMgr<Player>;

//==========================================================
void AchievementGlobalMgr::LoadAchievementCriteriaList()
{
    uint32 oldMSTime = getMSTime();

    if (sCriteriaMap.size() == 0)
    {
        sLog->outError(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 achievement criteria.");
        return;
    }

    uint32 criterias = 0;
    uint32 guildCriterias = 0;

    for (uint32 i = 0; i < sAchievementStore.GetNumRows(); ++i)
    {
        AchievementEntry const* achievement = sAchievementStore.LookupEntry(i);
        if (!achievement)
            continue;

        if (!achievement->CriteriaTree)
            continue;

        auto itr = sLinkedCriteriaTreeMap.find(achievement->CriteriaTree);
        if (itr == sLinkedCriteriaTreeMap.end())
            continue;

        std::vector<CriteriaTreeEntry const*> const& trees = itr->second;
        for (CriteriaTreeEntry const* tree : trees)
        {
            CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(tree->CriteriaID);
            WPFatal(criteria, "Criteria is null!!!!!");

            m_AchievementCriteriaListByAchievement[achievement->ID].push_back(tree);
            if (achievement->flags & ACHIEVEMENT_FLAG_GUILD)
                ++guildCriterias, m_GuildAchievementCriteriasByType[criteria->Type].push_back(tree);
            else
                ++criterias, m_AchievementCriteriasByType[criteria->Type].push_back(tree);

            if (criteria->StartTimer)
                m_AchievementCriteriasByTimedType[criteria->StartEvent].push_back(tree);
        }
    }

    //for (uint32 i = 0; i < 10000000; ++i)
        sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u achievement criteria and %u guild achievement crieteria in %u ms", criterias, guildCriterias, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadAchievementReferenceList()
{
    uint32 oldMSTime = getMSTime();

    if (sAchievementStore.GetNumRows() == 0)
    {
        sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 achievement references.");
        return;
    }

    uint32 count = 0;

    for (uint32 entryId = 0; entryId < sAchievementStore.GetNumRows(); ++entryId)
    {
        AchievementEntry const* achievement = sAchievementMgr->GetAchievement(entryId);
        if (!achievement || !achievement->SharesCriteria)
            continue;

        m_AchievementListByReferencedId[achievement->SharesCriteria].push_back(achievement);
        ++count;
    }

    // Once Bitten, Twice Shy (10 player) - Icecrown Citadel
    if (AchievementEntry const* achievement = sAchievementMgr->GetAchievement(4539))
        const_cast<AchievementEntry*>(achievement)->mapID = 631;    // Correct map requirement (currently has Ulduar)

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u achievement references in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadAchievementCriteriaData()
{
    uint32 oldMSTime = getMSTime();

    m_criteriaDataMap.clear();                              // Need for reload case

    QueryResult result = WorldDatabase.Query("SELECT criteria_id, type, value1, value2, ScriptName FROM achievement_criteria_data");

    if (!result)
    {
        sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 additional achievement criteria data. DB table `achievement_criteria_data` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 criteria_id = fields[0].GetUInt32();

        CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(criteria_id);
        if (!criteria)
        {
            sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` has data for non-existing criteria (Entry: %u), ignore.", criteria_id);
            continue;
        }

        uint32 dataType = fields[1].GetUInt8();
        const char* scriptName = fields[4].GetCString();
        uint32 scriptId = 0;
        if (strcmp(scriptName, "")) // Not empty
        {
            if (dataType != ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT)
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_criteria_data` has ScriptName set for non-scripted data type (Entry: %u, type %u), useless data.", criteria_id, dataType);
            else
                scriptId = sObjectMgr->GetScriptId(scriptName);
        }

        AchievementCriteriaData data(dataType, fields[2].GetUInt32(), fields[3].GetUInt32(), scriptId);

        if (!data.IsValid(criteria))
            continue;

        // This will allocate empty data set storage
        AchievementCriteriaDataSet& dataSet = m_criteriaDataMap[criteria_id];
        dataSet.SetCriteriaId(criteria_id);

        // Add real data only for not NONE data types
        if (data.dataType != ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE)
            dataSet.Add(data);

        // Counting data by and data types
        ++count;
    }
    while (result->NextRow());

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u additional achievement criteria data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadCompletedAchievements()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = CharacterDatabase.Query("SELECT DISTINCT achievement FROM character_achievement GROUP BY achievement");
    if (!result)
    {
        sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 completed achievements. DB table `character_achievement` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint16 achievementId = fields[0].GetUInt16();
        const AchievementEntry* achievement = sAchievementMgr->GetAchievement(achievementId);
        if (!achievement)
        {
            // Remove non existent achievements from all characters
            sLog->outError(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement %u data removed from table `character_achievement`.", achievementId);

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACHIEVMENT);
            stmt->setUInt16(0, uint16(achievementId));
            CharacterDatabase.Execute(stmt);
            continue;
        }
        else if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL | ACHIEVEMENT_FLAG_REALM_FIRST_GUILD))
            m_allCompletedAchievements.insert(std::make_pair(achievementId, true));
    }
    while (result->NextRow());

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %lu completed achievements in %u ms", (unsigned long)m_allCompletedAchievements.size(), GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadRewards()
{
    uint32 oldMSTime = getMSTime();

    m_achievementRewards.clear();                           // Need for reload case

    //                                               0      1        2        3     4       5        6
    QueryResult result = WorldDatabase.Query("SELECT entry, title_A, title_H, item, sender, subject, text FROM achievement_reward");

    if (!result)
    {
        sLog->outError(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 achievement rewards. DB table `achievement_reward` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        const AchievementEntry* pAchievement = GetAchievement(entry);
        if (!pAchievement)
        {
            sLog->outError(LOG_FILTER_SQL, "Table `achievement_reward` has wrong achievement (Entry: %u), ignored.", entry);
            continue;
        }

        AchievementReward reward;
        reward.titleId[0] = fields[1].GetUInt32();
        reward.titleId[1] = fields[2].GetUInt32();
        reward.itemId     = fields[3].GetUInt32();
        reward.sender     = fields[4].GetUInt32();
        reward.subject    = fields[5].GetString();
        reward.text       = fields[6].GetString();

        // Must be title or mail at least
        if (!reward.titleId[0] && !reward.titleId[1] && !reward.sender)
        {
            sLog->outError(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) does not have title or item reward data, ignored.", entry);
            continue;
        }

        if (pAchievement->requiredFaction == ACHIEVEMENT_FACTION_ANY && ((reward.titleId[0] == 0) != (reward.titleId[1] == 0)))
            sLog->outError(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) has title (A: %u H: %u) for only one team.", entry, reward.titleId[0], reward.titleId[1]);

        if (reward.titleId[0])
        {
            CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(reward.titleId[0]);
            if (!titleEntry)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) has invalid title id (%u) in `title_A`, set to 0", entry, reward.titleId[0]);
                reward.titleId[0] = 0;
            }
        }

        if (reward.titleId[1])
        {
            CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(reward.titleId[1]);
            if (!titleEntry)
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) has invalid title id (%u) in `title_H`, set to 0", entry, reward.titleId[1]);
                reward.titleId[1] = 0;
            }
        }

        // Check mail data before item for report including wrong item case
        if (reward.sender)
        {
            if (!sObjectMgr->GetCreatureTemplate(reward.sender))
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) has invalid creature entry %u as sender, mail reward skipped.", entry, reward.sender);
                reward.sender = 0;
            }
        }
        else
        {
            if (reward.itemId)
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) does not have sender data but has item reward, item will not be rewarded.", entry);

            if (!reward.subject.empty())
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) does not have sender data but has mail subject.", entry);

            if (!reward.text.empty())
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) does not have sender data but has mail text.", entry);
        }

        if (reward.itemId)
        {
            if (!sObjectMgr->GetItemTemplate(reward.itemId))
            {
                sLog->outError(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) has invalid item id %u, reward mail will not contain item.", entry, reward.itemId);
                reward.itemId = 0;
            }
        }

        m_achievementRewards[entry] = reward;
        ++count;

    }
    while (result->NextRow());

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u achievement rewards in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadRewardLocales()
{
    uint32 oldMSTime = getMSTime();

    m_achievementRewardLocales.clear();                       // Need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry, subject_loc1, text_loc1, subject_loc2, text_loc2, subject_loc3, text_loc3, subject_loc4, text_loc4, "
                                             "subject_loc5, text_loc5, subject_loc6, text_loc6, subject_loc7, text_loc7, subject_loc8, text_loc8, subject_loc9, text_loc9,"
                                             "subject_loc10, text_loc10 FROM locales_achievement_reward");

    if (!result)
    {
        sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 achievement reward locale strings.  DB table `locales_achievement_reward` is empty");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        if (m_achievementRewards.find(entry) == m_achievementRewards.end())
        {
            sLog->outError(LOG_FILTER_SQL, "Table `locales_achievement_reward` (Entry: %u) has locale strings for non-existing achievement reward.", entry);
            continue;
        }

        AchievementRewardLocale& data = m_achievementRewardLocales[entry];

        for (int i = 1; i < TOTAL_LOCALES; ++i)
        {
            LocaleConstant locale = (LocaleConstant) i;
            ObjectMgr::AddLocaleString(fields[1 + 2 * (i - 1)].GetString(), locale, data.subject);
            ObjectMgr::AddLocaleString(fields[1 + 2 * (i - 1) + 1].GetString(), locale, data.text);
        }
    }
    while (result->NextRow());

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %lu achievement reward locale strings in %u ms", (unsigned long)m_achievementRewardLocales.size(), GetMSTimeDiffToNow(oldMSTime));
}

AchievementEntry const* AchievementGlobalMgr::GetAchievement(uint32 achievementId) const
{
    return sAchievementStore.LookupEntry(achievementId);
}

AchievementEntry const* AchievementGlobalMgr::GetAchievement(CriteriaTreeEntry const* tree) const
{
    auto itr2 = sCriteriaAchievementMap.find(tree->ID);
    if (itr2 == sCriteriaAchievementMap.end())
        return nullptr;
    return itr2->second;
}

std::vector<ModifierTreeEntry const*> const* AchievementGlobalMgr::GetModifiers(uint32 modifierId) const
{
    auto value = sModifierTreeMap.find(modifierId);
    if (value == sModifierTreeMap.end())
        return NULL;
    return &value->second;
}

void AchievementGlobalMgr::PrepareCriteriaUpdateTaskThread()
{
    AchievementCriteriaUpdateTask l_Task;
    for (auto l_Iterator = m_LockedPlayersAchievementCriteriaTask.begin(); l_Iterator != m_LockedPlayersAchievementCriteriaTask.end(); l_Iterator++)
    {
        while ((*l_Iterator).second.next(l_Task))
            m_PlayersAchievementCriteriaTask[(*l_Iterator).first].push(l_Task);
    }
}

AchievementCriteriaUpdateRequest::AchievementCriteriaUpdateRequest(MapUpdater* p_Updater, AchievementCriteriaTaskQueue p_TaskQueue)
: MapUpdaterTask(p_Updater), m_CriteriaUpdateTasks(p_TaskQueue)
{

}

void AchievementCriteriaUpdateRequest::call()
{
    AchievementCriteriaUpdateTask l_Task;
    while (!m_CriteriaUpdateTasks.empty())
    {
        l_Task = m_CriteriaUpdateTasks.front();
        l_Task.Task(l_Task.PlayerGUID, l_Task.UnitGUID);
        m_CriteriaUpdateTasks.pop();
    }

    UpdateFinished();
}
