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
 
#ifndef ARENATEAM_H
#define ARENATEAM_H
 
#include "QueryResult.h"
#include <ace/Singleton.h>
#include <list>
#include <map>
 
class WorldSession;
class WorldPacket;
class Player;
class Group;
 
enum ArenaTeamCommandTypes
{
    ERR_ARENA_TEAM_CREATE_S                 = 0x00,
    ERR_ARENA_TEAM_INVITE_SS                = 0x01,
    ERR_ARENA_TEAM_QUIT_S                   = 0x03,
    ERR_ARENA_TEAM_FOUNDER_S                = 0x0E
};
 
enum ArenaType
{
    ARENA_TYPE_2v2          = 2,
    ARENA_TYPE_3v3          = 3,
    ARENA_TYPE_5v5          = 5
};
 
enum ArenaTeamCommandErrors
{
    ERR_ARENA_TEAM_CREATED                  = 0x01,
    ERR_ALREADY_IN_ARENA_TEAM               = 0x02,
    ERR_ALREADY_IN_ARENA_TEAM_S             = 0x03,
    ERR_INVITED_TO_ARENA_TEAM               = 0x04,
    ERR_ALREADY_INVITED_TO_ARENA_TEAM_S     = 0x05,
    ERR_ARENA_TEAM_NAME_INVALID             = 0x06,
    ERR_ARENA_TEAM_NAME_EXISTS_S            = 0x07,
    ERR_ARENA_TEAM_LEADER_LEAVE_S           = 0x08,
    ERR_ARENA_TEAM_PERMISSIONS              = 0x08,
    ERR_ARENA_TEAM_PLAYER_NOT_IN_TEAM       = 0x09,
    ERR_ARENA_TEAM_PLAYER_NOT_IN_TEAM_SS    = 0x0A,
    ERR_ARENA_TEAM_PLAYER_NOT_FOUND_S       = 0x0B,
    ERR_ARENA_TEAM_NOT_ALLIED               = 0x0C,
    ERR_ARENA_TEAM_IGNORING_YOU_S           = 0x13,
    ERR_ARENA_TEAM_TARGET_TOO_LOW_S         = 0x15,
    ERR_ARENA_TEAM_TARGET_TOO_HIGH_S        = 0x16,
    ERR_ARENA_TEAM_TOO_MANY_MEMBERS_S       = 0x17,
    ERR_ARENA_TEAM_NOT_FOUND                = 0x1B,
    ERR_ARENA_TEAMS_LOCKED                  = 0x1E,
    ERR_ARENA_TEAM_TOO_MANY_CREATE          = 0x21,
};
 
enum ArenaTeamEvents
{
    ERR_ARENA_TEAM_JOIN_SS                  = 4,            // player name + arena team name
    ERR_ARENA_TEAM_LEAVE_SS                 = 5,            // player name + arena team name
    ERR_ARENA_TEAM_REMOVE_SSS               = 6,            // player name + arena team name + captain name
    ERR_ARENA_TEAM_LEADER_IS_SS             = 7,            // player name + arena team name
    ERR_ARENA_TEAM_LEADER_CHANGED_SSS       = 8,            // old captain + new captain + arena team name
    ERR_ARENA_TEAM_DISBANDED_S              = 9             // captain name + arena team name
};
 
/*
need info how to send these ones:
ERR_ARENA_TEAM_YOU_JOIN_S - client show it automatically when accept invite
ERR_ARENA_TEAM_TARGET_TOO_LOW_S
ERR_ARENA_TEAM_TOO_MANY_MEMBERS_S
ERR_ARENA_TEAM_LEVEL_TOO_LOW_I
*/
 
enum ArenaTeamTypes
{
    ARENA_TEAM_2v2      = 2,
    ARENA_TEAM_3v3      = 3,
    ARENA_TEAM_5v5      = 5
};
 
struct ArenaTeamMember
{
    uint64 Guid;
    std::string Name;
    uint8 Class;
    uint16 WeekGames;
    uint16 WeekWins;
    uint16 SeasonGames;
    uint16 SeasonWins;
    uint16 PersonalRating;
    uint16 MatchMakerRating;
 
