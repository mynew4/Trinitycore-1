#include "bp_mgr.h"
#include "Config.h"
#include "Group.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "WorldSession.h"
/*
script_bot_giver by Graff (onlysuffering@mail.ru)
Complete - ???
Category - creature_cripts/custom/bots/
*/
const uint8 GroupIcons[TARGETICONCOUNT] =
{
    /*STAR        = */0x001,
    /*CIRCLE      = */0x002,
    /*DIAMOND     = */0x004,
    /*TRIANGLE    = */0x008,
    /*MOON        = */0x010,
    /*SQUARE      = */0x020,
    /*CROSS       = */0x040,
    /*SKULL       = */0x080,
};

enum GossipActions
{
    CREATE_NBOT_MENU        = 1,
    CREATE_NBOT             = 2,
    CREATE_PBOT_MENU        = 3,
    CREATE_PBOT             = 4,

    REMOVE_PBOT_MENU        = 5,
    REMOVE_PBOT             = 6,
    REMOVE_NBOT_MENU        = 7,
    REMOVE_NBOT             = 8,

    INFO_WHISPER            = 9
};

class script_bot_giver : public CreatureScript
{
public:
    script_bot_giver() : CreatureScript("script_bot_giver") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        switch (sender)
        {
            case CREATE_NBOT_MENU:  SendCreateNPCBotMenu(player, creature, action);     break;
            case CREATE_NBOT:       SendCreateNPCBot(player, creature, action);         break;
            case CREATE_PBOT_MENU:  SendCreatePlayerBotMenu(player, creature, action);  break;
            case CREATE_PBOT:       SendCreatePlayerBot(player, creature, action);      break;

            case REMOVE_PBOT_MENU:  SendRemovePlayerBotMenu(player, creature, action);  break;
            case REMOVE_PBOT:       SendRemovePlayerBot(player, creature, action);      break;
            case REMOVE_NBOT_MENU:  SendRemoveNPCBotMenu(player, creature, action);     break;
            case REMOVE_NBOT:       SendRemoveNPCBot(player, creature, action);         break;

            case INFO_WHISPER:      SendBotHelpWhisper(player, creature, action);       break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        WorldSession* session = player->GetSession();
        uint8 count = 0;
        uint8 maxcount = ConfigMgr::GetIntDefault("Bot.MaxPlayerbots", 9);
        bool allowPBots = ConfigMgr::GetBoolDefault("Bot.EnablePlayerBots", false);
        bool allowNBots = ConfigMgr::GetBoolDefault("Bot.EnableNpcBots", true) && !player->RestrictBots();

        if (PlayerbotMgr* mgr = player->GetPlayerbotMgr())
        {
            for (PlayerBotMap::const_iterator itr = mgr->GetPlayerBotsBegin(); itr != mgr->GetPlayerBotsEnd(); ++itr)
            {
                if (count == 0)
                    player->ADD_GOSSIP_ITEM(0, session->GetTrinityString(11130), REMOVE_PBOT_MENU, GOSSIP_ACTION_INFO_DEF + 100);
                ++count;
            }
        }
        if (count < maxcount && allowPBots)
            player->ADD_GOSSIP_ITEM(0, session->GetTrinityString(11131), CREATE_PBOT_MENU, GOSSIP_ACTION_INFO_DEF + 1);

        maxcount = player->GetMaxNpcBots();
        if (player->HaveBot())
        {
            count = player->GetNpcBotsCount();
            if (count > 0)
                player->ADD_GOSSIP_ITEM(0, session->GetTrinityString(11132), REMOVE_NBOT_MENU, GOSSIP_ACTION_INFO_DEF + 101);
            if (count < maxcount && allowNBots)
                player->ADD_GOSSIP_ITEM(0, session->GetTrinityString(11133), CREATE_NBOT_MENU, GOSSIP_ACTION_INFO_DEF + 2);
        }
        else if (allowNBots)
            player->ADD_GOSSIP_ITEM(0, session->GetTrinityString(11133), CREATE_NBOT_MENU, GOSSIP_ACTION_INFO_DEF + 2);

        player->ADD_GOSSIP_ITEM(0, session->GetTrinityString(11134), INFO_WHISPER, GOSSIP_ACTION_INFO_DEF + 200);

        player->PlayerTalkClass->SendGossipMenu(8446, creature->GetGUID());
        return true;
    }

