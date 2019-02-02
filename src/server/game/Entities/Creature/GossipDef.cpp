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

#include "QuestDef.h"
#include "GossipDef.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Formulas.h"

GossipMenu::GossipMenu()
{
    _menuId = 0;
}

GossipMenu::~GossipMenu()
{
    ClearMenu();
}

void GossipMenu::AddMenuItem(int32 menuItemId, uint8 icon, std::string const& message, uint32 sender, uint32 action, std::string const& boxMessage, uint32 boxMoney, bool coded /*= false*/)
{
    ASSERT(_menuItems.size() <= GOSSIP_MAX_MENU_ITEMS);

    // Find a free new id - script case
    if (menuItemId == -1)
    {
        menuItemId = 0;
        if (!_menuItems.empty())
        {
            for (GossipMenuItemContainer::const_iterator itr = _menuItems.begin(); itr != _menuItems.end(); ++itr)
            {
                if (int32(itr->first) > menuItemId)
                    break;

                menuItemId = itr->first + 1;
            }
        }
    }

    GossipMenuItem& menuItem = _menuItems[menuItemId];

    menuItem.MenuItemIcon    = icon;
    menuItem.Message         = message;
    menuItem.IsCoded         = coded;
    menuItem.Sender          = sender;
    menuItem.OptionType      = action;
    menuItem.BoxMessage      = boxMessage;
    menuItem.BoxMoney        = boxMoney;
}

/**
 * @name AddMenuItem
 * @brief Adds a localized gossip menu item from db by menu id and menu item id.
 * @param menuId Gossip menu id.
 * @param menuItemId Gossip menu item id.
 * @param sender Identifier of the current menu.
 * @param action Custom action given to OnGossipHello.
 */
void GossipMenu::AddMenuItem(uint32 menuId, uint32 menuItemId, uint32 sender, uint32 action)
{
    /// Find items for given menu id.
    GossipMenuItemsMapBounds bounds = sObjectMgr->GetGossipMenuItemsMapBounds(menuId);
    /// Return if there are none.
    if (bounds.first == bounds.second)
        return;

    /// Iterate over each of them.
    for (GossipMenuItemsContainer::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        /// Find the one with the given menu item id.
        if (itr->second.OptionIndex != menuItemId)
            continue;

        /// Store texts for localization.
        std::string strOptionText = itr->second.OptionText;
        std::string strBoxText = itr->second.BoxText;

        /// Check need of localization.
        if (GetLocale() > LOCALE_enUS)
            /// Find localizations from database.
            if (GossipMenuItemsLocale const* no = sObjectMgr->GetGossipMenuItemsLocale(MAKE_PAIR32(menuId, menuItemId)))
            {
                /// Translate texts if there are any.
                ObjectMgr::GetLocaleString(no->OptionText, GetLocale(), strOptionText);
                ObjectMgr::GetLocaleString(no->BoxText, GetLocale(), strBoxText);
            }

        /// Add menu item with existing method. Menu item id -1 is also used in ADD_GOSSIP_ITEM macro.
        AddMenuItem(-1, itr->second.OptionIcon, strOptionText, sender, action, strBoxText, itr->second.BoxMoney, itr->second.BoxCoded);
    }
}

void GossipMenu::AddGossipMenuItemData(uint32 menuItemId, uint32 gossipActionMenuId, uint32 gossipActionPoi)
{
    GossipMenuItemData& itemData = _menuItemData[menuItemId];

    itemData.GossipActionMenuId  = gossipActionMenuId;
    itemData.GossipActionPoi     = gossipActionPoi;
}

uint32 GossipMenu::GetMenuItemSender(uint32 menuItemId) const
{
    GossipMenuItemContainer::const_iterator itr = _menuItems.find(menuItemId);
    if (itr == _menuItems.end())
        return 0;

    return itr->second.Sender;
}

uint32 GossipMenu::GetMenuItemAction(uint32 menuItemId) const
{
    GossipMenuItemContainer::const_iterator itr = _menuItems.find(menuItemId);
    if (itr == _menuItems.end())
        return 0;

    return itr->second.OptionType;
}

bool GossipMenu::IsMenuItemCoded(uint32 menuItemId) const
{
    GossipMenuItemContainer::const_iterator itr = _menuItems.find(menuItemId);
    if (itr == _menuItems.end())
        return false;

    return itr->second.IsCoded;
}

void GossipMenu::ClearMenu()
{
    _menuItems.clear();
    _menuItemData.clear();
}

PlayerMenu::PlayerMenu(WorldSession* session) : _session(session)
{
    if (_session)
        _gossipMenu.SetLocale(_session->GetSessionDbLocaleIndex());
}

PlayerMenu::~PlayerMenu()
{
    ClearMenus();
}

void PlayerMenu::ClearMenus()
{
    _gossipMenu.ClearMenu();
    _questMenu.ClearMenu();
}

#define DEFAULT_GREETINGS_GOSSIP      68

