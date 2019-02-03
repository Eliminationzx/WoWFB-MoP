/*
    * Copyright © 2016 Emiliyan Kurtseliyanski a.k.a JunkyBulgaria
    * ------------------------------------------------------------
    * Do whatever you want with this code but DO NOT remove the
    * copyright and respect it's authors.
*/

// Rare Mob: Bufo 72775 (Frog)
// Progress: 100%

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"

enum eSpells
{
    SPELL_TOXIC_SKIN              = 147654,
    SPELL_GULP_FROG_TOXIN         = 147655,
};

enum eEvents
{
    EVENT_TOXIC_SKIN              = 1,
    EVENT_GULP_FROG_TOXIN         = 2,
};

class rare_mob_bufo : public CreatureScript
{
    public:
        rare_mob_bufo() : CreatureScript("rare_mob_bufo") { }

        struct rare_mob_bufoAI : public BossAI
        {
            rare_mob_bufoAI(Creature* creature) : BossAI(creature, 0) { }

            EventMap events;

            void Reset()
            {
                events.Reset();
                _Reset();
            }

            void EnterCombat(Unit* attacker)
            {
                events.ScheduleEvent(EVENT_TOXIC_SKIN, urand(10000, 15000));
                events.ScheduleEvent(EVENT_GULP_FROG_TOXIN, urand(10000, 15000));
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);
                switch (events.ExecuteEvent())
                {
                    case EVENT_TOXIC_SKIN:
                    {
                        me->CastSpell(me->getVictim(), SPELL_TOXIC_SKIN);
                        events.ScheduleEvent(EVENT_TOXIC_SKIN, urand(7500, 15000));
                        break;
                    }
                    case EVENT_GULP_FROG_TOXIN:
                    {
                        me->CastSpell(me->getVictim(), SPELL_GULP_FROG_TOXIN);
                        events.ScheduleEvent(EVENT_GULP_FROG_TOXIN, urand(7500, 15000));
                        break;
                    }
                    default:
                        break;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new rare_mob_bufoAI(creature);
        }
};

void AddSC_rare_mob_bufo()
{
    new rare_mob_bufo();
}
