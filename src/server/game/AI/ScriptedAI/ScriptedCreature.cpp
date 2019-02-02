/* Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 *
 * Thanks to the original authors: ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#include "ScriptedCreature.h"
#include "Item.h"
#include "Spell.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Cell.h"
#include "CellImpl.h"
#include "ObjectMgr.h"
#include "TemporarySummon.h"
#include "MoveSplineInit.h"

// Spell summary for ScriptedAI::SelectSpell
struct TSpellSummary
{
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
} extern* SpellSummary;

void SummonList::DoZoneInCombat(uint32 entry)
{
    for (iterator i = begin(); i != end();)
    {
        Creature* summon = Unit::GetCreature(*me, *i);
        ++i;
        if (summon && summon->IsAIEnabled
            && (!entry || summon->GetEntry() == entry))
            summon->AI()->DoZoneInCombat();
    }
}

void SummonList::DespawnEntry(uint32 entry)
{
    for (iterator i = begin(); i != end();)
    {
        Creature* summon = Unit::GetCreature(*me, *i);
        if (!summon)
            erase(i++);
        else if (summon->GetEntry() == entry)
        {
            erase(i++);
            summon->DespawnOrUnsummon();
        }
        else
            ++i;
    }
}

void SummonList::DespawnAll()
{
    while (!empty())
    {
        Creature* summon = Unit::GetCreature(*me, *begin());
        if (!summon)
            erase(begin());
        else
        {
            erase(begin());
            summon->DespawnOrUnsummon();
        }
    }
}

void SummonList::RemoveNotExisting()
{
    for (iterator i = begin(); i != end();)
    {
        if (Unit::GetCreature(*me, *i))
            ++i;
        else
            erase(i++);
    }
}

bool SummonList::HasEntry(uint32 entry)
{
    for (iterator i = begin(); i != end();)
    {
        Creature* summon = Unit::GetCreature(*me, *i);
        if (!summon)
            erase(i++);
        else if (summon->GetEntry() == entry)
            return true;
        else
            ++i;
    }

    return false;
}

ScriptedAI::ScriptedAI(Creature* creature) : CreatureAI(creature),
    me(creature),
    IsFleeing(false),
    _evadeCheckCooldown(2500),
    _isCombatMovementAllowed(true),
    summons(creature)
{
    _isHeroic = me->GetMap()->IsHeroic();
    _difficulty = Difficulty(me->GetMap()->GetSpawnMode());

    m_EmptyWarned = false;
}

void ScriptedAI::AttackStartNoMove(Unit* who)
{
    if (!who)
        return;

    if (me->Attack(who, false))
        DoStartNoMovement(who);
}

void ScriptedAI::UpdateOperations(uint32 const p_Diff)
{
    for (auto l_It = m_TimedDelayedOperations.begin(); l_It != m_TimedDelayedOperations.end(); l_It++)
    {
        l_It->first -= p_Diff;

        if (l_It->first < 0)
        {
            l_It->second();
            l_It->second = nullptr;
        }
    }

    uint32 l_TimedDelayedOperationCountToRemove = std::count_if(std::begin(m_TimedDelayedOperations), std::end(m_TimedDelayedOperations), [](const std::pair<int32, std::function<void()>> & p_Pair) -> bool
    {
        return p_Pair.second == nullptr;
    });

    for (uint32 l_I = 0; l_I < l_TimedDelayedOperationCountToRemove; l_I++)
    {
        auto l_It = std::find_if(std::begin(m_TimedDelayedOperations), std::end(m_TimedDelayedOperations), [](const std::pair<int32, std::function<void()>> & p_Pair) -> bool
        {
            return p_Pair.second == nullptr;
        });

        if (l_It != std::end(m_TimedDelayedOperations))
            m_TimedDelayedOperations.erase(l_It);
    }

    if (m_TimedDelayedOperations.empty() && !m_EmptyWarned)
    {
        m_EmptyWarned = true;
        LastOperationCalled();
    }
}

void ScriptedAI::UpdateAI(uint32 const p_Diff)
{
    UpdateOperations(p_Diff);

    /// Check if we have a current target
    if (!UpdateVictim())
        return;

    DoMeleeAttackIfReady();
}

void ScriptedAI::DoStartMovement(Unit* victim, float distance, float angle)
{
    if (victim)
        me->GetMotionMaster()->MoveChase(victim, distance, angle);
}

void ScriptedAI::DoStartNoMovement(Unit* victim)
{
    if (!victim)
        return;

    me->GetMotionMaster()->MoveIdle();
}

void ScriptedAI::DoStopAttack()
{
    if (me->getVictim())
        me->AttackStop();
}

void ScriptedAI::DoCastSpell(Unit* target, SpellInfo const* spellInfo, bool triggered)
{
    if (!target || me->IsNonMeleeSpellCasted(false))
        return;

    me->StopMoving();
    me->CastSpell(target, spellInfo, triggered ? TRIGGERED_FULL_MASK : TRIGGERED_NONE);
}

void ScriptedAI::DoPlaySoundToSet(WorldObject* source, uint32 soundId)
{
    if (!source)
        return;

    if (!sSoundEntriesStore.LookupEntry(soundId))
    {
        sLog->outError(LOG_FILTER_TSCR, "Invalid soundId %u used in DoPlaySoundToSet (Source: TypeId %u, GUID %u)", soundId, source->GetTypeId(), source->GetGUIDLow());
        return;
    }

    source->PlayDirectSound(soundId);
}

Creature* ScriptedAI::DoSpawnCreature(uint32 entry, float offsetX, float offsetY, float offsetZ, float angle, uint32 type, uint32 despawntime)
{
    return me->SummonCreature(entry, me->GetPositionX() + offsetX, me->GetPositionY() + offsetY, me->GetPositionZ() + offsetZ, angle, TempSummonType(type), despawntime);
}

SpellInfo const* ScriptedAI::SelectSpell(Unit* target, uint32 school, uint32 mechanic, SelectTargetType targets, uint32 powerCostMin, uint32 powerCostMax, float rangeMin, float rangeMax, SelectEffect effects)
{
    //No target so we can't cast
    if (!target)
        return NULL;

    //Silenced so we can't cast
    if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return NULL;

    //Using the extended script system we first create a list of viable spells
    SpellInfo const* apSpell[CREATURE_MAX_SPELLS];
    memset(apSpell, 0, CREATURE_MAX_SPELLS * sizeof(SpellInfo*));

    uint32 spellCount = 0;

    SpellInfo const* tempSpell = NULL;

    //Check if each spell is viable(set it to null if not)
    for (uint32 i = 0; i < CREATURE_MAX_SPELLS; i++)
    {
        tempSpell = sSpellMgr->GetSpellInfo(me->m_spells[i]);

        //This spell doesn't exist
        if (!tempSpell)
            continue;

        // Targets and Effects checked first as most used restrictions
        //Check the spell targets if specified
        if (targets && !(SpellSummary[me->m_spells[i]].Targets & (1 << (targets-1))))
            continue;

        //Check the type of spell if we are looking for a specific spell type
        if (effects && !(SpellSummary[me->m_spells[i]].Effects & (1 << (effects-1))))
            continue;

        //Check for school if specified
        if (school && (tempSpell->SchoolMask & school) == 0)
            continue;

        //Check for spell mechanic if specified
        if (mechanic && tempSpell->Mechanic != mechanic)
            continue;

        bool ok = false;
        for (auto itr : tempSpell->SpellPowers)
        {
            // Make sure that the spell uses the requested amount of power
            if (powerCostMin && itr->Cost < powerCostMin)
                continue;

            if (powerCostMax && itr->Cost > powerCostMax)
                continue;

            // Continue if we don't have the mana to actually cast this spell
            if (itr->Cost > uint32(me->GetPower(Powers(itr->powerType))))
                continue;

            ok = true;
        }

        if (!ok)
            continue;

        //Check if the spell meets our range requirements
        if (rangeMin && me->GetSpellMinRangeForTarget(target, tempSpell) < rangeMin)
            continue;
        if (rangeMax && me->GetSpellMaxRangeForTarget(target, tempSpell) > rangeMax)
            continue;

        //Check if our target is in range
        if (me->IsWithinDistInMap(target, float(me->GetSpellMinRangeForTarget(target, tempSpell))) || !me->IsWithinDistInMap(target, float(me->GetSpellMaxRangeForTarget(target, tempSpell))))
            continue;

        //All good so lets add it to the spell list
        apSpell[spellCount] = tempSpell;
        ++spellCount;
    }

    //We got our usable spells so now lets randomly pick one
    if (!spellCount)
        return NULL;

    return apSpell[urand(0, spellCount - 1)];
}

void ScriptedAI::DoResetThreat()
{
    if (!me->CanHaveThreatList() || me->getThreatManager().isThreatListEmpty())
    {
        sLog->outError(LOG_FILTER_TSCR, "DoResetThreat called for creature that either cannot have threat list or has empty threat list (me entry = %d)", me->GetEntry());
        return;
    }

    std::list<HostileReference*>& threatlist = me->getThreatManager().getThreatList();

    for (std::list<HostileReference*>::iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
    {
        Unit* unit = Unit::GetUnit(*me, (*itr)->getUnitGuid());

        if (unit && DoGetThreat(unit))
            DoModifyThreatPercent(unit, -100);
    }
}

float ScriptedAI::DoGetThreat(Unit* unit)
{
    if (!unit)
        return 0.0f;
    return me->getThreatManager().getThreat(unit);
}

void ScriptedAI::DoModifyThreatPercent(Unit* unit, int32 pct)
{
    if (!unit)
        return;
    me->getThreatManager().modifyThreatPercent(unit, pct);
}

void ScriptedAI::DoTeleportTo(float x, float y, float z, uint32 time)
{
    me->Relocate(x, y, z);
    float speed = me->GetDistance(x, y, z) / ((float)time * 0.001f);
    me->MonsterMoveWithSpeed(x, y, z, speed);
}

void ScriptedAI::DoTeleportTo(const float position[4])
{
    me->NearTeleportTo(position[0], position[1], position[2], position[3]);
}

void ScriptedAI::DoTeleportPlayer(Unit* unit, float x, float y, float z, float o)
{
    if (!unit)
        return;

    if (Player* player = unit->ToPlayer())
        player->TeleportTo(unit->GetMapId(), x, y, z, o, TELE_TO_NOT_LEAVE_COMBAT);
    else
        sLog->outError(LOG_FILTER_TSCR, "Creature " UI64FMTD " (Entry: %u) Tried to teleport non-player unit (Type: %u GUID: " UI64FMTD ") to x: %f y:%f z: %f o: %f. Aborted.", me->GetGUID(), me->GetEntry(), unit->GetTypeId(), unit->GetGUID(), x, y, z, o);
}

void ScriptedAI::DoTeleportAll(float x, float y, float z, float o)
{
    Map* map = me->GetMap();
    if (!map->IsDungeon())
        return;

    Map::PlayerList const& PlayerList = map->GetPlayers();
    for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
        if (Player* player = itr->getSource())
            if (player->isAlive() && player->InSamePhase(me))
                player->TeleportTo(me->GetMapId(), x, y, z, o, TELE_TO_NOT_LEAVE_COMBAT);
}

Unit* ScriptedAI::DoSelectLowestHpFriendly(float range, uint32 minHPDiff)
{
    Unit* unit = NULL;
    JadeCore::MostHPMissingInRange u_check(me, range, minHPDiff);
    JadeCore::UnitLastSearcher<JadeCore::MostHPMissingInRange> searcher(me, unit, u_check);
    me->VisitNearbyObject(range, searcher);

    return unit;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyCC(float range)
{
    std::list<Creature*> list;
    JadeCore::FriendlyCCedInRange u_check(me, range);
    JadeCore::CreatureListSearcher<JadeCore::FriendlyCCedInRange> searcher(me, list, u_check);
    me->VisitNearbyObject(range, searcher);
    return list;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyMissingBuff(float range, uint32 uiSpellid)
{
    std::list<Creature*> list;
    JadeCore::FriendlyMissingBuffInRange u_check(me, range, uiSpellid);
    JadeCore::CreatureListSearcher<JadeCore::FriendlyMissingBuffInRange> searcher(me, list, u_check);
    me->VisitNearbyObject(range, searcher);
    return list;
}

Player* ScriptedAI::GetPlayerAtMinimumRange(float minimumRange)
{
    Player* player = NULL;

    CellCoord pair(JadeCore::ComputeCellCoord(me->GetPositionX(), me->GetPositionY()));
    Cell cell(pair);
    cell.SetNoCreate();

    JadeCore::PlayerAtMinimumRangeAway check(me, minimumRange);
    JadeCore::PlayerSearcher<JadeCore::PlayerAtMinimumRangeAway> searcher(me, player, check);
    TypeContainerVisitor<JadeCore::PlayerSearcher<JadeCore::PlayerAtMinimumRangeAway>, GridTypeMapContainer> visitor(searcher);

    cell.Visit(pair, visitor, *me->GetMap(), *me, minimumRange);

    return player;
}

void ScriptedAI::SetEquipmentSlots(bool loadDefault, int32 mainHand /*= EQUIP_NO_CHANGE*/, int32 offHand /*= EQUIP_NO_CHANGE*/, int32 ranged /*= EQUIP_NO_CHANGE*/)
{
    if (loadDefault)
    {
        if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(me->GetEntry()))
            me->LoadEquipment(creatureInfo->equipmentId, true);

        return;
    }

    if (mainHand >= 0)
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, uint32(mainHand));

    if (offHand >= 0)
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, uint32(offHand));

    if (ranged >= 0)
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 2, uint32(ranged));
}

