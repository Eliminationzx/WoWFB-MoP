//////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2015 Millenium-studio SARL
///  All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

# include "stormstout_brewery.hpp"

/*
Intro
    Uncle Gao yells: Yes yes yes! So close! Just a pinch of...oops...
    Uncle Gao yells: Too bitter! Just a drop of honey, and a cane of sugar, and maybe some corn? Yes, more corn!
    Uncle Gao yells: Yes, yes, yes...no, no, no no no! Yes! No! Peppers!
    Uncle Gao yells: Ahh! Help! What is that thing? It doesn't matter, nothing will stop me now, not when I'm so close! Maybe if I don't look at it...?
Outro
    Uncle Gao says: Is it... can it be???
    * Gao runs over and kneels down by Yan-Zhu's remains and speaks reverently. *
    Uncle Gao says: Such harmony of flavor, such heady aroma! It is...the perfect brew!
    Uncle Gao yells: The name of Stormstout will be sung once again throughout the hills!
    * As he proclaims this, Chen runs into the Tasting Room and over to Yan-Zhu. *
    Chen Stormstout says: Uncle Gao, this brewery was left in your care! What have you done?
    Uncle Gao yells: You again! Don't you see? I have made a name for myself at last: I have brewed perfection!
    * Chen is dismayed with Gao's attitude. *
    Chen Stormstout says: At what cost? The Brewery is trashed! Infested!
    Uncle Gao says: Details, details.
    Chen Stormstout says: And there are Virmen in the main store!
    * Gao walks over to stand behind the stove as Chen follows to stand in front of it. *
    Uncle Gao says: Look, "Chen Stormstout", we can't all be heroes, running from our responsibilities, tromping around the Dread Wastes, saving the world. Some of us are "Artists".
    Chen Stormstout says: I think the brewery might be on fire.
    * Although Chen points this out, Gao's tone is dismissive, even bored. *
    Uncle Gao says: Yes, yes. That happens. 
*/

enum Yells
{
    // Intro / Outro.
};

enum Spells
{
    // Boss
    SPELL_BREW_BOLT           = 114548,

    // One of two from each category of the following abilities:

    // Bremastery: Wheat.
    SPELL_BLOAT               = 106546, // (1).
    SPELL_BLOAT_DUMMY         = 114929, // Tooltip says "Can use the Bloat ability."
    SPELL_BLOATED             = 106549, // Player aura triggering 106560 Gushing Brew - damage.
    SPELL_BLACKOUT_BREW       = 106851, // (2).
    SPELL_BLACKOUT_BREW_DUMMY = 114930, // Tooltip says "Can use the Blackout Brew ability."
    SPELL_BLACKOUT_DRUNK      = 106857, // At 10 stacks of SPELL_BLACKOUT_BREW.

    // Bremastery: Ale.
    SPELL_BUBBLE_SHIELD       = 106563, // (1).
    SPELL_BUBBLE_SHIELD_DUMMY = 114931, // Tooltip says "Can use the Bubble Shield ability."
    // Summons multiple NPC_YEASTY_BREW_ALEMENTAL_Y (2).
    SPELL_YEASTY_BREW_DUMMY   = 114932, // Tooltip says "Can summon Yeasty Brew minions."

    // Brewmastery: Stout.
    SPELL_CARBONATION         = 115003, // Triggers 114386 damage (1).
    SPELL_CARBONATION_AURA    = 114386,
    SPELL_CARBONATION_DUMMY   = 114934, // Tooltip says "Can use the Carbonation ability."
    // Summons multiple NPC_FIZZY_BUBBLE (if uses Carbonation).
    SPELL_FIZZY_BUBBLE_VISUAL = 114458, // Dummy visual for NPC.
    SPELL_FIZZY_BUBBLE        = 114459, // Player fly aura on spellclick.
    // Summons multiple NPC_WALL_OF_SUDS (if uses Wall of Suds) (2).
    SPELL_SUDSY               = 114468, // If uses Wall of Suds. Triggers 114470 multiple Jump height at Jumping. Player aura.

