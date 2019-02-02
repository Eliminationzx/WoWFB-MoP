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
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "GossipDef.h"
#include "QuestDef.h"
#include "ObjectAccessor.h"
#include "Group.h"
#include "Battleground.h"
#include "BattlegroundAV.h"
#include "ScriptMgr.h"
#include "GameObjectAI.h"

void WorldSession::HandleQuestgiverStatusQueryOpcode(WorldPacket & recvData)
{
    ObjectGuid guid;

    uint8 bitOrder[8] = {2, 7, 3, 1, 6, 0, 4, 5};
    recvData.ReadBitInOrder(guid, bitOrder);

    uint8 byteOrder[8] = {2, 3, 6, 5, 4, 1, 0, 7};
    recvData.ReadBytesSeq(guid, byteOrder);

    uint32 questStatus = DIALOG_STATUS_NONE;
    uint32 defstatus = DIALOG_STATUS_NONE;

    Object* questgiver = ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);
    if (!questgiver)
    {
        sLog->outInfo(LOG_FILTER_NETWORKIO, "Error in CMSG_QUESTGIVER_STATUS_QUERY, called for not found questgiver (Typeid: %u GUID: %u)", GuidHigh2TypeId(GUID_HIPART(guid)), GUID_LOPART(guid));
        return;
    }

    switch (questgiver->GetTypeId())
    {
        case TYPEID_UNIT:
        {
            sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUESTGIVER_STATUS_QUERY for npc, guid = %u", uint32(GUID_LOPART(guid)));
            Creature* cr_questgiver=questgiver->ToCreature();
            if (!cr_questgiver->IsHostileTo(_player))       // do not show quest status to enemies
            {
                questStatus = sScriptMgr->GetDialogStatus(_player, cr_questgiver);
                if (questStatus > 6)
                    questStatus = getDialogStatus(_player, cr_questgiver, defstatus);
            }
            break;
        }
        case TYPEID_GAMEOBJECT:
        {
            sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUESTGIVER_STATUS_QUERY for GameObject guid = %u", uint32(GUID_LOPART(guid)));
            GameObject* go_questgiver=(GameObject*)questgiver;
            questStatus = sScriptMgr->GetDialogStatus(_player, go_questgiver);
            if (questStatus > 6)
                questStatus = getDialogStatus(_player, go_questgiver, defstatus);
            break;
        }
        default:
            sLog->outError(LOG_FILTER_NETWORKIO, "QuestGiver called for unexpected type %u", questgiver->GetTypeId());
            break;
    }

    //inform client about status of quest
    _player->PlayerTalkClass->SendQuestGiverStatus(questStatus, guid);
}

void WorldSession::HandleQuestgiverHelloOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    uint8 bitOrder[8] = {2, 3, 5, 4, 7, 6, 0, 1};
    recvData.ReadBitInOrder(guid, bitOrder);

    uint8 byteOrder[8] = {7, 1, 2, 6, 3, 5, 0, 4};
    recvData.ReadBytesSeq(guid, byteOrder);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUESTGIVER_HELLO npc = %u", GUID_LOPART(guid));

    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
    if (!creature)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleQuestgiverHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.",
            GUID_LOPART(guid));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);
    // Stop the npc if moving
    creature->StopMoving();

    if (sScriptMgr->OnGossipHello(_player, creature))
        return;

    _player->PrepareGossipMenu(creature, creature->GetCreatureTemplate()->GossipMenuId, true);
    _player->SendPreparedGossip(creature);

    creature->AI()->sGossipHello(_player);
}

