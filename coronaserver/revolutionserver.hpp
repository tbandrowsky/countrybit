
#include "corona.hpp"

#include <windows.h>
#include <iostream>

namespace corona::apps
{   
    class revolution_server
    {
    public:

        bool operator()(comm_bus_service* _service, json& _command)
        {
            std::string class_name = _command["class_name"];

            /*
                {
      "class_name": "select_clear_command",
      "class_description": "command for one piece to activate another",
      "base_class_name": "sys_command",
      "fields": {
        "board_id": "->board",
        "page_id": "->page",
        "actor_id": "->actor"
      }
    },
    {
      "class_name": "select_extend_command",
      "class_description": "command for one piece to activate another",
      "base_class_name": "sys_command",
      "fields": {
        "board_id": "->board",
        "page_id": "->page",
        "actor_id": "->actor",
        "target_actor_id": "[ ->actor ]"
      }
    },
    {
      "class_name": "compose_command",
      "class_description": "command to consume selected pieces to create another",
      "base_class_name": "sys_command",
      "fields": {
        "board_id": "->board",
        "page_id": "->page",
        "actor_id": "->actor",
        "part_type": "string"
      }
    },
    {
      "class_name": "take_command",
      "class_description": "command to consume selected pieces to create another",
      "base_class_name": "sys_command",
      "fields": {
        "board_id": "->board",
        "page_id": "->page",
        "actor_id": "->actor",
        "part_type": "string"
      }
    },
    {
      "class_name": "drop_command",
      "class_description": "command to consume selected pieces to create another",
      "base_class_name": "sys_command",
      "fields": {
        "board_id": "->board",
        "page_id": "->page",
        "actor_id": "->actor",
        "part_type": "string"
      }
    },
    {
      "class_name": "trade_command",
      "class_description": "command to consume selected pieces to create another",
      "base_class_name": "sys_command",
      "fields": {
        "board_id": "->board",
        "page_id": "->page",
        "actor_id": "->actor",
        "counterparty": "->actor",
        "duecounterparty": "[ ->actor ]",
        "dueparty": "[ ->actor ]",
        "approval_party": "string",
        "approval_counterparty": "string"
      }
    },
    {
      "class_name": "accelerate_command",
      "class_description": "command to move selected pieces",
      "base_class_name": "sys_command",
      "fields": {
        "board_id": "->board",
        "page_id": "->page",
        "actor_id": "->actor",
        "target_actor_id": "->actor",
        "ax": "double",
        "ay": "double",
        "az": "double"
      }
    },
    {
      "class_name": "activate_command",
      "class_description": "command for one piece to activate another",
      "base_class_name": "sys_command",
      "fields": {
        "board_id": "->board",
        "page_id": "->page",
        "actor_id": "->actor",
        "target_actor_id": "->actor"
      }
    },
    {
      "class_name": "navigate_command",
      "class_description": "",
      "base_class_name": "sys_command",
      "fields": {
        "board_id": "->board",
        "page_id": "->page",
        "actor_id": "->actor"
      }
    },
    {
      "class_name": "join_game_command",
      "class_description": "command to join a game",
      "base_class_name": "sys_command",
      "fields": {
        "game_id": "->game"
      }
    },
    {
      "class_name": "exit_game_command",
      "class_description": "command to leave a game",
      "base_class_name": "sys_command",
      "fields": {
        "game_id": "->game"
      }
    }
            */

            if (class_name == "select_clear_command")
            {
                ;
            }
            else if (class_name == "select_extend_command")
            {
                ;
            }
            else if (class_name == "compose_command")
            {
                _service->get_data;
            }
            else if (class_name == "take_command")
            {
                ;
            }
            else if (class_name == "drop_command")
            {
                ;
            }
            else if (class_name == "trade_command")
            {
                ;
            }
            else if (class_name == "accelerate_command")
            {
                ;
            }
            else if (class_name == "activate_command")
            {
                ;
            }
            else if (class_name == "navigate_command")
            {
                ;
            }
            else if (class_name == "join_game_command")
            {
                ;
            }
            else if (class_name == "exit_game_command")
            {
                ;
            }

            return true;
        }
    };

}
