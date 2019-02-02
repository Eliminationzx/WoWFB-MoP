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
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "GossipDef.h"
#include "UpdateMask.h"
#include "ObjectAccessor.h"
#include "Creature.h"
#include "Pet.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "ScriptMgr.h"
#include "CreatureAI.h"
#include "SpellInfo.h"
#include "Guild.h"

enum StableResultCode
{
    STABLE_ERR_MONEY        = 0x01,                         // "you don't have enough money"
    STABLE_ERR_STABLE       = 0x06,                         // currently used in most fail cases
    STABLE_SUCCESS_STABLE   = 0x08,                         // stable success
    STABLE_SUCCESS_UNSTABLE = 0x09,                         // unstable/swap success
    STABLE_SUCCESS_BUY_SLOT = 0x0A,                         // buy slot success
    STABLE_ERR_EXOTIC       = 0x0C,                         // "you are unable to control exotic creatures"
};

void WorldSession::HandleTabardVendorActivateOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TABARDDESIGNER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleTabardVendorActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendTabardVendorActivate(guid);
}

void WorldSession::SendTabardVendorActivate(uint64 guid)
{
    ObjectGuid playerGuid = guid;
    WorldPacket data(SMSG_PLAYER_TABAR_VENDOR_SHOW, 1 + 8);

    uint8 bitsOrder[8] = { 6, 3, 0, 1, 4, 2, 5, 7 };
    data.WriteBitInOrder(playerGuid, bitsOrder);

    uint8 bytesOrder[8] = { 2, 5, 1, 7, 6, 0, 3, 4 };
    data.WriteBytesSeq(playerGuid, bytesOrder);

    SendPacket(&data);
}

void WorldSession::HandleBankerActivateOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_BANKER_ACTIVATE");

    uint8 bitsOrder[8] = { 3, 7, 0, 4, 1, 6, 5, 2 };
    recvData.ReadBitInOrder(guid, bitsOrder);

    uint8 bytesOrder[8] = { 0, 2, 4, 7, 6, 3, 1, 5 };
    recvData.ReadBytesSeq(guid, bytesOrder);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BANKER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleBankerActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendShowBank(guid);
}

void WorldSession::SendShowBank(uint64 guid)
{
    WorldPacket data(SMSG_SHOW_BANK, 8);
    ObjectGuid npcGuid = guid;

    uint8 bitsOrder[8] = { 7, 1, 6, 4, 3, 5, 0, 2 };
    data.WriteBitInOrder(npcGuid, bitsOrder);

    uint8 bytesOrder[8] = { 6, 0, 7, 3, 5, 1, 4, 2 };
    data.WriteBytesSeq(npcGuid, bytesOrder);

    SendPacket(&data);
}

void WorldSession::HandleTrainerListOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    uint8 bitsOrder[8] = { 2, 7, 1, 0, 3, 5, 4, 6 };
    recvData.ReadBitInOrder(guid, bitsOrder);

    uint8 bytesOrder[8] = { 3, 0, 2, 1, 7, 6, 4, 5 };
    recvData.ReadBytesSeq(guid, bytesOrder);

    SendTrainerList(guid);
}

void WorldSession::SendTrainerList(uint64 guid)
{
    std::string str = GetTrinityString(LANG_NPC_TAINER_HELLO);
    SendTrainerList(guid, str);
}

