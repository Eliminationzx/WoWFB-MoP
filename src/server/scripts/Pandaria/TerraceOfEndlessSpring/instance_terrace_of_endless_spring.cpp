//////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2015 Millenium-studio SARL
///  All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#include "terrace_of_endless_spring.hpp"

class instance_terrace_of_endless_spring : public InstanceMapScript
{
    public:
        instance_terrace_of_endless_spring() : InstanceMapScript("instance_terrace_of_endless_spring", 996) { }

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_terrace_of_endless_spring_InstanceMapScript(map);
        }

        struct instance_terrace_of_endless_spring_InstanceMapScript : public InstanceScript
        {
            instance_terrace_of_endless_spring_InstanceMapScript(Map* map) : InstanceScript(map) { }

            bool ritualOfPurification;
            bool introDone;
            bool m_CheckBoss;
            uint32 m_BossTimer;

            // Protectors of the Endless
            uint64 ancientRegailGuid;
            uint64 ancientAsaniGuid;
            uint64 protectorKaolanGuid;
            uint64 minionOfFearControllerGuid;

            // Tsulong
            uint64 tsulongGuid;

            // Lei Shi
            uint64 leiShiGuid;

            // Sha of Fear
            uint64 shaOfFearGuid;
            uint64 pureLightTerraceGuid;
            uint64 travelToDreadExpanseGuid;

            // Timers, old school style!
            uint32 tsulongEventTimer;

            // Council's Vortex
            uint64 wallOfCouncilsVortexGuid;
            uint64 councilsVortexGuid;

            // Lei Shi's Vortex
            uint64 wallOfLeiShisVortexGuid;
            uint64 leiShisVortexGuid;

            void Initialize()
            {
                SetBossNumber(DATA_MAX_BOSS_DATA);

                ritualOfPurification        = true;
                introDone                   = false;
                m_CheckBoss                 = false;
                m_BossTimer                 = 0;

                ancientRegailGuid           = 0;
                ancientAsaniGuid            = 0;
                protectorKaolanGuid         = 0;
                minionOfFearControllerGuid  = 0;

                tsulongGuid                 = 0;

                leiShiGuid                  = 0;

                shaOfFearGuid               = 0;
                pureLightTerraceGuid        = 0;
                travelToDreadExpanseGuid    = 0;

                tsulongEventTimer           = 0;

                wallOfCouncilsVortexGuid    = 0;
                councilsVortexGuid          = 0;

                wallOfLeiShisVortexGuid     = 0;
                leiShisVortexGuid           = 0;
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case NPC_ANCIENT_ASANI:
                        ancientAsaniGuid = creature->GetGUID();
                        break;
                    case NPC_ANCIENT_REGAIL:
                        ancientRegailGuid = creature->GetGUID();
                        break;
                    case NPC_PROTECTOR_KAOLAN:
                        protectorKaolanGuid = creature->GetGUID();
                        break;
                    case NPC_MINION_OF_FEAR_CONTROLLER:
                        minionOfFearControllerGuid = creature->GetGUID();
                        break;
                    case NPC_TSULONG:
                        tsulongGuid = creature->GetGUID();
                        break;
                    case NPC_LEI_SHI:
                        leiShiGuid = creature->GetGUID();
                        break;
                    case NPC_SHA_OF_FEAR:
                        shaOfFearGuid = creature->GetGUID();
                        break;
                    case NPC_PURE_LIGHT_TERRACE:
                        pureLightTerraceGuid = creature->GetGUID();
                        break;
                    case NPC_TRAVEL_TO_DREAD_EXPANSE:
                        travelToDreadExpanseGuid = creature->GetGUID();
                        break;
                    default:
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GOB_COUNCILS_VORTEX:
                        councilsVortexGuid = go->GetGUID();
                        if (GetBossState(DATA_PROTECTORS) == DONE)
                            go->SetGoState(GO_STATE_ACTIVE);
                        break;
                    case GOB_WALL_OF_COUNCILS_VORTEX:
                        wallOfCouncilsVortexGuid = go->GetGUID();
                        if (GetBossState(DATA_PROTECTORS) == DONE)
                            go->SetGoState(GO_STATE_ACTIVE);
                        break;
                    case GOB_WALL_OF_LEI_SHI:
                        wallOfLeiShisVortexGuid = go->GetGUID();
                        if (GetBossState(DATA_TSULONG) == DONE)
                            go->SetGoState(GO_STATE_ACTIVE);
                        break;
                    case GOB_LEI_SHIS_VORTEX:
                        leiShisVortexGuid = go->GetGUID();
                        if (GetBossState(DATA_TSULONG) == DONE)
                            go->SetGoState(GO_STATE_ACTIVE);
                        break;
                    default:
                        break;
                }
            }

            bool SetBossState(uint32 id, EncounterState state)
            {
                if (!InstanceScript::SetBossState(id, state))
                    return false;

                /// Need this to make a RaidEncounter::SaveToDB for each players
                Map::PlayerList const& l_PlayerList = instance->GetPlayers();
                if (!l_PlayerList.isEmpty())
                {
                    Player* l_Player = l_PlayerList.getFirst()->getSource();
                    l_Player->GetMap()->ToInstanceMap()->PermBindAllPlayersForRaid(l_Player);
                }

                if (state == DONE)
                    m_CheckBoss = false;

                if (id == DATA_PROTECTORS && state == DONE && GetBossState(DATA_TSULONG) != DONE)
                {
                    if (Creature* l_Tsulong = instance->GetCreature(tsulongGuid))
                        l_Tsulong->AI()->DoAction(ACTION_START_TSULONG_WAYPOINT);

                    if (Creature* shaOfFear = instance->GetCreature(shaOfFearGuid))
                        shaOfFear->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                }
                else if (id == DATA_TSULONG && state == DONE && GetBossState(DATA_LEI_SHI) != DONE)
                {
                    if (Creature* leiShi = instance->GetCreature(leiShiGuid))
                    {
                        leiShi->AI()->DoAction(ACTION_SHOW_LEI_SHI);
                        leiShi->AI()->Reset();
                    }
                }
                else if (id == DATA_LEI_SHI && state == DONE && GetBossState(DATA_SHA_OF_FEAR) != DONE)
                {
                    if (Creature* shaOfFear = instance->GetCreature(shaOfFearGuid))
                    {
                        shaOfFear->AI()->DoAction(ACTION_ACTIVATE_SHA_OF_FEAR);
                        shaOfFear->AI()->Reset();
                    }
                }

                return true;
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case SPELL_RITUAL_OF_PURIFICATION:
                        ritualOfPurification = data;
                        break;
                    case INTRO_DONE:
                    {
                        if (data > 0)
                        {
                            Creature* asani = instance->GetCreature(ancientAsaniGuid);
                            if (asani)
                                asani->AI()->DoAction(ACTION_INTRO_FINISHED);

                            Creature* regail = instance->GetCreature(ancientRegailGuid);
                            if (regail)
                                regail->AI()->DoAction(ACTION_INTRO_FINISHED);

                            Creature* kaolan = instance->GetCreature(protectorKaolanGuid);
                            if (kaolan)
                                kaolan->AI()->DoAction(ACTION_INTRO_FINISHED);
                        }

                        introDone = data > 0;

                        break;
                    }
                    default:
                        break;
                }
            }

            uint32 GetData(uint32 type)
            {
                switch (type)
                {
                    case SPELL_RITUAL_OF_PURIFICATION:
                        return ritualOfPurification;
                    case INTRO_DONE:
                        return introDone;
                    default:
                        return 0;
                }
            }

            uint64 GetData64(uint32 type)
            {
                switch (type)
                {
                    case NPC_ANCIENT_ASANI:
                        return ancientAsaniGuid;
                    case NPC_ANCIENT_REGAIL:
                        return ancientRegailGuid;
                    case NPC_PROTECTOR_KAOLAN:
                        return protectorKaolanGuid;
                    case NPC_MINION_OF_FEAR_CONTROLLER:
                        return minionOfFearControllerGuid;
                    case NPC_TSULONG:
                        return tsulongGuid;
                    case NPC_LEI_SHI:
                        return leiShiGuid;
                    case NPC_SHA_OF_FEAR:
                        return shaOfFearGuid;
                    case NPC_PURE_LIGHT_TERRACE:
                        return pureLightTerraceGuid;
                    case NPC_TRAVEL_TO_DREAD_EXPANSE:
                        return travelToDreadExpanseGuid;
                    case GOB_COUNCILS_VORTEX:
                        return councilsVortexGuid;
                    case GOB_WALL_OF_COUNCILS_VORTEX:
                        return wallOfCouncilsVortexGuid;
                    case GOB_WALL_OF_LEI_SHI:
                        return wallOfLeiShisVortexGuid;
                    case GOB_LEI_SHIS_VORTEX:
                        return leiShisVortexGuid;
                    default:
                        break;
                }

                return 0;
            }

            bool CheckRequiredBosses(uint32 bossId, Player const* player = NULL) const
            {
                if (!InstanceScript::CheckRequiredBosses(bossId, player))
                    return false;

                switch (bossId)
                {
                    case DATA_TSULONG:
                    case DATA_LEI_SHI:
                    case DATA_SHA_OF_FEAR:
                        if (GetBossState(bossId - 1) != DONE)
                            return false;
                    default:
                        break;
                }

                return true;
            }

            void OnPlayerEnter(Player* p_Player)
            {
                p_Player->GetMap()->SetObjectVisibility(350.f);
                if (!m_BossTimer && !m_CheckBoss)
                {
                    m_BossTimer = 2000;
                    m_CheckBoss = true;
                }
            }

            void OnPlayerExit(Player* p_Player)
            {
                if (p_Player->HasAura(SPELL_NIGHT_PHASE_EFFECT))
                    p_Player->RemoveAura(SPELL_NIGHT_PHASE_EFFECT);
            }

            void Update(uint32 p_Diff)
            {
                if (m_BossTimer)
                {
                    if (m_BossTimer > p_Diff)
                        m_BossTimer -= p_Diff;
                    else
                    {
                        m_BossTimer = 0;
                        EncounterState l_ProtectorState = GetBossState(DATA_PROTECTORS);
                        EncounterState l_TsulongState   = GetBossState(DATA_TSULONG);
                        EncounterState l_LeiShiState    = GetBossState(DATA_LEI_SHI);
                        EncounterState l_ShaState       = GetBossState(DATA_SHA_OF_FEAR);

                        /// Tsulong must come
                        if (l_ProtectorState == DONE && (l_TsulongState == NOT_STARTED || l_TsulongState == FAIL))
                        {
                            if (Creature* l_Tsulong = instance->GetCreature(tsulongGuid))
                                l_Tsulong->AI()->DoAction(ACTION_START_TSULONG_WAYPOINT);
                        }
                        /// Lei Shi must appear
                        else if (l_TsulongState == DONE && (l_LeiShiState == NOT_STARTED || l_LeiShiState == FAIL))
                        {
                            if (Creature* l_LeiShi = instance->GetCreature(leiShiGuid))
                            {
                                l_LeiShi->AI()->DoAction(ACTION_SHOW_LEI_SHI);
                                l_LeiShi->AI()->Reset();
                            }
                        }
                        /// Sha of Fear must prepare to fight
                        else if (l_LeiShiState == DONE && (l_ShaState == NOT_STARTED || l_ShaState == FAIL))
                        {
                            if (Creature* l_ShaOfFear = instance->GetCreature(shaOfFearGuid))
                            {
                                l_ShaOfFear->AI()->DoAction(ACTION_ACTIVATE_SHA_OF_FEAR);
                                l_ShaOfFear->AI()->Reset();
                            }
                        }
                    }
                }
            }
        };
};

void AddSC_instance_terrace_of_endless_spring()
{
    new instance_terrace_of_endless_spring();
}