void ScriptedAI::SetCombatMovement(bool allowMovement)
{
    _isCombatMovementAllowed = allowMovement;
}

enum eNPCs
{
    NPC_BROODLORD    = 12017,
    NPC_VOID_REAVER  = 19516,
    NPC_JAN_ALAI     = 23578,
    NPC_SARTHARION   = 28860,
    NPC_BLOOD_QUEEN  = 37955,
    NPC_GARFROST     = 36494,
    NPC_SHA_OF_ANGER = 60491
};

// Hacklike storage used for misc creatures that are expected to evade of outside of a certain area.
// It is assumed the information is found elswehere and can be handled by the core. So far no luck finding such information/way to extract it.
bool ScriptedAI::EnterEvadeIfOutOfCombatArea(uint32 const diff)
{
    if (_evadeCheckCooldown <= diff)
        _evadeCheckCooldown = 2500;
    else
    {
        _evadeCheckCooldown -= diff;
        return false;
    }

    if (me->IsInEvadeMode() || !me->getVictim())
        return false;

    float x = me->GetPositionX();
    float y = me->GetPositionY();
    float z = me->GetPositionZ();

    switch (me->GetEntry())
    {
        case NPC_BLOOD_QUEEN:
            if (z > 390.0f)
                return false;
            break;
        case NPC_GARFROST:
            if (me->GetDistance2d(688.93f, -197.52f) < 66.0f)
                return false;
            break;
        case NPC_BROODLORD:                                         // broodlord (not move down stairs)
            if (z > 448.60f)
                return false;
            break;
        case NPC_VOID_REAVER:                                         // void reaver (calculate from center of room)
            if (me->GetDistance2d(432.59f, 371.93f) < 105.0f)
                return false;
            break;
        case NPC_JAN_ALAI:                                         // jan'alai (calculate by Z)
            if (z > 12.0f)
                return false;
            break;
        case NPC_SARTHARION:                                         // sartharion (calculate box)
            if (x > 3218.86f && x < 3275.69f && y < 572.40f && y > 484.68f)
                return false;
            break;
        case NPC_SHA_OF_ANGER:
        {
            Position homePos = me->GetHomePosition();
            if (me->GetDistance2d(homePos.m_positionX, homePos.m_positionY) < 50.0f)
                return false;
            break;
        }
        default: // For most of creatures that certain area is their home area.
            sLog->outInfo(LOG_FILTER_GENERAL, "TSCR: EnterEvadeIfOutOfCombatArea used for creature entry %u, but does not have any definition. Using the default one.", me->GetEntry());

            auto const& homePos = me->GetHomePosition();
            uint32 homeAreaId = me->GetMap()->GetAreaId(homePos.GetPositionX(), homePos.GetPositionY(), homePos.GetPositionZ());
            if (me->GetAreaId() == homeAreaId)
                return false;
    }

    EnterEvadeMode();
    return true;
}

