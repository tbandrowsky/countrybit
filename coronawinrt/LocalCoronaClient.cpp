#include "pch.h"
#include "LocalCoronaClient.h"
#include "LocalCoronaClient.g.cpp"

namespace winrt::coronawinrt::implementation
{

    class json_interop
    {
        void put_json(ClassDefinition& _dest, corona::json _src)
        {

            _dest.ClassName(winrt::to_hstring((std::string)_src[class_name_field]));
            _dest.ClassDescription(winrt::to_hstring((std::string)_src["class_description"]));
            _dest.BaseClassName(winrt::to_hstring((std::string)_src["base_class_name"]));

            Windows::Foundation::Collections::IVector<hstring> parents{ winrt::single_threaded_vector<hstring>() };
            Windows::Foundation::Collections::IVector<hstring> ancestors{ winrt::single_threaded_vector<hstring>() };
            Windows::Foundation::Collections::IVector<hstring> descendants{ winrt::single_threaded_vector<hstring>() };
            Windows::Foundation::Collections::IVector<FieldDefinition> fields{ winrt::single_threaded_vector<FieldDefinition>() };
            Windows::Foundation::Collections::IVector<IndexDefinition> indexes{ winrt::single_threaded_vector<IndexDefinition>() };

            json jparents = _src["parents"];
            if (jparents.array()) {
                for (auto jparent : jparents) {
                    parents.Append(winrt::to_hstring((std::string)jparent));
                }
            }
            _dest.Parents(parents);

            json jancestors = _src["ancestors"];
            if (jancestors.array()) {
                for (auto jancestor : jancestors) {
                    ancestors.Append(winrt::to_hstring((std::string)jancestor));
                }
            }
            _dest.AncestorClasses(ancestors);

            json jdescendants = _src["descendants"];
            if (jdescendants.array()) {
                for (auto jdescendant : jdescendants) {
                    descendants.Append(winrt::to_hstring((std::string)jdescendant));
                }
            }

            _dest.DescendantClasses(descendants);

            json jfields = _src["fields"];
            if (jfields.object()) {
                auto jmembers = jfields.get_members();
                for (auto jmember : jmembers) {
                    FieldDefinition new_field = winrt::make<FieldDefinition>();
                    put_json(new_field, jmember.second);
                    fields.Append(new_field);
                }
            }
            _dest.Fields(fields);

            json jindexes = _src["indexes"];
            if (jindexes.object()) {
                auto jmembers = jindexes.get_members();
                for (auto jmember : jmembers) {
                    IndexDefinition new_field = winrt::make<IndexDefinition>();
                    put_json(new_field, jmember.second);
                    indexes.Append(new_field);
                }
            }
            _dest.Indexes(indexes);
        }

        void get_json(corona::json& _dest, ClassDefinition& _src)
        {
            json_parser jp;

            _dest.put_member(class_name_field, winrt::to_string(_src.ClassName()));
            _dest.put_member("class_description", winrt::to_string(_src.ClassDescription()));
            _dest.put_member("base_class_name", winrt::to_string(_src.BaseClassName()));

            json ja = jp.create_array();
            for (auto p : _src.Parents())
            {
                ja.push_back(winrt::to_string(p));
            }
            _dest.put_member("parents", ja);

            if (_src.Fields().Size() > 0) {
                json jtable_fields = jp.create_array();
                for (auto tf : _src.Fields()) {
                    jtable_fields.push_back(winrt::to_string(tf.FieldName()));
                }
                _dest.put_member("table_fields", jtable_fields);
            }

            if (_src.Fields().Size() > 0) {
                json jfield_object = jp.create_object();
                for (auto field : _src.Fields()) {
                    json jfield_definition = jp.create_object();
                    get_json(jfield_definition, field);
                    jfield_object.put_member(winrt::to_string(field.FieldName()), jfield_definition);
                }
                _dest.put_member("fields", jfield_object);
            }

            if (_src.Indexes().Size() > 0) {
                json jindex_object = jp.create_object();
                for (auto index : _src.Indexes()) {
                    json jindex_definition = jp.create_object();
                    get_json(jindex_definition, index);
                    jindex_object.put_member(winrt::to_string(index.IndexName()), jindex_definition);
                }
                _dest.put_member("indexes", jindex_object);
            }

            if (_src.AncestorClasses().Size() > 0) {
                json jancestor_array = jp.create_array();
                for (auto class_ancestor : _src.AncestorClasses()) {
                    jancestor_array.push_back(winrt::to_string(class_ancestor));
                }
                _dest.put_member("ancestors", jancestor_array);
            }

            if (_src.DescendantClasses().Size() > 0) {
                json jdescendants_array = jp.create_array();
                for (auto class_descendant : _src.DescendantClasses()) {
                    jdescendants_array.push_back(winrt::to_string(class_descendant));
                }
                _dest.put_member("descendants", jdescendants_array);
            }

            /*
            if (sql) {
                json jsql = jp.create_object();
                sql->get_json(jsql);
                _dest.put_member("sql", jsql);
            }
            */

        }