void PlayerMenu::SendGossipMenu(uint32 titleTextId, uint64 objectGUID) const
{
    ObjectGuid guid = objectGUID;

    WorldPacket data(SMSG_GOSSIP_MESSAGE, 100);         // guess size

    data.WriteBit(guid[7]);
    data.WriteBit(guid[6]);
    data.WriteBit(guid[1]);
    data.WriteBits(_questMenu.GetMenuItemCount(), 19);      // max count 0x20
    data.WriteBit(guid[0]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[5]);
    data.WriteBit(guid[2]);
    data.WriteBit(guid[3]);
    data.WriteBits(_gossipMenu.GetMenuItemCount(), 20);     // max count 0x10

    for (GossipMenuItemContainer::const_iterator itr = _gossipMenu.GetMenuItems().begin(); itr != _gossipMenu.GetMenuItems().end(); ++itr)
    {
        GossipMenuItem const& item = itr->second;
        data.WriteBits(item.BoxMessage.size(), 12);
        data.WriteBits(item.Message.size(), 12);
    }

    for (uint32 iI = 0; iI < _questMenu.GetMenuItemCount(); ++iI)
    {
        QuestMenuItem const& item = _questMenu.GetItem(iI);
        uint32 questID = item.QuestId;
        Quest const* quest = sObjectMgr->GetQuestTemplate(questID);
        std::string title = quest->GetTitle();

        int locale = _session->GetSessionDbLocaleIndex();
        if (locale >= 0)
            if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(questID))
                ObjectMgr::GetLocaleString(localeData->Title, locale, title);

        data.WriteBit(quest->IsRepeatable());                   // 3.3.3 changes icon: blue question or yellow exclamation, is repeatable
        data.WriteBits(title.size(), 9);
    }

    //data.FlushBits();

    for (uint32 iI = 0; iI < _questMenu.GetMenuItemCount(); ++iI)
    {
        QuestMenuItem const& item = _questMenu.GetItem(iI);
        uint32 questID = item.QuestId;
        Quest const* quest = sObjectMgr->GetQuestTemplate(questID);
        std::string title = quest->GetTitle();

        int locale = _session->GetSessionDbLocaleIndex();
        if (locale >= 0)
            if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(questID))
                ObjectMgr::GetLocaleString(localeData->Title, locale, title);

        Player* plr = _session->GetPlayer();

        uint32 questStat = plr ? plr->GetQuestStatus(questID) : 0;

        if (questStat == QUEST_STATUS_COMPLETE || questStat == QUEST_STATUS_INCOMPLETE)
        {
            if (quest->IsRepeatable())
                questStat = 0;
            else
                questStat = 4;
        }
        else if (questStat == QUEST_STATE_NONE)
            questStat = 2;

        data << uint32(0);                              // quest flags 2
        data << int32(questID);
        data << int32(quest->GetQuestLevel());          // quest level
        data << uint32(item.QuestIcon);                 // quest icon
        data << uint32(quest->GetFlags());              // quest flags
        data.WriteString(title);                        // quest title
    }
    
    data.WriteByteSeq(guid[6]);

    for (GossipMenuItemContainer::const_iterator itr = _gossipMenu.GetMenuItems().begin(); itr != _gossipMenu.GetMenuItems().end(); ++itr)
    {
        GossipMenuItem const& item = itr->second;
        data << uint32(item.BoxMoney);                      // money required to open menu, 2.0.3
        data.WriteString(item.Message);                     // text for gossip item
        data << uint32(itr->first);
        data << uint8(item.MenuItemIcon);
        data.WriteString(item.BoxMessage);                  // accept text (related to money) pop up box, 2.0.
        data << uint8(item.IsCoded);                        // makes pop up box password
    }

    data.WriteByteSeq(guid[2]);
    
    data << int32(titleTextId);

    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[5]);

    /*if (titleTextId == DEFAULT_GOSSIP_MESSAGE && !_gossipMenu.GetMenuId())
        data << uint32(DEFAULT_GREETINGS_GOSSIP);           // default greeting ID
    else
        data << uint32(_gossipMenu.GetMenuId());            // new 2.4.0*/
    data << int32(_gossipMenu.GetMenuId());                 // new 2.4.0
    data << int32(0);                                       // friend faction ID?

    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[0]);

    _session->SendPacket(&data);
}

void PlayerMenu::SendCloseGossip() const
{
    WorldPacket data(SMSG_GOSSIP_COMPLETE, 0);
    _session->SendPacket(&data);
}

void PlayerMenu::SendPointOfInterest(uint32 poiId) const
{
    PointOfInterest const* poi = sObjectMgr->GetPointOfInterest(poiId);
    if (!poi)
    {
        sLog->outError(LOG_FILTER_SQL, "Request to send non-existing POI (Id: %u), ignored.", poiId);
        return;
    }

    std::string iconText = poi->icon_name;
    int32 locale = _session->GetSessionDbLocaleIndex();
    if (locale >= 0)
        if (PointOfInterestLocale const* localeData = sObjectMgr->GetPointOfInterestLocale(poiId))
            ObjectMgr::GetLocaleString(localeData->IconName, locale, iconText);

    WorldPacket data(SMSG_GOSSIP_POI, 4 + 4 + 4 + 4 + 4 + 10);  // guess size
    data << uint32(poi->flags);
    data << float(poi->x);
    data << float(poi->y);
    data << uint32(poi->icon);
    data << uint32(poi->data);
    data << iconText;

    _session->SendPacket(&data);
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

QuestMenu::QuestMenu()
{
    _questMenuItems.reserve(16);                                   // can be set for max from most often sizes to speedup push_back and less memory use
}

QuestMenu::~QuestMenu()
{
    ClearMenu();
}

void QuestMenu::AddMenuItem(uint32 QuestId, uint8 Icon)
{
    if (!sObjectMgr->GetQuestTemplate(QuestId))
        return;

    ASSERT(_questMenuItems.size() <= GOSSIP_MAX_MENU_ITEMS);

    QuestMenuItem questMenuItem;

    questMenuItem.QuestId        = QuestId;
    questMenuItem.QuestIcon      = Icon;

    _questMenuItems.push_back(questMenuItem);
}

bool QuestMenu::HasItem(uint32 questId) const
{
    for (QuestMenuItemList::const_iterator i = _questMenuItems.begin(); i != _questMenuItems.end(); ++i)
        if (i->QuestId == questId)
            return true;

    return false;
}

void QuestMenu::ClearMenu()
{
    _questMenuItems.clear();
}

void PlayerMenu::SendQuestGiverQuestList(QEmote eEmote, const std::string& Title, uint64 npcGUID)
{
    ByteBuffer questData;

    ObjectGuid guid = npcGUID;
    WorldPacket data(SMSG_QUESTGIVER_QUEST_LIST, 100);    // guess size
    data << uint32(eEmote._Emote);                          // NPC emote
    data << uint32(eEmote._Delay);                          // player emote
    data.WriteBit(guid[5]);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[2]);
    data.WriteBit(guid[6]);
    data.WriteBit(guid[1]);
    data.WriteBits(Title.size(), 11);
    data.WriteBit(guid[0]);
    data.WriteBits(_questMenu.GetMenuItemCount(), 19);

    uint32 count = 0;
    for (; count < _questMenu.GetMenuItemCount(); ++count)
    {
        QuestMenuItem const& qmi = _questMenu.GetItem(count);

        uint32 questID = qmi.QuestId;

        if (Quest const* quest = sObjectMgr->GetQuestTemplate(questID))
        {
            std::string title = quest->GetTitle();

            int loc_idx = _session->GetSessionDbLocaleIndex();
            if (loc_idx >= 0)
                if (QuestLocale const* ql = sObjectMgr->GetQuestLocale(questID))
                    ObjectMgr::GetLocaleString(ql->Title, loc_idx, title);

            data.WriteBit(quest->IsRepeatable());                               // unknown bit
            data.WriteBits(title.size(), 9);

            Player* plr = _session->GetPlayer();

            uint32 questStat = plr ? plr->GetQuestStatus(questID) : 0;

            if (questStat == QUEST_STATUS_COMPLETE || questStat == QUEST_STATUS_INCOMPLETE)
            {
                if (quest->IsRepeatable())
                    questStat = 0;
                else
                    questStat = 4;
            }
            else if (questStat == QUEST_STATE_NONE)
                questStat = 2;

            questData << uint32(quest->GetQuestLevel());
            questData << uint32(quest->GetFlags());
            questData.WriteString(title);
            questData << uint32(questStat);
            questData << uint32(0);                              // quest flags 2
            questData << uint32(questID);
        }
    }

    data.WriteBit(guid[7]);

    data.WriteByteSeq(guid[3]);
    data.append(questData);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[0]);
    data.WriteString(Title);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[7]);

    _session->SendPacket(&data);
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_QUESTGIVER_QUEST_LIST NPC Guid=%u", GUID_LOPART(npcGUID));
}

