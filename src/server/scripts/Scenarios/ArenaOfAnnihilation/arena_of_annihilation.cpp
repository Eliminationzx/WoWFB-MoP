#include "arena_of_annihilation.h"

enum Spells
{
    // Scar-shell
    SPELL_CRUSHING_BITE = 123918,
    SPELL_STONE_SPIN    = 123928,

    // Jol'grum
    SPELL_ILL_TEMPERED  = 123933,
    SPELL_HEADBUTT      = 123931,
    SPELL_SLAM          = 123934,

	// Little Liu'yang
	SPELL_FLAME_WALL    = 123966,
	SPELL_LAVA_POOL     = 123967, // Flame wall damage
};

enum Events
{
    // Scar-shell
    EVENT_CRUSHING_BITE = 1,
    EVENT_STONE_SPIN    = 2,

    // Jol'grum
    EVENT_HEADBUTT      = 3,
    EVENT_SLAM          = 4,
};

class npc_scarshell : public CreatureScript
{
    public:
        npc_scarshell() : CreatureScript("npc_scarshell") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_scarshell_AI(pCreature);
        }

        struct npc_scarshell_AI : public ScriptedAI
        {
            npc_scarshell_AI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = me->GetInstanceScript();
                me->SetPhaseMask(2, true);
            }

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                events.ScheduleEvent(EVENT_CRUSHING_BITE, urand(7 * IN_MILLISECONDS, 10 * IN_MILLISECONDS));
                events.ScheduleEvent(EVENT_STONE_SPIN, 30 * IN_MILLISECONDS);
            }

            void JustDied(Unit* killer)
            {
                events.Reset();
                if (pInstance)
                {
                    pInstance->SetData(DATA_SCARSHELL, DONE);
                    if (GameObject* gob = pInstance->instance->GetGameObject(pInstance->GetData(DATA_GONG)))
                        gob->SetGoState(GOState::GO_STATE_ACTIVE);
                }
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CRUSHING_BITE:
                            DoCastVictim(SPELL_CRUSHING_BITE);
                            events.ScheduleEvent(EVENT_CRUSHING_BITE, urand(7 * IN_MILLISECONDS, 10 * IN_MILLISECONDS));
                            break;
                        // Needs waypoints for cricle
                        case EVENT_STONE_SPIN:
                            DoCast(SPELL_STONE_SPIN);
                            events.ScheduleEvent(EVENT_STONE_SPIN, 30 * IN_MILLISECONDS);
                            break;
                        default:
                            break;
                    }
                }
            }

            private:
                InstanceScript* pInstance;
        };
};

class npc_jolgrum : public CreatureScript
{
    public:
        npc_jolgrum() : CreatureScript("npc_jolgrum") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_jolgrum_AI(pCreature);
        }

        struct npc_jolgrum_AI : public ScriptedAI
        {
            npc_jolgrum_AI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = me->GetInstanceScript();
                me->SetPhaseMask(2, true);
            }

            void Reset()
            {
                events.Reset();
                if (!me->HasAura(SPELL_ILL_TEMPERED))
                    me->AddAura(SPELL_ILL_TEMPERED, me);
            }

            void EnterCombat(Unit* who)
            {
                events.ScheduleEvent(EVENT_HEADBUTT, 8 * IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_SLAM, 14 * IN_MILLISECONDS);
            }

            void JustDied(Unit* killer)
            {
                events.Reset();
                if (pInstance)
                {
                    pInstance->SetData(DATA_JOLGRUM, DONE);
                    if (GameObject* gob = pInstance->instance->GetGameObject(pInstance->GetData(DATA_GONG)))
                        gob->SetGoState(GOState::GO_STATE_ACTIVE);
                }

                if (me->HasAura(SPELL_ILL_TEMPERED))
                    me->RemoveAura(SPELL_ILL_TEMPERED);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_HEADBUTT:
                            DoCastVictim(SPELL_HEADBUTT);
                            events.ScheduleEvent(EVENT_HEADBUTT, 8 * IN_MILLISECONDS);
                            break;
                        case EVENT_SLAM:
                            DoCastAOE(SPELL_SLAM);
                            events.ScheduleEvent(EVENT_SLAM, 14 * IN_MILLISECONDS);
                        default:
                            break;
                    }
                }
            }

            private:
                InstanceScript* pInstance;
        };
};

class npc_little_liuyang : public CreatureScript
{
    public:
        npc_little_liuyang() : CreatureScript("npc_little_liuyang") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_little_liuyang_AI(pCreature);
        }

        struct npc_little_liuyang_AI : public ScriptedAI
        {
            npc_little_liuyang_AI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = me->GetInstanceScript();
                me->SetPhaseMask(2, true);
            }

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {

            }

            void JustDied(Unit* killer)
            {
                events.Reset();
                if (pInstance)
                {
                    pInstance->SetData(DATA_LITTLE_LIUYANG, DONE);
                    if (GameObject* gob = pInstance->instance->GetGameObject(pInstance->GetData(DATA_GONG)))
                        gob->SetGoState(GOState::GO_STATE_ACTIVE);
                }
            }

			void DamageTaken(Unit* attacker, uint32& damage)
			{
				if (attacker->GetTypeId() != TYPEID_PLAYER)
					return;

				// If boss is casting flame wall he reflects 50% of the dmg taken back
				if (me->HasAura(SPELL_FLAME_WALL))
					me->DealDamage(attacker, damage / 2);
			}

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    /*switch (eventId)
                    {

                    }*/
                }
            }

            private:
                InstanceScript* pInstance;
        };
};

