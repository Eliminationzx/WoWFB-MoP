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
#include "UpdateMask.h"
#include "Opcodes.h"
#include "World.h"
#include "ObjectAccessor.h"
#include "DatabaseEnv.h"
#include "GridNotifiers.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "ScriptMgr.h"
#include "Transport.h"

DynamicObject::DynamicObject(bool isWorldObject) : WorldObject(isWorldObject),
    _aura(NULL), _removedAura(NULL), _caster(NULL), _duration(0), _isViewpoint(false)
{
    m_objectType |= TYPEMASK_DYNAMICOBJECT;
    m_objectTypeId = TYPEID_DYNAMICOBJECT;

    m_updateFlag = UPDATEFLAG_STATIONARY_POSITION;

    m_valuesCount = DYNAMICOBJECT_END;
}

DynamicObject::~DynamicObject()
{
    // make sure all references were properly removed
    ASSERT(!_aura);
    ASSERT(!_caster);
    ASSERT(!_isViewpoint);
}

void DynamicObject::AddToWorld()
{
    ///- Register the dynamicObject for guid lookup and for caster
    if (!IsInWorld())
    {
        GetMap()->GetObjectsStore().Insert<DynamicObject>(GetGUID(), this);
        WorldObject::AddToWorld();
        BindToCaster();
    }
}

void DynamicObject::RemoveFromWorld()
{
    ///- Remove the dynamicObject from the accessor and from all lists of objects in world
    if (IsInWorld())
    {
        if (_isViewpoint)
            RemoveCasterViewpoint();

        if (_aura)
            RemoveAura();

        // dynobj could get removed in Aura::RemoveAura
        if (!IsInWorld())
            return;

        UnbindFromCaster();
        WorldObject::RemoveFromWorld();
        GetMap()->GetObjectsStore().Remove<DynamicObject>(GetGUID());
    }
}

bool DynamicObject::CreateDynamicObject(uint32 guidlow, Unit* caster, SpellInfo const* spell, Position const& pos, float radius, DynamicObjectType type)
{
    _spell = spell;
    _type = type;

    SetMap(caster->GetMap());
    Relocate(pos);
    if (!IsPositionValid())
    {
        sLog->outError(LOG_FILTER_GENERAL, "DynamicObject (spell %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)", spell->Id, GetPositionX(), GetPositionY());
        return false;
    }

    WorldObject::_Create(guidlow, HighGuid::DynamicObject, caster->GetPhaseMask());

    SetEntry(spell->Id);
    SetObjectScale(1.0f);
    SetUInt64Value(DYNAMICOBJECT_CASTER, caster->GetGUID());

    // The lower word of DYNAMICOBJECT_FIELD_TYPE_AND_VISUAL_ID must be 0x0001. This value means that the visual radius will be overriden
    // by client for most of the "ground patch" visual effect spells and a few "skyfall" ones like Hurricane.
    // If any other value is used, the client will _always_ use the radius provided in DYNAMICOBJECT_RADIUS, but
    // precompensation is necessary (eg radius *= 2) for many spells. Anyway, blizz sends 0x0001 for all the spells
    // I saw sniffed...

    // Blizz set visual spell Id in 3 first byte of DYNAMICOBJECT_FIELD_TYPE_AND_VISUAL_ID after 5.X
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell->Id);
    if (spellInfo)
    {
        uint32 visual = spellInfo->SpellVisual[0] ? spellInfo->SpellVisual[0] : spellInfo->SpellVisual[1];
        
        // HACK: !!!Temporary hack
        // We need to check spell auras 403 (or cached changers) from caster
        // and replace spell visual using visual from those spell auras
        if (spellInfo->Id == 31725) // Summon horse
        {
            if (caster->HasAura(101508)) // green fire spells
                visual = 34309;
        }
        else if (spellInfo->Id == 104232 || spellInfo->Id == 5740) // Rain of Fire
        {
            if (caster->HasAura(101508)) // green fire spells
                visual = 25761;
        }

        SetUInt32Value(DYNAMICOBJECT_FIELD_TYPE_AND_VISUAL_ID, visual | (type << 28));
    }

    SetUInt32Value(DYNAMICOBJECT_SPELLID, spell->Id);
    SetFloatValue(DYNAMICOBJECT_RADIUS, radius);
    SetUInt32Value(DYNAMICOBJECT_CASTTIME, getMSTime());

    if (IsWorldObject())
        setActive(true);    //must before add to map to be put in world container

    Transport* transport = caster->GetTransport();
    if (transport)
    {
        float x, y, z, o;
        pos.GetPosition(x, y, z, o);
        transport->CalculatePassengerOffset(x, y, z, &o);
        m_movementInfo.transport.pos.Relocate(x, y, z, o);

        // This object must be added to transport before adding to map for the client to properly display it
        transport->AddPassenger(this);
    }

    if (!GetMap()->AddToMap(this))
    {
        // Returning false will cause the object to be deleted - remove from transport
        if (transport)
            transport->RemovePassenger(this);
        return false;
    }

    return true;
}

