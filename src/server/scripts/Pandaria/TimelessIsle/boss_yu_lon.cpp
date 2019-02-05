/*
	* Copyright © 2016 Emiliyan Kurtseliyanski a.k.a JunkyBulgaria
	* ------------------------------------------------------------
	* Do whatever you want with this code but DO NOT remove the
	* copyright and respect it's authors.
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "MoveSplineInit.h"
#include "timeless_isle.h"

#define BOSS_YU_LON 0

#define INITIAL_HEALTH_POINTS 92300000
#define MAX_HEALTH_POINTS 371000000
#define INCREMENTAL 3000000
#define WALLS_MAX 6
#define WALLS_Y_M 145

enum eSpells
{
    SPELL_JADEFIRE_BUFFET      = 144630,
    SPELL_JADEFIRE_WALL_VISUAL = 144533,
    SPELL_JADEFIRE_WALL_DMG    = 144539,
    SPELL_JADEFIRE_BLAZE_BOLT  = 144532
};

enum eEvents
{
    EVENT_TIMER_HEALTH_POOL = 1,
    EVENT_TIMER_JADE_BREATH,
    EVENT_TIMER_JADEFIRE_WALL,
    EVENT_TIMER_JADEFIRE_BOLT,
    EVENT_TIMER_JADEFIRE_BUFFET,
    EVENT_TIMER_SHAO_DO_INTRO,
    EVENT_TIMER_SHAO_DO_INTRO_ATTACKABLE,
    EVENT_TIMER_SHAO_DO_OUTRO,
    EVENT_TIMER_DEATH
};

enum eCreatures
{
    MOB_JADEFIRE_WALL   = 72020,
    MOB_JADEFIRE_BLAZE  = 72016
};

enum eActions
{
    ACTION_JADEFIRE_WALL = 1,
    ACTION_JADEFIRE_BLAZE = 2
};

enum wallEvent
{
    WALL_EVENT_TIMER_DESPAWN = 1,
    WALL_EVENT_TIMER_GO,
    WALL_EVENT_TIMER_DAMAGE
};

enum wallAction
{
    WALL_ACTION_GO = 1,
    WALL_ACTION_RETURN
};

enum wallMovement
{
    WALL_MOVEMENT_FORWARD = 1
};

Position wallStartPos = { -561.17f, -5091.70f, -6.277f, MIDDLE_FACING_ANGLE };

class PlayerCheck
{
public:
    bool operator()(Unit* target) const
    {
        return !target->ToPlayer();
    }
};

class boss_yulon : public CreatureScript
{
public:
    boss_yulon() : CreatureScript("boss_yulon") { }

    struct boss_yulonAI : public ScriptedAI
    {
        boss_yulonAI(Creature* creature) : ScriptedAI(creature), summons(me) {}

        EventMap events;
        SummonList summons;
        bool EventProgress;
        bool wallrand;

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
            events.ScheduleEvent(EVENT_JADEFLAME_BUFFET, 20000);
            events.ScheduleEvent(EVENT_JADEFIRE_BREATH, 4000);
            events.ScheduleEvent(EVENT_JADEFIRE_BOLT, 21000);
            events.ScheduleEvent(EVENT_FIRE_WALL, 46000);
        }

        void EnterEvadeMode()
        {
            events.Reset();
            summons.DespawnAll();
            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());

            if (EventProgress)
            {
                events.ScheduleEvent(2, 5500);
                if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                    if (Creature* Shao = me->ToTempSummon()->GetSummoner()->ToCreature())
                        Shao->AI()->DoAction(ACTION_YULON_FAIL);
            }
            else
                events.ScheduleEvent(2, 5000);

            ScriptedAI::EnterEvadeMode();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->ToPlayer())
                Talk(SAY_YULON_PLAYER_DEATH);
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
                    Talk(SAY_YULON_END);
                    if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                        if (Creature* Shao = me->ToTempSummon()->GetSummoner()->ToCreature())
                            Shao->AI()->DoAction(ACTION_YULON_END);
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

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);

            if (summon->GetEntry() == NPC_JADEFIRE_BLAZE)
                summon->AI()->DoCast(SPELL_JADEFIRE_BLAZE);

            if (summon->GetEntry() == NPC_JADEFIRE_WALL)
            {
                summon->AI()->DoCast(SPELL_JADEFIRE_WALL);
                float x, y, z;
                summon->GetClosePoint(x, y, z, me->GetObjectSize(), 150.0f);
                summon->GetMotionMaster()->MovePoint(1, x, y, z);
                summon->DespawnOrUnsummon(15000);
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
                    case EVENT_JADEFLAME_BUFFET:
                        DoCast(SPELL_JADEFLAME_BUFFET);
                        events.ScheduleEvent(EVENT_JADEFLAME_BUFFET, 20000);
                        break;
                    case EVENT_JADEFIRE_BREATH:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_JADEFIRE_BREATH);
                        events.ScheduleEvent(EVENT_JADEFIRE_BREATH, 18000);
                        break;
                    case EVENT_JADEFIRE_BOLT:
                        DoCast(SPELL_JADEFIRE_BOLT);
                        events.ScheduleEvent(EVENT_JADEFIRE_BOLT, 18000);
                        break;
                    case EVENT_FIRE_WALL:
                    {
                        Talk(urand(SAY_YULON_WALL_1, SAY_YULON_WALL_2));
                        bool wall = true;
                        for(uint8 i = 0; i < 7; i++)
                        {
                            if (wall && i != 7 && roll_chance_i(15))
                                wall = false;
                            else
                                me->SummonCreature(72020, WallPos[i]);
                        }
                        events.ScheduleEvent(EVENT_FIRE_WALL, 60000);
                        break;
                    }
                    case 2:
                        DoCast(SPELL_CELESTIAL_SPAWN);
                        me->NearTeleportTo(summonPos[2].GetPositionX(), summonPos[2].GetPositionY(), summonPos[2].GetPositionZ(), summonPos[2].GetOrientation());
                        me->SetHomePosition(summonPos[2]);
                        break;
                }
            }          
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_yulonAI (creature);
    }
};

// Jadefire Wall
class mob_jadefire_wall : public CreatureScript
{
    public:
        mob_jadefire_wall() : CreatureScript("mob_jadefire_wall") { }

        struct mob_jadefire_wallAI : public ScriptedAI
        {
            mob_jadefire_wallAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
                me->setActive(true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
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

                if (point == WALL_MOVEMENT_FORWARD)
                    DoAction(WALL_ACTION_RETURN);
            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case WALL_ACTION_GO:
                    {
                        events.ScheduleEvent(WALL_EVENT_TIMER_GO, 1500);
                        break;
                    }
                    case WALL_ACTION_RETURN:
                    {
                        events.ScheduleEvent(WALL_EVENT_TIMER_DESPAWN, 5000);
                        hitPlayers.clear();
                        me->RemoveAura(SPELL_JADEFIRE_WALL_VISUAL);
                        shouldTele = true;
                        break;
                    }
                    default:
                        break;
                };
            };

            void UpdateAI(const uint32 diff)
            {
                events.Update(diff);
                switch (events.ExecuteEvent())
                {
                    case WALL_EVENT_TIMER_GO:
                    {
                        me->GetMotionMaster()->MovePoint(WALL_MOVEMENT_FORWARD, me->GetPositionX(), me->GetPositionY() + WALLS_Y_M, me->GetPositionZ());
                        events.ScheduleEvent(WALL_EVENT_TIMER_DAMAGE, 500);
                        break;
                    }
                    case WALL_EVENT_TIMER_DESPAWN:
                    {
                        if (!shouldTele)
                            return;

                        //me->Respawn();
                        Position home = me->GetHomePosition();
                        me->NearTeleportTo(home.GetPositionX(), home.GetPositionY(), home.GetPositionZ(), home.GetOrientation());
                        shouldTele = false;
                        break;
                    }
                    case WALL_EVENT_TIMER_DAMAGE:
                    {
                        if (!me->HasAura(SPELL_JADEFIRE_WALL_VISUAL))
                            return;

                        std::list<Player*> plrs;
                        me->GetPlayerListInGrid(plrs, 15.0f);
                        for (auto plr : hitPlayers)
                            plrs.remove(plr);

                        for (auto plr : plrs)
                        {
                            if (!plr->isInFront(me))
                                continue;

                            plr->CastSpell(plr, SPELL_JADEFIRE_WALL_DMG, true, NULL, NULL, me->GetGUID());
                            hitPlayers.push_back(plr);
                        }

                        events.ScheduleEvent(WALL_EVENT_TIMER_DAMAGE, 500);
                    }
                    default: break;
                }
            }

            private:
                bool shouldTele = false;
                EventMap events;
                std::list<Player*> hitPlayers;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_jadefire_wallAI(creature);
        }
};

void AddSC_boss_yu_lon()
{
    new mob_jadefire_wall();
    new boss_yulon();
}
