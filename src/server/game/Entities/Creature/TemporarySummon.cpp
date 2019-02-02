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

#include "Log.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "ObjectMgr.h"
#include "TemporarySummon.h"
#include "DB2Stores.h"

TempSummon::TempSummon(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject) :
Creature(isWorldObject), m_Properties(properties), m_type(TEMPSUMMON_MANUAL_DESPAWN),
m_timer(0), m_lifetime(0)
{
    m_summonerGUID = owner ? owner->GetGUID() : 0;
    m_unitTypeMask |= UNIT_MASK_SUMMON;
}

Unit* TempSummon::GetSummoner() const
{
    return m_summonerGUID ? ObjectAccessor::GetUnit(*this, m_summonerGUID) : NULL;
}

void TempSummon::Update(uint32 diff)
{
    Creature::Update(diff);

    if (m_deathState == DEAD)
    {
        UnSummon();
        return;
    }
    switch (m_type)
    {
        case TEMPSUMMON_MANUAL_DESPAWN:
            break;
        case TEMPSUMMON_TIMED_DESPAWN:
        {
            if (m_timer <= diff)
            {
                if (IsAIEnabled)
                    AI()->JustDespawned();

                UnSummon();
                return;
            }

            m_timer -= diff;
            break;
        }
        case TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT:
        {
            if (!isInCombat())
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;

            break;
        }

        case TEMPSUMMON_CORPSE_TIMED_DESPAWN:
        {
            if (m_deathState == CORPSE)
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= diff;
            }
            break;
        }
        case TEMPSUMMON_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            break;
        }
        case TEMPSUMMON_DEAD_DESPAWN:
        {
            if (m_deathState == DEAD)
            {
                UnSummon();
                return;
            }
            break;
        }
        case TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (!isInCombat())
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }
                else
                    m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        case TEMPSUMMON_TIMED_OR_DEAD_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (!isInCombat() && isAlive())
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }
                else
                    m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        default:
            UnSummon();
            sLog->outError(LOG_FILTER_UNITS, "Temporary summoned creature (entry: %u) have unknown type %u of ", GetEntry(), m_type);
            break;
    }
}

void TempSummon::InitStats(uint32 duration)
{
    ASSERT(!isPet());

    m_timer = duration;
    m_lifetime = duration;

    if (m_type == TEMPSUMMON_MANUAL_DESPAWN)
        m_type = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;

    Unit* owner = GetSummoner();

    if (owner && isTrigger() && m_spells[0])
    {
        setFaction(owner->getFaction());
        SetLevel(owner->getLevel());
        if (owner->GetTypeId() == TYPEID_PLAYER)
            m_ControlledByPlayer = true;
    }

    if (!m_Properties)
        return;

    // Fix Force of Nature treants stats
    if (owner && owner->getClass() == CLASS_DRUID && owner->HasSpell(106737))
    {
        float damage = 0.0f;

        switch (GetEntry())
        {
            case ENTRY_TREANT_RESTO:
            case ENTRY_TREANT_BALANCE:
                SetMaxHealth(owner->CountPctFromMaxHealth(40));
                break;
            case ENTRY_TREANT_GUARDIAN:
                SetMaxHealth(owner->CountPctFromMaxHealth(40));
                // (Attack power / 14 * 2 * 0.75) * 0.2f
                damage = ((owner->GetTotalAttackPowerValue(BASE_ATTACK) / 14.0f) * 2.0f * 0.75f) * 0.2f;
                SetStatFloatValue(UNIT_FIELD_MINDAMAGE, damage);
                SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, damage);
                break;
            case ENTRY_TREANT_FERAL:
                SetMaxHealth(owner->CountPctFromMaxHealth(40));
                // Attack power / 14 * 2 * 0.75
                damage = (owner->GetTotalAttackPowerValue(BASE_ATTACK) / 14.0f) * 2.0f * 0.75f;
                SetStatFloatValue(UNIT_FIELD_MINDAMAGE, damage);
                SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, damage);
                break;
            default:
                break;
        }
    }

    // Fix Wild Imps stats
    if (owner && owner->getClass() == CLASS_WARLOCK && GetEntry() == ENTRY_WILD_IMP)
    {
        m_baseSpellCritChance = owner->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_MASK_FIRE);
    }

    if (owner)
    {
        if (uint32 slot = m_Properties->Slot)
        {
            // Totemic Persistence
            if (slot != 1 && isTotem() && owner->HasAura(108284) && owner->m_SummonSlot[slot] && !owner->m_SummonSlot[slot + MAX_TOTEM_SLOT - 1])
                slot += MAX_TOTEM_SLOT - 1;

            if (owner->m_SummonSlot[slot] && owner->m_SummonSlot[slot] != GetGUID())
            {
                Creature* oldSummon = GetMap()->GetCreature(owner->m_SummonSlot[slot]);
                if (oldSummon && oldSummon->isSummon())
                    oldSummon->ToTempSummon()->UnSummon();
            }
            owner->m_SummonSlot[slot] = GetGUID();
        }
    }

    if (m_Properties->Faction)
        setFaction(m_Properties->Faction);
    else if (IsVehicle() && owner) // properties should be vehicle
        setFaction(owner->getFaction());
}