void WorldSession::SendTrainerList(uint64 guid, const std::string& strTitle)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList");

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // trainer list loaded at check;
    if (!unit->isCanTrainingOf(_player, true))
        return;

    CreatureTemplate const* ci = unit->GetCreatureTemplate();

    if (!ci)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList - (GUID: %u) NO CREATUREINFO!", GUID_LOPART(guid));
        return;
    }

    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList - Training spells not found for creature (GUID: %u Entry: %u)",
            GUID_LOPART(guid), unit->GetEntry());
        return;
    }

    ByteBuffer dataBuffer(128);

    // reputation discount
    float fDiscountMod = _player->GetReputationPriceDiscount(unit);
    bool can_learn_primary_prof = GetPlayer()->GetFreePrimaryProfessionPoints() > 0;

    uint32 count = 0;
    for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
    {
        TrainerSpell const* tSpell = &itr->second;

        bool valid = true;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (!tSpell->learnedSpell[i])
                continue;
            if (!_player->IsSpellFitByClassAndRace(tSpell->learnedSpell[i]))
            {
                valid = false;
                break;
            }
        }
        if (!valid)
            continue;

        TrainerSpellState state = _player->GetTrainerSpellState(tSpell);

        dataBuffer << uint8(state == TRAINER_SPELL_GREEN_DISABLED ? TRAINER_SPELL_GREEN : state);
        dataBuffer << uint32(tSpell->spell);
        dataBuffer << uint32(tSpell->reqSkill);
        dataBuffer << uint32(floor(tSpell->spellCost * fDiscountMod));

        // spells required (3 max)
        uint8 maxReq = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (!tSpell->learnedSpell[i])
                continue;
            SpellsRequiringSpellMapBounds spellsRequired = sSpellMgr->GetSpellsRequiredForSpellBounds(tSpell->learnedSpell[i]);
            for (SpellsRequiringSpellMap::const_iterator itr2 = spellsRequired.first; itr2 != spellsRequired.second && maxReq < 2; ++itr2)
            {
                dataBuffer << uint32(itr2->second);
                ++maxReq;
            }
            if (maxReq == 3)
                break;
        }
        while (maxReq < 3)
        {
            dataBuffer << uint32(0);
            ++maxReq;
        }
        dataBuffer << uint8(tSpell->reqLevel);
        dataBuffer << uint32(tSpell->reqSkillValue);
        ++count;
    }

    WorldPacket data(SMSG_TRAINER_LIST, 128 + 24);
    ObjectGuid npcGuid = guid;
    data.WriteBits(count, 19);
    data.WriteBit(npcGuid[3]);
    data.WriteBit(npcGuid[2]);
    data.WriteBit(npcGuid[0]);
    data.WriteBit(npcGuid[7]);
    data.WriteBit(npcGuid[1]);
    data.WriteBit(npcGuid[5]);
    data.WriteBits(strTitle.size(), 11);
    data.WriteBit(npcGuid[6]);
    data.WriteBit(npcGuid[4]);

    data.WriteByteSeq(npcGuid[3]);

    if (dataBuffer.size() > 0)
        data.append(dataBuffer);
        
    data.WriteByteSeq(npcGuid[1]);
    data.WriteByteSeq(npcGuid[6]);
    data.WriteByteSeq(npcGuid[0]);

    if (strTitle.size() > 0)
        data.append(strTitle.c_str(), strTitle.size());

    data << uint32(unit->GetCreatureTemplate()->trainer_type);
    data.WriteByteSeq(npcGuid[2]);
    data.WriteByteSeq(npcGuid[4]);
    data.WriteByteSeq(npcGuid[5]);
    data.WriteByteSeq(npcGuid[7]);
    data << uint32(1); // different value for each trainer, also found in CMSG_TRAINER_BUY_SPELL

    SendPacket(&data);
}

void WorldSession::HandleTrainerBuySpellOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;
    uint32 spellId;
    int32 trainerId;

    recvData >> trainerId >> spellId;

    uint8 bitsOrder[8] = { 6, 2, 0, 7, 5, 3, 1, 4 };
    recvData.ReadBitInOrder(guid, bitsOrder);

    uint8 bytesOrder[8] = { 6, 0, 5, 1, 7, 4, 2, 3 };
    recvData.ReadBytesSeq(guid, bytesOrder);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_TRAINER_BUY_SPELL NpcGUID=%u, learn spell id is: %u", uint32(GUID_LOPART(guid)), spellId);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleTrainerBuySpellOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (!unit->isCanTrainingOf(_player, true))
    { 
        SendTrainerService(guid, spellId, 0);
        return;
    }

    // check present spell in trainer spell list
    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
    {
        SendTrainerService(guid, spellId, 0);
        return;
    }

    // not found, cheat?
    TrainerSpell const* trainer_spell = trainer_spells->Find(spellId);
    if (!trainer_spell)
    { 
        SendTrainerService(guid, spellId, 0);
        return;
    }

    // can't be learn, cheat? Or double learn with lags...
    if (_player->GetTrainerSpellState(trainer_spell) != TRAINER_SPELL_GREEN)
    { 
        SendTrainerService(guid, spellId, 0);
        return;
    }

    // apply reputation discount
    uint32 nSpellCost = uint32(floor(trainer_spell->spellCost * _player->GetReputationPriceDiscount(unit)));

    // check money requirement
    if (!_player->HasEnoughMoney(uint64(nSpellCost)))
    { 
        SendTrainerService(guid, spellId, 1);
        return;
    }

    _player->ModifyMoney(-int64(nSpellCost));

    unit->SendPlaySpellVisualKit(179, 0);       // 53 SpellCastDirected
    _player->SendPlaySpellVisualKit(362, 1);    // 113 EmoteSalute

    // learn explicitly or cast explicitly
    if (trainer_spell->IsCastable())
        _player->CastSpell(_player, trainer_spell->spell, true);
    else
        _player->learnSpell(spellId, false);

    SendTrainerService(guid, spellId, 2);

}