void PlayerMenu::SendQuestGiverStatus(uint32 questStatus, uint64 npcGUID) const
{
    ObjectGuid guid = npcGUID;
    WorldPacket data(SMSG_QUESTGIVER_STATUS, 13);
    uint8 bitOrder[8] = {1, 5, 2, 0, 4, 3, 7, 6};
    data.WriteBitInOrder(guid, bitOrder);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[4]);
    data << uint32(questStatus);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[5]);

    _session->SendPacket(&data);
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_QUESTGIVER_STATUS NPC Guid=%u, status=%u", GUID_LOPART(npcGUID), questStatus);
}

void PlayerMenu::SendQuestGiverQuestDetails(Quest const* quest, uint64 npcGUID, bool activateAccept) const
{
    std::string questTitle           = quest->GetTitle();
    std::string questDetails         = quest->GetDetails();
    std::string questObjectives      = quest->GetObjectives();
    std::string questGiverTextWindow = quest->GetQuestGiverTextWindow();
    std::string questGiverTargetName = quest->GetQuestGiverTargetName();
    std::string questTurnTextWindow  = quest->GetQuestTurnTextWindow();
    std::string questTurnTargetName  = quest->GetQuestTurnTargetName();

    int32 locale = _session->GetSessionDbLocaleIndex();
    if (locale >= 0)
    {
        if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(quest->GetQuestId()))
        {
            ObjectMgr::GetLocaleString(localeData->Title, locale, questTitle);
            ObjectMgr::GetLocaleString(localeData->Details, locale, questDetails);
            ObjectMgr::GetLocaleString(localeData->Objectives, locale, questObjectives);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTextWindow, locale, questGiverTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTargetName, locale, questGiverTargetName);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTextWindow, locale, questTurnTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTargetName, locale, questTurnTargetName);
        }
    }
    ByteBuffer objData;
    int objCount = 0;

    /*
     *  -- Objective Structure --
     *  uint32 - objective count (value required to complete objective)
     *  uint32 - objective Id (NPC, GO, item, spell, currency or faction Id)
     *  uint8  - objective type (see QuestObjectiveType enum)
     *  uint32 - unknown
     */

    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if (quest->RequiredNpcOrGo[i] != 0)
        {
            int32 requiredObjId = quest->RequiredNpcOrGo[i];

            objData << int32(quest->RequiredNpcOrGoCount[i]);
            objData << uint32(requiredObjId < 0 ? requiredObjId * -1 : requiredObjId);
            objData << uint8(requiredObjId < 0 ? QUEST_OBJECTIVE_TYPE_GO : QUEST_OBJECTIVE_TYPE_NPC);
            objData << uint32(0);

            objCount++;
        }
    }

    for (uint8 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; i++)
    {
        if (quest->RequiredItemId[i] != 0)
        {
            objData << int32(quest->RequiredItemCount[i]);
            objData << uint32(quest->RequiredItemId[i]);
            objData << uint8(QUEST_OBJECTIVE_TYPE_ITEM);
            objData << uint32(0);

            objCount++;
        }
    }

    for (uint8 i = 0; i < QUEST_REQUIRED_CURRENCY_COUNT; i++)
    {
        if (quest->RequiredCurrencyId[i] != 0)
        {
            objData << int32(quest->RequiredCurrencyCount[i]);
            objData << uint32(quest->RequiredCurrencyId[i]);
            objData << uint8(QUEST_OBJECTIVE_TYPE_CURRENCY);
            objData << uint32(0);

            objCount++;
        }
    }

    if (quest->GetRequiredSpell() != 0)
    {
        objData << int32(1);
        objData << uint32(quest->GetRequiredSpell());
        objData << uint8(QUEST_OBJECTIVE_TYPE_SPELL);
        objData << uint32(0);

        objCount++;
    }

    if (quest->GetRepObjectiveFaction() != 0)
    {
        objData << int32(quest->GetRepObjectiveValue());
        objData << uint32(quest->GetRepObjectiveFaction());
        objData << uint8(QUEST_OBJECTIVE_TYPE_FACTION_REP);
        objData << uint32(0);

        objCount++;
    }

    if (quest->GetRepObjectiveFaction2() != 0)
    {
        objData << int32(quest->GetRepObjectiveValue2());
        objData << uint32(quest->GetRepObjectiveFaction2());
        objData << uint8(QUEST_OBJECTIVE_TYPE_FACTION_REP2);
        objData << uint32(0);

        objCount++;
    }

    // only seen in quests 13233 (No Mercy!) and 13234 (Make Them Pay!)
    if (quest->GetPlayersSlain() != 0)
    {
        objData << int32(quest->GetPlayersSlain());
        objData << uint32(0x149);
        objData << uint8(QUEST_OBJECTIVE_TYPE_PLAYER);
        objData << uint32(0);

        objCount++;
    }

    ObjectGuid guid = npcGUID;
    ObjectGuid guid2 = npcGUID;                             // quest ender guid?
    bool l_HiddenRewardItem = quest->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS);

    WorldPacket data(SMSG_QUESTGIVER_QUEST_DETAILS, 100);   // guess size
    data << uint32(0);                                      // unknown
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardItemId[0]);

    for (uint8 i = 0; i < QUEST_REPUTATIONS_COUNT; i++)
    {
        data << uint32(quest->RewardFactionId[i]);
        data << uint32(quest->RewardFactionValueId[i]);
        data << uint32(quest->RewardFactionValueIdOverride[i]);
    }

    data << uint32(quest->XPValue(_session->GetPlayer()) * _session->GetPlayer()->GetRate<float>(RateType::XpQuest));

    for (uint8 i = 0; i < QUEST_REWARD_CURRENCY_COUNT; i++)
    {
        data << uint32(quest->RewardCurrencyCount[i]);
        data << uint32(quest->RewardCurrencyId[i]);
    }

    uint32 rewItemDisplayId[QUEST_REWARDS_COUNT];
    uint32 rewChoiceItemDisplayId[QUEST_REWARD_CHOICES_COUNT];

    for (uint8 i = 0; i < QUEST_REWARDS_COUNT; i++)
    {
        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RewardItemId[i]))
            rewItemDisplayId[i] = itemTemplate->DisplayInfoID;
        else
            rewItemDisplayId[i] = 0;
    }

    for (uint8 i = 0; i < QUEST_REWARD_CHOICES_COUNT; i++)
    {
        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RewardChoiceItemId[i]))
            rewChoiceItemDisplayId[i] = itemTemplate->DisplayInfoID;
        else
            rewChoiceItemDisplayId[i] = 0;
    }

    // values need rechecking and zero values need more research
    data << uint32(rewItemDisplayId[3]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemId[5]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemCount[5]);
    data << uint32(quest->GetQuestId());
    data << uint32(rewChoiceItemDisplayId[5]);
    data << uint32(rewChoiceItemDisplayId[1]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemCount[2]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemId[1]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardItemId[3]);
    data << uint32(quest->GetBonusTalents());
    data << uint32(0);                                      // model Id
    data << uint32(quest->GetFlags());
    data << uint32(rewChoiceItemDisplayId[3]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemCount[4]);

    if (l_HiddenRewardItem)
        data << uint32(0);
    else
        data << uint32(quest->GetRewOrReqMoney());

    data << uint32(0);                                      // unknown
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemId[0]);
    data << uint32(quest->GetRewItemsCount());
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardItemIdCount[2]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardItemIdCount[0]);
    data << uint32(quest->GetCharTitleId());
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemCount[0]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardItemIdCount[1]);
    data << uint32(rewItemDisplayId[2]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemId[2]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardItemId[1]);
    data << uint32(0);                                      // unknown
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemId[4]);
    data << uint32(quest->GetRewSpell());
    data << uint32(rewItemDisplayId[1]);
    data << uint32(rewChoiceItemDisplayId[2]);
    data << uint32(0);                                      // unknown
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardItemId[2]);
    data << uint32(0);                                      // unknown
    data << uint32(quest->GetRewChoiceItemsCount());
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemCount[3]);
    data << uint32(rewChoiceItemDisplayId[0]);
    data << uint32(0);                                      // unknown
    data << uint32(quest->GetRewardReputationMask());
    data << uint32(quest->GetSuggestedPlayers());
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardItemIdCount[3]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemCount[1]);
    data << uint32(rewChoiceItemDisplayId[4]);
    data << uint32(rewItemDisplayId[0]);
    data << uint32(l_HiddenRewardItem ? 0 : quest->RewardChoiceItemId[3]);

    data.WriteBit(guid2[5]);
    data.WriteBits(questGiverTargetName.size(), 8);
    data.WriteBits(questTurnTargetName.size(), 8);
    data.WriteBits(0, 22);                                  // unknown counter
    data.WriteBit(false);                                   // StartCheat
    data.WriteBit(guid2[1]);
    data.WriteBits(questDetails.size(), 12);
    data.WriteBit(guid2[0]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[1]);
    data.WriteBits(questTurnTextWindow.size(), 10);
    data.WriteBits(QUEST_EMOTE_COUNT, 21);
    data.WriteBits(objCount, 20);
    data.WriteBit(guid[3]);
    data.WriteBit(guid2[3]);
    data.WriteBit(activateAccept);                             // DisplayPopup
    data.WriteBit(guid[2]);
    data.WriteBit(guid2[2]);
    data.WriteBits(questGiverTextWindow.size(), 10);
    data.WriteBits(questTitle.size(), 9);
    data.WriteBits(questObjectives.size(), 12);
    data.WriteBit(guid2[4]);
    data.WriteBit(guid[5]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[6]);
    data.WriteBit(guid[0]);
    data.WriteBit(false);                                     // AutoLaunched
    data.WriteBit(guid2[6]);
    data.WriteBit(guid2[7]);

    for (uint8 i = 0; i < QUEST_EMOTE_COUNT; i++)
    {
        data << uint32(quest->DetailsEmote[i]);
        data << uint32(quest->DetailsEmoteDelay[i]);        // DetailsEmoteDelay (in ms)
    }

    data.WriteByteSeq(guid2[2]);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid2[1]);
    data.WriteString(questGiverTextWindow);
    data.append(objData);
    data.WriteByteSeq(guid2[7]);
    data.WriteString(questTurnTargetName);
    data.WriteString(questTitle);
    data.WriteString(questGiverTargetName);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid2[3]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid2[5]);

    /*for (uint i = 0; i < unkCounterBits22; i++)
        data << uint32(0)*/

    data.WriteByteSeq(guid2[4]);
    data.WriteByteSeq(guid2[0]);
    data.WriteByteSeq(guid2[6]);
    data.WriteByteSeq(guid[4]);
    data.WriteString(questObjectives);
    data.WriteString(questTurnTextWindow);
    data.WriteString(questDetails);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[0]);

    _session->SendPacket(&data);

    /*
     *  -- Missing Values from Cata could be an unknown --
     *  quest->GetQuestGiverPortrait()
     *  quest->GetQuestTurnInPortrait()
     *  GetRewSpellCast()
     *  GetRewardSkillId()
     *  GetRewardSkillPoints()
     */

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS NPCGuid=%u, questid=%u", GUID_LOPART(npcGUID), quest->GetQuestId());
}

