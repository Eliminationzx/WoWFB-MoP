/*
    * Copyright © 2016 Emiliyan Kurtseliyanski a.k.a JunkyBulgaria
    * ------------------------------------------------------------
    * Do whatever you want with this code but DO NOT remove the
    * copyright and respect it's authors.
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "MoveSplineInit.h"
#include "MapManager.h"
#include "timeless_isle.h"

#define MAX_HEALTH_POINTS 349000000
#define INITIAL_HEALTH_POINTS 87250000
#define INCREMENTAL 2800000

enum nSpells
{
    SPELL_FIRESTORM_VISUAL			= 144463,
    SPELL_BLAZING_NOVA_AURA         = 144493
};

enum nEvents
{
    EVENT_BEACON_OF_HOPE = 1,	
    EVENT_HEALTH_66_PERCENT,
    EVENT_HEALTH_33_PERCENT,
    EVENT_SHAO_DO_OUTRO,
    EVENT_DEATH,
    EVENT_SHAO_DO_INTRO,
    EVENT_SHAO_DO_INTRO_ATTACKABLE,
    EVENT_HEALTH_POOL_TIMER
};

enum mActions
{
    ACTION_CHILDREN_OF_CHIJI_33 = 1,
    ACTION_CHILDREN_OF_CHIJI_66 = 2
};

class boss_chiji : public CreatureScript
{
public:
    boss_chiji() : CreatureScript("boss_chiji") { }

    struct boss_chijiAI : public ScriptedAI
    {
        boss_chijiAI(Creature* creature) : ScriptedAI(creature), summons(me) {}

        EventMap events;
        SummonList summons;
        uint32 orient;

        bool EventProgress;
        bool bEvent_1;
        bool bEvent_2;

        void Reset() 
        {
            bEvent_1 = false;
            bEvent_2 = false;
            EventProgress = false;
            me->setActive(true);
            me->SetWalk(true);
            me->setFaction(35);
            me->RemoveAllAuras();
            summons.DespawnAll();
        }

        void EnterCombat(Unit* who)
        {
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_FIRESTORM, 10000);
            events.ScheduleEvent(EVENT_INSPIRING_SONG, 20000);
            events.ScheduleEvent(EVENT_BEACON, 40000);
            events.ScheduleEvent(EVENT_BLAZING_SONG, 44000);
        }

        void EnterEvadeMode()
        {
            events.Reset();
            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());

            if (EventProgress)
            {
                events.ScheduleEvent(2, 5500);
                if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                    if (Creature* Shao = me->ToTempSummon()->GetSummoner()->ToCreature())
                        Shao->AI()->DoAction(ACTION_CHIJI_FAIL);
            }
            else
                events.ScheduleEvent(2, 5000);

            ScriptedAI::EnterEvadeMode();
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
                case ACTION_MOVE_CENTR_POSS:
                    EventProgress = true;
                    me->SetHomePosition(CentrPos[0]);
                    me->GetMotionMaster()->MovePoint(1, CentrPos[0]);
                    break;
            }
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (me->HealthBelowPct(66) && !bEvent_1)
            {
                bEvent_1 = true;
                Talk(SAY_CHIJI_RUSH_1);
                DoCast(SPELL_CRANE_RUSH);
            }

            if (me->HealthBelowPct(33) && !bEvent_2)
            {
                bEvent_2 = true;
                Talk(SAY_CHIJI_RUSH_2);
                DoCast(SPELL_CRANE_RUSH);
            }

            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (EventProgress)
                {
                    EventProgress = false;
                    QuestCredit();
                    me->setFaction(35);
                    me->RemoveAllAuras();
                    summons.DespawnAll();
                    Talk(SAY_CHIJI_END);
                    if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                        if (Creature* Shao = me->ToTempSummon()->GetSummoner()->ToCreature())
                            Shao->AI()->DoAction(ACTION_CHIJI_END);
                }
                EnterEvadeMode();
            }
        }

        void QuestCredit()
        {
            std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
            for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
            {
                Player *pTarget = Player::GetPlayer(*me, (*itr)->getUnitGuid());
                if (!pTarget)
                    continue;

                pTarget->KilledMonsterCredit(me->GetEntry());
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
            {
                Talk(SAY_ENTER_POS);
                me->SetFacingTo(1.5f);
                events.ScheduleEvent(1, 15000);
            }
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);

            if (summon->GetEntry() == NPC_FIRESTORM)
                summon->AI()->DoCast(SPELL_FIRESTORM_AURA);

            if (summon->GetEntry() == NPC_BEACON_OF_HOPE)
                summon->AI()->DoCast(SPELL_BEACON_OF_HOPE_AURA);

            if (summon->GetEntry() == NPC_CHILD_OF_CHIJI)
            {
                summon->AI()->DoCast(SPELL_BLAZING_NOVA);
                orient = summon->GetOrientation();
                summon->SetOrientation(urand(0, 6));
                float x, y, z;
                summon->GetClosePoint(x, y, z, me->GetObjectSize(), 70.0f);
                summon->GetMotionMaster()->MovePoint(1, x, y, z);
                summon->DespawnOrUnsummon(10000);
            }
        } 

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            EnterEvadeIfOutOfCombatArea(diff);
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) || me->HasAura(SPELL_CRANE_RUSH))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case 1:
                        me->setFaction(190);
                        break;
                    case EVENT_FIRESTORM:
                        DoCast(SPELL_FIRESTORM);
                        events.ScheduleEvent(EVENT_FIRESTORM, 20000);
                        break;
                    case EVENT_INSPIRING_SONG:
                        DoCast(SPELL_INSPIRING_SONG);
                        events.ScheduleEvent(EVENT_INSPIRING_SONG, 28000);
                        break;
                    case EVENT_BEACON:
                        Talk(SAY_CHIJI_BEACON);
                        if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                            DoCast(pTarget, SPELL_BEACON_OF_HOPE);
                        events.ScheduleEvent(EVENT_BEACON, 76000);
                        break;
                    case EVENT_BLAZING_SONG:
                        Talk(SAY_CHIJI_BLAZ_SONG);
                        DoCast(SPELL_BLAZING_SONG);
                        events.ScheduleEvent(EVENT_BLAZING_SONG, 76000);
                        break;
                    case 2:
                        DoCast(SPELL_CELESTIAL_SPAWN);
                        me->NearTeleportTo(summonPos[0].GetPositionX(), summonPos[0].GetPositionY(), summonPos[0].GetPositionZ(), summonPos[0].GetOrientation());
                        me->SetHomePosition(summonPos[0]);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_chijiAI (creature);
    }
};

enum ChildEvents
{
    CHILD_EVENT_RUN = 1
};

enum ChildMovement
{
    CHILD_MOVEMENT = 1
};

enum ChildAction
{
    CHILD_ACTION_RUN_FORWARD = 1
};

class mob_child_of_chi_ji : public CreatureScript
{
    public:
        mob_child_of_chi_ji() : CreatureScript("mob_child_of_chi_ji") { }

        struct mob_child_of_chi_jiAI : public ScriptedAI
        {
            mob_child_of_chi_jiAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
                me->setActive(true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);

                events.ScheduleEvent(CHILD_EVENT_RUN, 1000);
                DoCast(SPELL_BLAZING_NOVA_AURA);
                _runCount = 1;
            }

            void EnterCombat(Unit* target)
            {
            }

            void EnterEvadeMode()
            {
            }

            void MovementInform(uint32 type, uint32 point)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (point == CHILD_MOVEMENT)
                {
                    if (_runCount > 3)
                        me->DisappearAndDie();
                    else
                        events.ScheduleEvent(CHILD_EVENT_RUN, 1000);
                }

            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case CHILD_ACTION_RUN_FORWARD:
                    {
                        Position pos;
                        float x, y;
                        GetPositionWithDistInOrientation(me, 15.0f, me->GetOrientation(), x, y);
                        if (MapManager::IsValidMapCoord(me->GetMapId(), x, y))
                        {
                            if (me->IsWithinLOS(x, y, me->GetPositionZ()))
                            {
                                ++_runCount;
                                pos.Relocate(x, y, me->GetPositionZ());
                                me->GetMotionMaster()->MovePoint(CHILD_MOVEMENT, pos);
                                return;
                            }
                        }

                        me->DisappearAndDie();
                        break;
                    }
                    default:
                        break;
                }
            };

            void UpdateAI(const uint32 diff)
            {
                events.Update(diff);
                switch (events.ExecuteEvent())
                {
                    case CHILD_EVENT_RUN:
                    {
                        DoAction(CHILD_ACTION_RUN_FORWARD);
                        break;
                    }
                    default: break;
                }
            }

            private:
                uint8 _runCount;
                EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_child_of_chi_jiAI(creature);
        }
};

// Blazing Of Hope - 144475
class spell_chi_ji_beacon_of_hope : public SpellScriptLoader
{
    public:
        spell_chi_ji_beacon_of_hope() : SpellScriptLoader("spell_chi_ji_beacon_of_hope") { }

        class spell_chi_ji_beacon_of_hope_Spellcript : public SpellScript
        {
            PrepareSpellScript(spell_chi_ji_beacon_of_hope_Spellcript);

            void DeselectNonPlayer(std::list<WorldObject*>& targets)
            {
                targets.remove(GetCaster());

                for (auto itr : targets)
                {
                    if (itr->GetTypeId() == TYPEID_UNIT)
                    {
                        if (itr->GetEntry() == BOSS_CHI_JI)
                        {
                            targets.remove(itr);
                            continue;
                        }

                        if (Creature* unit = sObjectAccessor->GetCreature(*GetCaster(), itr->ToUnit()->GetOwnerGUID()))
                            if (unit->GetEntry() == BOSS_CHI_JI || unit->GetEntry() == BOSS_CHI_JI)
                                targets.remove(itr);
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_chi_ji_beacon_of_hope_Spellcript::DeselectNonPlayer, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_chi_ji_beacon_of_hope_Spellcript();
        }
};

void AddSC_boss_chi_ji()
{
    new spell_chi_ji_beacon_of_hope();
    new mob_child_of_chi_ji();
    new boss_chiji();
}