void WorldSession::HandleQuestgiverAcceptQuestOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;
    uint32 questId;
    uint8 unk1;

    recvData >> questId;

    guid[3] = recvData.ReadBit();
    guid[2] = recvData.ReadBit();
    guid[7] = recvData.ReadBit();
    unk1 = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    guid[6] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    guid[4] = recvData.ReadBit();

    uint8 byteOrder[8] = {3, 4, 7, 2, 5, 1, 6, 0};
    recvData.ReadBytesSeq(guid, byteOrder);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUESTGIVER_ACCEPT_QUEST npc = %u, quest = %u, unk1 = %u", uint32(GUID_LOPART(guid)), questId, unk1);

    Object* object = ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT|TYPEMASK_ITEM|TYPEMASK_PLAYER);

    // no or incorrect quest giver
    if (!object || (object->GetTypeId() != TYPEID_PLAYER && !object->hasQuest(questId)) ||
        (object->GetTypeId() == TYPEID_PLAYER && object != _player && !object->ToPlayer()->CanShareQuest(questId)))
    {
        _player->PlayerTalkClass->SendCloseGossip();
        _player->SetDivider(0);
        return;
    }

    //if (object && object->GetTypeId() == TYPEID_PLAYER && object->ToPlayer()->GetQuestStatus(questId) == QUEST_STATUS_NONE)
        //return;

    // some kind of WPE protection
    if (!_player->CanInteractWithQuestGiver(object))
        return;

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
    {
        // prevent cheating
        if (!GetPlayer()->CanTakeQuest(quest, true))
        {
            _player->PlayerTalkClass->SendCloseGossip();
            _player->SetDivider(0);
            return;
        }

        if (object && object->GetTypeId() == TYPEID_PLAYER && !quest->HasFlag(QUEST_FLAGS_SHARABLE) && !quest->HasFlag(QUEST_FLAGS_AUTO_TAKE))
            return;

        if (_player->GetDivider() != 0)
        {
            Player* player = ObjectAccessor::FindPlayer(_player->GetDivider());
            if (player)
            {
                player->SendPushToPartyResponse(_player, QUEST_PARTY_MSG_ACCEPT_QUEST);
                _player->SetDivider(0);
            }
        }

        if (_player->CanAddQuest(quest, true))
        {
            _player->AddQuest(quest, object);

            if (quest->HasFlag(QUEST_FLAGS_PARTY_ACCEPT))
            {
                if (Group* group = _player->GetGroup())
                {
                    for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
                    {
                        Player* player = itr->getSource();

                        if (!player || player == _player)     // not self
                            continue;

                        if (player->CanTakeQuest(quest, true))
                        {
                            player->SetDivider(_player->GetGUID());

                            //need confirmation that any gossip window will close
                            player->PlayerTalkClass->SendCloseGossip();

                            _player->SendQuestConfirmAccept(quest, player);
                        }
                    }
                }
            }


            if (_player->CanCompleteQuest(questId))
                _player->CompleteQuest(questId);

            switch (object->GetTypeId())
            {
                case TYPEID_UNIT:
                    sScriptMgr->OnQuestAccept(_player, (object->ToCreature()), quest);
                    (object->ToCreature())->AI()->sQuestAccept(_player, quest);
                    break;
                case TYPEID_ITEM:
                case TYPEID_CONTAINER:
                {
                    sScriptMgr->OnQuestAccept(_player, ((Item*)object), quest);

                    // destroy not required for quest finish quest starting item
                    bool destroyItem = true;
                    for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
                    {
                        if ((quest->RequiredItemId[i] == ((Item*)object)->GetEntry()) && (((Item*)object)->GetTemplate()->MaxCount > 0))
                        {
                            destroyItem = false;
                            break;
                        }
                    }

                    if (destroyItem)
                        _player->DestroyItem(((Item*)object)->GetBagSlot(), ((Item*)object)->GetSlot(), true);

                    break;
                }
                case TYPEID_GAMEOBJECT:
                    sScriptMgr->OnQuestAccept(_player, ((GameObject*)object), quest);
                    (object->ToGameObject())->AI()->QuestAccept(_player, quest);
                    break;
                default:
                    break;
            }
            _player->PlayerTalkClass->SendCloseGossip();

            if (quest->GetSrcSpell() > 0)
                _player->CastSpell(_player, quest->GetSrcSpell(), true);

            if (quest->IsAutoComplete())
            {
                // Add quest items for quests that require items
                for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
                {
                    uint32 id = quest->RequiredItemId[x];
                    uint32 count = quest->RequiredItemCount[x];
                    if (!id || !count)
                        continue;

                    uint32 curItemCount = _player->GetItemCount(id, true);

                    ItemPosCountVec dest;
                    uint8 msg = _player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count-curItemCount);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = _player->StoreNewItem(dest, id, true);
                        _player->SendNewItem(item, count-curItemCount, true, false);
                    }
                }

                // All creature/GO slain/casted (not required, but otherwise it will display "Creature slain 0/10")
                for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
                {
                    int32 creature = quest->RequiredNpcOrGo[i];
                    uint32 creaturecount = quest->RequiredNpcOrGoCount[i];

                    if (uint32 spell_id = quest->RequiredSpellCast[i])
                    {
                        for (uint16 z = 0; z < creaturecount; ++z)
                            if (creature > 0)
                                _player->CastedCreatureOrGOForQuest(creature, true, spell_id);
                            else
                                _player->CastedCreatureOrGOForQuest(creature, false, spell_id);
                    }
                    else if (creature > 0)
                    {
                        if (CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(creature))
                            for (uint16 z = 0; z < creaturecount; ++z)
                                _player->KilledMonster(cInfo, 0);
                    }
                    else if (creature < 0)
                    {
                        for (uint16 z = 0; z < creaturecount; ++z)
                            _player->CastedCreatureOrGO(creature, 0, 0);
                    }
                }

                // If the quest requires currency to complete
                for (uint8 y = 0; y < QUEST_REQUIRED_CURRENCY_COUNT; y++)
                {
                    uint32 currency = quest->RequiredCurrencyId[y];
                    uint32 currencyCount = quest->RequiredCurrencyCount[y];

                    if (!currency || !currencyCount)
                        continue;

                    _player->ModifyCurrency(currency, currencyCount);
                }

                // If the quest requires reputation to complete
                if (uint32 repFaction = quest->GetRepObjectiveFaction())
                {
                    uint32 repValue = quest->GetRepObjectiveValue();
                    uint32 curRep = _player->GetReputationMgr().GetReputation(repFaction);
                    if (curRep < repValue)
                        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                            _player->GetReputationMgr().SetReputation(factionEntry, repValue);
                }

                // If the quest requires a SECOND reputation to complete
                if (uint32 repFaction = quest->GetRepObjectiveFaction2())
                {
                    uint32 repValue2 = quest->GetRepObjectiveValue2();
                    uint32 curRep = _player->GetReputationMgr().GetReputation(repFaction);
                    if (curRep < repValue2)
                        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                            _player->GetReputationMgr().SetReputation(factionEntry, repValue2);
                }

                // If the quest requires money
                int32 ReqOrRewMoney = quest->GetRewOrReqMoney();
                if (ReqOrRewMoney < 0)
                    _player->ModifyMoney(-ReqOrRewMoney);

                _player->CompleteQuest(quest->GetQuestId());
            }

            return;
        }
    }

    _player->PlayerTalkClass->SendCloseGossip();
}

