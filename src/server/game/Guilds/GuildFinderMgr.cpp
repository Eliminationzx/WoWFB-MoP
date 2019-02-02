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

#include "ObjectMgr.h"
#include "GuildFinderMgr.h"
#include "GuildMgr.h"
#include "World.h"

GuildFinderMgr::GuildFinderMgr()
{
}

GuildFinderMgr::~GuildFinderMgr()
{
}

void GuildFinderMgr::LoadFromDB()
{
    LoadGuildSettings();
    LoadMembershipRequests();
}

void GuildFinderMgr::LoadGuildSettings()
{
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, "Loading guild finder guild-related settings...");
    //                                                           0                1             2                  3             4           5             6         7
    QueryResult result = CharacterDatabase.Query("SELECT gfgs.guildId, gfgs.availability, gfgs.classRoles, gfgs.interests, gfgs.level, gfgs.listed, gfgs.comment, c.race "
                                                 "FROM guild_finder_guild_settings gfgs "
                                                 "LEFT JOIN guild_member gm ON gm.guildid=gfgs.guildId "
                                                 "LEFT JOIN characters c ON c.guid = gm.guid LIMIT 1");
    
    if (!result)
    {
        sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild finder guild-related settings. Table `guild_finder_guild_settings` is empty.");
        return;
    }
    
    uint32 count = 0;
    uint32 oldMSTime = getMSTime();
    do
    {
        Field* fields = result->Fetch();
        uint32 guildId      = fields[0].GetUInt32();
        uint8  availability = fields[1].GetUInt8();
        uint8  classRoles   = fields[2].GetUInt8();
        uint8  interests    = fields[3].GetUInt8();
        uint8  level        = fields[4].GetUInt8();
        bool   listed       = (fields[5].GetUInt8() != 0);
        std::string comment = fields[6].GetString();

        TeamId guildTeam = TEAM_ALLIANCE;
        if (ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(fields[7].GetUInt8()))
            if (raceEntry->TeamID == 1)
                guildTeam = TEAM_HORDE;

        LFGuildSettings settings(listed, guildTeam, guildId, classRoles, availability, interests, level, comment);
        _guildSettings[guildId] = settings;
        
        ++count;
    }
    while (result->NextRow());

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild finder guild-related settings in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void GuildFinderMgr::LoadMembershipRequests()
{
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, "Loading guild finder membership requests...");
    //                                                      0         1           2            3           4         5         6
    QueryResult result = CharacterDatabase.Query("SELECT guildId, playerGuid, availability, classRole, interests, comment, submitTime "
                                                 "FROM guild_finder_applicant");
    
    if (!result)
    {
        sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild finder membership requests. Table `guild_finder_applicant` is empty.");
        return;
    }
    
    uint32 count = 0;
    uint32 oldMSTime = getMSTime();
    do
    {
        Field* fields = result->Fetch();
        uint32 guildId      = fields[0].GetUInt32();
        uint32 playerId     = fields[1].GetUInt32();
        uint8  availability = fields[2].GetUInt8();
        uint8  classRoles   = fields[3].GetUInt8();
        uint8  interests    = fields[4].GetUInt8();
        std::string comment = fields[5].GetString();
        uint32 submitTime   = fields[6].GetUInt32();
        
        MembershipRequest request(playerId, guildId, availability, classRoles, interests, comment, time_t(submitTime));

        _membershipRequests[guildId].push_back(request);
        
        ++count;
    }
    while (result->NextRow());

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild finder membership requests in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void GuildFinderMgr::AddMembershipRequest(uint32 guildGuid, MembershipRequest const& request)
{
    _membershipRequests[guildGuid].push_back(request);
    
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_GUILD_FINDER_APPLICANT);
    stmt->setUInt32(0, request.GetGuildId());
    stmt->setUInt32(1, request.GetPlayerGUID());
    stmt->setUInt8(2, request.GetAvailability());
    stmt->setUInt8(3, request.GetClassRoles());
    stmt->setUInt8(4, request.GetInterests());
    stmt->setString(5, request.GetComment());
    stmt->setUInt32(6, request.GetSubmitTime());
    trans->Append(stmt);
    CharacterDatabase.CommitTransaction(trans);

    // Notify the applicant his submittion has been added
    if (Player* player = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(request.GetPlayerGUID(), 0, HighGuid::Player)))
        SendMembershipRequestListUpdate(*player);

    // Notify the guild master and officers the list changed
    if (Guild* guild = sGuildMgr->GetGuildById(guildGuid))
        SendApplicantListUpdate(*guild);
}

