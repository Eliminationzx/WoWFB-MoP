#include "ScriptPCH.h"
#include "ScriptMgr.h"
#include "Config.h"

class panda_faction_chooser : public PlayerScript
{
public:
    panda_faction_chooser() : PlayerScript("panda_faction_chooser") { }
 
    void OnLogin(Player* player)
    {
            if (player->getRace() == RACE_PANDAREN_NEUTRAL)
            {
                player->ShowNeutralPlayerFactionSelectUI();
            }
    }
};

#define GOSSIP_CHOOSE_FACTION     "I'm ready to choose my destiny."

class npc_panda_faction_chooser : public CreatureScript
{
    public:
        npc_panda_faction_chooser() : CreatureScript("npc_panda_faction_chooser") { }

        bool OnGossipHello(Player* player, Creature* creature)
        {
            if (player->getRace() == RACE_PANDAREN_NEUTRAL)
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_CHOOSE_FACTION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            }

            player->PlayerTalkClass->SendGossipMenu(1, creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
        {
            if (action == GOSSIP_ACTION_INFO_DEF + 1)
                player->ShowNeutralPlayerFactionSelectUI();

            player->PlayerTalkClass->SendCloseGossip();
            return true;
        }
};

void AddSC_misc_scripts()
{
    new panda_faction_chooser();
    new npc_panda_faction_chooser();
}