void WorldSession::SendTrainerService(uint64 guid, uint32 spellId, uint32 result)
{ 
    WorldPacket data(SMSG_TRAINER_SERVICE, 16);
    ObjectGuid npcGuid = guid;

    uint8 bitsOrder[8] = { 4, 5, 1, 2, 3, 6, 0, 7 };
    data.WriteBitInOrder(npcGuid, bitsOrder);

    data.WriteByteSeq(npcGuid[4]);
    data.WriteByteSeq(npcGuid[0]);
    data.WriteByteSeq(npcGuid[6]);
    data << uint32(spellId);        // should be same as in packet from client
    data << uint32(result);         // 2 == Success. 1 == "Not enough money for trainer service." 0 == "Trainer service %d unavailable."
    data.WriteByteSeq(npcGuid[3]);
    data.WriteByteSeq(npcGuid[7]);
    data.WriteByteSeq(npcGuid[2]);
    data.WriteByteSeq(npcGuid[1]);
    data.WriteByteSeq(npcGuid[5]);

    SendPacket(&data);
}

void WorldSession::HandleGossipHelloOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GOSSIP_HELLO");

    ObjectGuid guid;

    uint8 bitsOrder[8] = { 6, 3, 4, 5, 1, 7, 2, 0 };
    recvData.ReadBitInOrder(guid, bitsOrder);

    uint8 bytesOrder[8] = { 6, 0, 1, 7, 2, 5, 4, 3 };
    recvData.ReadBytesSeq(guid, bytesOrder);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleGossipHelloOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // set faction visible if needed
    if (FactionTemplateEntry const* factionTemplateEntry = sFactionTemplateStore.LookupEntry(unit->getFaction()))
        _player->GetReputationMgr().SetVisible(factionTemplateEntry);

    GetPlayer()->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);
    // remove fake death
    //if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
    //    GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (unit->isArmorer() || unit->isCivilian() || unit->isQuestGiver() || unit->isServiceProvider() || unit->isGuard())
        unit->StopMoving();

    // If spiritguide, no need for gossip menu, just put player into resurrect queue
    if (unit->isSpiritGuide())
    {
        Battleground* bg = _player->GetBattleground();
        if (bg)
        {
            bg->AddPlayerToResurrectQueue(unit->GetGUID(), _player->GetGUID());
            sBattlegroundMgr->SendAreaSpiritHealerQueryOpcode(_player, bg, unit->GetGUID());
            return;
        }
    }

    if (!sScriptMgr->OnGossipHello(_player, unit))
    {
//        _player->TalkedToCreature(unit->GetEntry(), unit->GetGUID());
        _player->PrepareGossipMenu(unit, unit->GetCreatureTemplate()->GossipMenuId, true);
        _player->SendPreparedGossip(unit);
    }
    unit->AI()->sGossipHello(_player);
}

void WorldSession::HandleSpiritHealerActivateOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_SPIRIT_HEALER_ACTIVATE");

    ObjectGuid guid;

    uint8 bitsOrder[8] = { 5, 3, 2, 4, 7, 1, 0, 6 };
    recvData.ReadBitInOrder(guid, bitsOrder);

    uint8 bytesOrder[8] = { 1, 6, 7, 3, 5, 4, 2, 0 };
    recvData.ReadBytesSeq(guid, bytesOrder);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_SPIRITHEALER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleSpiritHealerActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendSpiritResurrect();
}

