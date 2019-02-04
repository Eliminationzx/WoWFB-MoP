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

enum Spells
{
    SPELL_FIRESTORM					= 144461,
    SPELL_INSPIRING_SONG			= 144468,
    SPELL_BEACON_OF_HOPE			= 144473,
    SPELL_BLAZING_SONG				= 144471,
    SPELL_CRANE_RUSH				= 144470,
    SPELL_FIRESTORM_VISUAL			= 144463,
    SPELL_BLAZING_NOVA_AURA         = 144493,
    SPELL_BLAZING_NOVA				= 144494,
    SPELL_BEACON_OF_HOPE_AURA       = 144475
};

enum Events
{
    EVENT_FIRESTORM					= 1,
    EVENT_INSPIRING_SONG,
    EVENT_BEACON_OF_HOPE,	
    EVENT_HEALTH_66_PERCENT,
    EVENT_HEALTH_33_PERCENT,
    EVENT_SHAO_DO_OUTRO,
    EVENT_DEATH,
    EVENT_SHAO_DO_INTRO,
    EVENT_SHAO_DO_INTRO_ATTACKABLE,
    EVENT_HEALTH_POOL_TIMER
};

enum Says
{
    SAY_AGGRO                       = 0,
    SAY_INTRO                       = 1,
    SAY_DEATH                       = 2,
    SAY_KILL                        = 3,
    SAY_SPELL                       = 4,
    SAY_SPELL_2                     = 5,
    SAY_SPELL_3                     = 6
};

enum Actions
{
    ACTION_CHILDREN_OF_CHIJI_33 = 1,
    ACTION_CHILDREN_OF_CHIJI_66 = 2
};

class boss_chi_ji : public CreatureScript
{
    public:
        boss_chi_ji() : CreatureScript("boss_chi_ji") { }

        struct boss_chi_jiAI : public BossAI
        {
            boss_chi_jiAI(Creature* creature) : BossAI(creature, DATA_CHI_JI) {	}

            void Reset()
            {
                events.Reset();
                _Reset();

                me->setFaction(35);
                summons.DespawnAll();
                me->SetWalk(true);
                me->setActive(true);
            }

            void EnterCombat(Unit* /*target*/) override
            {
                me->SetWalk(false);
                death = false;
                Talk(SAY_AGGRO);
                events.ScheduleEvent(urand(EVENT_FIRESTORM, EVENT_INSPIRING_SONG), urand(10000, 15000));
                events.ScheduleEvent(EVENT_BEACON_OF_HOPE, 50000);
            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case ACTION_CHILDREN_OF_CHIJI_66:
                    {
                        DoCast(SPELL_CRANE_RUSH);
                        Talk(SAY_SPELL_3);
                        break;
                    }
                    case ACTION_CHILDREN_OF_CHIJI_33:
                    {
                        DoCast(SPELL_CRANE_RUSH);
                        Talk(SAY_SPELL_2);
                        break;
                    }
                    default: break;
                };
            }

            void DamageTaken(Unit* caster, uint32 &dmg) override
            {
                if (me->GetHealthPct() > 66.6f)
                {
                    _children66 = false;
                    _children33 = false;
                }

                if (me->GetHealthPct() < 66.6f && !_children66)
                {
                    _children66 = true;
                    events.ScheduleEvent(EVENT_HEALTH_66_PERCENT, 500);
                }

                if (me->GetHealthPct() < 33.3f && !_children33)
                {
                    _children33 = true;
                    events.ScheduleEvent(EVENT_HEALTH_33_PERCENT, 500);
                }

                if (dmg >= me->GetHealth())
                {
                    if (death)
                        return;

                    std::list<HostileReference*>& threatlist = me->getThreatManager().getThreatList();
                    for (auto itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                        if (Unit* unit = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                            if (unit->IsWithinDist(me, 100.0f))
                                if (unit->ToPlayer())
                                    unit->ToPlayer()->KilledMonsterCredit(me->GetEntry(), me->GetGUID());

                    dmg = 0;
                    Talk(SAY_DEATH);
                    
                    me->setFaction(35);
                   
                    me->StopMoving();
                    me->RemoveAllAuras();
                    me->GetMotionMaster()->Clear();
                    me->CombatStop(true);
                    me->SetHealth(me->GetMaxHealth());

                    me->SetFacingTo(MIDDLE_FACING_ANGLE);
                    me->DeleteThreatList();

                    events.Reset();
                    summons.DespawnAll();
                    events.ScheduleEvent(EVENT_SHAO_DO_OUTRO, 20000);
                    events.ScheduleEvent(EVENT_DEATH, 13000);
                    death = true;
                }
            }

            void MovementInform(uint32 type, uint32 point)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (point == 1)
                {
                    events.ScheduleEvent(EVENT_SHAO_DO_INTRO, CELESTIAL_COURT_BOSS_INTRO_TIMER_1);
                    me->SetFacingTo(MIDDLE_FACING_ANGLE);
                    me->setFaction(FACTION_HOSTILE_NEUTRAL);
                    me->SetHomePosition(_timelessIsleMiddle);
                }
            }

            void KilledUnit(Unit* who)
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
                        return;
            }