void TempSummon::InitSummon()
{
    Unit* owner = GetSummoner();
    if (owner)
    {
        if (owner->GetTypeId() == TYPEID_UNIT && owner->ToCreature()->IsAIEnabled)
            owner->ToCreature()->AI()->JustSummoned(this);
        if (IsAIEnabled)
            AI()->IsSummonedBy(owner);
    }
}

void TempSummon::SetTempSummonType(TempSummonType type)
{
    m_type = type;
}

void TempSummon::UnSummon(uint32 msTime)
{
    if (msTime)
    {
        ForcedUnsummonDelayEvent* pEvent = new ForcedUnsummonDelayEvent(*this);

        m_Events.AddEvent(pEvent, m_Events.CalculateTime(msTime));
        return;
    }

    //ASSERT(!isPet());
    if (isPet())
    {
        if (ToPet()->getPetType() == HUNTER_PET)
            ToPet()->Remove(PET_SLOT_ACTUAL_PET_SLOT, false, ToPet()->m_Stampeded);
        else
            ToPet()->Remove(PET_SLOT_OTHER_PET, false, ToPet()->m_Stampeded);
        ASSERT(!IsInWorld());
        return;
    }

    Unit* owner = GetSummoner();
    if (owner && owner->GetTypeId() == TYPEID_UNIT && owner->ToCreature()->IsAIEnabled)
        owner->ToCreature()->AI()->SummonedCreatureDespawn(this);
    
    if (owner && owner->GetTypeId() == TYPEID_PLAYER)
        owner->ToPlayer()->DecrementCounter(GetEntry());

    AddObjectToRemoveList();

    // Glyph of Efflorescence
    if (this->GetEntry() == 47649 && owner && owner->getClass() == CLASS_DRUID && owner->HasAura(145529))
    {
        if (DynamicObject* dynObj = owner->GetDynObject(81262))
            dynObj->Remove();
        this->RemoveAura(81262);
        owner->RemoveAura(81262);
    }
}

bool ForcedUnsummonDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    m_owner.UnSummon();
    return true;
}

void TempSummon::RemoveFromWorld()
{
    if (GetOwner() && GetOwner()->GetTypeId() == TYPEID_PLAYER)
    {
        GetOwner()->ToPlayer()->GetPetJournal()->SetCurrentSummon(NULL);
        GetOwner()->ToPlayer()->GetPetJournal()->SetCurrentSummonId(0);
    }

    if (!IsInWorld())
        return;

    if (m_Properties)
        if (uint32 slot = m_Properties->Slot)
            if (Unit* owner = GetSummoner())
                if (owner->m_SummonSlot[slot] == GetGUID())
                    owner->m_SummonSlot[slot] = 0;

    //if (GetOwnerGUID())
    //    sLog->outError(LOG_FILTER_UNITS, "Unit %u has owner guid when removed from world", GetEntry());

    Creature::RemoveFromWorld();
}

Minion::Minion(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject) : TempSummon(properties, owner, isWorldObject)
, m_owner(owner), m_BattlePetTemplate(nullptr), m_isBattlePetCreated(false)
{
    ASSERT(m_owner);
    m_unitTypeMask |= UNIT_MASK_MINION;
    m_followAngle = PET_FOLLOW_ANGLE;
}

Minion::~Minion()
{
    if (m_isBattlePetCreated)
    {
        delete m_BattlePetTemplate;
        m_BattlePetTemplate = nullptr;
    }
}

void Minion::InitStats(uint32 duration)
{
    TempSummon::InitStats(duration);

    SetReactState(REACT_PASSIVE);

    SetCreatorGUID(m_owner->GetGUID());
    setFaction(m_owner->getFaction());

    m_owner->SetMinion(this, true, PET_SLOT_UNK_SLOT);
}

void Minion::RemoveFromWorld()
{
    if (!IsInWorld())
        return;

    m_owner->SetMinion(this, false, PET_SLOT_UNK_SLOT);
    TempSummon::RemoveFromWorld();
}

bool Minion::IsGuardianPet() const
{
    return isPet() || (m_Properties && m_Properties->Category == SUMMON_CATEGORY_PET);
}

bool Minion::IsWarlockPet() const
{
    switch (GetEntry())
    {
        case ENTRY_INFERNAL:
        case ENTRY_IMP:
        case ENTRY_VOIDWALKER:
        case ENTRY_SUCCUBUS:
        case ENTRY_FELHUNTER:
        case ENTRY_FELGUARD:
        case ENTRY_FEL_IMP:
        case ENTRY_VOIDLORD:
        case ENTRY_SHIVARRA:
        case ENTRY_OBSERVER:
        case ENTRY_WRATHGUARD:
        case ENTRY_WILD_IMP:
            return isPet();
    }
    return false;
}