        void put_json(FieldDefinition& _dest, json _src)
        {
            auto fns = (std::string)_src["field_name"];
            _dest.FieldName(winrt::to_hstring(fns));
            fns = (std::string)_src["field_description"];
            _dest.FieldDescription(winrt::to_hstring(fns));
            fns = (std::string)_src["field_type"];

            if (fns == field_type_names[field_types::ft_array])
            {
                _dest.FieldType(FieldTypes::FieldArray);
                ArrayFieldOptions afo = winrt::make<ArrayFieldOptions>();
                put_json(afo, _src);
                _dest.FieldOptions(afo);
            }
            else if (fns == field_type_names[field_types::ft_object])
            {
                _dest.FieldType(FieldTypes::FieldObject);
                ObjectFieldOptions afo = winrt::make<ObjectFieldOptions>();
                put_json(afo, _src);
                _dest.FieldOptions(afo);
            }
            else if (fns == field_type_names[field_types::ft_blob])
            {
                _dest.FieldType(FieldTypes::FieldBlob);
            }
            else if (fns == field_type_names[field_types::ft_query])
            {
                _dest.FieldType(FieldTypes::FieldQuery);
                QueryFieldOptions afo = winrt::make<QueryFieldOptions>();
                put_json(afo, _src);
                _dest.FieldOptions(afo);
            }
            else if (fns == field_type_names[field_types::ft_reference])
            {
                _dest.FieldType(FieldTypes::FieldReference);
            }
            else if (fns == field_type_names[field_types::ft_int64])
            {
                _dest.FieldType(FieldTypes::FieldInt64);
                Int64FieldOptions afo = winrt::make<Int64FieldOptions>();
                put_json(afo, _src);
                _dest.FieldOptions(afo);
            }
            else if (fns == field_type_names[field_types::ft_double])
            {
                _dest.FieldType(FieldTypes::FieldDouble);
                DoubleFieldOptions afo = winrt::make<DoubleFieldOptions>();
                put_json(afo, _src);
                _dest.FieldOptions(afo);
            }
            else if (fns == field_type_names[field_types::ft_string])
            {
                _dest.FieldType(FieldTypes::FieldString);
                StringFieldOptions afo = winrt::make<StringFieldOptions>();
                put_json(afo, _src);
                _dest.FieldOptions(afo);
            }
            else if (fns == field_type_names[field_types::ft_function])
            {
                _dest.FieldType(FieldTypes::FieldFunction);
            }
        }

