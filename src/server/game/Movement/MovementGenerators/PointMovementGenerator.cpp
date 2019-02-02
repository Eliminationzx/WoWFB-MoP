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

#include "PointMovementGenerator.h"
#include "Errors.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "World.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Player.h"
#include "ObjectMgr.h"

//----- Point Movement Generator
template<class T>
void PointMovementGenerator<T>::Initialize(T &unit)
{
    if (!unit.IsStopped())
        unit.StopMoving();

    unit.AddUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);
    i_recalculateSpeed = false;
    Movement::MoveSplineInit init(unit);
    init.MoveTo(i_x, i_y, i_z);
    if (speed > 0.0f)
        init.SetVelocity(speed);
    init.Launch();
}

template<class T>
bool PointMovementGenerator<T>::Update(T &unit, const uint32 & /*diff*/)
{
    if (!&unit)
        return false;

    if (unit.HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED))
    {
        unit.ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }

    unit.AddUnitState(UNIT_STATE_ROAMING_MOVE);

    if (i_recalculateSpeed && !unit.movespline->Finalized())
    {
        i_recalculateSpeed = false;
        Movement::MoveSplineInit init(unit);
        init.MoveTo(i_x, i_y, i_z);
        if (speed > 0.0f) // Default value for point motion type is 0.0, if 0.0 spline will use GetSpeed on unit
            init.SetVelocity(speed);
        init.Launch();
    }

    return !unit.movespline->Finalized();
}

template<class T>
void PointMovementGenerator<T>::Finalize(T &unit)
{
    unit.ClearUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);

    if (unit.movespline->Finalized())
        MovementInform(unit);
}

template<class T>
void PointMovementGenerator<T>::Reset(T &unit)
{
    if (!unit.IsStopped())
        unit.StopMoving();

    unit.AddUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);
}

enum specialSpells
{
    BABY_ELEPHANT_TAKES_A_BATH  = 108938,
    BABY_ELEPHANT_TAKES_A_BATH_2= 108937,
    MONK_CLASH                  = 126452,
    MONK_CLASH_IMPACT           = 126451,
};

template<class T>
void PointMovementGenerator<T>::MovementInform(T & /*unit*/)
{
}

template <> void PointMovementGenerator<Creature>::MovementInform(Creature &unit)
{
    if (unit.AI())
        unit.AddMovementInform(POINT_MOTION_TYPE, id);

    switch (id)
    {
        case BABY_ELEPHANT_TAKES_A_BATH:
            unit.CastSpell(&unit, BABY_ELEPHANT_TAKES_A_BATH_2, true);
            break;
        default:
            break;
    }

    if (afterMovement)
    {
        switch (afterMovement->m_action)
        {
        case TRIGGER_AFTER_MOVEMENT_CAST:
            if (Unit* target = sObjectAccessor->GetUnit(unit, afterMovement->m_target))
                unit.CastSpell(target, afterMovement->m_data, true);
            break;
        }
        afterMovement = nullptr;
    }
}

template <> void PointMovementGenerator<Player>::MovementInform(Player& unit)
{
    switch (id)
    {
        case MONK_CLASH:
            unit.CastSpell(&unit, MONK_CLASH_IMPACT, true);
            break;
        default:
            break;
    }

    if (afterMovement)
    {
        if (unit.IsInWorld())
        {
            switch (afterMovement->m_action)
            {
            case TRIGGER_AFTER_MOVEMENT_CAST:
                if (Unit* target = sObjectAccessor->GetUnit(unit, afterMovement->m_target))
                    if (target->IsInWorld())
                        unit.CastSpell(target, afterMovement->m_data, true);
                break;
            }
        }

        afterMovement = nullptr;
    }
}

template void PointMovementGenerator<Player>::Initialize(Player&);
template void PointMovementGenerator<Creature>::Initialize(Creature&);
template void PointMovementGenerator<Player>::Finalize(Player&);
template void PointMovementGenerator<Creature>::Finalize(Creature&);
template void PointMovementGenerator<Player>::Reset(Player&);
template void PointMovementGenerator<Creature>::Reset(Creature&);
template bool PointMovementGenerator<Player>::Update(Player &, const uint32 &);
template bool PointMovementGenerator<Creature>::Update(Creature&, const uint32 &);

void AssistanceMovementGenerator::Finalize(Unit &unit)
{
    unit.ToCreature()->SetNoCallAssistance(false);
    unit.ToCreature()->CallAssistance();
    if (unit.isAlive())
        unit.GetMotionMaster()->MoveSeekAssistanceDistract(sWorld->getIntConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY));
}

bool EffectMovementGenerator::Update(Unit &unit, const uint32&)
{
    return !unit.movespline->Finalized();
}

void EffectMovementGenerator::Finalize(Unit &unit)
{
    MovementInform(unit);
}

void EffectMovementGenerator::MovementInform(Unit &unit)
{
    if (unit.GetTypeId() == TYPEID_UNIT)
    {
        Creature* creature = unit.ToCreature();

        if (creature->AI())
            creature->AddMovementInform(EFFECT_MOTION_TYPE, m_Id);
    }
    else if (unit.GetTypeId() == TYPEID_PLAYER)
    {
    }

    if (m_afterMovement)
    {
        if (unit.IsInWorld())
        {
            switch (m_afterMovement->m_action)
            {
                case TRIGGER_AFTER_MOVEMENT_CAST:
                    if (Unit* target = sObjectAccessor->GetUnit(unit, m_afterMovement->m_target))
                        if (target->IsInWorld())
                            unit.CastSpell(target, m_afterMovement->m_data, true);
                    break;
            }
        }

        m_afterMovement = nullptr;
    }
}
