#include "coronaserver.hpp"

namespace corona::apps::insurance
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

    class insurance_simulation : public corona_simulation_interface
    {


        template <typename object_type> std::shared_ptr<object_type> from_object(json& _object)
        {
            std::shared_ptr<object_type> result = nullptr;

            result = std::make_shared<object_type>();

            if (_object.object())
            {
                result->put_json(_object);
            }

            return result;
        }

        template <typename object_type> std::shared_ptr<object_type> get_object(std::string _class_name, int64_t _object_id, bool _include_children)
        {
            std::shared_ptr<object_type> result = nullptr;

            json response = service->get_object(_class_name, _object_id, _include_children);

            if ((bool)response[success_field] == false)
            {
                // Handle error case
                return nullptr;
            }

            result = std::make_shared<object_type>();

            json result_data = response["data"];
            if (result_data.object())
            {
                result->put_json(result_data);
            }

            return result;
        }

        template <typename object_type> void put_object(std::shared_ptr<object_type> _obj)
        {
            json_parser jp;
            json obj = jp.create_object();

            _obj->get_json(obj);

            json response = service->put_object(obj);

            if (response[success_field])
            {
                json result_data = response["data"];

                if (result_data.object())
                {
                    _obj->put_json(result_data);
                }
            }
        }

        template <typename object_type> std::shared_ptr<object_type> create_object(std::string _class_name)
        {
            json_parser jp;
            std::shared_ptr<object_type> result = nullptr;

            json response = service->create_object(_class_name);

            if (response[success_field])
            {
                json result_data = response["data"];
                if (result_data.object())
                {
                    result = std::make_shared<object_type>();
                    result->put_json(result_data);
                }
            }
            return result;
        }

        std::string get_command_user(json _token)
        {
            ;
        }

    public:

        virtual void on_frame(json& _commands)
        {
            if (_commands.object())
            {
                execute(_commands);
            }
            else if (_commands.array())
            {
                for (auto command : _commands)
                {
                    execute(command);
                }
            }
        }

        virtual void execute(json& _command)
        {
            std::string class_name = _command["class_name"];
            int64_t object_id = (int64_t)_command["object_id"];

            try {

                if (class_name == "")
                {
                }
                else if (class_name == "")
                {
                }

                service->delete_object(class_name, object_id);
            }
            catch (std::exception& e)
            {
                service->log_exception(e, __FILE__, __LINE__);
            }
        }
    };

}

int main(int argc, char* argv[])
{
    char exePath[MAX_PATH];

    // Get the full path of the executable
    if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0) {
        std::cerr << "Failed to get executable path. Error: " << GetLastError() << std::endl;
        return 1;
    }

    // Convert to std::string for easier manipulation
    std::string path(exePath);

    // Find the last backslash to isolate the directory
    size_t lastSlash = path.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        path = path.substr(0, lastSlash); // Extract the directory
    }

    // Set the current working directory
    if (!SetCurrentDirectory(path.c_str())) {
        std::cerr << "Failed to set working directory. Error: " << GetLastError() << std::endl;
        return 1;
    }


    std::shared_ptr<corona::apps::insurance::insurance_simulation> simulation = std::make_shared<corona::apps::insurance::insurance_simulation>();
    return CoronaMain(simulation, argc, argv);
}


