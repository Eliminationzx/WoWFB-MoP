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
#include "timeless_isle.h"

#define BOSS_NIUZAO 0

#define MAX_HEALTH_POINTS 392500000
#define INITIAL_HEALTH_POINTS 98125000
#define INCREMENTAL 3212500

const float _chargePos[5][3] = 
{
    { -740.79f, -5019.06f, -6.277f },
    { -740.41f, -4962.37f, -6.277f },
    { -563.80f, -4963.35f, -6.277f },
    { -563.88f, -5069.82f, -6.277f },
    { -737.17f, -5072.31f, -6.277f }
};

enum eSpells
{
    SPELL_NIUZAO_CHARGE = 144608,
    SPELL_NIUZAO_HEADBUTT = 144610,
    SPELL_NIUZAO_MASSIVE_QUAKE = 144611,
    SPELL_NIUZAO_OXEN_FORTITUDE = 144606,
    SPELL_NIUZAO_OXEN_FORTITUDE_ACTIVE = 144607
};

enum eEvents
{
    EVENT_NIUZAO_OXEN_FORTITUDE = 1,
    EVENT_NIUZAO_HEADBUTT,
    EVENT_NIUZAO_MASSIVE_QUAKE,
    EVENT_NIUZAO_HEALTH_66_PERCENT,
    EVENT_NIUZAO_HEALTH_33_PERCENT,
    EVENT_NIUZAO_DEFEATED,
    EVENT_NIUZAO_DEATH,
    EVENT_NIUZAO_CHARGE,
    EVENT_NIUZAO_SHAO_DO_INTRO,
    EVENT_NIUZAO_SHAO_DO_INTRO_ATTACKABLE,
    EVENT_NIUZAO_SHAO_DO_OUTRO
};

enum eActions
{
    ACTION_NIUZAO_CHARGE_66 = 1,
    ACTION_NIUZAO_CHARGE_33,
    ACTION_NIUZAO_CHARGE
};

const Position telePos[1] = 
{
    {-551.16f, -5080.01f, -6.27f, 2.28f}
};

enum eMovement
{
    MOVEMENT_NIUZAO_CHARGE = 2
};

class boss_niuzao : public CreatureScript
{
public:
    boss_niuzao() : CreatureScript("boss_niuzao") { }

    struct boss_niuzaoAI : public ScriptedAI
    {
        boss_niuzaoAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;
        bool EventProgress;
        bool bEvent_1;
        bool bEvent_2;
        bool ChargeEvent;