    void ModifyPersonalRating(Player* player, int32 mod, uint32 type);
    void ModifyMatchmakerRating(int32 mod, uint32 slot);
};
 
struct ArenaTeamStats
{
    uint16 Rating;
    uint16 WeekGames;
    uint16 WeekWins;
    uint16 SeasonGames;
    uint16 SeasonWins;
    uint32 Rank;
};

enum ArenaSlots
{
    SLOT_ARENA_2V2 = 0,
    SLOT_ARENA_3V3 = 1,
    SLOT_ARENA_5V5 = 2,
    MAX_ARENA_SLOT = 3,
    SLOT_RBG       = 3,
    MAX_PVP_SLOT   = 4
};

#define MAX_ARENA_TYPE 6                                    // type : 2, 3 or 5
 
namespace Arena
{
    inline uint8 GetSlotByType(uint32 type)
    {
        switch (type)
        {
            case ARENA_TEAM_2v2: return SLOT_ARENA_2V2;
            case ARENA_TEAM_3v3: return SLOT_ARENA_3V3;
            case ARENA_TEAM_5v5: return SLOT_ARENA_5V5;
            default:
                break;
        }
        sLog->outError(LOG_FILTER_ARENAS, "FATAL: Unknown arena team type %u for some arena team", type);
        return 0xFF;
    }
 
    inline uint8 GetTypeBySlot(uint8 slot)
    {
        switch (slot)
        {
            case 0:
                return ARENA_TYPE_2v2;
            case 1:
                return ARENA_TYPE_3v3;
            case 2:
                return ARENA_TYPE_5v5;
            default:
                break;
        }
        sLog->outError(LOG_FILTER_ARENAS, "FATAL: Unknown arena team slot %u for some arena team", slot);
        return 0xFF;
    }
 
    inline float GetChanceAgainst(uint32 ownRating, uint32 opponentRating)
    {
        // Returns the chance to win against a team with the given rating, used in the rating adjustment calculation
        // ELO system
        return 1.0f / (1.0f + exp(log(10.0f) * (float)((float)opponentRating - (float)ownRating) / 650.0f));
    }
 
    inline int32 GetRatingMod(uint32 ownRating, uint32 opponentRating, bool won /*, float confidence_factor*/)
    {
        // 'Chance' calculation - to beat the opponent
        // This is a simulation. Not much info on how it really works
        float chance = GetChanceAgainst(ownRating, opponentRating);
        float won_mod = (won) ? 1.0f : 0.0f;
 
        // Calculate the rating modification
        float mod;
 
        // TODO: Replace this hack with using the confidence factor (limiting the factor to 2.0f)
        if (won && ownRating < 1300)
        {
            if (ownRating < 1000)
                mod = 48.0f * (won_mod - chance);
            else
                mod = (24.0f + (24.0f * (1300.0f - float(ownRating)) / 300.0f)) * (won_mod - chance);
        }
        else
            mod = 24.0f * (won_mod - chance);
 
        return (int32)ceil(mod);
    }
 
    inline int32 GetMatchmakerRatingMod(uint32 ownRating, uint32 opponentRating, bool won /*, float& confidence_factor*/)
    {
        // 'Chance' calculation - to beat the opponent
        // This is a simulation. Not much info on how it really works
        float chance = GetChanceAgainst(ownRating, opponentRating);
        float won_mod = (won) ? 1.0f : 0.0f;
        float mod = won_mod - chance;
 
        // Work in progress:
        /*
        // This is a simulation, as there is not much info on how it really works
        float confidence_mod = min(1.0f - fabs(mod), 0.5f);
 
        // Apply confidence factor to the mod:
        mod *= confidence_factor
 
        // And only after that update the new confidence factor
        confidence_factor -= ((confidence_factor - 1.0f) * confidence_mod) / confidence_factor;
        */
 
        // Real rating modification
        mod *= 24.0f;
 
        return (int32)ceil(mod);
    }
};
 
#endif