void WorldSession::SendSpiritResurrect()
{
    _player->ResurrectPlayer(0.5f, true);

    _player->DurabilityLossAll(0.25f, true);

    // get corpse nearest graveyard
    WorldSafeLocsEntry const* corpseGrave = NULL;
    WorldLocation corpseLocation = _player->GetCorpseLocation();
    if (_player->HasCorpse())
    {
        corpseGrave = sObjectMgr->GetClosestGraveYard(corpseLocation.GetPositionX(), corpseLocation.GetPositionY(),
            corpseLocation.GetPositionZ(), corpseLocation.GetMapId(), _player->GetTeam());
    }

    // now can spawn bones
    _player->SpawnCorpseBones();

    // teleport to nearest from corpse graveyard, if different from nearest to player ghost
    if (corpseGrave)
    {
        WorldSafeLocsEntry const* ghostGrave = sObjectMgr->GetClosestGraveYard(
            _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId(), _player->GetTeam());

        if (corpseGrave != ghostGrave)
            _player->TeleportTo(corpseGrave->map_id, corpseGrave->x, corpseGrave->y, corpseGrave->z, _player->GetOrientation());
        // or update at original position
        else
            _player->UpdateObjectVisibility();
    }
    // or update at original position
    else
        _player->UpdateObjectVisibility();
}

void WorldSession::HandleBinderActivateOpcode(WorldPacket& recvData)
{
    ObjectGuid npcGUID;

    uint8 bitsOrder[8] = { 5, 4, 0, 7, 2, 3, 6, 1 };
    recvData.ReadBitInOrder(npcGUID, bitsOrder);

    uint8 bytesOrder[8] = { 6, 3, 7, 4, 5, 0, 1, 2 };
    recvData.ReadBytesSeq(npcGUID, bytesOrder);

    if (!GetPlayer()->IsInWorld() || !GetPlayer()->isAlive())
        return;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_INNKEEPER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleBinderActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(npcGUID)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendBindPoint(unit);
}

void WorldSession::SendBindPoint(Creature* npc)
{
    // prevent set homebind to instances in any case
    if (GetPlayer()->GetMap()->Instanceable())
        return;

    uint32 bindspell = 3286;

    // update sql homebind
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PLAYER_HOMEBIND);
    stmt->setUInt16(0, _player->GetMapId());
    stmt->setUInt16(1, _player->GetAreaId());
    stmt->setFloat (2, _player->GetPositionX());
    stmt->setFloat (3, _player->GetPositionY());
    stmt->setFloat (4, _player->GetPositionZ());
    stmt->setUInt32(5, _player->GetGUIDLow());
    CharacterDatabase.Execute(stmt);

    _player->m_homebindMapId = _player->GetMapId();
    _player->m_homebindAreaId = _player->GetAreaId();
    _player->m_homebindX = _player->GetPositionX();
    _player->m_homebindY = _player->GetPositionY();
    _player->m_homebindZ = _player->GetPositionZ();

    // send spell for homebinding (3286)
    _player->CastSpell(_player, bindspell, true);

    SendTrainerService(npc->GetGUID(), bindspell, 2);
    _player->PlayerTalkClass->SendCloseGossip();
}

void WorldSession::HandleListStabledPetsOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recv CMSG_LIST_STABLED_PETS");
    ObjectGuid npcGUID;

    uint8 bitsOrder[8] = { 6, 4, 0, 5, 7, 2, 1, 3 };
    recvData.ReadBitInOrder(npcGUID, bitsOrder);

    uint8 bytesOrder[8] = { 5, 0, 2, 3, 6, 4, 1, 7 };
    recvData.ReadBytesSeq(npcGUID, bytesOrder);

    if (!CheckStableMaster(npcGUID))
        return;

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // remove mounts this fix bug where getting pet from stable while mounted deletes pet.
    if (GetPlayer()->IsMounted())
        GetPlayer()->RemoveAurasByType(SPELL_AURA_MOUNTED);

    SendStablePet(npcGUID);
}