void PlayerMenu::SendQuestQueryResponse(Quest const* quest) const
{
    std::string questTitle = quest->GetTitle();
    std::string questDetails = quest->GetDetails();
    std::string questObjectives = quest->GetObjectives();
    std::string questEndText = quest->GetEndText();
    std::string questCompletedText = quest->GetCompletedText();
    std::string questGiverTextWindow = quest->GetQuestGiverTextWindow();
    std::string questGiverTargetName = quest->GetQuestGiverTargetName();
    std::string questTurnTextWindow = quest->GetQuestTurnTextWindow();
    std::string questTurnTargetName = quest->GetQuestTurnTargetName();

    std::string questObjectiveText[QUEST_OBJECTIVES_COUNT];
    for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        questObjectiveText[i] = quest->ObjectiveText[i];

    int32 locale = _session->GetSessionDbLocaleIndex();
    if (locale >= 0)
    {
        if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(quest->GetQuestId()))
        {
            ObjectMgr::GetLocaleString(localeData->Title, locale, questTitle);
            ObjectMgr::GetLocaleString(localeData->Details, locale, questDetails);
            ObjectMgr::GetLocaleString(localeData->Objectives, locale, questObjectives);
            ObjectMgr::GetLocaleString(localeData->EndText, locale, questEndText);
            ObjectMgr::GetLocaleString(localeData->CompletedText, locale, questCompletedText);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTextWindow, locale, questGiverTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTargetName, locale, questGiverTargetName);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTextWindow, locale, questTurnTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTargetName, locale, questTurnTargetName);

            for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
                ObjectMgr::GetLocaleString(localeData->ObjectiveText[i], locale, questObjectiveText[i]);
        }
    }
    WorldPacket data(SMSG_QUEST_QUERY_RESPONSE, 100);   // guess size
    data << uint32(quest->GetQuestId());

    data.WriteBit(1);                                   // has data
    data.WriteBits(questEndText.size(), 9);
    data.WriteBits(questTitle.size(), 9);
    data.WriteBits(questGiverTextWindow.size(), 10);
    data.WriteBits(questTurnTargetName.size(), 8);
    data.WriteBits(questObjectives.size(), 12);
    data.WriteBits(questGiverTargetName.size(), 8);
    data.WriteBits(questDetails.size(), 12);
    data.WriteBits(questTurnTextWindow.size(), 10);
    data.WriteBits(questCompletedText.size(), 11);

    ByteBuffer objData;
    uint8 objCount = 0;

    size_t countPos = data.wpos();
    uint8 bitPos = data.bitpos();

    data.WriteBits(objCount, 19);

    /*
     *  -- Objective Structure --
     *  -- Bits --
     *  bits22 - unknown counter
     *  bits8  - objective text length
     *  -- Bytes --
     *  uint32 - unknown
     *  string - objective text (WoW string, not null terminated and no length prefix)
     *  uint8  - objective type (see QuestObjectiveType enum)
     *  uint8  - objective index
     *  int32  - objective count (value required to complete objective)
     *  uint   - unknown
     *  uint   - objective Id (NPC, GO, item, spell, currency or faction Id)
     */

    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if (quest->RequiredNpcOrGo[i] != 0)
        {
            int32 requiredObjId = quest->RequiredNpcOrGo[i];

            data.WriteBits(0, 22);
            data.WriteBits(questObjectiveText[i].size(), 8);

            objData << uint32(0);
            objData.WriteString(questObjectiveText[i]);
            objData << uint8(requiredObjId < 0 ? QUEST_OBJECTIVE_TYPE_GO : QUEST_OBJECTIVE_TYPE_NPC);
            objData << uint8(objCount);
            objData << int32(quest->RequiredNpcOrGoCount[i]);
            objData << uint32(0);
            objData << uint32(requiredObjId < 0 ? requiredObjId * -1 : requiredObjId);

            objCount++;
        }
    }

    for (uint8 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; i++)
    {
        if (quest->RequiredItemId[i] != 0)
        {
            // better way of doing this?
            std::string objText = i < 4 ? questObjectiveText[i] : "";

            data.WriteBits(0, 22);
            data.WriteBits(objText.size(), 8);

            objData << uint32(0);
            objData.WriteString(objText);
            objData << uint8(QUEST_OBJECTIVE_TYPE_ITEM);
            objData << uint8(objCount);
            objData << int32(quest->RequiredItemCount[i]);
            objData << uint32(0);
            objData << uint32(quest->RequiredItemId[i]);

            objCount++;
        }
    }

    for (uint8 i = 0; i < QUEST_REQUIRED_CURRENCY_COUNT; i++)
    {
        if (quest->RequiredCurrencyId[i] != 0)
        {
            data.WriteBits(0, 22);
            data.WriteBits(0, 8);

            objData << uint32(0);
            objData << uint8(QUEST_OBJECTIVE_TYPE_CURRENCY);
            objData << uint8(0xFF);
            objData << int32(quest->RequiredCurrencyCount[i]);
            objData << uint32(0);
            objData << uint32(quest->RequiredCurrencyId[i]);

            objCount++;
        }
    }

    if (quest->GetRequiredSpell() != 0)
    {
        data.WriteBits(0, 22);
        data.WriteBits(questObjectiveText[objCount].size(), 8);

        objData << uint32(0);
        objData.WriteString(questObjectiveText[objCount]);
        objData << uint8(QUEST_OBJECTIVE_TYPE_SPELL);
        objData << uint8(0xFF);
        objData << int32(1);
        objData << uint32(0);
        objData << uint32(quest->GetRequiredSpell());

        objCount++;
    }

    if (quest->GetRepObjectiveFaction() != 0)
    {
        data.WriteBits(0, 22);
        data.WriteBits(0, 8);

        objData << uint32(0);
        objData << uint8(QUEST_OBJECTIVE_TYPE_FACTION_REP);
        objData << uint8(0xFF);
        objData << int32(quest->GetRepObjectiveValue());
        objData << uint32(0);
        objData << uint32(quest->GetRepObjectiveFaction());

        objCount++;
    }

    if (quest->GetRepObjectiveFaction2() != 0)
    {
        data.WriteBits(0, 22);
        data.WriteBits(0, 8);

        objData << uint32(0);
        objData << uint8(QUEST_OBJECTIVE_TYPE_FACTION_REP2);
        objData << uint8(0xFF);
        objData << int32(quest->GetRepObjectiveValue2());
        objData << uint32(0);
        objData << uint32(quest->GetRepObjectiveFaction2());

        objCount++;
    }

    // only seen in quests 13233 (No Mercy!) and 13234 (Make Them Pay!)
    if (quest->GetPlayersSlain() != 0)
    {
        data.WriteBits(0, 22);
        data.WriteBits(0, 8);

        objData << uint32(0);
        objData << uint8(QUEST_OBJECTIVE_TYPE_PLAYER);
        objData << uint8(objCount);
        objData << int32(quest->GetPlayersSlain());
        objData << uint32(0);
        objData << uint32(0x149);

        objCount++;
    }

    if (objCount > 0)
        data.PutBits(objCount, 19, countPos, bitPos);

    bool hiddenReward = quest->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS);

    // values need rechecking and zero values need more research
    data.append(objData);
    data << uint32(hiddenReward ? 0 : quest->RewardItemIdCount[3]);
    data << uint32(quest->RequiredSourceItemCount[0]);
    data << uint32(quest->GetRewSpell());                                   // reward spell, this spell will display (icon) (casted if RewSpellCast == 0)
    data << float(quest->GetPointX());
    data << uint32(quest->GetNextQuestInChain());                           // client will request this quest from NPC, if not 0
    data << uint32(0);                                                      // unknown
    data << float(quest->GetRewHonorMultiplier());
    data << uint32(hiddenReward ? 0 : quest->RewardItemId[3]);
    data << uint32(quest->GetType());                                       // quest type
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemCount[4]);
    data << uint32(hiddenReward ? 0 : quest->RewardItemIdCount[1]);
    data.WriteString(questGiverTargetName);
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemId[3]);
    data << uint32(quest->GetXPId());                                       // used for calculating rewarded experience
    data << uint32(quest->RequiredSourceItemCount[2]);
    data << uint32(quest->RequiredSourceItemId[0]);
    data << int32(quest->GetQuestLevel());                                  // may be -1, static data, in other cases must be used dynamic level: Player::GetQuestLevel (0 is not known, but assuming this is no longer valid for quest intended for client)
    data << uint32(quest->GetRewardSkillId());                              // reward skill id);
    data.WriteString(questCompletedText);
    data.WriteString(questEndText);
    data << uint32(quest->GetSoundAccept());
    data << uint32(quest->GetRewardReputationMask());                       // rep mask (unsure on what it does)
    data << uint32(quest->GetXPId());                                       // seems to always have the same value as the first XP ID field
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemId[0]);
    data << uint32(quest->GetSoundTurnIn());
    data << float(quest->GetPointY());
    data << uint32(quest->GetRewMoneyMaxLevel());                           // used in XP calculation at client
    data << uint32(quest->GetPointOpt());
    data.WriteString(questGiverTextWindow);

    for (uint32 i = 0; i < QUEST_REWARD_CURRENCY_COUNT; i++)
    {
        data << uint32(quest->RewardCurrencyId[i]);
        data << uint32(quest->RewardCurrencyCount[i]);
    }

    data.WriteString(questObjectives);
    data << uint32(quest->RequiredSourceItemId[3]);
    data << uint32(quest->RequiredSourceItemCount[1]);
    data << uint32(quest->GetRewSpellCast());                               // casted spell

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; i++)
    {
        data << uint32(quest->RewardFactionValueId[i]);                     // columnid+1 QuestFactionReward.dbc?
        data << uint32(quest->RewardFactionValueIdOverride[i]);             // unknown usage
        data << uint32(quest->RewardFactionId[i]);                          // reward factions ids
    }

    data.WriteString(questTurnTargetName);
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemId[2]);
    data << uint32(quest->GetQuestGiverPortrait());
    data << uint32(quest->GetBonusTalents());                               // bonus talents
    data << uint32(hiddenReward ? 0 : quest->RewardItemId[0]);
    data << int32(quest->GetZoneOrSort());                                  // zone or sort to display in quest log
    data << uint32(quest->GetRewHonorAddition());                           // rewarded honor points
    data << uint32(0);                                                      // unknown
    data << uint32(hiddenReward ? 0 : quest->GetRewOrReqMoney());           // reward money (below max lvl)
    data << uint32(quest->RequiredSourceItemId[2]);
    data.WriteString(questDetails);
    data << uint32(quest->GetQuestTurnInPortrait());
    data << uint32(quest->GetQuestId());                                    // quest id
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemCount[2]);
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemCount[5]);
    data << uint32(0);                                                      // unknown
    data << uint32(0);                                                      // Flags 2
    data << int32(quest->GetMinLevel());                                    // min level
    data << uint32(quest->RequiredSourceItemId[1]);
    data << uint32(0);                                                      // unknown
    data << uint32(hiddenReward ? 0 : quest->RewardItemId[3]);
    data << uint32(quest->GetCharTitleId());                                // CharTitleId, new 2.4.0, player gets this title (id from CharTitles)
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemId[4]);
    data << uint32(quest->GetSrcItemId());                                  // source item id
    data << uint32(0);                                                      // unknown
    data << uint32(hiddenReward ? 0 : quest->RewardItemIdCount[0]);
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemCount[1]);
    data << uint32(quest->GetSuggestedPlayers());                           // suggested players count
    data << uint32(quest->GetPointMapId());
    data.WriteString(questTitle);
    data << uint32(hiddenReward ? 0 : quest->RewardItemId[2]);
    data << uint32(0);                                                      // unknown
    data << uint32(quest->RequiredSourceItemCount[3]);
    data << uint32(0);                                                      // unknown
    data.WriteString(questTurnTextWindow);
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemId[5]);
    data << uint32(hiddenReward ? 0 : quest->RewardItemId[1]);
    data << uint32(hiddenReward ? 0 : quest->RewardItemIdCount[2]);
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemCount[0]);
    data << uint32(quest->GetFlags() & 0xFFFF);                             // quest flags
    data << uint32(quest->GetRewardSkillPoints());                          // reward skill points
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemId[1]);
    data << uint32(quest->GetQuestMethod());                                // Accepted values: 0, 1 or 2. 0 == IsAutoComplete() (skip objectives/details)
    data << uint32(hiddenReward ? 0 : quest->RewardChoiceItemCount[3]);
    _session->SendPacket(&data);
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_QUEST_QUERY_RESPONSE questid=%u", quest->GetQuestId());
}