    // NPCs
    SPELL_YEASTY_BREW_BOLT    = 116155, // Yeasty Brew Alementals can cast a less powerful version of Brew Bolt, inflicting 9750 to 10250 Frost damage.
    SPELL_YEASTY_SUMMON_VIS   = 116259, // Yeasty Brew Alemental spawn visual.
    SPELL_FERMENT             = 106859, // Channeled beam, triggers 114451 - 1% Hp / Mana restore on target.
    SPELL_WALL_OF_SUDS        = 114467, // Triggers 114466 damage and stun.
    SPELL_WALL_OF_SUDS_DUMMY  = 114933  // Tooltip says "Can summon walls of suds."
};

enum Events
{
    // Boss
    EVENT_BREW_BOLT           = 1,

    // Bremastery: Wheat.
    EVENT_BLOAT,
    EVENT_BLACKOUT_BREW,

    // Bremastery: Ale.
    EVENT_BUBBLE_SHIELD,
    EVENT_SET_BUBBLE_SHIELD_STACKS,
    EVENT_YEASTY_BREW_ELEMENTALS,

    // Brewmastery: Stout.
    EVENT_CARBONATION,
    EVENT_WALL_OF_SUDS,

    // NPCs

    // Yeasty Brew Alemental
    EVENT_YEASTY_BREW_BOLT,
    EVENT_FERMENT
};

enum Abilities
{
    // Bremastery: Wheat.
    ABILITY_BLOAT              = 0,
    ABILITY_BLACKOUT_BREW,

    // Bremastery: Ale.
    ABILITY_BUBBLE_SHIELD,
    ABILITY_YEASTY_BREW_ELEMENTALS,

    // Brewmastery: Stout.
    ABILITY_CARBONATION,
    ABILITY_WALL_OF_SUDS
};

// TODO.
Position SudPositions[2][2] =
{
    {
        // Left side
        {0,0,0,0}, // Start
        {0,0,0,0},  // End
    },
    {
        // Right side
        {0,0,0,0}, // Start
        {0,0,0,0},  // End
    }
};

class boss_yan_zhu_the_uncasked : public CreatureScript
{
    public :
        boss_yan_zhu_the_uncasked() : CreatureScript("boss_yan_zhu_the_uncasked") { }

        struct boss_yan_zhu_the_uncasked_AI : public BossAI
        {
            boss_yan_zhu_the_uncasked_AI(Creature* creature) : BossAI(creature, DATA_YANZHU_THE_UNCASKED_EVENT), summons(me)
            {
                m_Instance = creature->GetInstanceScript();
            }

            InstanceScript* m_Instance;
            EventMap events;
            SummonList summons;
            uint8 m_RandomWheatAbility;
            uint8 m_RandomAleAbility;
            uint8 m_RandomStoutAbility;

            void InitializeAI()
            {
                if (!me->isDead())
                    Reset();
            }

