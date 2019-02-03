#include "ScriptMgr.h"
#include "SpellScript.h"
#include "ScriptedCreature.h"

enum Spells
{
    SPELL_INFERNO_CHARGE           = 119405,
    SPELL_INFERNO_CHARGE_TRIGGERED = 119299,
    SPELL_FLAME_BREATH             = 119420
};

enum Events
{
    EVENT_FLAME_BREATH             = 1,
    EVENT_INFERNO                  = 2
};

class boss_adarogg : public CreatureScript
{
    public:
        boss_adarogg() : CreatureScript("boss_adarogg") { }

        struct boss_adaroggAI : public ScriptedAI
        {
            boss_adaroggAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset() { }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_FLAME_BREATH, 5 * IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_INFERNO, 15 * IN_MILLISECONDS);
            }

            void JustDied(Unit* /*killer*/) { }

            void UpdateAI(uint32 const diff)
            {
                if(!UpdateVictim())
                    return;

                events.Update(diff);

                if(me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_FLAME_BREATH:
                            DoCastVictim(SPELL_FLAME_BREATH);
                            events.ScheduleEvent(EVENT_FLAME_BREATH, 10 * IN_MILLISECONDS);
                            break;
                        case EVENT_INFERNO:
                            DoCastRandom(SPELL_INFERNO_CHARGE, 100);
                            events.ScheduleEvent(EVENT_INFERNO, 18 * IN_MILLISECONDS);
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_adaroggAI(creature);
        }
};

class spell_inferno_charge : public SpellScriptLoader
{
    public:
        spell_inferno_charge() : SpellScriptLoader("spell_inferno_charge") { }

        class spell_inferno_charge_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_inferno_charge_AuraScript);

            void HandleTriggerSpell(AuraEffect const* aurEff)
            {
                PreventDefaultAction();
                Unit* caster = GetCaster();
                caster->CastSpell(caster, SPELL_INFERNO_CHARGE_TRIGGERED, false);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_inferno_charge_AuraScript::HandleTriggerSpell, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_inferno_charge_AuraScript();
        }
};

void AddSC_boss_adarogg()
{
    new boss_adarogg();
    new spell_inferno_charge();
}