void ScriptedAI::DespawnCreaturesInArea(uint32 entry, WorldObject* object)
{
    std::list<Creature*> creatures;
    GetCreatureListWithEntryInGrid(creatures, object,entry, 300.0f);
    if (creatures.empty())
        return;

    for (std::list<Creature*>::iterator itr = creatures.begin(); itr != creatures.end(); ++itr)
        (*itr)->DespawnOrUnsummon();
}

void ScriptedAI::DespawnGameObjectsInArea(uint32 entry, WorldObject* object)
{
    std::list<GameObject*> gameobjects;
    GetGameObjectListWithEntryInGrid(gameobjects, object, entry, 300.0f);
    if (gameobjects.empty())
        return;

    for (std::list<GameObject*>::iterator itr = gameobjects.begin(); itr != gameobjects.end();)
    {
        (*itr)->SetRespawnTime(0);
        (*itr)->Delete();
        itr = gameobjects.erase(itr);
    }
}

void ScriptedAI::ApplyAllImmunities(bool apply)
{
    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, apply);
    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, apply);
    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, apply);
    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, apply);
    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, apply);
    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, apply);
    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, apply);
    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);
    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, apply);
    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, apply);
    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, apply);
    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, apply);
    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, apply);
    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, apply);
}

void Scripted_NoMovementAI::AttackStart(Unit* target)
{
    if (!target)
        return;

    if (me->Attack(target, true))
        DoStartNoMovement(target);
}