        void get_json(json _dest, FieldDefinition& _src)
        {
            _dest.put_member("field_name", winrt::to_string(_src.FieldName()));
            _dest.put_member("field_description", winrt::to_string(_src.FieldDescription()));

            if (_src.FieldType() == FieldTypes::FieldArray)
            {
                _dest.put_member("field_type", field_type_names[field_types::ft_array]);
                auto afo = _src.FieldOptions().as<ArrayFieldOptions>();
                get_json(_dest, afo);
            }
            else if (_src.FieldType() == FieldTypes::FieldObject)
            {
                _dest.put_member("field_type", field_type_names[field_types::ft_object]);
                auto afo = _src.FieldOptions().as<ObjectFieldOptions>();
                get_json(_dest, afo);
            }
            else if (_src.FieldType() == FieldTypes::FieldString)
            {
                _dest.put_member("field_type", field_type_names[field_types::ft_string]);
                auto afo = _src.FieldOptions().as<StringFieldOptions>();
                get_json(_dest, afo);
            }
            else if (_src.FieldType() == FieldTypes::FieldDateTime)
            {
                _dest.put_member("field_type", field_type_names[field_types::ft_datetime]);
                auto afo = _src.FieldOptions().as<DateTimeFieldOptions>();
                get_json(_dest, afo);
            }
            else if (_src.FieldType() == FieldTypes::FieldDouble)
            {
                _dest.put_member("field_type", field_type_names[field_types::ft_double]);
                auto afo = _src.FieldOptions().as<DoubleFieldOptions>();
                get_json(_dest, afo);
            }
            else if (_src.FieldType() == FieldTypes::FieldQuery)
            {
                _dest.put_member("field_type", field_type_names[field_types::ft_query]);
                auto afo = _src.FieldOptions().as<QueryFieldOptions>();
                get_json(_dest, afo);
            }
            else if (_src.FieldType() == FieldTypes::FieldBlob)
            {
                _dest.put_member("field_type", field_type_names[field_types::ft_blob]);
            }
            else if (_src.FieldType() == FieldTypes::FieldReference)
            {
                _dest.put_member("field_type", field_type_names[field_types::ft_reference]);
            }
        }

        void put_json(IndexDefinition& _dest, json _src)
        {
            Windows::Foundation::Collections::IVector<hstring> keys{ winrt::single_threaded_vector<hstring>() };

            std::string iname = _src["index_name"];
            json jkeys = _src["index_keys"];
            if (jkeys.array()) {
                for (auto jkey : jkeys) {
                    keys.Append(winrt::to_hstring((std::string)jkey));
                }
            }

            _dest.IndexName(winrt::to_hstring(iname));
            _dest.IndexKeys(keys);
        }

        void get_json(json& _dest, IndexDefinition& _src)
        {
            _dest.put_member("index_name", winrt::to_string(_src.IndexName()));

            json_parser jp;
            json jpa = jp.create_array();

            for (auto key : _src.IndexKeys()) 
            {
                jpa.push_back(winrt::to_string(key));
            }

            _dest.put_member("index_keys", jpa);
        }

        void put_json(ObjectFieldOptions& _dest, json _src)
        {
            

        }

        void get_json(json& _dest, ObjectFieldOptions& _src)
        {

        }

        void put_json(ArrayFieldOptions& _dest, json _src)
        {

        }

        void get_json(json& _dest, ArrayFieldOptions& _src)
        {

        }

        void put_json(QueryFieldOptions& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryFieldOptions& _src)
        {

        }

        void put_json(Int64FieldOptions& _dest, json _src)
        {

        }

        void get_json(json& _dest, Int64FieldOptions& _src)
        {

        }

        void put_json(DateTimeFieldOptions& _dest, json _src)
        {

        }

        void get_json(json& _dest, DateTimeFieldOptions& _src)
        {

        }

        void put_json(DoubleFieldOptions& _dest, json _src)
        {

        }

        void get_json(json& _dest, DoubleFieldOptions& _src)
        {

        }

        void put_json(StringFieldOptions& _dest, json _src)
        {

        }

        void get_json(json& _dest, StringFieldOptions& _src)
        {

        }

        void put_json(QueryFilter& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryFilter& _src)
        {

        }

        void put_json(QueryProject& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryProject& _src)
        {

        }

        void put_json(QueryJoin& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryJoin& _src)
        {

        }

        void put_json(QueryFrom& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryFrom& _src)
        {

        }

        void put_json(QueryCondition& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryCondition& _src)
        {

        }


        void put_json(QueryConditionAll& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryConditionAll& _src)
        {

        }

        void put_json(QueryConditionAny& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryConditionAny& _src)
        {

        }

        void put_json(QueryConditionContains& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryConditionContains& _src)
        {

        }

        void put_json(QueryConditionEq& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryConditionEq& _src)
        {

        }

        void put_json(QueryConditionLt& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryConditionLt& _src)
        {

        }

        void put_json(QueryConditionGt& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryConditionGt& _src)
        {

        }

        void put_json(QueryConditionGtEq& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryConditionGtEq& _src)
        {

        }

        void put_json(QueryConditionLtEq& _dest, json _src)
        {

        }

        void get_json(json& _dest, QueryConditionNone _src)
        {

        }

        void put_json(QueryConditionNone& _dest, json _src)
        {

        }

    };