void GuildFinderMgr::RemoveAllMembershipRequestsFromPlayer(uint32 playerId)
{
    for (MembershipRequestStore::iterator itr = _membershipRequests.begin(); itr != _membershipRequests.end(); ++itr)
    {
        std::vector<MembershipRequest>::iterator itr2 = itr->second.begin();
        for (; itr2 != itr->second.end(); ++itr2)
            if (itr2->GetPlayerGUID() == playerId)
                break;

        if (itr2 == itr->second.end())
            continue;

        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_FINDER_APPLICANT);
        stmt->setUInt32(0, itr2->GetGuildId());
        stmt->setUInt32(1, itr2->GetPlayerGUID());
        trans->Append(stmt);

        CharacterDatabase.CommitTransaction(trans);
        itr->second.erase(itr2);

        // Notify the guild master and officers the list changed
        // if (Guild* guild = sGuildMgr->GetGuildById(itr->first))
        //    SendApplicantListUpdate(*guild);
    }
}

void GuildFinderMgr::RemoveMembershipRequest(uint32 playerId, uint32 guildId)
{
    std::vector<MembershipRequest>::iterator itr = _membershipRequests[guildId].begin();
    for (; itr != _membershipRequests[guildId].end(); ++itr)
        if (itr->GetPlayerGUID() == playerId)
            break;

    if (itr == _membershipRequests[guildId].end())
        return;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_FINDER_APPLICANT);
    stmt->setUInt32(0, itr->GetGuildId());
    stmt->setUInt32(1, itr->GetPlayerGUID());
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);

    _membershipRequests[guildId].erase(itr);

    // Notify the applicant his submittion has been removed
    if (Player* player = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(playerId, 0, HighGuid::Player)))
        SendMembershipRequestListUpdate(*player);

    // Notify the guild master and officers the list changed
    if (Guild* guild = sGuildMgr->GetGuildById(guildId))
        SendApplicantListUpdate(*guild);
}

std::list<MembershipRequest> GuildFinderMgr::GetAllMembershipRequestsForPlayer(uint32 playerGuid)
{
    std::list<MembershipRequest> resultSet;
    for (MembershipRequestStore::const_iterator itr = _membershipRequests.begin(); itr != _membershipRequests.end(); ++itr)
    {
        std::vector<MembershipRequest> const& guildReqs = itr->second;
        for (std::vector<MembershipRequest>::const_iterator itr2 = guildReqs.begin(); itr2 != guildReqs.end(); ++itr2)
        {
            if (itr2->GetPlayerGUID() == playerGuid)
            {
                resultSet.push_back(*itr2);
                break;
            }
        }
    }
    return resultSet;
}

uint8 GuildFinderMgr::CountRequestsFromPlayer(uint32 playerId)
{
    uint8 result = 0;
    for (MembershipRequestStore::const_iterator itr = _membershipRequests.begin(); itr != _membershipRequests.end(); ++itr)
    {
        for (std::vector<MembershipRequest>::const_iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2)
        {
            if (itr2->GetPlayerGUID() != playerId)
                continue;
            ++result;
            break;
        }
    }
    return result;
}

LFGuildStore GuildFinderMgr::GetGuildsMatchingSetting(LFGuildPlayer& settings, TeamId faction)
{
    LFGuildStore resultSet;
    for (LFGuildStore::const_iterator itr = _guildSettings.begin(); itr != _guildSettings.end(); ++itr)
    {
        LFGuildSettings const& guildSettings = itr->second;

        if (guildSettings.GetTeam() != faction)
            continue;

        if (!(guildSettings.GetAvailability() & settings.GetAvailability()))
            continue;

        if (!(guildSettings.GetClassRoles() & settings.GetClassRoles()))
            continue;

        if (!(guildSettings.GetInterests() & settings.GetInterests()))
            continue;

        if (!(guildSettings.GetLevel() & settings.GetLevel()))
            continue;

        resultSet.insert(std::make_pair(itr->first, guildSettings));
    }

    return resultSet;
}