// BossAI - for instanced bosses

BossAI::BossAI(Creature* creature, uint32 bossId) : ScriptedAI(creature),
    instance(creature->GetInstanceScript()),
    summons(creature),
    _boundary(instance ? instance->GetBossBoundary(bossId) : NULL),
    _bossId(bossId), _checkareaTimer(0), m_IsCompletedEncounterProcessed(false)
{
}

void BossAI::_Reset()
{
    if (!me->isAlive())
        return;

    me->ResetLootMode();
    events.Reset();
    summons.DespawnAll();
    if (instance)
        instance->SetBossState(_bossId, NOT_STARTED);
}

void BossAI::_JustDied()
{
    events.Reset();
    summons.DespawnAll();
    if (instance)
    {
        instance->SetBossState(_bossId, DONE);
        instance->SaveToDB();
    }

    ProcessCompletedEncounter();
}

void BossAI::_EnterCombat()
{
    me->setActive(true);
    DoZoneInCombat();
    if (instance)
    {
        // bosses do not respawn, check only on enter combat
        if (!instance->CheckRequiredBosses(_bossId))
        {
            EnterEvadeMode();
            return;
        }
        instance->SetBossState(_bossId, IN_PROGRESS);
    }
}

void BossAI::TeleportCheaters()
{
    float x, y, z;
    me->GetPosition(x, y, z);
    std::list<HostileReference*>& threatList = me->getThreatManager().getThreatList();
    for (std::list<HostileReference*>::iterator itr = threatList.begin(); itr != threatList.end(); ++itr)
        if (Unit* target = (*itr)->getTarget())
            if (target->GetTypeId() == TYPEID_PLAYER && !CheckBoundary(target))
                target->NearTeleportTo(x, y, z, 0);
}

