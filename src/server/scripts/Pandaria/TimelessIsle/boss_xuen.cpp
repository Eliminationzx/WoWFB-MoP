/*
    * Copyright © 2016 Emiliyan Kurtseliyanski a.k.a JunkyBulgaria
    * ------------------------------------------------------------
    * Do whatever you want with this code but DO NOT remove the
    * copyright and respect it's authors.
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ObjectMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "MoveSplineInit.h"
#include "ScriptMgr.h"
#include "timeless_isle.h"

#define MAX_HEALTH 360000000
#define INITIAL_HEALTH_POINTS 93100000
#define MAX_HEALTH_POINTS 392000000
#define INCREMENTAL 3000000

#define BOSS_XUEN 0

class PlayerCheck
{
public:
    bool operator()(Unit* target) const
    {
        return !target->ToPlayer();
    }
};

class boss_xuen : public CreatureScript
{
public:
    boss_xuen() : CreatureScript("boss_xuen") { }

    struct boss_xuenAI : public ScriptedAI
    {
        boss_xuenAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;
        bool EventProgress;

        void Reset() 
        {
            EventProgress = false;
            me->setActive(true);
            me->setFaction(35);
            me->SetWalk(true);
            me->RemoveAllAuras();
        }

        void EnterCombat(Unit* who)
        {
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_CHI_BARRAGE, 18000);
            events.ScheduleEvent(EVENT_LIGHTNING, 36000);
            events.ScheduleEvent(EVENT_LEAP, 12000);
            events.ScheduleEvent(EVENT_SPECTRAL_SWIPE, 8000);
            events.ScheduleEvent(EVENT_AGILITY, 28000);
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
                        Shao->AI()->DoAction(ACTION_XUEN_FAIL);
            }
            else
                events.ScheduleEvent(2, 5000);

            ScriptedAI::EnterEvadeMode();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->ToPlayer())
                Talk(SAY_XUEN_PLAYER_DEATH);
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
            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (EventProgress)
                {
                    EventProgress = false;
                    QuestCredit();
                    me->setFaction(35);
                    me->RemoveAllAuras();
                    Talk(SAY_XUEN_END);
                    if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                        if (Creature* Shao = me->ToTempSummon()->GetSummoner()->ToCreature())
                            Shao->AI()->DoAction(ACTION_XUEN_END);
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

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case 1:
                        me->setFaction(190);
                        break;
                    case EVENT_CHI_BARRAGE:
                        Talk(SAY_XUEN_BARRAGE);
                        DoCast(SPELL_CHI_BARRAGE);
                        events.ScheduleEvent(EVENT_CHI_BARRAGE, 34000);
                        break;
                    case EVENT_LIGHTNING:
                        Talk(SAY_XUEN_LIGHTNING);
                        DoCast(SPELL_CRACKLING_LIGHTNING);
                        events.ScheduleEvent(EVENT_LIGHTNING, 46000);
                        break;
                    case EVENT_LEAP:
                        if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 70.0f, true))
                            DoCast(pTarget, SPELL_LEAP);
                        events.ScheduleEvent(EVENT_LEAP, 48000);
                        break;
                    case EVENT_SPECTRAL_SWIPE:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_SPECTRAL_SWIPE);
                        events.ScheduleEvent(EVENT_SPECTRAL_SWIPE, 14000);
                        break;
                    case EVENT_AGILITY:
                        Talk(SAY_XUEN_AGGILITY);
                        DoCast(SPELL_AGILITY);
                        events.ScheduleEvent(EVENT_AGILITY, 60000);
                        break;
                    case 2:
                        DoCast(SPELL_CELESTIAL_SPAWN);
                        me->NearTeleportTo(summonPos[1].GetPositionX(), summonPos[1].GetPositionY(), summonPos[1].GetPositionZ(), summonPos[1].GetOrientation());
                        me->SetHomePosition(summonPos[1]);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_xuenAI (creature);
    }
};

// Chi Barrage - 144642
class spell_xuen_ti_chi_barrage : public SpellScriptLoader
{
    public:
        spell_xuen_ti_chi_barrage() : SpellScriptLoader("spell_xuen_ti_chi_barrage") { }

        class spell_xuen_ti_chi_barrage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_xuen_ti_chi_barrage_SpellScript);

            void HandleDummy(SpellEffIndex effIndex)
            {
                if (Unit* target = GetHitUnit())
                    if (target->GetTypeId() == TYPEID_PLAYER)
                        GetCaster()->CastSpell(target, GetSpellInfo()->Effects[effIndex].BasePoints, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_xuen_ti_chi_barrage_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_xuen_ti_chi_barrage_SpellScript();
        }
};

// Crackling Lightning - 144634
class spell_xuen_ti_crackling_lightning : public SpellScriptLoader
{
    public:
        spell_xuen_ti_crackling_lightning() : SpellScriptLoader("spell_xuen_ti_crackling_lightning") { }

        class spell_xuen_ti_crackling_lightning_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_xuen_ti_crackling_lightning_SpellScript);

            void HandleChain(SpellEffIndex effIndex)
            {
                if (Unit* target = GetHitUnit())
                    if (target->GetTypeId() == TYPEID_PLAYER)
                        GetCaster()->CastSpell(target, GetSpellInfo()->Effects[effIndex].BasePoints, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_xuen_ti_crackling_lightning_SpellScript::HandleChain, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_xuen_ti_crackling_lightning_SpellScript();
        }
};

void AddSC_boss_xuen()
{
    new spell_xuen_ti_crackling_lightning();
    new spell_xuen_ti_chi_barrage();
    new boss_xuen();
}