void WorldSession::HandleQuestgiverQueryQuestOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;
    uint32 questId;
    bool unk1;

    recvData >> questId;
    guid[6] = recvData.ReadBit();
    guid[3] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    unk1 = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid[4] = recvData.ReadBit();
    guid[2] = recvData.ReadBit();
    guid[7] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();

    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[7]);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUESTGIVER_QUERY_QUEST npc = %u, quest = %u, unk1 = %u", uint32(GUID_LOPART(guid)), questId, unk1);

    // Verify that the guid is valid and is a questgiver or involved in the requested quest
    Object* object = ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT | TYPEMASK_ITEM);
    if (!object || (!object->hasQuest(questId) && !object->hasInvolvedQuest(questId)))
    {
        _player->PlayerTalkClass->SendCloseGossip();
        return;
    }

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
    {
        // not sure here what should happen to quests with QUEST_FLAGS_AUTOCOMPLETE
        // if this breaks them, add && object->GetTypeId() == TYPEID_ITEM to this check
        // item-started quests never have that flag
        if (_player->GetQuestStatus(questId) == QUEST_STATUS_NONE && !_player->CanTakeQuest(quest, true))
            return;

        if ( _player->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
            _player->PlayerTalkClass->SendQuestGiverOfferReward(quest, object->GetGUID(), true);
        else if (_player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
            _player->PlayerTalkClass->SendQuestGiverRequestItems(quest, object->GetGUID(), _player->CanCompleteQuest(quest->GetQuestId()), true);
        else
        {
            if (quest->IsAutoAccept())
            {
                if (Creature* pQuestGiver = ObjectAccessor::GetCreature(*_player, guid))
                    if (pQuestGiver->IsAIEnabled)
                        sScriptMgr->OnQuestAccept(_player, pQuestGiver, quest);

                _player->AddQuest(quest, object);
                if (_player->CanCompleteQuest(questId))
                    _player->CompleteQuest(questId);
            }
            _player->PlayerTalkClass->SendQuestGiverQuestDetails(quest, object->GetGUID(), true);
        }
    }
}

void WorldSession::HandleQuestQueryOpcode(WorldPacket & recvData)
{
    if (!_player)
        return;

    uint32 questId;
    recvData >> questId;
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUEST_QUERY quest = %u", questId);

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
        _player->PlayerTalkClass->SendQuestQueryResponse(quest);
}

void WorldSession::HandleQuestgiverChooseRewardOpcode(WorldPacket& recvData)
{
    uint32 questId, reward = 0;
    uint32 slot = 0;
    ObjectGuid guid;
    recvData >> slot >> questId;

    uint8 bitOrder[8] = {2, 0, 7, 5, 6, 4, 1, 3};
    recvData.ReadBitInOrder(guid, bitOrder);
    uint8 byteOrder[8] = {7, 2, 4, 5, 3, 6, 1, 0}; 
    recvData.ReadBytesSeq(guid, byteOrder);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUESTGIVER_CHOOSE_REWARD npc = %u, quest = %u, reward = %u", uint32(GUID_LOPART(guid)), questId, reward);

    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest)
        return;

    Object* object = _player;

    if (!quest->HasFlag(QUEST_FLAGS_AUTO_SUBMIT))
    {
        //TODO: Doing something less dirty
        for (int i = 0; i < QUEST_REWARD_CHOICES_COUNT; i++)
            if (quest->RewardChoiceItemId[i] == slot)
                reward = i;

        if (quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_DYNAMIC_ITEM_REWARD))
        {
            uint32 index = 0;
            for (auto itr : quest->DynamicRewards)
            {
                ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itr.itemID);
                if (!itemTemplate)
                    continue;

                // @TODO: Check if we really need to check specialisation id or just player's class
                // (if player doesn't have choosen spec, he doesn't have reward ??)
                //if (itemTemplate->HasSpec() && !itemTemplate->HasSpec(plr->GetSpecializationId(plr->GetActiveSpec())))
                //    continue;

                if (itemTemplate->HasSpec() && !itemTemplate->HasClassSpec(_player->getClass()))
                    continue;

                if (itr.itemID == slot)
                {
                    reward = index;
                    break;
                }

                index++;
            }
        }

        if (reward >= QUEST_REWARD_CHOICES_COUNT)
        {
            sLog->outError(LOG_FILTER_NETWORKIO, "Error in CMSG_QUESTGIVER_CHOOSE_REWARD: player %s (guid %d) tried to get invalid reward (%u) (probably packet hacking)", _player->GetName(), _player->GetGUIDLow(), reward);
            return;
        }
        object = ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);
        if (!object || !object->hasInvolvedQuest(questId))
            return;

        // some kind of WPE protection
        if (!_player->CanInteractWithQuestGiver(object))
            return;

    }
    if ((!_player->CanSeeStartQuest(quest) &&  _player->GetQuestStatus(questId) == QUEST_STATUS_NONE) ||
        (_player->GetQuestStatus(questId) != QUEST_STATUS_COMPLETE && !quest->IsAutoComplete()))
    {
        sLog->outError(LOG_FILTER_NETWORKIO, "HACK ALERT: Player %s (guid: %u) is trying to complete quest (id: %u) but he has no right to do it!",
            _player->GetName(), _player->GetGUIDLow(), questId);
        return;
    }

    if (_player->CanRewardQuest(quest, reward, true))
    { 
        _player->RewardQuest(quest, reward, object);

        switch (object->GetTypeId())
        { 
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
            {
                //For AutoSubmition was added plr case there as it almost same exclude AI script cases.
                Creature *creatureQGiver = object->ToCreature();
                if (!creatureQGiver || !(sScriptMgr->OnQuestReward(_player, creatureQGiver, quest, reward)))
                {
                    // Send next quest
                    if (Quest const* nextQuest = _player->GetNextQuest(guid, quest))
                    {
                        if (nextQuest->IsAutoAccept() && _player->CanAddQuest(nextQuest, true) && _player->CanTakeQuest(nextQuest, true))
                        {
                            _player->AddQuest(nextQuest, object);
                            if (_player->CanCompleteQuest(nextQuest->GetQuestId()))
                                _player->CompleteQuest(nextQuest->GetQuestId());
                        }

                        _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextQuest, guid, true);
                    }
                    if (creatureQGiver)
                        creatureQGiver->AI()->sQuestReward(_player, quest, reward);
                }
                break;
            }
        case TYPEID_GAMEOBJECT:
            if (!sScriptMgr->OnQuestReward(_player, ((GameObject*)object), quest, reward))
            {
                // Send next quest
                if (Quest const* nextQuest = _player->GetNextQuest(guid, quest))
                {
                    if (nextQuest->IsAutoAccept() && _player->CanAddQuest(nextQuest, true) && _player->CanTakeQuest(nextQuest, true))
                    {
                        _player->AddQuest(nextQuest, object);
                        if (_player->CanCompleteQuest(nextQuest->GetQuestId()))
                            _player->CompleteQuest(nextQuest->GetQuestId());
                    }

                    _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextQuest, guid, true);
                }

                object->ToGameObject()->AI()->QuestReward(_player, quest, reward);
            }
            break;
        default:
            break;
        }
    }
    else 
        _player->PlayerTalkClass->SendQuestGiverOfferReward(quest, guid, true);
}

