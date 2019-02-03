#include "arena_of_annihilation.h"

class instance_arena_of_annihilation : public InstanceMapScript
{
    public:
        instance_arena_of_annihilation() : InstanceMapScript("instance_arena_of_annihilation", 1031) { }

        InstanceScript* GetInstanceScript(InstanceMap* map) const override
        {
            return new instance_arena_of_annihilation_InstanceScript(map);
        }

        struct instance_arena_of_annihilation_InstanceScript : public InstanceScript
        {
            instance_arena_of_annihilation_InstanceScript(Map* map) :
                InstanceScript(map), m_Scenario(map, SCENARIO_ID, STEP_6)
            {
                Initialize();
            }

            void Initialize()
            {
                m_Scenario.GetStep(STEP_1).AddCriteria(CRITERIA_GONG_RANG, 1);
                m_Scenario.GetStep(STEP_1).AddCriteria(CRITERIA_SCARSHELL, 1);
                m_Scenario.GetStep(STEP_2).AddCriteria(CRITERIA_GONG_RANG, 1);
                m_Scenario.GetStep(STEP_2).AddCriteria(CRITERIA_JOLGRUM, 1);
                m_Scenario.GetStep(STEP_3).AddCriteria(CRITERIA_GONG_RANG, 1);
                m_Scenario.GetStep(STEP_3).AddCriteria(CRITERIA_LITTLE_LIUYANG, 1);
                m_Scenario.GetStep(STEP_4).AddCriteria(CRITERIA_GONG_RANG, 1);
                m_Scenario.GetStep(STEP_4).AddCriteria(CRITERIA_CHAGAN_FIREHOOF, 1);
                m_Scenario.GetStep(STEP_5).AddCriteria(CRITERIA_GONG_RANG, 1);
                m_Scenario.GetStep(STEP_5).AddCriteria(CRITERIA_FINAL_CHALLENGER, 1);

                gongGUID            = 0;
                scarshellGUID       = 0;
                jolgrumGUID         = 0;
                littleLiuyangGUID   = 0;
                chaganFirehoofGUID  = 0;
                makiWaterbladeGUID  = 0;
                satayByuGUID        = 0;
                cloudbenderKoboGUID = 0;
            }

            void OnPlayerEnter(Player* who)
            {
                m_Scenario.SendScenarioState(who);
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case NPC_SCARSHELL:
                        scarshellGUID = creature->GetGUID();
                        break;
                    case NPC_JOLGRUM:
                        jolgrumGUID = creature->GetGUID();
                        break;
                    case NPC_LITTLE_LIUYANG:
                        littleLiuyangGUID = creature->GetGUID();
                        break;
                    case NPC_CHAGAN_FIREHOOF:
                        chaganFirehoofGUID = creature->GetGUID();
                        break;
                    case NPC_MAKI_WATERBLADE:
                        makiWaterbladeGUID = creature->GetGUID();
                        break;
                    case NPC_SATAY_BYU:
                        satayByuGUID = creature->GetGUID();
                        break;
                    case NPC_CLOUDBENDER_KOBO:
                        cloudbenderKoboGUID = creature->GetGUID();
                        break;
                }
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_GONG:
                        m_Scenario.UpdateCurrentStepCriteria(CRITERIA_GONG_RANG);
                        ActivateCreature();
                        break;
                    case DATA_SCARSHELL:
                        m_Scenario.UpdateCurrentStepCriteria(CRITERIA_SCARSHELL);
                        break;
                    case DATA_JOLGRUM:
                        m_Scenario.UpdateCurrentStepCriteria(CRITERIA_JOLGRUM);
                        break;
                    case DATA_LITTLE_LIUYANG:
                        m_Scenario.UpdateCurrentStepCriteria(CRITERIA_LITTLE_LIUYANG);
                        break;
                    case DATA_CHAGAN_FIREHOOF:
                        m_Scenario.UpdateCurrentStepCriteria(CRITERIA_CHAGAN_FIREHOOF);
                        break;