bool GuildFinderMgr::HasRequest(uint32 playerId, uint32 guildId)
{
    for (std::vector<MembershipRequest>::const_iterator itr = _membershipRequests[guildId].begin(); itr != _membershipRequests[guildId].end(); ++itr)
        if (itr->GetPlayerGUID() == playerId)
            return true;
    return false;
}

void GuildFinderMgr::SetGuildSettings(uint32 guildGuid, LFGuildSettings const& settings)
{
    _guildSettings[guildGuid] = settings;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_GUILD_FINDER_GUILD_SETTINGS);
    stmt->setUInt32(0, settings.GetGUID());
    stmt->setUInt8(1, settings.GetAvailability());
    stmt->setUInt8(2, settings.GetClassRoles());
    stmt->setUInt8(3, settings.GetInterests());
    stmt->setUInt8(4, settings.GetLevel());
    stmt->setUInt8(5, settings.IsListed());
    stmt->setString(6, settings.GetComment());
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

void GuildFinderMgr::DeleteGuild(uint32 guildId)
{
    std::vector<MembershipRequest>::iterator itr = _membershipRequests[guildId].begin();
    while (itr != _membershipRequests[guildId].end())
    {
        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        uint32 applicant = itr->GetPlayerGUID();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_FINDER_APPLICANT);
        stmt->setUInt32(0, itr->GetGuildId());
        stmt->setUInt32(1, applicant);
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_FINDER_GUILD_SETTINGS);
        stmt->setUInt32(0, itr->GetGuildId());
        trans->Append(stmt);
            
        CharacterDatabase.CommitTransaction(trans);

        // Notify the applicant his submition has been removed
        if (Player* player = ObjectAccessor::FindPlayer(MAKE_NEW_GUID(applicant, 0, HighGuid::Player)))
            SendMembershipRequestListUpdate(*player);

        ++itr;
    }

    _membershipRequests.erase(guildId);
    _guildSettings.erase(guildId);

    // Notify the guild master the list changed (even if he's not a GM any more, not sure if needed)
    if (Guild* guild = sGuildMgr->GetGuildById(guildId))
        SendApplicantListUpdate(*guild);
}

void GuildFinderMgr::SendApplicantListUpdate(Guild& guild)
{
    std::vector<MembershipRequest> recruitsList = sGuildFinderMgr->GetAllMembershipRequestsForGuild(guild.GetId());
    uint32 recruitCount = recruitsList.size();

    ByteBuffer dataBuffer;
    WorldPacket data(SMSG_LF_GUILD_RECRUIT_LIST_UPDATED);

    data << uint32(time(NULL)); // Unk time
    data.WriteBits(recruitCount, 18);

    for (std::vector<MembershipRequest>::const_iterator itr = recruitsList.begin(); itr != recruitsList.end(); ++itr)
    {
        MembershipRequest request = *itr;
        ObjectGuid playerGuid(MAKE_NEW_GUID(request.GetPlayerGUID(), 0, HighGuid::Player));

        data.WriteBits(request.GetComment().size(), 10);
        data.WriteBit(playerGuid[6]);
        data.WriteBit(playerGuid[3]);
        data.WriteBit(playerGuid[1]);
        data.WriteBit(playerGuid[4]);
        data.WriteBit(playerGuid[0]);
        data.WriteBit(playerGuid[5]);
        data.WriteBit(playerGuid[2]);
        data.WriteBits(request.GetName().size(), 6);
        data.WriteBit(playerGuid[7]);

        dataBuffer << uint32(request.GetClass());
        dataBuffer << uint32(request.GetLevel());
        dataBuffer << uint32(0);
        dataBuffer << uint32(time(NULL) <= request.GetExpiryTime());
        dataBuffer << uint32(request.GetAvailability());
        dataBuffer.WriteByteSeq(playerGuid[7]);
        dataBuffer << uint32(time(NULL) - request.GetSubmitTime()); // Time in seconds since application submitted.

        if (request.GetComment().size() > 0)
            dataBuffer.append(request.GetComment().c_str(), request.GetComment().size());

        dataBuffer.WriteByteSeq(playerGuid[6]);

        if (request.GetName().size() > 0)
            dataBuffer.append(request.GetName().c_str(), request.GetName().size());

        dataBuffer << uint32(request.GetClassRoles());
        dataBuffer.WriteByteSeq(playerGuid[1]);
        dataBuffer << uint32(request.GetInterests());
        dataBuffer << uint32(request.GetExpiryTime() - time(NULL)); // TIme in seconds until application expires.
        dataBuffer.WriteByteSeq(playerGuid[2]);
        dataBuffer.WriteByteSeq(playerGuid[5]);
        dataBuffer.WriteByteSeq(playerGuid[0]);
        dataBuffer.WriteByteSeq(playerGuid[4]);
        dataBuffer.WriteByteSeq(playerGuid[3]);
    }

    data.append(dataBuffer);

    if (Player* player = ObjectAccessor::FindPlayer(guild.GetLeaderGUID()))
        player->SendDirectMessage(&data);

    guild.BroadcastPacketToRank(&data, GR_OFFICER);
}