void BossAI::ProcessCompletedEncounter()
{
    if (m_IsCompletedEncounterProcessed)
        return;

    m_IsCompletedEncounterProcessed = true;

    CreatureBonusLootProcessor bonusLoot(me);
    bonusLoot.SendBonusLootToPlayers();
}

bool BossAI::CheckBoundary(Unit* who)
{
    if (!GetBoundary() || !who)
        return true;

    for (BossBoundaryMap::const_iterator itr = GetBoundary()->begin(); itr != GetBoundary()->end(); ++itr)
    {
        switch (itr->first)
        {
            case BOUNDARY_N:
                if (me->GetPositionX() > itr->second)
                    return false;
                break;
            case BOUNDARY_S:
                if (me->GetPositionX() < itr->second)
                    return false;
                break;
            case BOUNDARY_E:
                if (me->GetPositionY() < itr->second)
                    return false;
                break;
            case BOUNDARY_W:
                if (me->GetPositionY() > itr->second)
                    return false;
                break;
            case BOUNDARY_NW:
                if (me->GetPositionX() + me->GetPositionY() > itr->second)
                    return false;
                break;
            case BOUNDARY_SE:
                if (me->GetPositionX() + me->GetPositionY() < itr->second)
                    return false;
                break;
            case BOUNDARY_NE:
                if (me->GetPositionX() - me->GetPositionY() > itr->second)
                    return false;
                break;
            case BOUNDARY_SW:
                if (me->GetPositionX() - me->GetPositionY() < itr->second)
                    return false;
                break;
            default:
                break;
        }
    }

    return true;
}