    static void SendCreatePlayerBot(Player* player, Creature*  /*creature*/, uint32 action)
    {
        WorldSession* session = player->GetSession();
        std::string plName;
        std::list<std::string>* names = new std::list<std::string>;
        uint32 accId = player->GetSession()->GetAccountId();
        QueryResult results = CharacterDatabase.PQuery("SELECT name FROM characters WHERE account = '%u' AND guid != '%u'", accId, player->GetGUIDLow());
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                plName = fields[0].GetString();
                if (sObjectAccessor->FindPlayerByName1(plName)) continue;
                names->insert(names->end(), plName);
            } while(results->NextRow());
        }

        if (names->empty())
        {
            player->CLOSE_GOSSIP_MENU();
            return;
        }

        PlayerbotMgr* mgr = player->GetPlayerbotMgr();
        if (!mgr)
        {
            mgr = new PlayerbotMgr(player);
            player->SetPlayerbotMgr(mgr);
        }

        int8 x = action - GOSSIP_ACTION_INFO_DEF - 1;

        for (std::list<std::string>::iterator iter = names->begin(); iter != names->end(); iter++)
        {
            if (x == 0)
            {
                uint64 guid = sObjectMgr->GetPlayerGUIDByName((*iter).c_str());
                if (mgr->GetPlayerBot(guid) != NULL)
                    return;
                mgr->AddPlayerBot(guid);
            }
            else
            {
                if (x == 1)
                {
                    uint64 guid = sObjectMgr->GetPlayerGUIDByName((*iter).c_str());
                    if (mgr->GetPlayerBot(guid) != NULL)
                        return;
                    mgr->AddPlayerBot(guid);
                    break;
                }
                --x;
            }
        }
        player->CLOSE_GOSSIP_MENU();
        delete names;
    }

    static void SendCreatePlayerBotMenu(Player* player, Creature* creature, uint32 /*action*/)
    {
        WorldSession* session = player->GetSession();
        std::string plName;
        std::list<std::string>* names = new std::list<std::string>;
        uint32 accId = player->GetSession()->GetAccountId();
        QueryResult results = CharacterDatabase.PQuery("SELECT name FROM characters WHERE account = '%u' AND guid != '%u'", accId, player->GetGUIDLow());
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                plName = fields[0].GetString();
                if (sObjectAccessor->FindPlayerByName1(plName)) continue;
                names->insert(names->end(), plName);
            } while(results->NextRow());
        }

        if (names->empty())
        {
            player->CLOSE_GOSSIP_MENU();
            return;
        }

        player->PlayerTalkClass->ClearMenus();
        player->ADD_GOSSIP_ITEM(9, session->GetTrinityString(11136) , CREATE_PBOT, GOSSIP_ACTION_INFO_DEF + 1);
        int8 x = 2;

        for (std::list<std::string>::iterator iter = names->begin(); iter != names->end(); iter++)
        {
            player->ADD_GOSSIP_ITEM(9, (*iter).c_str() , CREATE_PBOT, GOSSIP_ACTION_INFO_DEF + x);
            ++x;
        }
        player->PlayerTalkClass->SendGossipMenu(8446, creature->GetGUID());

        delete names;
    } //end SendCreatePlayerBotMenu

    static void SendRemovePlayerBotAll(Player* player, Creature* creature)
    {
        for (int8 x = 2; x<=10; x++ )
        {
            SendRemovePlayerBot(player, creature, GOSSIP_ACTION_INFO_DEF + 2);
        }
    }

    static void SendRemovePlayerBot(Player* player, Creature* creature, uint32 action)
    {
        int8 x = action - GOSSIP_ACTION_INFO_DEF - 1;

        if (x == 0)
        {
            SendRemovePlayerBotAll(player, creature);
            return;
        }

        if (PlayerbotMgr* mgr = player->GetPlayerbotMgr())
        {
            for (PlayerBotMap::const_iterator itr = mgr->GetPlayerBotsBegin(); itr != mgr->GetPlayerBotsEnd(); ++itr)
            {
                if (x == 1 && itr->second)
                {
                    mgr->LogoutPlayerBot(itr->second->GetGUID());
                    break;
                }
                --x;
            }
        }
        player->CLOSE_GOSSIP_MENU();
    } //end SendRemovePlayerBot

    static void SendRemovePlayerBotMenu(Player* player, Creature* creature, uint32 /*action*/)
    {
        WorldSession* session = player->GetSession();
        player->PlayerTalkClass->ClearMenus();
        player->ADD_GOSSIP_ITEM(9, session->GetTrinityString(11135), REMOVE_PBOT, GOSSIP_ACTION_INFO_DEF + 1);

        uint8 x = 2;
        if (PlayerbotMgr* mgr = player->GetPlayerbotMgr())
        {
            for (PlayerBotMap::const_iterator itr = mgr->GetPlayerBotsBegin(); itr != mgr->GetPlayerBotsEnd(); ++itr)
            {
                Player* bot = itr->second;
                player->ADD_GOSSIP_ITEM(9, bot->GetName(), REMOVE_PBOT, GOSSIP_ACTION_INFO_DEF + x);
                ++x;
            }
        }
        player->PlayerTalkClass->SendGossipMenu(8446, creature->GetGUID());
    } //end SendRemovePlayerBotMenu

    static void SendRemoveNPCBot(Player* player, Creature*  /*creature*/, uint32 action)
    {
        int8 x = action - GOSSIP_ACTION_INFO_DEF;
        if (x == 1)
        {
            player->CLOSE_GOSSIP_MENU();
            for (uint8 i = 0; i != player->GetMaxNpcBots(); ++i)
                player->RemoveBot(player->GetBotMap(i)->_Guid(), true);
            return;
        }
        for (uint8 i = 0; i != player->GetMaxNpcBots(); ++i)
        {
            if (!player->GetBotMap(i)->_Cre())
                continue;
            if (x == 2)
            {
                player->RemoveBot(player->GetBotMap(i)->_Guid(), true);
                break;
            }
            --x;
        }
        player->CLOSE_GOSSIP_MENU();
    }

    static void SendRemoveNPCBotMenu(Player* player, Creature* creature, uint32 /*action*/)
    {
        WorldSession* session = player->GetSession();
        player->PlayerTalkClass->ClearMenus();
        if (player->GetNpcBotsCount() == 1)
        {
            for (uint8 i = 0; i != player->GetMaxNpcBots(); ++i)
                player->RemoveBot(player->GetBotMap(i)->_Guid(), true);
            player->CLOSE_GOSSIP_MENU();
            return;
        }
        player->ADD_GOSSIP_ITEM(9, session->GetTrinityString(11135), REMOVE_NBOT, GOSSIP_ACTION_INFO_DEF + 1);

        uint8 x = 2;
        for (uint8 i = 0; i != player->GetMaxNpcBots(); ++i)
        {
            Creature* bot = player->GetBotMap(i)->_Cre();
            if (!bot) continue;
            player->ADD_GOSSIP_ITEM(9, bot->GetName(), REMOVE_NBOT, GOSSIP_ACTION_INFO_DEF + x);
            ++x;
        }
        player->PlayerTalkClass->SendGossipMenu(8446, creature->GetGUID());
    }

    static void SendCreateNPCBot(Player* player, Creature*  /*creature*/, uint32 action)
    {
        uint8 bot_class = 0;
        if (action == GOSSIP_ACTION_INFO_DEF + 1)//"Back"
        {
            player->CLOSE_GOSSIP_MENU();
            return;
        }
        else if (action == GOSSIP_ACTION_INFO_DEF + 2)
            bot_class = CLASS_WARRIOR;
        //else if (action == GOSSIP_ACTION_INFO_DEF + 3)
        //    bot_class = CLASS_HUNTER;
        else if (action == GOSSIP_ACTION_INFO_DEF + 4)
            bot_class = CLASS_PALADIN;
        else if (action == GOSSIP_ACTION_INFO_DEF + 5)
            bot_class = CLASS_SHAMAN;
        else if (action == GOSSIP_ACTION_INFO_DEF + 6)
            bot_class = CLASS_ROGUE;
        else if (action == GOSSIP_ACTION_INFO_DEF + 7)
            bot_class = CLASS_DRUID;
        else if (action == GOSSIP_ACTION_INFO_DEF + 8)
            bot_class = CLASS_MAGE;
        else if (action == GOSSIP_ACTION_INFO_DEF + 9)
            bot_class = CLASS_PRIEST;
        else if (action == GOSSIP_ACTION_INFO_DEF + 10)
            bot_class = CLASS_WARLOCK;
        //else if (action == GOSSIP_ACTION_INFO_DEF + 11)
        //    bot_class = CLASS_DEATH_KNIGHT;

        if (bot_class != 0)
            player->CreateNPCBot(bot_class);
        player->CLOSE_GOSSIP_MENU();
        return;
    }

    static void SendCreateNPCBotMenu(Player* player, Creature* creature, uint32 /*action*/)
    {
        WorldSession* session = player->GetSession();
        std::string str = player->GetNpcBotCostStr();

        player->PlayerTalkClass->ClearMenus();
        player->ADD_GOSSIP_ITEM(9, session->GetTrinityString(11137) + str, CREATE_NBOT, GOSSIP_ACTION_INFO_DEF + 2);
        //player->ADD_GOSSIP_ITEM(9, session->GetTrinityString(11138) + str, CREATE_NBOT, GOSSIP_ACTION_INFO_DEF + 3);
        player->ADD_GOSSIP_ITEM(9, session->GetTrinityString(11139) + str, CREATE_NBOT, GOSSIP_ACTION_INFO_DEF + 4);
        player->ADD_GOSSIP_ITEM(9, session->GetTrinityString(11140) + str, CREATE_NBOT, GOSSIP_ACTION_INFO_DEF + 5);
        player->ADD_GOSSIP_ITEM(9, session->GetTrinityString(11141) + str, CREATE_NBOT, GOSSIP_ACTION_INFO_DEF + 6);
        player->ADD_GOSSIP_ITEM(3, session->GetTrinityString(11142) + str, CREATE_NBOT, GOSSIP_ACTION_INFO_DEF + 7);
        player->ADD_GOSSIP_ITEM(3, session->GetTrinityString(11143) + str, CREATE_NBOT, GOSSIP_ACTION_INFO_DEF + 8);
        player->ADD_GOSSIP_ITEM(3, session->GetTrinityString(11144) + str, CREATE_NBOT, GOSSIP_ACTION_INFO_DEF + 9);
        player->ADD_GOSSIP_ITEM(3, session->GetTrinityString(11145) + str, CREATE_NBOT, GOSSIP_ACTION_INFO_DEF + 10);
        //player->ADD_GOSSIP_ITEM(9, session->GetTrinityString(11146) + str, CREATE_NBOT, GOSSIP_ACTION_INFO_DEF + 11);
        player->PlayerTalkClass->SendGossipMenu(8446, creature->GetGUID());
    }

    static void SendBotHelpWhisper(Player* player, Creature* creature, uint32 /*action*/)
    {
        player->CLOSE_GOSSIP_MENU();
        //Basic
        std::string msg1 = "爪牙是一个强力的伙伴，他可以帮助你克服一个人无法完成的副本!爪牙是永久存在的,在召唤爪牙之后，召唤的爪牙越多，你获得的经验值就越低!";
        creature->MonsterWhisper(msg1.c_str(), player->GetGUID());
        //Heal Icons
        uint8 mask = ConfigMgr::GetIntDefault("Bot.HealTargetIconsMask", 8);
        std::string msg4 = "";
        if (mask == 255)
        {
            msg4 = "If you want your npcbots to heal someone out of your party set any raid target icon on them";
            creature->MonsterWhisper(msg4.c_str(), player->GetGUID());
        }
        else if (mask != 0)
        {
            msg4 = "If you want your npcbots to heal someone out of your party set proper raid target icon on them, one of these: ";
            std::string iconrow = "";
            uint8 count = 0;
            for (uint8 i = 0; i != TARGETICONCOUNT; ++i)
            {
                if (mask & GroupIcons[i])
                {
                    if (count != 0)
                        iconrow += ", ";
                    ++count;
                    switch (i)
                    {
                        case 0: iconrow += "star"; break;
                        case 1: iconrow += "circle"; break;
                        case 2: iconrow += "diamond"; break;
                        case 3: iconrow += "triangle"; break;
                        case 4: iconrow += "moon"; break;
                        case 5: iconrow += "square"; break;
                        case 6: iconrow += "cross"; break;
                        case 7: iconrow += "skull"; break;
                        //debug
                        default: iconrow += "unknown icon"; break;
                    }
                }
            }
            msg4 += iconrow;
            creature->MonsterWhisper(msg4.c_str(), player->GetGUID());
        }
        //End
    }
};

//This function is called when the player clicks an option on the gossip menu
void AddSC_script_bot_giver()
{
    new script_bot_giver();
}