void WorldSession::SendStablePet(uint64 guid)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_SLOTS_DETAIL);

    stmt->setUInt32(0, _player->GetGUIDLow());
    stmt->setUInt8(1, PET_SLOT_HUNTER_FIRST);
    stmt->setUInt8(2, PET_SLOT_STABLE_LAST);

    _sendStabledPetCallback.SetParam(guid);
    _sendStabledPetCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::SendStablePetCallback(PreparedQueryResult result, uint64 guid)
{
    if (!GetPlayer())
        return;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recv SMSG_PET_STABLE_LIST Send.");

    WorldPacket data(SMSG_PET_STABLE_LIST, 200);           // guessed size
    ObjectGuid npcGuid = guid;
    ByteBuffer dataBuffer;

    data.WriteBit(npcGuid[5]);
    data.WriteBit(npcGuid[1]);
    data.WriteBit(npcGuid[6]);
    data.WriteBit(npcGuid[0]);
    data.WriteBit(npcGuid[4]);
    data.WriteBits(result ? result->GetRowCount() : 0, 19);
    data.WriteBit(npcGuid[7]);
    data.WriteBit(npcGuid[2]);

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            std::string name = fields[5].GetString();

            data.WriteBits(name.size(), 8);

            uint32 modelId = 0;

            if (CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(fields[3].GetUInt32()))
                modelId = cInfo->Modelid1 ? cInfo->Modelid1 : cInfo->Modelid2;

            dataBuffer << uint32(modelId);                        // creature modelid
            dataBuffer << uint8(fields[1].GetUInt8() < uint8(PET_SLOT_STABLE_FIRST) ? 1 : 3);       // 1 = current, 2/3 = in stable (any from 4, 5, ... create problems with proper show)
            dataBuffer << uint32(fields[2].GetUInt32());          // petnumber
            dataBuffer << uint32(fields[3].GetUInt32());          // creature entry
            dataBuffer << uint32(fields[1].GetUInt8());           // slot
            dataBuffer << uint32(fields[4].GetUInt16());          // level

            if (name.size())
                dataBuffer.append(name.c_str(), name.size());
        }
        while (result->NextRow());
    }

    data.WriteBit(npcGuid[3]);

    if (dataBuffer.size())
        data.append(dataBuffer);

    data.WriteByteSeq(npcGuid[2]);
    data.WriteByteSeq(npcGuid[6]);
    data.WriteByteSeq(npcGuid[5]);
    data.WriteByteSeq(npcGuid[1]);
    data.WriteByteSeq(npcGuid[3]);
    data.WriteByteSeq(npcGuid[4]);
    data.WriteByteSeq(npcGuid[7]);
    data.WriteByteSeq(npcGuid[0]);

    SendPacket(&data);
}

void WorldSession::SendStableResult(uint8 res)
{
    WorldPacket data(SMSG_STABLE_RESULT, 1);
    data << uint8(res);
    SendPacket(&data);
}

void WorldSession::HandleStableSetPetSlot(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recv CMSG_SET_PET_SLOT.");
    ObjectGuid npcGuid;
    uint32 pet_number;
    uint8 new_slot;

    recvData >> pet_number >> new_slot;

    uint8 bitsOrder[8] = { 3, 2, 5, 7, 4, 0, 1, 6 };
    recvData.ReadBitInOrder(npcGuid, bitsOrder);

    uint8 bytesOrder[8] = { 3, 6, 1, 4, 2, 5, 0, 7 };
    recvData.ReadBytesSeq(npcGuid, bytesOrder);

    if (!CheckStableMaster(npcGuid))
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    if (new_slot > MAX_PET_STABLES)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    Pet* pet = _player->GetPet();

    //If we move the pet already summoned...
    if (pet && pet->GetCharmInfo() && pet->GetCharmInfo()->GetPetNumber() == pet_number)
        _player->RemovePet(pet, PET_SLOT_ACTUAL_PET_SLOT, false, pet->m_Stampeded);

    //If we move to the pet already summoned...
    if (pet && GetPlayer()->m_currentPetSlot == new_slot)
        _player->RemovePet(pet, PET_SLOT_ACTUAL_PET_SLOT, false, pet->m_Stampeded);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_SLOT_BY_ID);

    stmt->setUInt32(0, _player->GetGUIDLow());
    stmt->setUInt32(1, pet_number);

    _setPetSlotCallback.SetParam(new_slot);
    _setPetSlotCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::HandleStableSetPetSlotCallback(PreparedQueryResult result, uint32 petId)
{
    if (!GetPlayer())
        return;

    if (!result)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    Field* fields = result->Fetch();

    uint32 slot         = fields[0].GetUInt8();
    uint32 petEntry     = fields[1].GetUInt32();
    uint32 pet_number   = fields[2].GetUInt32();

    if (!petEntry)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(petEntry);
    if (!creatureInfo || !creatureInfo->isTameable(_player->CanTameExoticPets()))
    {
        // if problem in exotic pet
        if (creatureInfo && creatureInfo->isTameable(true))
            SendStableResult(STABLE_ERR_EXOTIC);
        else
            SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PET_DATA_OWNER);
    {
        stmt->setUInt32(0, petId);
        stmt->setUInt32(1, slot);
        stmt->setUInt32(2, GetPlayer()->GetGUIDLow());

        trans->Append(stmt);
    }
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PET_DATA_OWNER_ID);
    {
        stmt->setUInt32(0, slot);
        stmt->setUInt32(1, petId);
        stmt->setUInt32(2, GetPlayer()->GetGUIDLow());
        stmt->setUInt32(3, pet_number);

        trans->Append(stmt);
    }

    CharacterDatabase.CommitTransaction(trans);


    if (petId != 100)
    {
        // We need to remove and add the new pet to there diffrent slots
        // GetPlayer()->setPetSlotUsed((PetSlot)slot, false);
        _player->setPetSlotUsed((PetSlot)slot, false);
        _player->setPetSlotUsed((PetSlot)petId, true);
        SendStableResult(STABLE_SUCCESS_UNSTABLE);
    }
    else
        SendStableResult(STABLE_ERR_STABLE);
}

