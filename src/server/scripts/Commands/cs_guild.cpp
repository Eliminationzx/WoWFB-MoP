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

/* ScriptData
Name: guild_commandscript
%Complete: 100
Comment: All guild related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "ObjectAccessor.h"

class guild_commandscript : public CommandScript
{
public:
    guild_commandscript() : CommandScript("guild_commandscript") { }

    ChatCommand* GetCommands() const
    {
        static ChatCommand guildCommandTable[] =
        {
            { "create",         SEC_GAMEMASTER,     true,  &HandleGuildCreateCommand,           "", NULL },
            { "delete",         SEC_GAMEMASTER,     true,  &HandleGuildDeleteCommand,           "", NULL },
            { "invite",         SEC_GAMEMASTER,     true,  &HandleGuildInviteCommand,           "", NULL },
            { "uninvite",       SEC_GAMEMASTER,     true,  &HandleGuildUninviteCommand,         "", NULL },
            { "rank",           SEC_GAMEMASTER,     true,  &HandleGuildRankCommand,             "", NULL },
            { "rename",         SEC_GAMEMASTER,     true,  &HandleGuildRenameCommand,           "", NULL },
            { "givexp",         SEC_GAMEMASTER,     true,  &HandleGuildXpCommand,               "", NULL },
            { "levelup",        SEC_GAMEMASTER,     true,  &HandleGuildLevelUpCommand,          "", NULL },
            { NULL,             0,                  false, NULL,                                "", NULL }
        };
        static ChatCommand commandTable[] =
        {
            { "guild",          SEC_ADMINISTRATOR,  true, NULL,                                 "", guildCommandTable },
            { NULL,             0,                  false, NULL,                                "", NULL }
        };
        return commandTable;
    }

    /** \brief GM command level 3 - Create a guild.
     *
     * This command allows a GM (level 3) to create a guild.
     *
     * The "args" parameter contains the name of the guild leader
     * and then the name of the guild.
     *
     */
    static bool HandleGuildCreateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // if not guild name only (in "") then player name
        Player* target;
        if (!handler->extractPlayerTarget(*args != '"' ? (char*)args : NULL, &target))
            return false;

        char* tailStr = *args != '"' ? strtok(NULL, "") : (char*)args;
        if (!tailStr)
            return false;

        char* guildStr = handler->extractQuotedArg(tailStr);
        if (!guildStr)
            return false;

        std::string guildName = guildStr;

        if (target->GetGuildId())
        {
            handler->SendSysMessage(LANG_PLAYER_IN_GUILD);
            return true;
        }

        Guild* guild = new Guild;
        if (!guild->Create(target, guildName))
        {
            delete guild;
            handler->SendSysMessage(LANG_GUILD_NOT_CREATED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        sGuildMgr->AddGuild(guild);

        return true;
    }

    static bool HandleGuildDeleteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* guildStr = handler->extractQuotedArg((char*)args);
        if (!guildStr)
            return false;

        std::string guildName = guildStr;

        Guild* targetGuild = sGuildMgr->GetGuildByName(guildName);
        if (!targetGuild)
            return false;

        targetGuild->Disband();
        delete targetGuild;

        return true;
    }

    static bool HandleGuildInviteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // if not guild name only (in "") then player name
        uint64 targetGuid;
        if (!handler->extractPlayerTarget(*args != '"' ? (char*)args : NULL, NULL, &targetGuid))
            return false;

        char* tailStr = *args != '"' ? strtok(NULL, "") : (char*)args;
        if (!tailStr)
            return false;

        char* guildStr = handler->extractQuotedArg(tailStr);
        if (!guildStr)
            return false;

        std::string guildName = guildStr;
        Guild* targetGuild = sGuildMgr->GetGuildByName(guildName);
        if (!targetGuild)
            return false;

        // player's guild membership checked in AddMember before add
        return targetGuild->AddMember(targetGuid);
    }

    static bool HandleGuildUninviteCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        uint64 targetGuid;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid))
            return false;

        uint32 guildId = target ? target->GetGuildId() : Player::GetGuildIdFromDB(targetGuid);
        if (!guildId)
            return false;

        Guild* targetGuild = sGuildMgr->GetGuildById(guildId);
        if (!targetGuild)
            return false;

        targetGuild->DeleteMember(targetGuid, false, true, true);
        return true;
    }

    static bool HandleGuildRankCommand(ChatHandler* handler, char const* args)
    {
        char* nameStr;
        char* rankStr;
        handler->extractOptFirstArg((char*)args, &nameStr, &rankStr);
        if (!rankStr)
            return false;

        Player* target;
        uint64 targetGuid;
        std::string target_name;
        if (!handler->extractPlayerTarget(nameStr, &target, &targetGuid, &target_name))
            return false;

        uint32 guildId = target ? target->GetGuildId() : Player::GetGuildIdFromDB(targetGuid);
        if (!guildId)
            return false;

        Guild* targetGuild = sGuildMgr->GetGuildById(guildId);
        if (!targetGuild)
            return false;

        uint8 newRank = uint8(atoi(rankStr));
       return targetGuild->ChangeMemberRank(targetGuid, newRank);
    }

    static bool HandleGuildRenameCommand(ChatHandler* handler, char const* _args)
    {
        if (!*_args)
            return false;

        char *args = (char *)_args;

        char const* oldGuildStr = handler->extractQuotedArg(args);
        if (!oldGuildStr)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* newGuildArg = strtok(NULL, "");
        if (!newGuildArg)
            return false;

        char const* newGuildStr = handler->extractQuotedArg(newGuildArg);
        if (!newGuildStr)
        {
            handler->SendSysMessage(LANG_INSERT_GUILD_NAME);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Guild* guild = sGuildMgr->GetGuildByName(oldGuildStr);
        if (!guild)
        {
            handler->PSendSysMessage(LANG_COMMAND_COULDNOTFIND, oldGuildStr);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (sGuildMgr->GetGuildByName(newGuildStr))
        {
            handler->PSendSysMessage(LANG_GUILD_RENAME_ALREADY_EXISTS, newGuildStr);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!guild->SetName(newGuildStr))
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_GUILD_RENAME_DONE, oldGuildStr, newGuildStr);
        return true;
    }

    static bool HandleGuildXpCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Guild* targetGuild;
        uint32 amount = 0;

        if (*args == '"')
        {
            char* guildStr = handler->extractQuotedArg((char*)args);
            if (!guildStr)
                return false;

            targetGuild = sGuildMgr->GetGuildByName(guildStr);
        }
        else
        {
            if (!handler->getSelectedPlayer())
                return false;

            Player* target = handler->getSelectedPlayer();

            if (!target->GetGuildId())
                return false;

            targetGuild = sGuildMgr->GetGuildById(target->GetGuildId());
        }

        amount = (uint32)atoi(args);

        if (!targetGuild)
            return false;

        targetGuild->GiveXP(amount, handler->GetSession() ? handler->GetSession()->GetPlayer() : NULL);
        return true;
    }

    static bool HandleGuildLevelUpCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Guild* targetGuild;
        uint32 amount = 0;

        if (*args == '"')
        {
            char* guildStr = handler->extractQuotedArg((char*)args);
            if (!guildStr)
                return false;

            targetGuild = sGuildMgr->GetGuildByName(guildStr);
        }
        else
        {
            if (!handler->getSelectedPlayer())
                return false;

            Player* target = handler->getSelectedPlayer();

            if (!target->GetGuildId())
                return false;

            targetGuild = sGuildMgr->GetGuildById(target->GetGuildId());
        }

        amount = (uint32)atoi(args);

        if (!targetGuild)
            return false;

        uint32 experience = 0;

        for (uint8 i = 0; i < amount; ++i)
        {
            if (targetGuild->GetLevel() >= sWorld->getIntConfig(CONFIG_GUILD_MAX_LEVEL))
                break;

            experience = sGuildMgr->GetXPForGuildLevel(targetGuild->GetLevel()) - targetGuild->GetExperience();
            targetGuild->GiveXP(experience, handler->GetSession() ? handler->GetSession()->GetPlayer() : NULL);
        }
        return true;
    }
};

void AddSC_guild_commandscript()
{
    new guild_commandscript();
}