void WorldSession::HandleQuestgiverRequestRewardOpcode(WorldPacket & recvData)
{
    uint32 questId;
    ObjectGuid guid;

    recvData >> questId;
    
    uint8 bitOrder[8] = {4, 1, 7, 0, 3, 2, 6, 5};
    uint8 byteOrder[8] = {7, 2, 6, 4, 3, 1, 5, 0};
    recvData.ReadBitInOrder(guid, bitOrder);
    recvData.ReadBytesSeq(guid, byteOrder);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUESTGIVER_REQUEST_REWARD npc = %u, quest = %u", uint32(GUID_LOPART(guid)), questId);

    Object* object = ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);
    if (!object || !object->hasInvolvedQuest(questId))
        return;

    // some kind of WPE protection
    if (!_player->CanInteractWithQuestGiver(object))
        return;

    if (_player->CanCompleteQuest(questId))
        _player->CompleteQuest(questId);

    if (_player->GetQuestStatus(questId) != QUEST_STATUS_COMPLETE)
        return;

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
        _player->PlayerTalkClass->SendQuestGiverOfferReward(quest, guid, true);
}

void WorldSession::HandleQuestLogRemoveQuest(WorldPacket& recvData)
{
    uint8 slot;
    recvData >> slot;

    enum QuestIds
    {
        QuestATestOfValorAlliance = 32474,
        QuestATestOfValorHorde = 32476
    };

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUESTLOG_REMOVE_QUEST slot = %u", slot);

    if (slot < MAX_QUEST_LOG_SIZE)
    {
        if (uint32 questId = _player->GetQuestSlotQuestId(slot))
        {
            if (!_player->TakeQuestSourceItem(questId, true))
                return;                                     // can't un-equip some items, reject quest cancel

            if (const Quest *quest = sObjectMgr->GetQuestTemplate(questId))
            {
                if (quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED))
                    _player->RemoveTimedQuest(questId);
            }

            _player->TakeQuestSourceItem(questId, true); // remove quest src item from player
            _player->RemoveActiveQuest(questId);
            _player->GetAchievementMgr().RemoveTimedAchievement(ACHIEVEMENT_TIMED_TYPE_QUEST, questId);

            sLog->outInfo(LOG_FILTER_NETWORKIO, "Player %u abandoned quest %u", _player->GetGUIDLow(), questId);

            /// Handle currencies for quest
            {
                if (questId == QuestIds::QuestATestOfValorHorde || questId == QuestIds::QuestATestOfValorAlliance)
                {
                    PreparedStatement* l_Stmt = CharacterDatabase.GetPreparedStatement(CHAR_RESET_QUEST_CURRENCY);

                    if (_player->m_PlayerQuestCurrency.find(_player->GetGUIDLow()) != _player->m_PlayerQuestCurrency.end())
                    {
                        auto l_QuestCurrenciesCount = _player->m_PlayerQuestCurrency[_player->GetGUIDLow()];

                        l_Stmt->setUInt32(0, _player->GetGUIDLow());
                        l_Stmt->setUInt32(1, CurrencyTypes::CURRENCY_TYPE_VALOR_POINTS);
                    }

                    CharacterDatabase.Execute(l_Stmt);
                }
            }
        }

        _player->SetQuestSlot(slot, 0);

        _player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED, 1);
    }
}