void BossAI::JustSummoned(Creature* summon)
{
    summons.Summon(summon);
    if (me->isInCombat())
        DoZoneInCombat(summon);
}

void BossAI::SummonedCreatureDespawn(Creature* summon)
{
    summons.Despawn(summon);
}

void BossAI::UpdateAI(uint32 const diff)
{
    if (!UpdateVictim())
        return;

    events.Update(diff);

    if (me->HasUnitState(UNIT_STATE_CASTING))
        return;

    while (uint32 eventId = events.ExecuteEvent())
        ExecuteEvent(eventId);

    DoMeleeAttackIfReady();
}

// WorldBossAI - for non-instanced bosses

WorldBossAI::WorldBossAI(Creature* creature) :
    ScriptedAI(creature),
    summons(creature)
{
}

void WorldBossAI::_Reset()
{
    if (!me->isAlive())
        return;

    events.Reset();
    summons.DespawnAll();
}

void WorldBossAI::_JustDied()
{
    events.Reset();
    summons.DespawnAll();
}

void WorldBossAI::_EnterCombat()
{
    Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true);
    if (target)
        AttackStart(target);
}

void WorldBossAI::JustSummoned(Creature* summon)
{
    summons.Summon(summon);
    Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true);
    if (target)
        summon->AI()->AttackStart(target);
}

void WorldBossAI::SummonedCreatureDespawn(Creature* summon)
{
    summons.Despawn(summon);
}

void WorldBossAI::UpdateAI(uint32 const diff)
{
    if (!UpdateVictim())
        return;

    events.Update(diff);

    if (me->HasUnitState(UNIT_STATE_CASTING))
        return;

    while (uint32 eventId = events.ExecuteEvent())
        ExecuteEvent(eventId);

    DoMeleeAttackIfReady();
}