BattlePet::PetTemplate* Minion::CreateBattlePet()
{
    uint32 npcId = GetEntry();
    uint32 speciesId = 0;
    for (uint32 i = 0; i < sBattlePetSpeciesStore.GetNumRows(); i++)
    {
        if (BattlePetSpeciesEntry const* speciesEntry = sBattlePetSpeciesStore.LookupEntry(i))
        {
            if (speciesEntry->NpcId == npcId)
            {
                speciesId = speciesEntry->SpeciesId;
                break;
            }
        }
    }

    ASSERT(speciesId > 0);

    m_isBattlePetCreated = true;

    uint64 id = sObjectMgr->BattlePetGetNewId();
    uint8 breed = sObjectMgr->BattlePetGetRandomBreed(speciesId);
    uint8 quality = sObjectMgr->BattlePetGetRandomQuality(speciesId);
    uint8 level = BATTLE_PET_INITIAL_LEVEL; // TODO: set level based on location
    
    BattlePet::PetTemplate* proto = new BattlePet::PetTemplate();
    if (!proto->Create(id, speciesId, level, quality, breed))
    {
        delete proto;
        return nullptr;
    }

    return proto;
}

void Minion::InitialBattlePet(BattlePet::PetTemplate* proto, uint64 const creatorGuid)
{
    ASSERT(m_BattlePetTemplate == nullptr);
    ASSERT(proto != nullptr);

    m_BattlePetTemplate = proto;

    SetCreateHealth(m_BattlePetTemplate->GetMaxHealth());
    SetMaxHealth(m_BattlePetTemplate->GetMaxHealth());
    SetHealth(m_BattlePetTemplate->GetCurrentHealth());

    SetUInt64Value(UNIT_FIELD_BATTLE_PET_COMPANION_GUID, m_BattlePetTemplate->GetId());
    SetUInt64Value(UNIT_FIELD_CREATEDBY, creatorGuid);
    SetUInt32Value(UNIT_FIELD_BATTLE_PET_COMPANION_NAME_TIMESTAMP, m_BattlePetTemplate->GetTimestamp());
    SetUInt32Value(UNIT_FIELD_WILD_BATTLE_PET_LEVEL, m_BattlePetTemplate->GetLevel());
}

Guardian::Guardian(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject) : Minion(properties, owner, isWorldObject)
, m_bonusSpellDamage(0)
{
    memset(m_statFromOwner, 0, sizeof(float)*MAX_STATS);
    m_unitTypeMask |= UNIT_MASK_GUARDIAN;
    if (properties && properties->Type == SUMMON_TYPE_PET)
    {
        m_unitTypeMask |= UNIT_MASK_CONTROLABLE_GUARDIAN;
        InitCharmInfo();
    }
}

void Guardian::InitStats(uint32 duration)
{
    Minion::InitStats(duration);

    InitStatsForLevel(m_owner->getLevel());

    if (m_owner->GetTypeId() == TYPEID_PLAYER && HasUnitTypeMask(UNIT_MASK_CONTROLABLE_GUARDIAN))
        m_charmInfo->InitCharmCreateSpells();

    SetReactState(REACT_AGGRESSIVE);
}

void Guardian::InitSummon()
{
    TempSummon::InitSummon();

    if (m_owner->GetTypeId() == TYPEID_PLAYER
        && m_owner->GetMinionGUID() == GetGUID()
        && !m_owner->GetCharmGUID())
        m_owner->ToPlayer()->CharmSpellInitialize();
}

Puppet::Puppet(SummonPropertiesEntry const* properties, Unit* owner) : Minion(properties, owner, false) //maybe true?
{
    ASSERT(owner->GetTypeId() == TYPEID_PLAYER);
    m_owner = (Player*)owner;
    m_unitTypeMask |= UNIT_MASK_PUPPET;
}

void Puppet::InitStats(uint32 duration)
{
    Minion::InitStats(duration);
    SetLevel(m_owner->getLevel());
    SetReactState(REACT_PASSIVE);
}

void Puppet::InitSummon()
{
    Minion::InitSummon();
    if (!SetCharmedBy(m_owner, CHARM_TYPE_POSSESS))
        ASSERT(false);
}

void Puppet::Update(uint32 time)
{
    Minion::Update(time);
    //check if caster is channelling?
    if (IsInWorld())
    {
        if (!isAlive())
        {
            UnSummon();
            // TODO: why long distance .die does not remove it
        }
    }
}

void Puppet::RemoveFromWorld()
{
    if (!IsInWorld())
        return;

    RemoveCharmedBy(NULL);
    Minion::RemoveFromWorld();
}
