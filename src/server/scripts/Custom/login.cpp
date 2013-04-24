#include "ScriptPCH.h"
#define CUSTOM_BLUE          "|cff00479E"
#define CUSTOM_RED            "|cffFF0000"
#define CUSTOM_LIGHTRED    "|cffD63931"
#define CUSTOM_WHITE        "|cffffffff"

class announce_login : public PlayerScript
{
    public:
                announce_login() : PlayerScript("announce_login") { }
        
                void OnLogin(Player * player, Creature * creature)
        {
                        WorldSession* session = player->GetSession();
                        char msg[500];
                        if(player->GetTeam() == ALLIANCE)
                        {
                                sprintf(msg, session->GetTrinityString(11147), CUSTOM_LIGHTRED, CUSTOM_WHITE, player->GetName(), CUSTOM_BLUE);
                                sWorld->SendWorldText(LANG_SYSTEMMESSAGE, msg);
                        }else{
                                sprintf(msg, session->GetTrinityString(11148), CUSTOM_LIGHTRED, CUSTOM_WHITE, player->GetName(), CUSTOM_RED);
                                sWorld->SendWorldText(LANG_SYSTEMMESSAGE, msg);
                        }
                }
};
 
void AddSC_announce_login()
{
        new announce_login;
}