void WorldSession::HandleQuestConfirmAccept(WorldPacket& recvData)
{
    uint32 questId;
    recvData >> questId;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUEST_CONFIRM_ACCEPT questId = %u", questId);

    if (const Quest* quest = sObjectMgr->GetQuestTemplate(questId))
    {
        if (!quest->HasFlag(QUEST_FLAGS_PARTY_ACCEPT))
            return;

        Player* pOriginalPlayer = ObjectAccessor::FindPlayer(_player->GetDivider());

        if (!pOriginalPlayer)
            return;

        if (quest->IsRaidQuest())
        {
            if (!_player->IsInSameRaidWith(pOriginalPlayer))
                return;
        }
        else
        {
            if (!_player->IsInSameGroupWith(pOriginalPlayer))
                return;
        }

        if (_player->CanAddQuest(quest, true))
            _player->AddQuest(quest, NULL);                // NULL, this prevent DB script from duplicate running

        _player->SetDivider(0);
    }
}

void WorldSession::HandleQuestgiverCompleteQuest(WorldPacket& recvData)
{
    uint32 questId;
    ObjectGuid playerGuid;
    bool autoCompleteMode;      // 0 - standard complete quest mode with npc, 1 - auto-complete mode
    recvData >> questId;

    playerGuid[6] = recvData.ReadBit();
    playerGuid[5] = recvData.ReadBit();
    playerGuid[7] = recvData.ReadBit();
    playerGuid[4] = recvData.ReadBit();
    playerGuid[3] = recvData.ReadBit();
    playerGuid[0] = recvData.ReadBit();
    autoCompleteMode = recvData.ReadBit();
    playerGuid[1] = recvData.ReadBit();
    playerGuid[2] = recvData.ReadBit();

    uint8 byteOrder[8] = {0, 5, 3, 2, 4, 6, 1, 7};
    recvData.ReadBytesSeq(playerGuid, byteOrder);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUESTGIVER_COMPLETE_QUEST npc = %u, questId = %u", uint32(GUID_LOPART(playerGuid)), questId);

    if (autoCompleteMode == 0)
    {
        Object* object = ObjectAccessor::GetObjectByTypeMask(*_player, playerGuid, TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);
        if (!object || !object->hasInvolvedQuest(questId))
            return;

    // some kind of WPE protection
    if (!_player->CanInteractWithQuestGiver(object))
        return;
    }

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
    {
        if (autoCompleteMode && !quest->HasFlag(QUEST_FLAGS_AUTOCOMPLETE))
        {
            sLog->outError(LOG_FILTER_NETWORKIO, "Possible hacking attempt: Player %s [playerGuid: %u] tried to complete questId [entry: %u] by auto-submit flag for quest witch not suport it.",
                _player->GetName(), _player->GetGUIDLow(), questId);
            return;
        }
        if (!_player->CanSeeStartQuest(quest) && _player->GetQuestStatus(questId) == QUEST_STATUS_NONE)
        {
            sLog->outError(LOG_FILTER_NETWORKIO, "Possible hacking attempt: Player %s [playerGuid: %u] tried to complete questId [entry: %u] without being in possession of the questId!",
                          _player->GetName(), _player->GetGUIDLow(), questId);
            return;
        }
        // TODO: need a virtual function
        if (_player->InBattleground())
            if (Battleground* bg = _player->GetBattleground())
                if (bg->GetTypeID() == BATTLEGROUND_AV)
                    ((BattlegroundAV*)bg)->HandleQuestComplete(questId, _player);

        if (_player->GetQuestStatus(questId) != QUEST_STATUS_COMPLETE)
        {
            if (quest->IsRepeatable())
                _player->PlayerTalkClass->SendQuestGiverRequestItems(quest, playerGuid, _player->CanCompleteRepeatableQuest(quest), false);
            else
                _player->PlayerTalkClass->SendQuestGiverRequestItems(quest, playerGuid, _player->CanRewardQuest(quest, false), false);
        }
        else
        {
            if (quest->GetReqItemsCount())                  // some items required
                _player->PlayerTalkClass->SendQuestGiverRequestItems(quest, playerGuid, _player->CanRewardQuest(quest, false), false);
            else                                            // no items required
            {
                _player->PlayerTalkClass->SendQuestGiverOfferReward(quest, playerGuid, !autoCompleteMode);
                _player->SaveToDB();
            }
        }
    }
}

