#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "Kenooby.h"

#define SAY_QUEST_ACCEPTED -1999982
#define SAY_STAY_1 -1999983
#define SAY_STAY_2 -1999984
#define SAY_STAY_3 -1999985
#define SAY_STAY_4 -1999986
#define SAY_STAY_5 -1999987
#define SAY_STAY_6 -1999988
#define SAY_QUEST_COMPLETE -1999989
#define SAY_ATTACKED_1 -1999990
#define EMOTE_DISAPPEAR -1999991

#define QUEST_Kenooby_THE_IMPORTER 2904

class npc_Kenooby : public CreatureScript
{
public:
    npc_Kenooby() : CreatureScript("npc_Kenooby") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_Kenooby_THE_IMPORTER)
        {
            CAST_AI(npc_escortAI, (creature->AI()))->Start(true, false, player->GetGUID());
            DoScriptText(SAY_QUEST_ACCEPTED, creature);
            creature->setFaction(113);
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_KenoobyAI(creature);
    }

    struct npc_KenoobyAI : public npc_escortAI
    {
        npc_KenoobyAI(Creature* creature) : npc_escortAI(creature) {}

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 7:
                    DoScriptText(SAY_STAY_1, me, player);
                    break;
                case 11:
                    DoScriptText(SAY_STAY_2, me, player);
                    break;
                case 19:
                    DoScriptText(SAY_STAY_3, me, player);
                    break;
                case 20:
                    DoScriptText(SAY_STAY_4, me, player);
                    break;
                case 21:
                    DoScriptText(SAY_STAY_5, me, player);
                    break;
                case 22:
                    DoScriptText(SAY_STAY_6, me, player);
                    break;
                case 26:
                    DoScriptText(SAY_QUEST_COMPLETE, me, player);
                    me->SetSpeed(MOVE_RUN, 1.2f, true);
                    me->SetWalk(false);
                    break;
                case 28:
					me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    if (player->GetTypeId() == TYPEID_PLAYER)
                        CAST_PLR(player)->GroupEventHappens(QUEST_Kenooby_THE_IMPORTER, me);
                    DoScriptText(EMOTE_DISAPPEAR, me);
                    break;
            }
        }

        void Reset() {}

        void EnterCombat(Unit* /*who*/)
        {
            DoScriptText(SAY_ATTACKED_1, me, NULL);
        }

        void JustSummoned(Creature* summoned)
        {
            summoned->AI()->AttackStart(me);
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Player* player = GetPlayerForEscort())
                CAST_PLR(player)->FailQuest(QUEST_Kenooby_THE_IMPORTER);
        }
    };

};

void AddSC_Kenooby()
{
    new npc_Kenooby();
}