            void UpdateHealth()
            {
                if (!me->isInCombat())
                    return;

                std::list<HostileReference*>& threatlist = me->getThreatManager().getThreatList();
                if (threatlist.empty())
                    return;

                uint8 count = 0;
                for (auto itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                    if (Unit* unit = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                        if (unit->IsWithinDist(me, 100.0f))
                            count++;

                uint32 hp = me->GetMaxHealth() - me->GetHealth();
                uint32 newhp = std::min<uint32>((INCREMENTAL * count), MAX_HEALTH_POINTS);
                if (newhp != me->GetMaxHealth() && newhp > INITIAL_HEALTH_POINTS)
                {
                    me->SetMaxHealth(std::min<uint32>((me->GetMaxHealth() * count), MAX_HEALTH_POINTS));
                    me->SetHealth(newhp - hp);
                }
            }; 

            void UpdateAI(const uint32 diff)
            {
                events.Update(diff);
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                switch (events.ExecuteEvent())
                {
                    case EVENT_SHAO_DO_INTRO:
                    {
                        Talk(SAY_INTRO);
                        events.ScheduleEvent(EVENT_SHAO_DO_INTRO_ATTACKABLE, 3000);
                        break;
                    }
                    case EVENT_SHAO_DO_INTRO_ATTACKABLE:
                    {
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        me->setFaction(190);
                        me->SetMaxHealth(INITIAL_HEALTH_POINTS);
                        break;
                    }
                    case EVENT_INSPIRING_SONG:
                    {
                        DoCast(me, SPELL_INSPIRING_SONG);
                        events.ScheduleEvent(urand(EVENT_FIRESTORM, EVENT_INSPIRING_SONG), urand(8000, 12000));
                        break;
                    }
                    case EVENT_FIRESTORM:
                    {
                        DoCast(SPELL_FIRESTORM);
                        events.ScheduleEvent(urand(EVENT_FIRESTORM, EVENT_INSPIRING_SONG), urand(8000, 12000));
                        break;
                    }
                    case EVENT_BEACON_OF_HOPE:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 15.0f, true))
                        {
                            DoCast(target, SPELL_BEACON_OF_HOPE, true);

                            DoCast(SPELL_BLAZING_SONG);
                            Talk(SAY_SPELL);
                        }
                        events.ScheduleEvent(EVENT_BEACON_OF_HOPE, 50000);
                        break;
                    }
                    case EVENT_HEALTH_66_PERCENT:
                    {
                        DoAction(ACTION_CHILDREN_OF_CHIJI_66);
                        break;
                    }
                    case EVENT_HEALTH_33_PERCENT:
                    {
                        DoAction(ACTION_CHILDREN_OF_CHIJI_33);
                        break;
                    }
                    case EVENT_SHAO_DO_OUTRO:
                    {
                        if (Creature* shao = me->FindNearestCreature(NPC_EMPEROR_SHAOHAO_TI, 300.0f, true))
                            shao->AI()->Talk(EMPEROR_TALK_OUTRO_CHIJI);
                        break;
                    }
                    case EVENT_DEATH:
                    {
                        if (death)
                        {
                            if (Creature* shao = me->FindNearestCreature(NPC_EMPEROR_SHAOHAO_TI, 300.0f, true))
                                shao->AI()->DoAction(ACTION_NIUZAO);

                            Movement::MoveSplineInit init(*me);
                            Position home = me->GetHomePosition();
                            init.MoveTo(home.GetPositionX(), home.GetPositionY(), home.GetPositionZ());
                            init.SetWalk(true);
                            init.Launch();
                            me->SetOrientation(home.GetOrientation());

                            //me->DisappearAndDie();
                            death = false;
                        }
                        break;
                    }
                    default:
                        break;
                }

                if (!death)
                    if (!UpdateVictim())
                        return;

                DoMeleeAttackIfReady();
            }

            private:
                bool _children66, _children33 = false;
                bool death = false;
                EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_chi_jiAI(creature);
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
    new boss_chi_ji();
}