void GuildFinderMgr::SendMembershipRequestListUpdate(Player& player)
{
    std::list<MembershipRequest> applicatedGuilds = sGuildFinderMgr->GetAllMembershipRequestsForPlayer(player.GetGUIDLow());
    uint32 applicationsCount = applicatedGuilds.size();

    WorldPacket data(SMSG_LF_GUILD_MEMBERSHIP_LIST_UPDATED);

    data << uint32(10 - sGuildFinderMgr->CountRequestsFromPlayer(player.GetGUIDLow())); // Applications count left

    data.WriteBits(applicationsCount, 19);

    if (applicationsCount > 0)
    {
        ByteBuffer bufferData;
        for (std::list<MembershipRequest>::const_iterator itr = applicatedGuilds.begin(); itr != applicatedGuilds.end(); ++itr)
        {
            Guild* guild = sGuildMgr->GetGuildById(itr->GetGuildId());

            if (!guild)
                continue;

            LFGuildSettings guildSettings = sGuildFinderMgr->GetGuildSettings(itr->GetGuildId());
            MembershipRequest request = *itr;

            ObjectGuid guildGuid = ObjectGuid(guild->GetGUID());

            data.WriteBits(request.GetComment().size(), 10);
            data.WriteBits(guild->GetName().size(), 7);
            data.WriteBit(guildGuid[2]);
            data.WriteBit(guildGuid[1]);
            data.WriteBit(guildGuid[4]);
            data.WriteBit(guildGuid[0]);
            data.WriteBit(guildGuid[6]);
            data.WriteBit(guildGuid[3]);
            data.WriteBit(guildGuid[5]);
            data.WriteBit(guildGuid[7]);

            bufferData << uint32(50397223);                             // unk Flags
            bufferData.WriteByteSeq(guildGuid[1]);
            bufferData.WriteByteSeq(guildGuid[5]);
            bufferData.WriteByteSeq(guildGuid[6]);

            if (request.GetComment().size() > 0)
                bufferData.append(request.GetComment().c_str(), request.GetComment().size());

            bufferData.WriteByteSeq(guildGuid[0]);
            bufferData.WriteByteSeq(guildGuid[2]);
            bufferData << uint32(guildSettings.GetClassRoles());
            bufferData.WriteByteSeq(guildGuid[4]);
            bufferData << uint32(guildSettings.GetAvailability());

            if (guild->GetName().size() > 0)
                bufferData.append(guild->GetName().c_str(), guild->GetName().size());

            bufferData << uint32(time(NULL) - request.GetSubmitTime()); // Time since application (seconds)
            bufferData << uint32(guildSettings.GetInterests());
            bufferData << uint32(request.GetExpiryTime() - time(NULL)); // Time left to application expiry (seconds)
            bufferData.WriteByteSeq(guildGuid[7]);
            bufferData.WriteByteSeq(guildGuid[3]);
        }

        data.append(bufferData);
    }

    player.SendDirectMessage(&data);
}