void WorldSession::HandlePushQuestToParty(WorldPacket& recvPacket)
{
    uint32 questId;
    recvPacket >> questId;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_PUSHQUESTTOPARTY questId = %u", questId);

    if (_player->GetQuestStatus(questId) == QUEST_STATUS_NONE || _player->GetQuestStatus(questId) == QUEST_STATUS_REWARDED)
        return;

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
    {
        if (Group* group = _player->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* player = itr->getSource();

                if (!player || player == _player)         // skip self
                    continue;

                _player->SendPushToPartyResponse(player, QUEST_PARTY_MSG_SHARING_QUEST);

                if (!player->SatisfyQuestStatus(quest, false))
                {
                    _player->SendPushToPartyResponse(player, QUEST_PARTY_MSG_HAVE_QUEST);
                    continue;
                }

                if (player->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
                {
                    _player->SendPushToPartyResponse(player, QUEST_PARTY_MSG_FINISH_QUEST);
                    continue;
                }

                if (!player->CanTakeQuest(quest, false))
                {
                    _player->SendPushToPartyResponse(player, QUEST_PARTY_MSG_CANT_TAKE_QUEST);
                    continue;
                }

                if (!player->SatisfyQuestLog(false))
                {
                    _player->SendPushToPartyResponse(player, QUEST_PARTY_MSG_LOG_FULL);
                    continue;
                }

                if (player->GetDivider() != 0)
                {
                    _player->SendPushToPartyResponse(player, QUEST_PARTY_MSG_BUSY);
                    continue;
                }

                player->PlayerTalkClass->SendQuestGiverQuestDetails(quest, _player->GetGUID(), true);
                player->SetDivider(_player->GetGUID());
            }
        }
    }
}