// @TODO: Replace unknow uint32 by missing RewardChoiceItemId/RewardChoiceItemCount tab (5 & 6) in packet
void PlayerMenu::SendQuestGiverOfferReward(Quest const* quest, uint64 npcGUID, bool enableNext) const
{
    std::string questTitle = quest->GetTitle();
    std::string questOfferRewardText = quest->GetOfferRewardText();
    std::string questGiverTextWindow = quest->GetQuestGiverTextWindow();
    std::string questGiverTargetName = quest->GetQuestGiverTargetName();
    std::string questTurnTextWindow = quest->GetQuestTurnTextWindow();
    std::string questTurnTargetName = quest->GetQuestTurnTargetName();

    int locale = _session->GetSessionDbLocaleIndex();
    if (locale >= 0)
    {
        if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(quest->GetQuestId()))
        {
            ObjectMgr::GetLocaleString(localeData->Title, locale, questTitle);
            ObjectMgr::GetLocaleString(localeData->OfferRewardText, locale, questOfferRewardText);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTextWindow, locale, questGiverTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestGiverTargetName, locale, questGiverTargetName);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTextWindow, locale, questTurnTextWindow);
            ObjectMgr::GetLocaleString(localeData->QuestTurnTargetName, locale, questTurnTargetName);
        }
    }

    uint32 rewItemDisplayId[QUEST_REWARDS_COUNT];
    uint32 rewChoiceItemDisplayId[QUEST_REWARD_CHOICES_COUNT];

    for (uint8 i = 0; i < QUEST_REWARDS_COUNT; i++)
    {
        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RewardItemId[i]))
            rewItemDisplayId[i] = itemTemplate->DisplayInfoID;
        else
            rewItemDisplayId[i] = 0;
    }

    for (uint8 i = 0; i < QUEST_REWARD_CHOICES_COUNT; i++)
    {
        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RewardChoiceItemId[i]))
            rewChoiceItemDisplayId[i] = itemTemplate->DisplayInfoID;
        else
            rewChoiceItemDisplayId[i] = 0;
    }

    uint32 rewEmoteCount = 0;
    for (uint8 i = 0; i < QUEST_EMOTE_COUNT; i++)
    {
        if (quest->OfferRewardEmote[i] <= 0)
            break;

        rewEmoteCount++;
    }

    float QuestXpRate = 1;
    if (_session->GetPlayer()->GetPersonnalXpRate())
        QuestXpRate = _session->GetPlayer()->GetPersonnalXpRate();
    else
        QuestXpRate = _session->GetPlayer()->GetRate<float>(RateType::XpQuest);

    uint32 RewardChoiceItemId[QUEST_REWARD_CHOICES_COUNT] ;
    uint32 RewardChoiceItemCount[QUEST_REWARD_CHOICES_COUNT];
    for (int i = 0; i < QUEST_REWARD_CHOICES_COUNT; i++)
    {
        RewardChoiceItemId[i] = 0;
        RewardChoiceItemCount[i] = 0;
    }

    uint32 dynamicItemRewardCount = 0;
    if (quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_DYNAMIC_ITEM_REWARD))
    {
        Player* plr = _session->GetPlayer();
        uint32 index = 0;
        for (auto dynamicReward : quest->DynamicRewards)
        {
            ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(dynamicReward.itemID);
            if (!itemTemplate)
                continue;

            // @TODO: Check if we really need to check specialisation id or just player's class
            // (if player doesn't have choosen spec, he doesn't have reward ??)
            //if (itemTemplate->HasSpec() && !itemTemplate->HasSpec(plr->GetSpecializationId(plr->GetActiveSpec())))
            //    continue;

            if (itemTemplate->HasSpec() && !itemTemplate->HasClassSpec(plr->getClass()))
                continue;

            if (index >= QUEST_REWARD_CHOICES_COUNT)
                continue;

            RewardChoiceItemId[index] = dynamicReward.itemID;
            RewardChoiceItemCount[index] = dynamicReward.count;
            index++;
            dynamicItemRewardCount++;
        }
    }
    else
    {
        for (int i = 0; i < QUEST_REWARD_CHOICES_COUNT; i++)
        {
            RewardChoiceItemId[i] = quest->RewardChoiceItemId[i];
            RewardChoiceItemCount[i] = quest->RewardChoiceItemCount[i];
        }
    }

    // TODO: QuestPackageItem.db2 data needs implmenting
    // values need rechecking and zero values need more research

    ObjectGuid guid = npcGUID;

    WorldPacket data(SMSG_QUESTGIVER_OFFER_REWARD, 50);     // guess size
    data.WriteBits(questGiverTextWindow.size(), 10);
    data.WriteBit(guid[6]);
    data.WriteBit(guid[0]);
    data.WriteBits(rewEmoteCount, 21);
    data.WriteBits(questGiverTargetName.size(), 8);
    data.WriteBit(guid[4]);
    data.WriteBits(questTurnTargetName.size(), 8);
    data.WriteBits(questOfferRewardText.size(), 12);
    data.WriteBit(guid[5]);
    data.WriteBit(enableNext);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[2]);
    data.WriteBit(guid[1]);
    data.WriteBits(questTitle.size(), 9);
    data.WriteBits(questTurnTextWindow.size(), 10);
    data.WriteBit(guid[7]);

    data << uint32(quest->RewardItemIdCount[3]);
    data << uint32(quest->GetBonusTalents());
    data << uint32(rewChoiceItemDisplayId[3]);

    for (uint8 i = 0; i < 4; i++)
        data << uint32(0) << uint32(0);

    data.WriteString(questGiverTextWindow);
    data << uint32(RewardChoiceItemCount[1]);
    data << uint32(quest->GetRewOrReqMoney());
    data << uint32(quest->GetRewItemsCount());
    data << uint32(rewItemDisplayId[3]);
    data << uint32(0);                                      // unknown
    data << uint32(rewChoiceItemDisplayId[1]);
    data << uint32(quest->RewardItemIdCount[0]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[2]);
    data << uint32(rewItemDisplayId[2]);

    for (uint8 i = 0; i < QUEST_REPUTATIONS_COUNT; i++)
    {
        data << uint32(quest->RewardFactionId[i]);
        data << uint32(quest->RewardFactionValueId[i]);
        data << uint32(quest->RewardFactionValueIdOverride[i]);
    }

    data << uint32(rewItemDisplayId[0]);
    data << uint32(RewardChoiceItemId[3]);
    data.WriteString(questGiverTargetName);

    if (!quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_DYNAMIC_ITEM_REWARD))
        data << uint32(quest->GetRewChoiceItemsCount());
    else
        data << uint32(dynamicItemRewardCount);

    data << uint32(quest->RewardItemIdCount[1]);
    data.WriteString(questOfferRewardText);
    data << uint32(quest->GetRewSpellCast());
    data << uint32(quest->RewardItemIdCount[2]);
    data << uint32(0);                                      // model ID
    data.WriteString(questTurnTargetName);
    data << uint32(quest->GetFlags());
    data << uint32(RewardChoiceItemCount[4]);
    data << uint32(rewChoiceItemDisplayId[4]);
    data << uint32(quest->RewardItemId[1]);
    data << uint32(RewardChoiceItemId[1]);
    data << uint32(0);                                      // unknown
    data.WriteString(questTurnTextWindow);
    data << uint32(quest->GetCharTitleId());
    data << uint32(RewardChoiceItemCount[5]);
    data << uint32(quest->RewardChoiceItemId[2]);

    for (uint8 i = 0; i < rewEmoteCount; i++)
    {
        data << uint32(quest->OfferRewardEmote[i]);
        data << uint32(quest->OfferRewardEmoteDelay[i]);
    }

    data << uint32(rewChoiceItemDisplayId[5]);
    data.WriteByteSeq(guid[1]);
    data << uint32(RewardChoiceItemId[4]);
    data << uint32(quest->GetQuestId());
    data << uint32(RewardChoiceItemId[0]);
    data << uint32(quest->RewardItemId[3]);
    data << uint32(RewardChoiceItemId[5]);
    data << uint32(rewItemDisplayId[1]);
    data << uint32(0);                                      // unknown
    data << uint32(quest->RewardItemId[2]);
    data << uint32(0);                                      // unknown
    data << uint32(RewardChoiceItemCount[0]);
    data.WriteByteSeq(guid[0]);
    data << uint32(rewChoiceItemDisplayId[2]);
    data.WriteByteSeq(guid[4]);
    data << uint32(RewardChoiceItemCount[3]);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[5]);
    data << uint32(rewChoiceItemDisplayId[0]);
    data << uint32(0);                                      // unknown
    data << uint32(0);                                      // unknown
    data.WriteString(questTitle);
    data << uint32(0);                                      // unknown
    data << uint32(quest->RewardItemId[0]);
    data << uint32(quest->XPValue(_session->GetPlayer()) * QuestXpRate);
    data << uint32(RewardChoiceItemCount[2]);
    data << uint32(0);                                      // unknown
    data << uint32(0);                                      // unknown
    _session->SendPacket(&data);
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD NPCGuid=%u, questid=%u", GUID_LOPART(npcGUID), quest->GetQuestId());
}