void DynamicObject::Update(uint32 p_time)
{
    // caster has to be always available and in the same map
    ASSERT(_caster);
    ASSERT(_caster->GetMap() == GetMap());

    bool expired = false;

    if (_aura)
    {
        if (!_aura->IsRemoved())
            _aura->UpdateOwner(p_time, this);

        // _aura may be set to null in Aura::UpdateOwner call
        if (_aura && (_aura->IsRemoved() || _aura->IsExpired()))
            expired = true;
    }
    else
    {
        if (GetDuration() > int32(p_time))
            _duration -= p_time;
        else
            expired = true;
    }

    if (_aura && !_aura->IsRemoved() && _caster->getClass() == CLASS_DRUID && _aura->GetSpellInfo()->Id == 81262 && _caster->HasAura(145529))
        expired = false;

    if (expired)
        Remove();
    else
        sScriptMgr->OnDynamicObjectUpdate(this, p_time);
}

void DynamicObject::Remove()
{
    if (IsInWorld())
    {
        SendObjectDeSpawnAnim(GetGUID());
        RemoveFromWorld();
        AddObjectToRemoveList();
    }
}

int32 DynamicObject::GetDuration() const
{
    if (!_aura)
        return _duration;
    else
        return _aura->GetDuration();
}

void DynamicObject::SetDuration(int32 newDuration)
{
    if (!_aura)
        _duration = newDuration;
    else
        _aura->SetDuration(newDuration);
}

void DynamicObject::Delay(int32 delaytime)
{
    SetDuration(GetDuration() - delaytime);
}

void DynamicObject::SetAura(Aura* aura)
{
    ASSERT(!_aura && aura);
    _aura = aura;
}

void DynamicObject::RemoveAura()
{
    ASSERT(_aura && !_removedAura);
    _removedAura = _aura;
    _aura = nullptr;
    if (!_removedAura->IsRemoved())
        _removedAura->_Remove(AURA_REMOVE_BY_DEFAULT);
}

void DynamicObject::SetCasterViewpoint()
{
    if (Player* caster = _caster->ToPlayer())
    {
        caster->SetViewpoint(this, true);
        _isViewpoint = true;
    }
}

void DynamicObject::RemoveCasterViewpoint()
{
    if (Player* caster = _caster->ToPlayer())
    {
        caster->SetViewpoint(this, false);
        _isViewpoint = false;
    }
}

void DynamicObject::BindToCaster()
{
    //ASSERT(!_caster);
    _caster = ObjectAccessor::GetUnit(*this, GetCasterGUID());
    //ASSERT(_caster);
    //ASSERT(_caster->GetMap() == GetMap());
    if (_caster)
        _caster->_RegisterDynObject(this);
}

void DynamicObject::UnbindFromCaster()
{
    //ASSERT(_caster);
    _caster->_UnregisterDynObject(this);
    _caster = NULL;
}