    hstring LocalCoronaClient::ApplicationName()
    {
        return application_name;
    }

    void LocalCoronaClient::ApplicationName(hstring const& value)
    {
        application_name = value;
    }

    hstring LocalCoronaClient::ApplicationFolderName()
    {
        return application_name;
    }

    void LocalCoronaClient::ApplicationFolderName(hstring const& value)
    {
        application_name = value;
    }

    hstring LocalCoronaClient::ConfigFileNameBase()
    {
        return application_name;
    }

    void LocalCoronaClient::ConfigFileNameBase(hstring const& value)
    {
        application_name = value;
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::BaseResponse> LocalCoronaClient::Open()
    {
        co_await winrt::resume_background(); // Return control; resume on thread pool.

        winrt::coronawinrt::BaseResponse response;

        using namespace corona;

        system_monitoring_interface::start(); // this will create the global log queue.
        timer tx;
        date_time t = corona::date_time::now();
        json_parser jp;

        ready_for_polling = false;

        database_config_filename = winrt::to_string(config_file_name_base);
        database_config_mon.filename = database_config_filename;

        auto result = database_config_mon.poll_contents(app.get(), local_db_config);
        if (result == null_row) {
            system_monitoring_interface::global_mon->log_warning(std::format("config file {0} not found", database_config_filename), __FILE__, __LINE__);
            response.Message( L"config file not found" );
            response.Success(false);
            co_return response;
        }
        else {
            system_monitoring_interface::global_mon->log_information(std::format("using config file {0}", database_config_filename), __FILE__, __LINE__);
        }

        server_config = local_db_config["Server"];

        database_filename = server_config["database_filename"];
        database_schema_filename = server_config["schema_filename"];

        if (database_filename.empty() or database_schema_filename.empty())
        {
            response.Message(L"database file or schema file not specified");
            response.Success(false);
            co_return response;
        }

        database_schema_mon.filename = database_schema_filename;

        if (not app) {
            app = std::make_shared<application>();
            app->application_name = server_config["application_name"];
        }

        if (app->application_name.empty())
        {
            response.Message(L"application_name not specified");
            response.Success(false);
            co_return response;
        }

        system_monitoring_interface::global_mon->log_information("Startup user name " + app->get_user_display_name());

        if (not app->file_exists(database_filename))
        {
            db_file = app->open_file_ptr(database_filename, file_open_types::create_always);
            local_db = std::make_shared<corona_database>(db_file);
            local_db->apply_config(local_db_config);

            json create_database_response = local_db->create_database();

            bool success = (bool)create_database_response[success_field];
            if (!success) {
                system_monitoring_interface::global_mon->log_json(create_database_response);
                response.Message(L"Could not create database");
                response.Success(false);
                co_return response;
            }

            relative_ptr_type result = database_config_mon.poll(app.get());
            ready_for_polling = true;
        }
        else
        {
            db_file = app->open_file_ptr(database_filename, file_open_types::open_existing);
            local_db = std::make_shared<corona_database>(db_file);
            local_db->apply_config(local_db_config);

            local_db->open_database(0);

            relative_ptr_type result = database_config_mon.poll(app.get());
            ready_for_polling = true;
        }

        listen_point = server_config["listen_point"];

        if (not listen_point.empty()) {

            bind_web_server(db_api_server);
            db_api_server.start();

            system_monitoring_interface::global_mon->log_information("listening on :" + listen_point, __FILE__, __LINE__);
            for (auto path : api_paths) {
                system_monitoring_interface::global_mon->log_information(path, __FILE__, __LINE__);
            }
            system_monitoring_interface::global_mon->log_command_stop("comm_service_bus", "startup complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

            response.ExecutionTimeSeconds( tx.get_elapsed_seconds() );
            response.Message(L"server started");
            response.Success(true);
            co_return response;
        }
    }

	corona::json LocalCoronaClient::get_banner()
	{
        using namespace corona;

		json_parser jp;
		json body = jp.parse_object(R"(
{
	"Name":"Corona Server",
	"Version":"1.0"
}
)");
		return body;
	}

    corona::json LocalCoronaClient::parse_request(corona::http_request _request)
	{
        using namespace corona;

		json_parser jph;
		json request;

		if (_request.body.get_size() > 1)
		{
			request = jph.parse_object(_request.body.get_ptr());
		}
		else
		{
			request = jph.create_object();
		}

		return request;
	}

    corona::http_response LocalCoronaClient::check_parse_error(corona::json _request)
	{
        using namespace corona;

		http_response response;

		response.content_type = "application/json";
		response.http_status_code = 200;

		if (_request.is_member("ClassName", parse_error_class)) {
			response.http_status_code = 504;
			response.response_body = _request.to_buffer();
			response.content_length = response.response_body.get_size();
			return response;
		}

		return response;
	}

    corona::http_response LocalCoronaClient::create_response(int _http_status_code, corona::json _source)
	{
        using namespace corona;
        timer tx;

		http_response response;
		response.http_status_code = _http_status_code;
		response.content_type = "application/json";
		response.response_body = _source.to_buffer();
		response.content_length = response.response_body.get_size();
		response.server = "Corona 1.0";
		response.system_result = os_result(0);
		return response;
	}

    std::string LocalCoronaClient::get_token(http_action_request& _request)
    {
        std::string token;
        std::string bearer = "Bearer ";
        if (_request.authorization.starts_with(bearer.c_str())) {
            token = _request.authorization.substr(bearer.size());
        }
        else
        {
            token = _request.authorization;
        }
        return token;
    }

	void LocalCoronaClient::bind_web_server(corona::http_server& _server)
	{
        using namespace corona;

		std::string _root_path = listen_point;

		if (not _root_path.ends_with('/')) {
			_root_path += "/";
		}

		std::string path = _root_path + "test/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbGET, path, corona_test);

		path = _root_path + "login/createuser/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_users_create);

		path = _root_path + "login/loginuser/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_login);

		path = _root_path + "login/confirmuser/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_users_confirm);

		path = _root_path + "login/senduser/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_users_send_confirm);

		path = _root_path + "login/passworduser/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_user_password);

		path = _root_path + "classes/get/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_classes_get);

		path = _root_path + "classes/get/details/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_class_get);

		path = _root_path + "classes/put/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_classes_put);

		path = _root_path + "objects/get/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_objects_get);

		path = _root_path + "objects/query/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_objects_query);

		path = _root_path + "objects/create/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_objects_create);

		path = _root_path + "objects/put/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_objects_put);

		path = _root_path + "objects/delete/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_objects_delete);

		path = _root_path + "objects/edit/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_objects_edit);

		path = _root_path + "objects/run/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_objects_run);

		path = _root_path + "objects/copy/";
		api_paths.push_back(path);
		_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_objects_copy);

	}



    void LocalCoronaClient::poll_db()
    {
        using namespace corona;

        if (ready_for_polling) {
            timer tx;
            json_parser jp;
            json temp;
            date_time start_time = date_time::now();

            bool show_listen = false;

            if (database_schema_mon.poll_contents(app.get(), temp) != null_row) {
                system_monitoring_interface::global_mon->log_command_start("poll_db", "apply schema", start_time, __FILE__, __LINE__);
                auto tempo = local_db->apply_schema(temp);
                system_monitoring_interface::global_mon->log_command_stop("poll_db", "schema applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
                show_listen = true;
            }
            timer tx2;
            if (database_config_mon.poll_contents(app.get(), temp) != null_row) {
                system_monitoring_interface::global_mon->log_command_start("poll_db", "apply config", start_time, __FILE__, __LINE__);
                local_db->apply_config(temp);
                system_monitoring_interface::global_mon->log_command_stop("poll_db", "config applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
                show_listen = true;
            }

            if (show_listen) {
                system_monitoring_interface::global_mon->log_information("listening on :" + listen_point, __FILE__, __LINE__);
                for (auto path : api_paths) {
                    system_monitoring_interface::global_mon->log_information(path, __FILE__, __LINE__);
                }
            }

        }
    }


    corona::json LocalCoronaClient::get_local_token()
    {
        using namespace corona;
        json_parser jp;
        json login_request = jp.create_object();
        json server_config = local_db_config["Server"];
        std::string user_name = server_config[sys_user_name_field];
        std::string password = server_config[sys_user_password_field];
        login_request.put_member(sys_user_name_field, user_name);
        login_request.put_member(sys_user_password_field, password);
        corona::json result = local_db->login_user(login_request);
        return result[token_field];
    }


    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::BaseResponse> LocalCoronaClient::Close()
    {
        co_await winrt::resume_background(); // Return control; resume on thread pool.

        timer tx;
        date_time t = corona::date_time::now();

        winrt::coronawinrt::BaseResponse response;

        using namespace corona;

        db_api_server = {};

        local_db = nullptr;
        db_file = nullptr;

        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CreateUserResponse> LocalCoronaClient::CreateUser(winrt::coronawinrt::CreateUserRequest request)
    {
        CreateUserResponse response;
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;
        json new_user_request = jp.create_object();
        json token = get_local_token();
        json user_information = jp.create_object();

        user_information.put_member(user_name_field, winrt::to_string(request.Username()));
        user_information.put_member(user_street1_field, winrt::to_string(request.Address1()));
        user_information.put_member(user_street2_field, winrt::to_string(request.Address2()));
        user_information.put_member(user_city_field, winrt::to_string(request.City()));
        user_information.put_member(user_state_field, winrt::to_string(request.State()));
        user_information.put_member(user_zip_field, winrt::to_string(request.Zip()));
        user_information.put_member("password1", winrt::to_string(request.Password1()));
        user_information.put_member("password2", winrt::to_string(request.Password2()));
        user_information.put_member(user_email_field, winrt::to_string(request.Email()));

        new_user_request.put_member(token_field, token);
        new_user_request.put_member(data_field, user_information);

        json j = local_db->create_user(new_user_request);

        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::LoginUserResponse> LocalCoronaClient::LoginUser(winrt::coronawinrt::LoginUserRequest request)
    {
        LoginUserResponse response;
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;
        json new_request = jp.create_object();
        json information = jp.create_object();
        information.put_member(user_name_field, winrt::to_string(request.UserName()));
        information.put_member(user_password_field, winrt::to_string(request.Password()));
        new_request.put_member(data_field, information);

        json response = local_db->login_user(new_request);

        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::ConfirmUserResponse> LocalCoronaClient::ConfirmUser(winrt::coronawinrt::ConfirmUserRequest request)
    {
        ConfirmUserResponse response;
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;

        json new_request = jp.create_object();
        json information = jp.create_object();

        new_request.put_member(token_field, winrt::to_string(request.Token()));
        information.put_member(user_name_field, winrt::to_string(request.Email()));
        information.put_member("validation_code", winrt::to_string(request.ValidationCode()));
        new_request.put_member(data_field, information);
        local_db->user_confirm_user_code(new_request);

        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::SendUserResponse> LocalCoronaClient::SendUser(winrt::coronawinrt::SendUserRequest request)
    {
        SendUserResponse response;
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;
        json new_request = jp.create_object();
        json information = jp.create_object();

        new_request.put_member(token_field, winrt::to_string(request.Token()));
        information.put_member(data_field, winrt::to_string(request.Email()));
        new_request.put_member(data_field, information);

        local_db->send_user_confirm_code(new_request);

        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PasswordUserResponse> LocalCoronaClient::PasswordUser(winrt::coronawinrt::PasswordUserRequest request)
    {
        PasswordUserResponse response;
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;
        json new_request = jp.create_object();
        json information = jp.create_object();
        new_request.put_member(token_field, winrt::to_string(request.Token()));

        information.put_member(user_name_field, winrt::to_string(request.UserName()));
        information.put_member("validation_code", winrt::to_string(request.ValidationCode()));
        information.put_member("password1", winrt::to_string(request.Password1()));
        information.put_member("password2", winrt::to_string(request.Password2()));
        new_request.put_member(data_field, information);

        local_db->set_user_password(new_request);

        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetClassResponse> LocalCoronaClient::GetClass(winrt::coronawinrt::GetClassRequest request)
    {
        GetClassResponse response;
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;
        json new_request = jp.create_object();
        new_request.put_member(token_field, winrt::to_string(request.Token()));
        new_request.put_member(class_name_field, winrt::to_string(request.ClassName()));

        local_db->get_class(new_request);

        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetClassesResponse> LocalCoronaClient::GetClasses(winrt::coronawinrt::GetClassesRequest request)
    {
        GetClassesResponse response;
        timer tx;
        date_time t = corona::date_time::now();
        json_parser jp;
        json new_request = jp.create_object();
        new_request.put_member(token_field, winrt::to_string(request.Token()));
        new_request.put_member(class_name_field, winrt::to_string(request.ClassName()));
        local_db->get_classes(new_request);

        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PutClassesResponse> LocalCoronaClient::PutClasses(winrt::coronawinrt::PutClassesRequest request)
    {
        PutClassesResponse response;
        timer tx;
        date_time t = corona::date_time::now();
        json_parser jp;
        json new_request = jp.create_object();
        json information = jp.create_object();
                       
        auto src_def = request.ClassDef();

        src_def.BaseClassName();
        src_def.ClassDescription();
        src_def.ClassName();
        src_def.Fields();
        src_def.Indexes();

        new_request.put_member(token_field, token);
        new_request.put_member(data_field, information);

        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetObjectResponse> LocalCoronaClient::GetObjects(winrt::coronawinrt::GetObjectRequest request)
    {
        GetObjectResponse response;
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;
        json new_request = jp.create_object();
        json token = get_local_token();
        json information = jp.create_object();
        new_request.put_member(token_field, token);
        new_request.put_member("Data", information);


        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PutObjectsResponse> LocalCoronaClient::PutObjects(winrt::coronawinrt::PutObjectsRequest request)
    {
        PutObjectsResponse response;
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;
        json new_request = jp.create_object();
        json token = get_local_token();
        json information = jp.create_object();
        new_request.put_member(token_field, token);
        new_request.put_member("Data", information);


        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::QueryObjectsResponse> LocalCoronaClient::QueryObjects(winrt::coronawinrt::QueryObjectsRequest request)
    {
        QueryObjectsResponse response;
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;
        json new_request = jp.create_object();
        json token = get_local_token();
        json information = jp.create_object();
        new_request.put_member(token_field, token);
        new_request.put_member("Data", information);


        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::DeleteObjectsResponse> LocalCoronaClient::DeleteObjects(winrt::coronawinrt::DeleteObjectsRequest request)
    {
        DeleteObjectsResponse response;
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;
        json new_request = jp.create_object();
        json token = get_local_token();
        json information = jp.create_object();
        new_request.put_member(token_field, token);
        new_request.put_member("Data", information);


        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CreateObjectResponse> LocalCoronaClient::CreateObject(winrt::coronawinrt::CreateObjectRequest request)
    {
        CreateObjectResponse response;
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;
        json new_request = jp.create_object();
        json token = get_local_token();
        json information = jp.create_object();
        new_request.put_member(token_field, token);
        new_request.put_member("Data", information);



        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::EditObjectResponse> LocalCoronaClient::EditObject(winrt::coronawinrt::EditObjectRequest request)
    {
        EditObjectResponse response = winrt::make<RunObjectResponse>();
        timer tx;
        date_time t = corona::date_time::now();

        json_parser jp;
        json new_request = jp.create_object();
        json token = get_local_token();
        json information = jp.create_object();
        new_request.put_member(token_field, token);
        new_request.put_member("Data", information);

        json response = local_db->edit_object(request);

        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::RunObjectResponse> LocalCoronaClient::RunObject(winrt::coronawinrt::RunObjectRequest request)
    {
        RunObjectResponse response = winrt::make<RunObjectResponse>();
        timer tx;

        date_time t = corona::date_time::now();
        json_parser jp;
        json new_request = jp.create_object();
        json token = get_local_token();

        json information = jp.create_object();

        information.put_member(request.)

        new_request.put_member(token_field, token);
        new_request.put_member("Data", information);

        json response = local_db->run_object(new_request);

        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CopyObjectResponse> LocalCoronaClient::CopyObject(winrt::coronawinrt::CopyObjectRequest request)
    {
        CopyObjectResponse response = winrt::make<CopyObjectResponse>();

        timer tx;
        date_time t = corona::date_time::now();
        json_parser jp;
        json new_request = jp.create_object();
        json token = get_local_token();
        json information = jp.create_object();
        new_request.put_member(token_field, token);
        new_request.put_member("Data", information);



        response.ExecutionTimeSeconds(tx.get_elapsed_seconds());
        response.Message(L"closed");
        response.Success(true);

        co_return response;
    }
    hstring LocalCoronaClient::FileName()
    {        
        return file_name;
    }
    void LocalCoronaClient::FileName(hstring const& value)
    {
        file_name = value;
    }

}
