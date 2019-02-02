/*
    * Copyright © 2016 Emiliyan Kurtseliyanski a.k.a JunkyBulgaria
    * ------------------------------------------------------------
    * Do whatever you want with this code but DO NOT remove the
    * copyright and respect it's authors.
*/

// Rare Mob: Chelon 72045 (Turtle)
// Progress: 100%

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"

enum eSpells
{
    SPELL_GEYSER         	= 147572,
    SPELL_SHELL_SPIN 		= 147571,
    SPELL_SNAPPING_BITE 	= 147569,
};

enum eEvents
{
    EVENT_GEYSER,
	EVENT_SPIN,
	EVENT_BITE,
};

class rare_mob_chelon : public CreatureScript
{
    public:
        rare_mob_chelon() : CreatureScript("rare_mob_chelon") { }

        struct rare_mob_chelonAI : public BossAI
        {
            rare_mob_chelonAI(Creature* creature) : BossAI(creature, 0) { }

            EventMap events;

            void Reset()
            {
                events.Reset();
                _Reset();
            }

            void EnterCombat(Unit* /*attacker*/)
            {
				events.ScheduleEvent(EVENT_GEYSER, urand(7500, 15000));
				events.ScheduleEvent(EVENT_SPIN, urand(7500, 15000));
				events.ScheduleEvent(EVENT_BITE, urand(7500, 15000));
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
                    case EVENT_GEYSER:
                    {
						me->CastSpell(me->getVictim(), SPELL_GEYSER);
                        events.ScheduleEvent(EVENT_GEYSER, urand(7500, 15000));
                        break;
                    }
                    case EVENT_SPIN:
                    {
						me->CastSpell(me->getVictim(), SPELL_SHELL_SPIN);
                        events.ScheduleEvent(EVENT_SPIN, urand(7500, 15000));
                        break;
                    }
                    case EVENT_BITE:
                    {
						me->CastSpell(me->getVictim(), SPELL_SNAPPING_BITE);
                        events.ScheduleEvent(EVENT_BITE, urand(7500, 15000));
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
            return new rare_mob_chelonAI(creature);
        }
};

void AddSC_rare_mob_chelon()
{
    new rare_mob_chelon();
}