            void Reset()
            {
                events.Reset();
                summons.DespawnAll();

                if (m_Instance)
                {
                    m_Instance->SetBossState(DATA_YANZHU_THE_UNCASKED_EVENT, NOT_STARTED);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BREW_BOLT);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLOAT);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLOATED);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CARBONATION_AURA);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLACKOUT_BREW);
                }

                m_RandomWheatAbility = RAND(ABILITY_BLOAT, ABILITY_BLACKOUT_BREW);
                m_RandomAleAbility   = RAND(ABILITY_BUBBLE_SHIELD, ABILITY_YEASTY_BREW_ELEMENTALS);
                m_RandomStoutAbility = RAND(ABILITY_CARBONATION, ABILITY_WALL_OF_SUDS);

                _Reset();
            }

            void EnterCombat(Unit* /*who*/)
			{
                if (m_Instance)
                {
                    m_Instance->SetBossState(DATA_YANZHU_THE_UNCASKED_EVENT, IN_PROGRESS);
                    m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me); // Add
                }

                // Melee range check.
                events.ScheduleEvent(EVENT_BREW_BOLT, 5000);

                // Bremastery: Wheat.
                if (m_RandomWheatAbility == ABILITY_BLOAT)
                {
                    me->AddAura(SPELL_BLOAT_DUMMY, me); // Add the visual tooltip.
                    events.ScheduleEvent(EVENT_BLOAT, urand(11000, 13000));
                }
                else
                {
                    me->AddAura(SPELL_BLACKOUT_BREW_DUMMY, me); // Add the visual tooltip.
                    events.ScheduleEvent(EVENT_BLACKOUT_BREW, urand(9000, 11000));
                }

                // Bremastery: Ale.
                if (m_RandomAleAbility == ABILITY_BUBBLE_SHIELD)
                {
                    me->AddAura(SPELL_BUBBLE_SHIELD_DUMMY, me); // Add the visual tooltip.
                    events.ScheduleEvent(EVENT_BUBBLE_SHIELD, urand(16000, 19000));
                }
                else
                {
                    me->AddAura(SPELL_YEASTY_BREW_DUMMY, me); // Add the visual tooltip.
                    events.ScheduleEvent(EVENT_YEASTY_BREW_ELEMENTALS, urand(18000, 20000));
                }

                // Brewmastery: Stout.
                if (m_RandomStoutAbility == ABILITY_CARBONATION)
                {
                    me->AddAura(SPELL_CARBONATION_DUMMY, me); // Add the visual tooltip.
                    events.ScheduleEvent(EVENT_CARBONATION, urand(22500, 24500));
                }
                else
                {
                    me->AddAura(SPELL_WALL_OF_SUDS_DUMMY, me); // Add the visual tooltip.
                    events.ScheduleEvent(ABILITY_WALL_OF_SUDS, urand(25000, 27000));
                }

                _EnterCombat();
            }

            void EnterEvadeMode()
            {
                Reset();
                me->DeleteThreatList();
                me->CombatStop(false);
                me->GetMotionMaster()->MoveTargetedHome();

                if (m_Instance)
                {
                    m_Instance->SetBossState(DATA_YANZHU_THE_UNCASKED_EVENT, FAIL);
                    m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BREW_BOLT);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLOAT);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLOATED);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CARBONATION_AURA);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLACKOUT_BREW);
                }

                _EnterEvadeMode();
            }

            void JustDied(Unit* /*killer*/)
            {
                summons.DespawnAll();

                if (m_Instance)
                {
                    m_Instance->SetBossState(DATA_YANZHU_THE_UNCASKED_EVENT, DONE);
                    m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BREW_BOLT);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLOAT);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLOATED);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CARBONATION_AURA);
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLACKOUT_BREW);
                }

                _JustDied();
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                summon->setActive(true);

		        if (me->isInCombat())
                    summon->SetInCombatWithZone();

                if (summon->GetEntry() == NPC_FIZZY_BUBBLE)
                {
                    summon->SetReactState(REACT_PASSIVE);
                    summon->AddAura(SPELL_FIZZY_BUBBLE_VISUAL, summon);
                    summon->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

                    summon->SetCanFly(true);
                    summon->SetSpeed(MOVE_WALK, 0.7f);
                    summon->SetSpeed(MOVE_RUN, 0.7f);
                    summon->SetSpeed(MOVE_FLIGHT, 0.7f);

                    float x, y, z;
                    summon->GetClosePoint(x, y, z, summon->GetObjectSize() / 3, 2.0f);
                    summon->GetMotionMaster()->MovePoint(1, x, y, z + 30.0f); // Move up 30y slowly, and disappear.
                    summon->DespawnOrUnsummon(20000);
                }

                if (summon->GetEntry() == NPC_WALL_OF_SUDS)
                {
                    summon->SetReactState(REACT_PASSIVE);
                    summon->AddAura(SPELL_WALL_OF_SUDS, summon);
                    summon->SetFlag(UNIT_NPC_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    summon->SetSpeed(MOVE_WALK, 1.2f);
                    summon->SetSpeed(MOVE_RUN, 1.2f);

                    float x, y, z;
                    summon->GetClosePoint(x, y, z, summon->GetObjectSize() / 3, 50.0f);
                    summon->GetMotionMaster()->MovePoint(1, x, y, z); // Move 50 yards across the room, to the other side.
                }
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        // Melee range check.
                        case EVENT_BREW_BOLT:
                        {
                            if (!HasHostileInMeleeRange())
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                    DoCast(target, SPELL_BREW_BOLT);
                            }
                            events.ScheduleEvent(EVENT_BREW_BOLT, 2500);
                            break;
                        }
                        // Bremastery: Wheat.
                        case EVENT_BLOAT:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_BLOAT);
                            events.ScheduleEvent(EVENT_BLOAT, urand(15500, 17500)); // every 14.5 seconds + 2c.
                            break;
                        }
                        case EVENT_BLACKOUT_BREW:
                        {
                            DoCast(me, SPELL_BLACKOUT_BREW);
                            events.ScheduleEvent(EVENT_BLACKOUT_BREW, urand(10500, 12500)); // every 10.5 seconds + 1c.
                            break;
                        }
                        // Bremastery: Ale.
                        case EVENT_BUBBLE_SHIELD:
                        {
                            SpawnInCircle(1.0f, 8, NPC_BUBBLE_SHIELD, TEMPSUMMON_MANUAL_DESPAWN);
                            DoCast(me, SPELL_BUBBLE_SHIELD);
                            events.ScheduleEvent(EVENT_SET_BUBBLE_SHIELD_STACKS, 2100); // 2s cast time.
                            events.ScheduleEvent(EVENT_BUBBLE_SHIELD, urand(43000, 45000)); // every 42 seconds + 2c.
                            break;
                        }
                        case EVENT_SET_BUBBLE_SHIELD_STACKS:
                        {
                            if (Aura* aura = me->GetAura(SPELL_BUBBLE_SHIELD))
                                me->SetAuraStack(SPELL_BUBBLE_SHIELD, me, 8);
                            break;
                        }
                        case EVENT_YEASTY_BREW_ELEMENTALS:
                        {
                            for (uint8 i = 0; i < 4; i++)
                                me->SummonCreature(NPC_YEASTY_BREW_ALEMENTAL_Y, me->GetPositionX() + frand(-4.0f, 4.0f), me->GetPositionY() + frand(-4.0f, 4.0f), me->GetPositionZ() + 1.0f, me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                            events.ScheduleEvent(EVENT_YEASTY_BREW_ELEMENTALS, urand(69000, 73000)); // every 71 seconds.
                            break;
                        }
                        // Brewmastery: Stout.
                        case EVENT_CARBONATION:
                        {
                            DoCast(me, SPELL_CARBONATION);
                            events.ScheduleEvent(EVENT_CARBONATION, urand(66000, 68000)); // every 64 seconds + 3c.
                            break;
                        }
                        case EVENT_WALL_OF_SUDS:
                        {
                            // SummonSuds(); TODO.
                            events.ScheduleEvent(EVENT_WALL_OF_SUDS, urand(70000, 75000)); // every 72.5 seconds.
                            break;
                        }
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

    // Particular AI functions.
    private:
        // Check for pets or players in Melee range.
        bool HasHostileInMeleeRange()
        {
            // Check for tank.
            if (Unit* tank = me->getVictim())
            {
                if (tank->IsWithinDistInMap(me, MELEE_RANGE))
                    return true;
            }

            // Check for pets.
            if (Unit* unit = me->SelectNearbyTarget(NULL, MELEE_RANGE))
            {
                if (unit->isPet())
                    return true;
            }

            // Check for players.
            if (Player* nearPlayer = me->FindNearestPlayer(MELEE_RANGE))
            {
                if (nearPlayer->IsWithinDistInMap(me, MELEE_RANGE))
                {
                    if (!nearPlayer->isGameMaster())
                        return true;
                }
            }

            return false;
        }

        // Used for spawning NPC's in a circle around the caster.
        void SpawnInCircle(float row, uint8 summonNumber, uint32 unitEntry, TempSummonType summonType = TEMPSUMMON_MANUAL_DESPAWN, uint32 despawnTimer = 30000)
        {
            float teta = (2 * M_PI) / summonNumber;

            for (uint8 i = 0 ; i < summonNumber ; i++)
            {
                float x = cos(teta * i) * row + me->GetPositionX();
                float y = sin(teta * i) * row + me->GetPositionY();

                if (summonType = TEMPSUMMON_MANUAL_DESPAWN)
                    me->SummonCreature(unitEntry, x, y, me->GetPositionZ() + 2.0f, 0, summonType);
                else
                    me->SummonCreature(unitEntry, x, y, me->GetPositionZ() + 2.0f, 0, summonType, despawnTimer);
            }
        }

        // Used for summoning the Wall of Suds.
        void SummonSuds()
        {
            bool justOnce = false;
            uint32 j = 4;

            for (uint8 i = 1; i < 41; i += 5)
            {
                if (!justOnce)
                {
                    justOnce = true;
                    SpawnInLine(&(SudPositions[0][0]), &(SudPositions[0][1]), NPC_WALL_OF_SUDS, j);
                    SpawnInLine(&(SudPositions[1][0]), &(SudPositions[1][1]), NPC_WALL_OF_SUDS, j);
                }
                else
                {
                    justOnce = false;
                    j += 2;
                    SpawnInLine(&(SudPositions[0][0]), &(SudPositions[0][1]), NPC_WALL_OF_SUDS, j);
                    SpawnInLine(&(SudPositions[1][0]), &(SudPositions[1][1]), NPC_WALL_OF_SUDS, j);
                }
            }

            // Add Sudsy aura to players.
            Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
            if (!playerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator c_iter = playerList.begin(); c_iter != playerList.end(); ++c_iter)
                {
                    if (Player* player = c_iter->getSource())
                        me->AddAura(SPELL_SUDSY, player);
                }
            }
        }

        void SpawnInLine(Position* posStart, Position* posEnd, uint32 entry, uint32 number)
        {
            float coeff = (posStart->GetPositionY() - posEnd->GetPositionY()) / (posStart->GetPositionX() - posEnd->GetPositionX()); // Coefficient direction from right
            float ord = posStart->GetPositionY() - posStart->GetPositionX() * coeff; // Order

            float distBetween = (std::max(posStart->GetPositionX(), posEnd->GetPositionX()) - std::min(posStart->GetPositionX(), posEnd->GetPositionX())) / number;

            for (float x = posStart->GetPositionX(); x < posEnd->GetPositionX(); x += distBetween)
                me->SummonCreature(entry, x, coeff * x + ord, me->GetPositionZ() + 2.0f, 0);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_yan_zhu_the_uncasked_AI(creature);
    }
};