void WorldSession::HandleQuestPushResult(WorldPacket& recvPacket)
{
    uint64 guid;
    uint8 msg;
    recvPacket >> guid >> msg;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received SMSG_QUEST_PUSH_RESULT");

    if (_player->GetDivider() != 0)
    {
        Player* player = ObjectAccessor::FindPlayer(_player->GetDivider());
        if (player)
        {
            WorldPacket data(SMSG_QUEST_PUSH_RESULT, (8+1));
            ObjectGuid guidObj = player->GetGUID();
            uint8 bitOrder[8] = {1, 0, 6, 5, 7, 4, 3, 2};
            uint8 byteOrder[8] = {7, 5, 1, 6, 3, 2, 4, 0};
            data.WriteBitInOrder(guidObj, bitOrder);
            data.WriteBytesSeq(guidObj, byteOrder);
            data << uint8(msg); 
            player->GetSession()->SendPacket(&data);
            _player->SetDivider(0);
        }
    }
}

uint32 WorldSession::getDialogStatus(Player* player, Object* questgiver, uint32 defstatus)
{
    uint32 result = defstatus;

    QuestRelationBounds qr;
    QuestRelationBounds qir;

    switch (questgiver->GetTypeId())
    {
        case TYPEID_GAMEOBJECT:
        {
            qr  = sObjectMgr->GetGOQuestRelationBounds(questgiver->GetEntry());
            qir = sObjectMgr->GetGOQuestInvolvedRelationBounds(questgiver->GetEntry());
            break;
        }
        case TYPEID_UNIT:
        {
            qr  = sObjectMgr->GetCreatureQuestRelationBounds(questgiver->GetEntry());
            qir = sObjectMgr->GetCreatureQuestInvolvedRelationBounds(questgiver->GetEntry());
            break;
        }
        default:
            //its imposible, but check ^)
            sLog->outError(LOG_FILTER_NETWORKIO, "Warning: GetDialogStatus called for unexpected type %u", questgiver->GetTypeId());
            return DIALOG_STATUS_NONE;
    }

    for (QuestRelations::const_iterator i = qir.first; i != qir.second; ++i)
    {
        uint32 result2 = 0;
        uint32 quest_id = i->second;
        Quest const* quest = sObjectMgr->GetQuestTemplate(quest_id);
        if (!quest)
            continue;

        if (!sConditionMgr->IsObjectMeetingNotGroupedConditions(CONDITION_SOURCE_TYPE_QUEST_SHOW_MARK, quest->GetQuestId(), player))
            continue;

        QuestStatus status = player->GetQuestStatus(quest_id);
        if ((status == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(quest_id)) ||
            (quest->IsAutoComplete() && player->CanTakeQuest(quest, false)))
        {
            if (quest->IsAutoComplete() && quest->IsRepeatable())
                result2 = DIALOG_STATUS_REWARD_REP;
            else
                result2 = DIALOG_STATUS_REWARD;
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
            result2 = DIALOG_STATUS_INCOMPLETE;

        if (result2 > result)
            result = result2;
    }

    for (QuestRelations::const_iterator i = qr.first; i != qr.second; ++i)
    {
        uint32 result2 = 0;
        uint32 quest_id = i->second;
        Quest const* quest = sObjectMgr->GetQuestTemplate(quest_id);
        if (!quest)
            continue;

        if (!sConditionMgr->IsObjectMeetingNotGroupedConditions(CONDITION_SOURCE_TYPE_QUEST_SHOW_MARK, quest->GetQuestId(), player))
            continue;

        QuestStatus status = player->GetQuestStatus(quest_id);
        if (status == QUEST_STATUS_NONE)
        {
            if (player->CanSeeStartQuest(quest))
            {
                if (player->SatisfyQuestLevel(quest, false))
                {
                    if (quest->IsAutoComplete() || (quest->IsRepeatable() && player->IsQuestRewarded(quest_id)))
                        result2 = DIALOG_STATUS_REWARD_REP;
                    else if (player->getLevel() <= ((player->GetQuestLevel(quest) == -1) ? player->getLevel() : player->GetQuestLevel(quest) + sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF)))
                    {
                        if (quest->HasFlag(QUEST_FLAGS_DAILY) || quest->HasFlag(QUEST_FLAGS_WEEKLY))
                            result2 = DIALOG_STATUS_AVAILABLE_REP;
                        else
                            result2 = DIALOG_STATUS_AVAILABLE;
                    }
                    else
                        result2 = DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
                }
                else
                    result2 = DIALOG_STATUS_UNAVAILABLE;
            }
        }

        if (result2 > result)
            result = result2;
    }

    return result;
}