void PlayerMenu::SendQuestGiverRequestItems(Quest const* quest, uint64 npcGUID, bool canComplete, bool closeOnCancel) const
{
    // We can always call to RequestItems, but this packet only goes out if there are actually
    // items.  Otherwise, we'll skip straight to the OfferReward

    std::string questTitle = quest->GetTitle();
    std::string requestItemsText = quest->GetRequestItemsText();

    int32 locale = _session->GetSessionDbLocaleIndex();
    if (locale >= 0)
    {
        if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(quest->GetQuestId()))
        {
            ObjectMgr::GetLocaleString(localeData->Title, locale, questTitle);
            ObjectMgr::GetLocaleString(localeData->RequestItemsText, locale, requestItemsText);
        }
    }

    if (!quest->GetReqItemsCount() && canComplete)
    {
        SendQuestGiverOfferReward(quest, npcGUID, true);
        return;
    }

    ObjectGuid guid = npcGUID;

    WorldPacket data(SMSG_QUESTGIVER_REQUEST_ITEMS, 50);    // guess size
    data.WriteBits(questTitle.size(), 9);
    data.WriteBits(requestItemsText.size(), 12);
    data.WriteBit(guid[6]);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[7]);
    data.WriteBits(quest->GetReqItemsCount(), 20);
    data.WriteBits(quest->GetReqCurrencyCount(), 21);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[2]);
    data.WriteBit(guid[1]);
    data.WriteBit(guid[0]);
    data.WriteBit(closeOnCancel);
    data.WriteBit(guid[5]);

    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[5]);

    if (canComplete)
        data << quest->GetCompleteEmote();
    else
        data << quest->GetIncompleteEmote();

    for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; i++)
    {
        if (!quest->RequiredItemId[i])
            continue;

        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RequiredItemId[i]))
            data << uint32(itemTemplate->DisplayInfoID);
        else
            data << uint32(0);

        data << uint32(quest->RequiredItemId[i]);
        data << uint32(quest->RequiredItemCount[i]);
    }

    data.WriteString(requestItemsText);

    for (int i = 0; i < QUEST_REQUIRED_CURRENCY_COUNT; i++)
    {
        if (!quest->RequiredCurrencyId[i])
            continue;

        data << uint32(quest->RequiredCurrencyCount[i]);
        data << uint32(quest->RequiredCurrencyId[i]);
    }

    data.WriteByteSeq(guid[7]);
    data << uint32(0);
    data.WriteString(questTitle);
    data << uint32(quest->GetRewOrReqMoney() < 0 ? -quest->GetRewOrReqMoney() : 0);
    data << uint32(quest->GetQuestId());
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[4]);
    data << uint32(0);
    data << uint32(0);
    data << uint32(canComplete ? 0x5F : 0x00);      // unknown (also seen 5B), if not 5F continue button is greyed out
    data.WriteByteSeq(guid[6]);
    data << uint32(0);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[1]);
    data << uint32(quest->GetFlags());

    /*

    data << uint64(npcGUID);
    data << uint32(quest->GetQuestId());
    data << questTitle;
    data << requestItemsText;



    data << uint32(0);                                   // unknown

    if (canComplete)
        data << quest->GetCompleteEmote();
    else
        data << quest->GetIncompleteEmote();

    // Close Window after cancel
    data << uint32(closeOnCancel);

    data << uint32(quest->GetFlags());                      // 3.3.3 questFlags
    data << uint32(quest->GetSuggestedPlayers());           // SuggestedGroupNum

    // Required Money
    data << uint32(quest->GetRewOrReqMoney() < 0 ? -quest->GetRewOrReqMoney() : 0);
    data << uint32(0);
    data << uint32(quest->GetReqItemsCount());
    for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
    {
        if (!quest->RequiredItemId[i])
            continue;

        data << uint32(quest->RequiredItemId[i]);
        data << uint32(quest->RequiredItemCount[i]);

        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RequiredItemId[i]))
            data << uint32(itemTemplate->DisplayInfoID);
        else
            data << uint32(0);
    }

    data << uint32(quest->GetReqCurrencyCount());
    for (int i = 0; i < QUEST_REQUIRED_CURRENCY_COUNT; ++i)
    {
        if (!quest->RequiredCurrencyId[i])
            continue;

        data << uint32(quest->RequiredCurrencyId[i]);
        data << uint32(quest->RequiredCurrencyCount[i]);
    }

    if (!canComplete)            // Experimental; there are 6 similar flags, if any of them
        data << uint32(0x00);    // of them is 0 player can't complete quest (still unknown meaning)
    else
        data << uint32(0x02);

    data << uint32(0x04);
    data << uint32(0x08);
    data << uint32(0x10);
    data << uint32(0x40);*/

    _session->SendPacket(&data);
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS NPCGuid=%u, questid=%u", GUID_LOPART(npcGUID), quest->GetQuestId());
}