// Bubble Shield 65522.
class npc_bubble_shield_yanzhu : public CreatureScript
{
    public :
        npc_bubble_shield_yanzhu() : CreatureScript("npc_bubble_shield_yanzhu") { }

        struct npc_bubble_shield_yanzhu_AI : public ScriptedAI
        {
            npc_bubble_shield_yanzhu_AI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            Creature* bossYanzhu;

            void IsSummonedBy(Unit* summoner)
            {
                if (summoner)
                {
                    bossYanzhu = summoner->ToCreature();
                    me->SetFacingTo(summoner->GetOrientation());
                }
                else
                    bossYanzhu = NULL;

                Reset();
            }

            void Reset()
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            }

            void JustDied(Unit* /*killer*/)
            {
                if (bossYanzhu)
                {
                    if (Aura* bubble = bossYanzhu->GetAura(SPELL_BUBBLE_SHIELD))
                    {
                        if (bubble->GetStackAmount() > 1)
                            bubble->SetStackAmount(bubble->GetStackAmount() - 1);
                        else
                            bossYanzhu->RemoveAurasDueToSpell(SPELL_BUBBLE_SHIELD);
                    }
                }
            }

            void UpdateAI(const uint32 diff) { } // No melee.
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_bubble_shield_yanzhu_AI(creature);
        }
};