void WorldSession::HandleRepairItemOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_REPAIR_ITEM");

    ObjectGuid npcGUID, itemGUID;
    bool guildBank;                                         // new in 2.3.2, bool that means from guild bank money

    npcGUID[3] = recvData.ReadBit();
    itemGUID[3] = recvData.ReadBit();
    itemGUID[6] = recvData.ReadBit();
    guildBank = recvData.ReadBit();
    npcGUID[2] = recvData.ReadBit();
    itemGUID[0] = recvData.ReadBit();
    npcGUID[7] = recvData.ReadBit();
    npcGUID[6] = recvData.ReadBit();
    npcGUID[0] = recvData.ReadBit();
    itemGUID[7] = recvData.ReadBit();
    itemGUID[4] = recvData.ReadBit();
    npcGUID[5] = recvData.ReadBit();
    itemGUID[5] = recvData.ReadBit();
    npcGUID[4] = recvData.ReadBit();
    itemGUID[2] = recvData.ReadBit();
    npcGUID[1] = recvData.ReadBit();
    itemGUID[1] = recvData.ReadBit();

    recvData.ReadByteSeq(itemGUID[6]);
    recvData.ReadByteSeq(npcGUID[1]);
    recvData.ReadByteSeq(npcGUID[0]);
    recvData.ReadByteSeq(npcGUID[2]);
    recvData.ReadByteSeq(itemGUID[2]);
    recvData.ReadByteSeq(itemGUID[5]);
    recvData.ReadByteSeq(itemGUID[1]);
    recvData.ReadByteSeq(npcGUID[6]);
    recvData.ReadByteSeq(npcGUID[7]);
    recvData.ReadByteSeq(itemGUID[3]);
    recvData.ReadByteSeq(itemGUID[7]);
    recvData.ReadByteSeq(npcGUID[3]);
    recvData.ReadByteSeq(npcGUID[4]);
    recvData.ReadByteSeq(npcGUID[5]);
    recvData.ReadByteSeq(itemGUID[0]);
    recvData.ReadByteSeq(itemGUID[4]);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_REPAIR);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleRepairItemOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(npcGUID)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // reputation discount
    float discountMod = _player->GetReputationPriceDiscount(unit);

    if (itemGUID)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "ITEM: Repair item, itemGUID = %u, npcGUID = %u", GUID_LOPART(itemGUID), GUID_LOPART(npcGUID));

        Item* item = _player->GetItemByGuid(itemGUID);
        if (item)
            _player->DurabilityRepair(item->GetPos(), true, discountMod, guildBank);
    }
    else
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "ITEM: Repair all items, npcGUID = %u", GUID_LOPART(npcGUID));
        _player->DurabilityRepairAll(true, discountMod, guildBank);
    }
}