                    // Final challenger
                    case DATA_MAKI_WATERBLADE:
                    case DATA_SATAY_BYU:
                    case DATA_CLOUDBENDER_KOBO:
                        m_Scenario.UpdateCurrentStepCriteria(CRITERIA_FINAL_CHALLENGER);
                        break;
                    default:
                        break;
                }

                if (m_Scenario.IsCompleted())
                {
                    ScenarioComplete();
                }
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GOBJECT_GONG:
                        gongGUID = go->GetGUID();
                        break;
                    default:
                        break;
                }
            }

            uint32 GetData(uint32 type)
            {
                switch (type)
                {
                    case DATA_GONG:
                        return gongGUID;
                    case DATA_SCARSHELL:
                        return scarshellGUID;
                    case DATA_JOLGRUM:
                        return jolgrumGUID;
                    case DATA_LITTLE_LIUYANG:
                        return littleLiuyangGUID;
                    case DATA_CHAGAN_FIREHOOF:
                        return chaganFirehoofGUID;
                    case DATA_MAKI_WATERBLADE:
                        return makiWaterbladeGUID;
                    case DATA_SATAY_BYU:
                        return satayByuGUID;
                    case DATA_CLOUDBENDER_KOBO:
                        return cloudbenderKoboGUID;
                    default:
                        break;
                }
				return 0;
            }

            private:
                ScenarioController m_Scenario;
                uint64 gongGUID;
                uint64 scarshellGUID;
                uint64 jolgrumGUID;
                uint64 littleLiuyangGUID;
                uint64 chaganFirehoofGUID;

                // Final challenger
                uint64 makiWaterbladeGUID;
                uint64 satayByuGUID;
                uint64 cloudbenderKoboGUID;

            private:
                void ActivateCreature()
                {
                    switch (m_Scenario.GetCurrentStep())
                    {
                        case STEP_1:
                            ActivateNextStep(scarshellGUID);
                            break;
                        case STEP_2:
                            ActivateNextStep(jolgrumGUID);
                            break;
                        case STEP_3:
                            ActivateNextStep(littleLiuyangGUID);
                            break;
                        case STEP_4:
                            ActivateNextStep(chaganFirehoofGUID);
                            break;
                        case STEP_5:
                            ActivateFinalStep();
                            break;
                        default:
                            break;
                    }
                }

                void ActivateNextStep(uint64 creatureGUID)
                {
                    if (Creature* creature = instance->GetCreature(creatureGUID))
                        creature->SetPhaseMask(1, true);
                }

                void ActivateFinalStep()
                {
                    // For final challenger there are 3 possible bosses
                    uint8 random = urand(1, 3);
                    if (random == 1)
                        if (Creature* creature = instance->GetCreature(makiWaterbladeGUID))
                            creature->SetPhaseMask(1, true);
                    else if (random == 2)
                        if (Creature* creature = instance->GetCreature(satayByuGUID))
                            creature->SetPhaseMask(1, true);
                    else if (random == 3)
                        if (Creature* creature = instance->GetCreature(cloudbenderKoboGUID))
                            creature->SetPhaseMask(1, true);
                }

                void ScenarioComplete()
                {
                    DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_SCENARIO, SCENARIO_ID, 1);

                    if (!instance->GetPlayers().isEmpty())
                    {
                        Player* firstPlayer = instance->GetPlayers().getFirst()->getSource();
                        if (firstPlayer)
                        {
                            uint64 gguid = firstPlayer->GetGroup() ? firstPlayer->GetGroup()->GetGUID() : firstPlayer->GetGUID();
                            uint32 gDungeonId = sLFGMgr->GetDungeon(gguid);

                            Map::PlayerList const& players = instance->GetPlayers();
                            if (!players.isEmpty())
                                for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                                    if (Player* player = i->getSource())
                                        sLFGMgr->RewardDungeonDoneFor(gDungeonId, player);
                        }
                    }
                }
        };
};

void AddSC_instance_arena_of_annihilation()
{
	new instance_arena_of_annihilation();
}