// Fizzy Bubble 59799.
class npc_fizzy_bubble_yanzhu : public CreatureScript
{
    public:
        npc_fizzy_bubble_yanzhu() : CreatureScript("npc_fizzy_bubble_yanzhu") { }

        struct npc_fizzy_bubble_yanzhuAI : public PassiveAI
        {
            npc_fizzy_bubble_yanzhuAI(Creature* creature) : PassiveAI(creature) { }

            void OnSpellClick(Unit* clicker)
            {
                clicker->AddAura(SPELL_FIZZY_BUBBLE, clicker);
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                me->DespawnOrUnsummon(100);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_fizzy_bubble_yanzhuAI(creature);
        }
};

// Yeasty Brew Alemental 66413.
class npc_yeasty_brew_elemental_yanzhu : public CreatureScript
{
    public :
        npc_yeasty_brew_elemental_yanzhu() : CreatureScript("mob_yeast_brew_elemental") { }

        struct npc_yeasty_brew_elemental_yanzhu_AI : public ScriptedAI
        {
            npc_yeasty_brew_elemental_yanzhu_AI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            EventMap events;
            InstanceScript* instance;
            Creature* bossYanzhu;
            uint64 fermentTargetGuid;

            void IsSummonedBy(Unit* summoner)
            {
                if (summoner)
                    bossYanzhu = summoner->ToCreature();
                else
                    bossYanzhu = NULL;

                DoCast(me, SPELL_YEASTY_SUMMON_VIS);
            }