void WorldSession::HandleQuestgiverStatusMultipleQuery(WorldPacket& /*recvPacket*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY");

    uint32 count = 0;

    ByteBuffer byteData;
    WorldPacket data(SMSG_QUESTGIVER_STATUS_MULTIPLE, 3 + count * (1 + 8 + 4));
    
    auto wpos = data.wpos();
    auto bpos = data.bitpos();
    data.WriteBits(0, 21);

    for (Player::ClientGUIDs::const_iterator itr = _player->m_clientGUIDs.begin(); itr != _player->m_clientGUIDs.end(); ++itr)
    {
        uint32 questStatus = DIALOG_STATUS_NONE;
        uint32 defstatus = DIALOG_STATUS_NONE;

        if (IS_CRE_OR_VEH_OR_PET_GUID(*itr))
        {
            // need also pet quests case support
            Creature* questgiver = ObjectAccessor::GetCreatureOrPetOrVehicle(*GetPlayer(), *itr);
            if (!questgiver || questgiver->IsHostileTo(_player))
                continue;
            if (!questgiver->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
                continue;
            questStatus = sScriptMgr->GetDialogStatus(_player, questgiver);
            if (questStatus > 6)
                questStatus = getDialogStatus(_player, questgiver, defstatus);

            ObjectGuid guid = questgiver->GetGUID();

            data.WriteBit(guid[7]);
            data.WriteBit(guid[0]);
            data.WriteBit(guid[6]);
            data.WriteBit(guid[2]);
            data.WriteBit(guid[5]);
            data.WriteBit(guid[1]);
            data.WriteBit(guid[4]);
            data.WriteBit(guid[3]);

            byteData.WriteByteSeq(guid[5]);
            byteData << uint32(questStatus);
            byteData.WriteByteSeq(guid[4]);
            byteData.WriteByteSeq(guid[2]);
            byteData.WriteByteSeq(guid[3]);
            byteData.WriteByteSeq(guid[6]);
            byteData.WriteByteSeq(guid[1]);
            byteData.WriteByteSeq(guid[7]);
            byteData.WriteByteSeq(guid[0]);

            ++count;

        }
        else if (IS_GAMEOBJECT_GUID(*itr))
        {
            GameObject* questgiver = GetPlayer()->GetMap()->GetGameObject(*itr);
            if (!questgiver)
                continue;
            if (questgiver->GetGoType() != GAMEOBJECT_TYPE_QUESTGIVER)
                continue;
            questStatus = sScriptMgr->GetDialogStatus(_player, questgiver);
            if (questStatus > 6)
                questStatus = getDialogStatus(_player, questgiver, defstatus);

            ObjectGuid guid = questgiver->GetGUID();

            data.WriteBit(guid[7]);
            data.WriteBit(guid[0]);
            data.WriteBit(guid[6]);
            data.WriteBit(guid[2]);
            data.WriteBit(guid[5]);
            data.WriteBit(guid[1]);
            data.WriteBit(guid[4]);
            data.WriteBit(guid[3]);

            byteData.WriteByteSeq(guid[5]);
            byteData << uint32(questStatus);
            byteData.WriteByteSeq(guid[4]);
            byteData.WriteByteSeq(guid[2]);
            byteData.WriteByteSeq(guid[3]);
            byteData.WriteByteSeq(guid[6]);
            byteData.WriteByteSeq(guid[1]);
            byteData.WriteByteSeq(guid[7]);
            byteData.WriteByteSeq(guid[0]);

            ++count;
        }
    }

    data.PutBits(count, 21, wpos, bpos);

    data.append(byteData);

    SendPacket(&data);
}

void WorldSession::HandleQueryQuestsCompleted(WorldPacket& /*recvData*/)
{
    size_t rew_count = _player->GetRewardedQuestCount();

    WorldPacket data(SMSG_QUERY_QUESTS_COMPLETED_RESPONSE, 4 + 4 * rew_count);
    data << uint32(rew_count);

    const RewardedQuestSet &rewQuests = _player->getRewardedQuests();
    for (RewardedQuestSet::const_iterator itr = rewQuests.begin(); itr != rewQuests.end(); ++itr)
        data << uint32(*itr);

    SendPacket(&data);
}