void SceneHelper::SceneActionTalk::DoAction()
{
    if (Creature* pActor = GetActor())
    {
        pActor->AI()->Talk(m_TalkIndex);
    }
}

void SceneHelper::SceneActionMove::DoAction()
{
    Creature* pActor = GetActor();
    if (!pActor)
        return;

    pActor->GetMotionMaster()->MovementExpired(false);

    if (m_IsManyPoints)
    {
        Movement::MoveSplineInit init(*pActor);

        for (std::vector<Position>::const_iterator itr = m_MovePositions.begin(); itr != m_MovePositions.end(); ++itr)
        {
            G3D::Vector3 point;
            point.x = itr->GetPositionX();
            point.y = itr->GetPositionY();
            point.z = itr->GetPositionZ();
            init.Path().push_back(point);
        }

        init.SetWalk(m_IsWalk);
        init.Launch();
    }
    else
    {
        Movement::MoveSplineInit init(*pActor);
        init.MoveTo(m_MovePosition.GetPositionX(), m_MovePosition.GetPositionY(), m_MovePosition.GetPositionZ());
        init.SetWalk(m_IsWalk);
        init.Launch();
    }
}

void SceneHelper::SceneActionSummon::DoAction()
{
    if (!GetActor())
        return;

    if (Map* map = GetActionMap())
    {
        map->SummonCreature(m_SummonEntry, m_SummonPosition);
    }    
}

void SceneHelper::SceneActionCast::DoAction()
{
    if (Creature* pActor = GetActor())
    {
        pActor->CastSpell(pActor, m_SpellEntry);
    } 
}

void SceneHelper::Update(const uint32 diff)
{
    if (IsStopped())
        return;

    if (m_SceneActions.empty())
    {
        Stop();
        return;
    }

    if (m_SceneTimer <= diff)
    {
        if (SceneAction* action = GetCurrentAction())
        {
            action->DoAction();

            delete action;
            m_SceneActions.pop_front();
        }

        if (SceneAction* nextAction = GetCurrentAction())
        {
            m_SceneTimer = nextAction->GetActionTimer();
        }
        else
        {
            Stop();
            return;
        }
    }
    else
    {
        m_SceneTimer -= diff;
    }
}

// SD2 grid searchers.
Creature* GetClosestCreatureWithEntry(WorldObject* source, uint32 entry, float maxSearchRange, bool alive /*= true*/)
{
    return source->FindNearestCreature(entry, maxSearchRange, alive);
}

GameObject* GetClosestGameObjectWithEntry(WorldObject* source, uint32 entry, float maxSearchRange)
{
    return source->FindNearestGameObject(entry, maxSearchRange);
}

void GetCreatureListWithEntryInGrid(std::list<Creature*>& list, WorldObject* source, uint32 entry, float maxSearchRange)
{
    source->GetCreatureListWithEntryInGrid(list, entry, maxSearchRange);
}

void GetGameObjectListWithEntryInGrid(std::list<GameObject*>& list, WorldObject* source, uint32 entry, float maxSearchRange)
{
    source->GetGameObjectListWithEntryInGrid(list, entry, maxSearchRange);
}

void GetPlayerListInGrid(std::list<Player*>& list, WorldObject* source, float maxSearchRange)
{
    source->GetPlayerListInGrid(list, maxSearchRange);
}

void GetPositionWithDistInOrientation(Unit* pUnit, float dist, float orientation, float& x, float& y)
{
    x = pUnit->GetPositionX() + (dist * cos(orientation));
    y = pUnit->GetPositionY() + (dist * sin(orientation));
}

void GetRandPosFromCenterInDist(float centerX, float centerY, float dist, float& x, float& y)
{
    float randOrientation = frand(0, 2*M_PI);

    x = centerX + (dist * cos(randOrientation));
    y = centerY + (dist * sin(randOrientation));
}