            void Reset()
            {
                events.Reset();
                fermentTargetGuid = 0;
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_YEASTY_BREW_BOLT, urand(3000, 5500));
                events.ScheduleEvent(EVENT_FERMENT, urand(8000, 19000));
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_YEASTY_BREW_BOLT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_YEASTY_BREW_BOLT);
                            events.ScheduleEvent(EVENT_YEASTY_BREW_BOLT, urand(5000, 7500));
                            break;
                        case EVENT_FERMENT:
                            DoChooseFermentVictim();
                            events.ScheduleEvent(EVENT_FERMENT, urand(21000, 29000));
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
        }

        // Particular AI functions
        private:
            // For casting Ferment.
            void DoChooseFermentVictim()
            {
                bool hasPlayer = false;

                if (bossYanzhu)
                {
                    Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
                    if (!playerList.isEmpty())
                    {
                        for (Map::PlayerList::const_iterator c_iter = playerList.begin(); c_iter != playerList.end(); ++c_iter)
                        {
                            if (Player* player = c_iter->getSource())
                            {
                                if (player->IsInBetween(me, bossYanzhu, 1.0f))
                                {
                                    hasPlayer = true;
                                    fermentTargetGuid = player->GetGUID();
                                    DoCast(player, SPELL_FERMENT, true);
                                    break;
                                }
                                else
                                    continue;
                            }
                        }
                    }

                    if (!hasPlayer)
                    {
                        fermentTargetGuid = bossYanzhu->GetGUID();
                        DoCast(bossYanzhu, SPELL_FERMENT);
                    }
                }
            }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_yeasty_brew_elemental_yanzhu_AI(creature);
    }
};