class npc_flamecoaxing_spirit : public CreatureScript
{
    public:
        npc_flamecoaxing_spirit() : CreatureScript("npc_flamecoaxing_spirit") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_flamecoaxing_spirit_AI(pCreature);
        }

        struct npc_flamecoaxing_spirit_AI : public ScriptedAI
        {
            npc_flamecoaxing_spirit_AI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = me->GetInstanceScript();
            }

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {

            }

            void JustDied(Unit* killer)
            {
                events.Reset();
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);
            }

            private:
                InstanceScript* pInstance;
        };
};

class npc_chagan_firehoof : public CreatureScript
{
    public:
        npc_chagan_firehoof() : CreatureScript("npc_chagan_firehoof") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_chagan_firehoof_AI(pCreature);
        }

        struct npc_chagan_firehoof_AI : public ScriptedAI
        {
            npc_chagan_firehoof_AI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = me->GetInstanceScript();
                me->SetPhaseMask(2, true);
            }

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {

            }

            void JustDied(Unit* killer)
            {
                events.Reset();
                if (pInstance)
                {
                    pInstance->SetData(DATA_CHAGAN_FIREHOOF, DONE);
                    if (GameObject* gob = pInstance->instance->GetGameObject(pInstance->GetData(DATA_GONG)))
                        gob->SetGoState(GOState::GO_STATE_ACTIVE);
                }
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    /*switch (eventId)
                    {

                    }*/
                }
            }

            private:
                InstanceScript* pInstance;
        };
};

class npc_batu : public CreatureScript
{
    public:
        npc_batu() : CreatureScript("npc_batu") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_batu_AI(pCreature);
        }

        struct npc_batu_AI : public ScriptedAI
        {
            npc_batu_AI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = me->GetInstanceScript();
                me->SetPhaseMask(2, true);
            }

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {

            }

            void JustDied(Unit* killer)
            {
                events.Reset();
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    /*switch (eventId)
                    {

                    }*/
                }
            }

            private:
                InstanceScript* pInstance;
        };
};

class npc_maki_waterblade : public CreatureScript
{
    public:
        npc_maki_waterblade() : CreatureScript("npc_maki_waterblade") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_maki_waterblade_AI(pCreature);
        }

        struct npc_maki_waterblade_AI : public ScriptedAI
        {
            npc_maki_waterblade_AI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = me->GetInstanceScript();
                me->SetPhaseMask(2, true);
            }

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {

            }

            void JustDied(Unit* killer)
            {
                events.Reset();
                if (pInstance)
                    pInstance->SetData(DATA_MAKI_WATERBLADE, DONE);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    /*switch (eventId)
                    {

                    }*/
                }
            }

            private:
                InstanceScript* pInstance;
        };
};

class npc_satay_byu : public CreatureScript
{
    public:
        npc_satay_byu() : CreatureScript("npc_satay_byu") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_satay_byu_AI(pCreature);
        }

        struct npc_satay_byu_AI : public ScriptedAI
        {
            npc_satay_byu_AI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = me->GetInstanceScript();
                me->SetPhaseMask(2, true);
            }

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {

            }

            void JustDied(Unit* killer)
            {
                events.Reset();
                if (pInstance)
                    pInstance->SetData(DATA_SATAY_BYU, DONE);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    /*switch (eventId)
                    {

                    }*/
                }
            }

            private:
                InstanceScript* pInstance;
        };
};

class npc_cloudbender_kobo : public CreatureScript
{
    public:
        npc_cloudbender_kobo() : CreatureScript("npc_cloudbender_kobo") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_cloudbender_kobo_AI(pCreature);
        }

        struct npc_cloudbender_kobo_AI : public ScriptedAI
        {
            npc_cloudbender_kobo_AI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = me->GetInstanceScript();
                me->SetPhaseMask(2, true);
            }

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {

            }

            void JustDied(Unit* killer)
            {
                events.Reset();
                if (pInstance)
                    pInstance->SetData(DATA_CLOUDBENDER_KOBO, DONE);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    /*switch (eventId)
                    {

                    }*/
                }
            }

            private:
                InstanceScript* pInstance;
        };
};

class go_gong : public GameObjectScript
{
    public:
        go_gong() : GameObjectScript("go_gong") { }

        bool OnGossipHello(Player* /*player*/, GameObject* go)
        {
            InstanceScript* pInstance = go->GetInstanceScript();
            if (pInstance)
                pInstance->SetData(DATA_GONG, DONE);

            go->SetGoState(GOState::GO_STATE_READY);
            return true;
        }
};

void AddSC_arena_of_annihilation()
{
    new npc_scarshell();        // 63311
    new npc_jolgrum();          // 63312
    new npc_little_liuyang();   // 63313
    new npc_chagan_firehoof();  // 63318
    new npc_batu();             // 
    new npc_maki_waterblade();  // 64280
    new npc_satay_byu();        // 64281
    new npc_cloudbender_kobo(); // 63316
    new go_gong();              // 212974
}
