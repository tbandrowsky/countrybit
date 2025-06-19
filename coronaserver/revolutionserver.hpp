
#include "corona.hpp"

#include <windows.h>
#include <iostream>

namespace corona::apps::revolution
{   
    void get_json(json& _dest, object_reference_type& _src)
    {
        _dest.put_member("class_name", _src.class_name);
        _dest.put_member_i64("object_id", _src.object_id);
    }

    void put_json(object_reference_type& _dest, json& _src)
    {
        _dest.class_name = _src["class_name"];
        _dest.object_id = (int64_t)_src["object_id"];
    }

    class base_object
    {
    public:
        std::string class_name;
        int64_t object_id;
        date_time created;
        std::string created_by;
        date_time updated;
        std::string updated_by;

        virtual void put_json(json& _src)
        {
            class_name = (int64_t)_src["class_name"];
            object_id = (int64_t)_src["object_id"];
            created = (date_time)_src["created"];
            created_by = (std::string)_src["created_by"];
            updated = (date_time)_src["updated"];
            updated_by = (std::string)_src["updated_by"];
        }

        virtual void get_json(json& _dest)
        {
            _dest.put_member_i64("object_id", object_id);
            _dest.put_member("class_name", class_name);
            _dest.put_member("created", created);
            _dest.put_member("created_by", created_by);
            _dest.put_member("updated", updated);
            _dest.put_member("updated", updated_by);
        }
    };


    class actor : public base_object
    {
    public:
        std::string name;
        std::string type;
        std::string state;

        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double dx = 0.0;
        double dy = 0.0;
        double dz = 0.0;
        double w = 0.0;
        double h = 0.0;
        double ax = 0.0;
        double ay = 0.0;
        double az = 0.0;

        std::vector<std::shared_ptr<actor>> children;
        std::vector<object_reference_type> selection;

        virtual void put_json(json& _src) override
        {
            base_object::put_json(_src);

            name = _src["name"];
            type = _src["type"];
            state = _src["state"];

            x = (double)_src["x"];
            y = (double)_src["y"];
            z = (double)_src["z"];

            dx = (double)_src["dx"];
            dy = (double)_src["dy"];
            dz = (double)_src["dz"];

            ax = (double)_src["ax"];
            ay = (double)_src["ay"];
            az = (double)_src["az"];

            json jchildren = _src["children"];
            if (jchildren.array()) {
                children.clear();
                for (json jchild : jchildren)
                {
                    std::shared_ptr<actor> sactor = std::make_shared<actor>();
                    sactor->put_json(jchild);
                    children.push_back(sactor);
                }
            }

            json jselection = _src["selection"];
            if (jselection.array()) {
                selection.clear();
                for (json jselected : jselection)
                {
                    object_reference_type xsort;
                    corona::apps::revolution::put_json(xsort, jselected);
                    selection.push_back(xsort);
                }
            }
        }

        virtual void get_json(json& _dest) override
        {
            base_object::get_json(_dest);

            json_parser jp; 
            json jselection = jp.create_array();

			_dest.put_member("name", name);
			_dest.put_member("type", type);
			_dest.put_member("state", state);   

            _dest.put_member("x", x);
			_dest.put_member("y", y);
			_dest.put_member("z", z);

            _dest.put_member("dx", dx);
            _dest.put_member("dy", dy);
            _dest.put_member("dz", dz);

            json jchildren = jp.create_array();

        }

    };

    class board : public base_object
    {
    public:

        std::string name;
        std::string type;
        std::string state;

        std::map<int64_t, std::shared_ptr<actor>> actors;

        virtual void put_json(json& _src) override
        {
            base_object::put_json(_src);

            actors.clear();

			json actors_json = _src["actors"];
            if (actors_json.array()) {
                for (auto actorj : actors_json)
                {
                    std::shared_ptr<actor> act = std::make_shared<actor>();
                    act->put_json(actorj);
                    actors.insert_or_assign(act->object_id, act);
                }
            }
        }

        virtual void get_json(json& _dest) override
        {
            base_object::get_json(_dest);
            json_parser jp;
			json jactors_json = jp.create_array();

            for (const auto& act : actors) {
                json jactor = jp.create_object();
				act.second->get_json(jactor);
				jactors_json.push_back(jactor);
            }
			_dest.put_member("actors", jactors_json);
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

        template <typename board_type> std::shared_ptr<board_type> get_object(comm_bus_service* _service, std::string _class_name, int64_t _object_id, bool _include_children)
        {
            std::shared_ptr<board_type> result = nullptr;

            json response = _service->get_object(_class_name, _object_id, _include_children);

            if (response[success_field] == false)
            {
                // Handle error case
                return nullptr;
            }

            result = std::make_shared<board_type>();

            json result_data = response["result"];
            if (result_data.object())
            {
                result->put_json(result_data);
            }

            // This is a placeholder for the actual implementation
            // that would retrieve a board by its ID.
            return result;
        }

    public:

        std::shared_ptr<board> get_board(comm_bus_service* _service, object_reference_type& _ort, bool _recursive)
        {
            // This is a placeholder for the actual implementation
            // that would retrieve a board by its ID.
            return get_object<board>(_service, "board", _ort.object_id, _recursive);
        }

        std::shared_ptr<game> get_game(comm_bus_service* _service, object_reference_type& _ort, bool _recursive)
        {
            // This is a placeholder for the actual implementation 
            // that would retrieve a board by its ID.
            return get_object<game>(_service, "game", _ort.object_id, _recursive);
        }

        std::shared_ptr<actor> get_actor(comm_bus_service* _service, object_reference_type& _ort, bool _recursive)
        {
            // This is a placeholder for the actual implementation 
            // that would retrieve a board by its ID.
            return get_object<actor>(_service, "actor", _ort.object_id, _recursive);
        }

        std::shared_ptr<board> get_board(comm_bus_service* _service, json& _src_command, bool _recursive)
        {
            std::shared_ptr<board> ret;
            // This is a placeholder for the actual implementation
            // that would retrieve a board by its ID.
            json ref = _src_command["board"];
            if (ref.reference()) {
                ref.reference_impl()->value.class_name;
                ref.reference_impl()->value.object_id;
                ret = get_board(_service, _src_command, true);
            }
            return ret;
        }

        std::shared_ptr<game> get_game(comm_bus_service* _service, json& _src_command, bool _recursive)
        {
            std::shared_ptr<game> ret;
            // This is a placeholder for the actual implementation
            // that would retrieve a board by its ID.
            json ref = _src_command["game"];
            if (ref.reference()) {
                ref.reference_impl()->value.class_name;
                ref.reference_impl()->value.object_id;
                ret = get_game(_service, _src_command, true);
            }
            return ret;
        }

        std::shared_ptr<actor> get_actor(comm_bus_service* _service, json& _src_command, bool _recursive)
        {
            // This is a placeholder for the actual implementation 
            // that would retrieve a board by its ID.
            std::shared_ptr<actor> ret;
            // This is a placeholder for the actual implementation
            // that would retrieve a board by its ID.
            json ref = _src_command["board"];
            if (ref.reference()) {
                ref.reference_impl()->value.class_name;
                ref.reference_impl()->value.object_id;
                ret = get_actor(_service, _src_command, true);
            }
            return ret;
        }

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