// Check for intercepting Ferment beam.
class FermentCheck : public std::unary_function<Unit*, bool>
{
    public:
        FermentCheck(Unit* caster, Unit* target) : _caster(caster), _target(target) { }

        bool operator()(WorldObject* object)
        {
            if (object != _target)
                return true;

            return false;
        }

    private:
        WorldObject* _caster;
        WorldObject* _target;
};

// Ferment triggered spell 114451.
class spell_yeasty_alemental_ferment : public SpellScriptLoader
{
    public:
        spell_yeasty_alemental_ferment() : SpellScriptLoader("spell_yeasty_alemental_ferment") { }

        class spell_yeasty_alemental_ferment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_yeasty_alemental_ferment_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty() || !GetCaster())
                    return;

                // Select the only target needed.
                //if (Unit* target = CAST_AI(npc_yeasty_brew_elemental_yanzhu::npc_yeasty_brew_elemental_yanzhu_AI, GetCaster()->ToCreature()->AI())->fermentTarget)
                uint64 l_TargetGuid = CAST_AI(npc_yeasty_brew_elemental_yanzhu::npc_yeasty_brew_elemental_yanzhu_AI, GetCaster()->ToCreature()->AI())->fermentTargetGuid;
                if (Unit* l_Target = Unit::GetUnit(*GetCaster(), l_TargetGuid))
                    targets.remove_if(FermentCheck(GetCaster(), l_Target));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_yeasty_alemental_ferment_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_130);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_yeasty_alemental_ferment_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_CONE_ENEMY_130);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_yeasty_alemental_ferment_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_CONE_ENEMY_130);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_yeasty_alemental_ferment_SpellScript();
        }
};

class spell_yanzhu_blackout_brew : public SpellScriptLoader
{
    public :
        spell_yanzhu_blackout_brew() : SpellScriptLoader("spell_yan_zhu_blackout_brew") { }

        class spell_yanzhu_blackout_brew_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_yanzhu_blackout_brew_SpellScript)

            bool Validate(const SpellInfo* spellInfo)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_BLACKOUT_BREW) || !sSpellMgr->GetSpellInfo(SPELL_BLACKOUT_DRUNK))
                    return false;

                return true;
            }

            void HandleOnHit()
            {
                // PreventDefaultAction();
                if (!GetHitUnit() || !GetCaster())
                    return;

                if (Aura* blackoutBrew = GetHitUnit()->GetAura(SPELL_BLACKOUT_BREW))
                {
                    blackoutBrew->SetStackAmount(blackoutBrew->GetStackAmount() + 3); // Increase the stacks by 3.

                    if (blackoutBrew->GetStackAmount() >= 10)
                    {
                        GetCaster()->AddAura(SPELL_BLACKOUT_DRUNK, GetHitUnit()); // Stun the player.
                        GetHitUnit()->RemoveAurasDueToSpell(SPELL_BLACKOUT_BREW); // Remove all the aura stacks.
                    }
                }
                else
                {
                    GetHitUnit()->AddAura(SPELL_BLACKOUT_BREW, GetHitUnit()); // Add the aura.
                    GetHitUnit()->SetAuraStack(SPELL_BLACKOUT_BREW, GetHitUnit(), 3); // Set it to three stacks.
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_yanzhu_blackout_brew_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_yanzhu_blackout_brew_SpellScript();
        }

        class spell_yanzhu_blackout_brew_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_yanzhu_blackout_brew_AuraScript)

            void OnPeriodic(AuraEffect const* /*aurEff*/)
            {
                if (!GetTarget())
                    return;

                // Remove a stack if the player is moving or jumping (Jumping checked by MOVEMENTFLAG_FALLING and PositionZ compared to boss one plus a small margin).
                if (GetTarget()->isMoving() || GetTarget()->HasUnitMovementFlag(MOVEMENTFLAG_FALLING) || GetTarget()->GetPositionZ() > GetCaster()->GetPositionZ() + 0.1f)
                {
                    if (Aura* blackoutBrew = GetTarget()->GetAura(SPELL_BLACKOUT_BREW))
                    {
                        if (blackoutBrew->GetStackAmount() > 1)
                            blackoutBrew->SetStackAmount(blackoutBrew->GetStackAmount() - 1);
                        else
                            GetTarget()->RemoveAurasDueToSpell(SPELL_BUBBLE_SHIELD);
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_yanzhu_blackout_brew_AuraScript::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_yanzhu_blackout_brew_AuraScript();
        }
};

// Bloat 106546.
class spell_yanzhu_bloat : public SpellScriptLoader
{
    public:
        spell_yanzhu_bloat() :  SpellScriptLoader("spell_yanzhu_bloat") { }

        class spell_yanzhu_bloat_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_yanzhu_bloat_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster() && GetTarget())
                {
                    if (Unit* target = GetTarget())
                    {
                        if (Aura* bloat = GetCaster()->AddAura(SPELL_BLOATED, target))
                        {
                            bloat->SetMaxDuration(GetMaxDuration());
                            bloat->SetDuration(GetDuration());
                        }
                    }
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_yanzhu_bloat_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_yanzhu_bloat_AuraScript();
        }
};