        void Reset() 
        {
            EventProgress = false;
            me->setActive(true);
            bEvent_1 = false;
            bEvent_2 = false;
            ChargeEvent = false;
            me->setFaction(35);
            me->SetWalk(true);
            me->RemoveAllAuras();
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterCombat(Unit* who)
        {
            Talk(SAY_AGGRO);
            me->SetWalk(false);
            events.ScheduleEvent(EVENT_MASSIVE_QUAKE, 45000);
            events.ScheduleEvent(EVENT_OXEN_FORTITUDE, 30000);
            events.ScheduleEvent(EVENT_HEADBUTT, 30000);
        }

        void EnterEvadeMode()
        {
            events.Reset();
            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveIdle();

            if (EventProgress)
            {
                events.ScheduleEvent(2, 5500);
                if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                    if (Creature* Shao = me->ToTempSummon()->GetSummoner()->ToCreature())
                        Shao->AI()->DoAction(ACTION_NIUZAO_FAIL);
            }
            else
                events.ScheduleEvent(2, 5000);

            ScriptedAI::EnterEvadeMode();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->ToPlayer())
                Talk(SAY_NIUZAO_PLAYER_DEATH);
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

        void DamageTaken(Unit* /*who*/, uint32& damage)
        {
            if (me->HealthBelowPct(66) && !bEvent_1 && !ChargeEvent)
            {
                bEvent_1 = true;
                Talk(SAY_NIUZAO_CHARGE_1);
                events.ScheduleEvent(EVENT_MOVE_PATH_1, 0);
            }

            if (me->HealthBelowPct(33) && !bEvent_2 && !ChargeEvent)
            {
                bEvent_2 = true;
                Talk(SAY_NIUZAO_CHARGE_2);
                events.ScheduleEvent(EVENT_MOVE_PATH_1, 0);
            }

            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (EventProgress)
                {
                    EventProgress = false;
                    ChargeEvent = false;
                    QuestCredit();
                    me->setFaction(35);
                    me->RemoveAllAuras();
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveIdle();
                    Talk(SAY_NIUZAO_END);
                    if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                        if (Creature* Shao = me->ToTempSummon()->GetSummoner()->ToCreature())
                            Shao->AI()->DoAction(ACTION_NIUZAO_END);
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
            if (type == POINT_MOTION_TYPE)
            {
                if (id == 1)
                {
                    Talk(SAY_ENTER_POS);
                    me->SetFacingTo(1.5f);
                    events.ScheduleEvent(1, 15000);
                }
                if (id == 2)
                {
                    DoCast(SPELL_CHARGE);
                    events.ScheduleEvent(EVENT_MOVE_PATH_2, 3000);
                }
            }
            if (type == WAYPOINT_MOTION_TYPE)
            {
                if (id == 7)
                {
                    ChargeEvent = false;
                    me->RemoveAura(SPELL_CHARGE);
                    me->SetReactState(REACT_AGGRESSIVE);
                }
            }
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

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            events.Update(diff);
            EnterEvadeIfOutOfCombatArea(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) || ChargeEvent)
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case 1:
                        me->setFaction(190);
                        break;
                    case EVENT_MASSIVE_QUAKE:
                        Talk(SAY_NIUZAO_QUAKE);
                        DoCast(SPELL_MASSIVE_QUAKE);
                        events.ScheduleEvent(EVENT_MASSIVE_QUAKE, 48000);
                        break;
                    case EVENT_OXEN_FORTITUDE:
                        DoCast(SPELL_OXEN_FORTITUDE);
                        events.ScheduleEvent(EVENT_OXEN_FORTITUDE, 46000);
                        break;
                    case EVENT_HEADBUTT:
                        if (Unit* target = me->getVictim())
                        {
                            DoCast(target, SPELL_HEADBUTT);
                            DoModifyThreatPercent(target, -100);
                        }
                        events.ScheduleEvent(EVENT_HEADBUTT, 30000);
                        break;
                    case EVENT_MOVE_PATH_1:
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        me->GetMotionMaster()->MovePoint(2, CentrPos[0]);
                        break;
                    case EVENT_MOVE_PATH_2:
                        me->GetMotionMaster()->MovePath(me->GetEntry() * 100, false);
                        ChargeEvent = true;
                        break;
                    case 2:
                        DoCast(SPELL_CELESTIAL_SPAWN);
                        me->NearTeleportTo(summonPos[3].GetPositionX(), summonPos[3].GetPositionY(), summonPos[3].GetPositionZ(), summonPos[3].GetOrientation());
                        me->SetHomePosition(summonPos[3]);
                        break;
                }
            }
            UpdateHealth();
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_niuzaoAI (creature);
    }
};

// Headbutt - 144610
class spell_niuzao_headbutt : public SpellScriptLoader
{
    public:
        spell_niuzao_headbutt() : SpellScriptLoader("spell_niuzao_headbutt") { }

        class spell_niuzao_headbutt_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_niuzao_headbutt_SpellScript);

            void RemoveThreat(SpellEffIndex /*eff*/)
            {
                if (!GetHitUnit())
                    return;

                if (Unit* target = GetHitUnit())
                    GetCaster()->SetReducedThreatPercent(1000, target->GetGUID());
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_niuzao_headbutt_SpellScript::RemoveThreat, EFFECT_1, SPELL_EFFECT_KNOCK_BACK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_niuzao_headbutt_SpellScript();
        }
};

// Charge - 144608
class spell_niuzao_charge : public SpellScriptLoader
{
    public:
        spell_niuzao_charge() : SpellScriptLoader("spell_niuzao_charge") { }

        class spell_niuzao_charge_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_niuzao_charge_AuraScript);

            void HandleEffectApply(AuraEffect const* pAuraEffect, AuraEffectHandleModes eMode)
            {
                if (!GetOwner())
                    return;

                if (Creature *pOwner = GetOwner()->ToCreature())
                    pOwner->AI()->DoAction(ACTION_NIUZAO_CHARGE);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_niuzao_charge_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_niuzao_charge_AuraScript();
        }
};

void AddSC_boss_niuzao()
{
    new spell_niuzao_headbutt();
    new spell_niuzao_charge();
    new boss_niuzao();
}
