
#include "corona.hpp"

#include <windows.h>
#include <iostream>

namespace corona::apps::revolution
{   

    class base_object
    {
    public:
        int64_t object_id;
        date_time created;
        std::string created_by;
        date_time modified;
        std::string modified_by;

        virtual void put_json(json& _src)
        {
            object_id = (int64_t)_src["object_id"];
            created = (date_time)_src["created"];
            created_by = (std::string)_src["created_by"];
            modified = (date_time)_src["updated"];
            modified_by = (std::string)_src["updated_by"];
        }

        virtual void get_json(json& _dest)
        {
            ;
        }
    };

    class actor : public base_object
    {
    public:
        std::string name;
        std::string type;
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double vx = 0.0;
        double vy = 0.0;
        double vz = 0.0;
        std::vector<std::string> parts; // part ids

        virtual void put_json(json& _src) override
        {
            base_object::put_json(_src);
        }

        virtual void get_json(json& _dest) override
        {
            base_object::get_json(_dest);
        }

    };

    class board : public base_object
    {
    public:
        std::vector<actor> actors;

        virtual void put_json(json& _src) override
        {
            base_object::put_json(_src);
        }

        virtual void get_json(json& _dest) override
        {
            base_object::get_json(_dest);
        }

    };

    class game : public base_object
    {
    public:
        std::map<std::string, std::shared_ptr<board>> boards;
        
        void put_json(json& _src)
        {
            base_object::put_json(_src);
        }

        void get_json(json& _dest)
        {
            base_object::get_json(_dest);
        }

    };

    class revolution_server
    {

        

    public:

        void clear_selection(comm_bus_service* _service, json& _command)
        {

        }

        void extend_selection(comm_bus_service* _service, json& _command)
        {

        }

        void compose(comm_bus_service* _service, json& _command)
        {

        }

        void take(comm_bus_service* _service, json& _command)
        {

        }

        void drop(comm_bus_service* _service, json& _command)
        {

        }

        void trade(comm_bus_service* _service, json& _command)
        {

        }

        void accelerate(comm_bus_service* _service, json& _command)
        {

        }

        void activate(comm_bus_service* _service, json& _command)
        {

        }

        void navigate(comm_bus_service* _service, json& _command)
        {

        }

        void join_game(comm_bus_service* _service, json& _command)
        {

        }

        void exit_game(comm_bus_service* _service, json& _command)
        {

        }

        bool operator()(comm_bus_service* _service, json& _command)
        {
            std::string class_name = _command["class_name"];

            if (class_name == "select_clear_command")
            {
                clear_selection(_service, _command);
            }
            else if (class_name == "select_extend_command")
            {
                extend_selection(_service, _command);;
            }
            else if (class_name == "compose_command")
            {
                compose(_service, _command);
            }
            else if (class_name == "take_command")
            {
                take(_service, _command);
            }
            else if (class_name == "drop_command")
            {
                drop(_service, _command);
            }
            else if (class_name == "trade_command")
            {
                trade(_service, _command);
            }
            else if (class_name == "accelerate_command")
            {
                accelerate(_service, _command);
            }
            else if (class_name == "activate_command")
            {
                activate(_service, _command);
            }
            else if (class_name == "navigate_command")
            {
                navigate(_service, _command);
            }
            else if (class_name == "join_game_command")
            {
                join_game(_service, _command);
            }
            else if (class_name == "exit_game_command")
            {
                exit_game(_service, _command);
            }

            return true;
        }
    };

}