// Bloated player target check.
class PlayerCheck : public std::unary_function<Unit*, bool>
{
    public:
        explicit PlayerCheck(Unit* _caster) : caster(_caster) { }
        bool operator()(WorldObject* object)
        {
            return object->GetTypeId() != TYPEID_PLAYER;
        }

    private:
        Unit* caster;
};

// Gushing Brew (Bloated trigger spell) 106560.
class spell_yanzhu_gushing_brew : public SpellScriptLoader
{
    public:
        spell_yanzhu_gushing_brew() : SpellScriptLoader("spell_yanzhu_gushing_brew") { }

        class spell_yanzhu_gushing_brew_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_yanzhu_gushing_brew_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty() || !GetCaster())
                    return;

                // Set targets.
                targets.remove_if(PlayerCheck(GetCaster()));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_yanzhu_gushing_brew_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENTRY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_yanzhu_gushing_brew_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_CONE_ENTRY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_yanzhu_gushing_brew_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_CONE_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_yanzhu_gushing_brew_SpellScript();
        }
};

// Carbonation 115003 - Summon Fizzy Bubbles script.
class spell_yanzhu_carbonation : public SpellScriptLoader
{
    public:
        spell_yanzhu_carbonation() : SpellScriptLoader("spell_yanzhu_carbonation") { }

        class spell_yanzhu_carbonation_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_yanzhu_carbonation_AuraScript)

            void PeriodicTick(AuraEffect const* /*aurEff*/)
            {
                for (uint8 i = 0; i < 2; i++)
                {
                    if (Unit* caster = GetCaster())
                        caster->SummonCreature(NPC_FIZZY_BUBBLE, caster->GetPositionX() + frand(-13.0f, 13.0f), caster->GetPositionY() + frand(-13.0f, 13.0f), caster->GetPositionZ() + 1.0f, caster->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_yanzhu_carbonation_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_yanzhu_carbonation_AuraScript();
        }
};

void AddSC_boss_yan_zhu_the_uncasked()
{
    new boss_yan_zhu_the_uncasked();        // 59479
    new npc_bubble_shield_yanzhu();         // 65522
    new npc_fizzy_bubble_yanzhu();          // 59799
    new npc_yeasty_brew_elemental_yanzhu(); // 66413
    new spell_yeasty_alemental_ferment();   // 114451
    new spell_yanzhu_blackout_brew();       // 106851
    new spell_yanzhu_bloat();               // 106546
    new spell_yanzhu_gushing_brew();        // 106560
    new spell_yanzhu_carbonation();         // 115003
}
