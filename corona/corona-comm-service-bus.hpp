/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
This is the service bus, which coordinates http and the database server.


Notes

For Future Consideration
*/

#ifndef CORONA_COMM_SERVICE_BUS
#define CORONA_COMM_SERVICE_BUS

namespace corona
{
	class comm_bus_service : public system_monitoring_interface
	{

	public:

		std::shared_ptr<corona_database>	local_db;
		std::shared_ptr<application>		app;
		std::shared_ptr<file>				db_file;

		std::shared_ptr<corona_simulation_interface> simulation;

		std::string database_schema_filename;
		std::string database_config_filename;
		int database_threads;

		json_file_watcher database_schema_mon;
		json_file_watcher database_config_mon;

		json local_db_config;

		std::string database_filename;
        bool database_recreate;

		bool ready_for_polling;

		json system_proof;
		json server_config;

		http_server db_api_server;

		std::string listen_point;
		std::function<void(const std::string& _msg, const char* _file, int _line)> on_logged_error;

		comm_bus_service(std::shared_ptr<corona_simulation_interface>& _simulation, 
			std::string _config_filename, 
			std::function<void(const std::string& _msg, const char* _file, int _line)> _on_logged_error,
			bool _is_service = false
		)
		{
            on_logged_error = _on_logged_error;
			system_monitoring_interface::start(); // this will create the global log queue.
			timer tx;
			date_time t = date_time::now();
			json_parser jp;

			simulation = _simulation;
			is_service = _is_service;

			log_command_start("comm_service_bus", "startup", t);

			log_information("Country Video Games Corona Database startup");

			ready_for_polling = false;

			database_config_filename = _config_filename;

			database_config_mon.filename = database_config_filename;

			database_config_mon.poll(app.get(), [this, &tx](json& _new_config) {
				local_db_config = _new_config;
				system_monitoring_interface::global_mon->log_information(std::format("using config file {0}", database_config_filename), __FILE__, __LINE__);

				server_config = local_db_config["Server"];

				database_filename = server_config["database_filename"];
				database_schema_filename = server_config["schema_filename"];
				database_threads = (int)server_config["database_threads"];
				database_recreate = (bool)server_config["database_recreate"];

				if (database_filename.empty() or database_schema_filename.empty())
				{
					throw std::logic_error("database file or schema file not specified");
				}

				database_schema_mon.filename = database_schema_filename;

				app = std::make_shared<application>(database_threads);
				app->application_name = server_config["application_name"];
				listen_point = server_config["listen_point"];

				if (app->application_name.empty())
				{
					throw std::logic_error("application_name not specified");
				}

				if (listen_point.empty())
				{
					throw std::logic_error("listen_point not specified");
				}

				if (not is_service)
				{
					log_information("Self test.");
					prove_system();
					log_information("Startup user name " + app->get_user_display_name());
				}

				if (database_recreate or not app->file_exists(database_filename))
				{
					db_file = app->open_file_ptr(database_filename, file_open_types::create_always);
					local_db = std::make_shared<corona_database>(db_file);
					local_db->apply_config(local_db_config);

					json create_database_response = local_db->create_database();

					bool success = (bool)create_database_response[success_field];
					if (!success) {
						log_json(create_database_response);
						throw std::exception("Coult not create database");
					}

					ready_for_polling = true;
				}
				else
				{
					db_file = app->open_file_ptr(database_filename, file_open_types::open_existing);
					local_db = std::make_shared<corona_database>(db_file);
					local_db->apply_config(local_db_config);

					local_db->open_database(0);

					ready_for_polling = true;
				}

				bind_web_server(db_api_server);
				db_api_server.start();

				log_information("listening on :" + listen_point, __FILE__, __LINE__);
				for (auto ipath : api_paths) {
					log_information(ipath.path, __FILE__, __LINE__);
				}
				log_command_stop("comm_service_bus", "startup complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			});
		}

		void prove_system()
		{
			timer tx;
			date_time t = date_time::now();

			json_parser jp;

			test_master tm;

			log_job_start("verification", "verification start", t, __FILE__, __LINE__);

			std::vector<std::string> dependencies;

			std::shared_ptr<test_set> testo = tm.create_test_set("locks", dependencies);
			test_locks(testo);

			testo = tm.create_test_set("rw locks", dependencies);
			test_rw_locks(testo);

			testo = tm.create_test_set("object", dependencies);
			test_object(testo, app);

			testo = tm.create_test_set("file block", dependencies);
			test_file_block(testo, app);

			testo = tm.create_test_set("child_field", dependencies);
			test_parse_child_field(testo);

			dependencies = { "file block", "object" };
			testo = tm.create_test_set("file", dependencies);
			test_file(testo, app);

			dependencies = { "file" };
			testo = tm.create_test_set("data block", dependencies);
			test_data_block(testo, app);

			dependencies = { "data block" };
			testo = tm.create_test_set("json node", dependencies);
			test_json_node(testo, app);

			dependencies = { "data block" };
			testo = tm.create_test_set("xrecord", dependencies);
			test_xrecord(testo, app);

			dependencies = { "xrecord" };
			testo = tm.create_test_set("xleaf", dependencies);
			test_xleaf(testo, app);

			dependencies = { "xleaf" };
			testo = tm.create_test_set("xbranch", dependencies);
			test_xbranch(testo, app);

			dependencies = { "xbranch" };
			testo = tm.create_test_set("xtable", dependencies);
			test_xtable(testo, app);

			dependencies = { "json node", "xtable" };
			testo = tm.create_test_set("master", dependencies);

			bool system_works = tm.prove("master");
			if (not system_works)
			{
				log_job_stop("verification", "verification failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else
			{
				log_job_stop("verification", "verification complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
		}

		std::string system_login()
		{
			json_parser jp;
			std::string sa_user = server_config[sys_user_name_field];
			std::string sa_password = server_config[sys_user_password_field];
			json login_request = jp.create_object();
			login_request.put_member(user_name_field, sa_user);
			login_request.put_member(user_password_field, sa_password);
			json login_result = local_db->login_user(login_request);
			bool success = (bool)login_result[success_field];
			if (not success) {
				log_warning(login_result[message_field], __FILE__, __LINE__);
			}
			return login_result[token_field];
		}

		json get_classes()
		{
			json_parser jp;
			json result;

			std::string token = system_login();

			if (token.empty()) {
				return result;
			}

			json request = jp.create_object();
			request.put_member(token_field, token);

			result = local_db->get_classes(request);

			return result;
		}

		json get_class(std::string _class_name)
		{
			json_parser jp;
			json result;

			std::string token = system_login();

			if (token.empty()) {
				return result;
			}

			json request = jp.create_object();
			request.put_member(token_field, token);
			request.put_member(class_name_field, _class_name);
			result = local_db->get_class(request);

			return result;
		}

		json get_object(std::string _class_name, int64_t _object_id, bool _include_children = false)
		{
			json_parser jp;
			json result;

			std::string token = system_login();

			if (token.empty()) {
				return result;
			}

			json request = jp.create_object();
			request.put_member(token_field, token);
			request.put_member(class_name_field, _class_name);
			request.put_member(include_children_field, _include_children);
			request.put_member_i64(object_id_field, _object_id);
			result = local_db->get_object(request);
			return result;
		}

		json delete_object(std::string _class_name, int64_t _object_id)
		{
			json_parser jp;
			json result;

			std::string token = system_login();

			if (token.empty()) {
				return result;
			}

			json request = jp.create_object();
			request.put_member(token_field, token);
			request.put_member(class_name_field, _class_name);
			request.put_member_i64(object_id_field, _object_id);
			result = local_db->delete_object(request);
			return result;
		}

		json create_object(std::string _class_name)
		{
			json_parser jp;
			json result;

			std::string token = system_login();

			if (token.empty()) {
				return result;
			}

			json request = jp.create_object();
			request.put_member(token_field, token);
			json data = jp.create_object();
			data.put_member(class_name_field, _class_name);
			result = local_db->create_object(request);
			return result;
		}

		json put_object(json _object)
		{
			json_parser jp;
			json result;

			std::string token = system_login();

			if (token.empty()) {
				return result;
			}

			json request = jp.create_object();
			request.put_member(token_field, token);
			request.put_member(data_field, _object);
			result = local_db->put_object(request);

			return result;
		}


		json get_data(std::string _class_name)
		{
			json_parser jp;
			json result;

			std::string token = system_login();

			if (token.empty()) {
				return result;
			}

			json request = R"(
{
	"token":"",
	"from": [
		{ 
			"class_name" : "",
			"name" : "data_source"
		}
	],
	"stages": [
		{
			"name" : "filter_it",
			"class_name" : "filter",
			"input" : "data_source"			
		}
	]
}
)"_jobject;


			request.put_member("token", token);
			json from = request["from"].get_element(0);
			from.put_member(class_name_field, _class_name);
			result = local_db->query(request);

			return result;
		}

		std::function<bool(comm_bus_service* _service, json& _command)> command_handler = [this](comm_bus_service* _service, json& _command)->bool {
			// this is the default command handler, which does nothing.
			// it can be overridden by the application.
			// 
			// _command is a json object with the command and parameters.
			// 
			// if you want to handle commands, you can do so here.
			// 
			// if you want to handle commands in the application, you can do so by overriding this function.
			// 
			// if you want to handle commands in the service, you can do so by overriding this function.
			// 
			// if you want to handle commands in the database, you can do so by overriding this function.
			// return true if the command can be deleted
			return true;
			};

		void frame_db()
		{
			if (ready_for_polling)
			{
				// get all commands, deleting them as we go.
				// 

				json jcommands = get_data("sys_command");
				if (jcommands.array())
				{
					for (auto jcommand : jcommands)
					{
						command_handler(this, jcommand);
					}
				}
			}
		}

		void poll_db()
		{
			if (ready_for_polling) {
				timer tx;
				json_parser jp;
				json temp;
				date_time start_time = date_time::now();

				bool show_listen = false;

				database_schema_mon.poll(app.get(), [this, start_time, &tx, &show_listen](json& _new_schema) {
					log_command_start("poll_db", "apply schema", start_time, __FILE__, __LINE__);
					auto tempo = local_db->apply_schema(_new_schema);
					log_command_stop("poll_db", "schema applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					show_listen = true;
					});

				timer tx2;
				database_config_mon.poll(app.get(), [this, start_time, &tx, &show_listen](json& _new_schema) {
					log_command_start("poll_db", "apply config", start_time, __FILE__, __LINE__);
					local_db->apply_config(_new_schema);
					log_command_stop("poll_db", "config applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					show_listen = true;
					});

				if (show_listen) {
					log_information("updated server, and listening on :" + listen_point, __FILE__, __LINE__);
					for (auto ipath : api_paths) {
						log_information(ipath.path, __FILE__, __LINE__);
					}
				}

				if (false && simulation) {
					json result = get_data("sys_command");
					if (result["success"]) {
						json data = result["data"];
						simulation->on_frame(data);
					}
				}
			}
		}

		virtual ~comm_bus_service()
		{

		}

		std::string get_token(http_action_request& _request)
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

		json get_banner()
		{
			json_parser jp;
			json body = jp.parse_object(R"(
{
	"Name":"Corona Server",
	"Version":"1.0"
}
)");
			return body;
		}

		json get_openapi()
		{
			json_parser jp;

			json jopenapi = jp.create_object();
			jopenapi.put_member("openapi", std::string("3.0.4"));

			json jinfo = jopenapi.build_member("info");
			jinfo.put_member("title", local_db->default_api_title);
			jinfo.put_member("version", local_db->default_api_version);
			jinfo.put_member("description", local_db->default_api_description);

			json jservers = jp.create_array();

			std::string server_url;
			std::string server_description;

			server_url = listen_point;
			server_description = local_db->default_api_description;

			// now we're building the servers tag.

			json jserver = jp.create_object();
			jserver.put_member("url", server_url);
			jserver.put_member("description", server_description);
			jservers.push_back(jserver);

			auto slistresp = get_data("sys_server");
			auto server_list = slistresp["data"];
			if (server_list.array())
			{
				for (auto server : server_list) {
					server_url = server["server_url"];
					server_description = server["server_description"];
					jserver = jp.create_object();
					jserver.put_member("url", server_url);
					jserver.put_member("description", server_description);
					jservers.push_back(jserver);
				}
			}
			jopenapi.put_member("servers", jservers);

			// now we're building the paths tag.

			json jpaths = jopenapi.build_member("paths");

			// Example: /describe endpoint
			for (auto path : api_paths) {
				json jendpoint = jp.create_object();
				json post_op = jp.create_object();
			
				post_op.put_member("summary", std::string("Returns the OpenAPI specification document."));
				post_op.put_member("operationId", std::string("describe"));
				jendpoint.put_member("post", post_op);

				json jpath = jpaths.build_member("/"+path.path);
				json jverb = jpath.build_member(path.verb);

				jverb.put_member("summary", path.description);

				if (path.request_class_name.size() > 0) {
					jverb.build_member("requestBody.content.application/json.schema.$ref", "#/components/schemas/" + path.request_class_name);
                }

				if (path.response_class_name.size() > 0) {
					jverb.build_member("responses.200.content.application/json.schema.$ref", "#/components/schemas/" + path.response_class_name);
				}

				if (path.response_class_name.size() > 0) {
					jverb.build_member("responses.default.content.application/json.schema.$ref", "#/components/schemas/" + path.response_class_name);
				}

			} 

			// and now we're building the schema itself.

			json jschema = local_db->get_openapi_schema("");
			json jschemas = jopenapi.build_member("components.schemas", jschema);

			for (auto path : api_paths) {
				std::string request_class_name = path.request_class_name;
				json jrschema = jp.parse_object(path.request_schema);
				if (jrschema.object() and not jrschema.error()) {
					jschemas.put_member(request_class_name, jrschema);
				}
				else if (jrschema.error() and path.request_schema.size() > 0) {
                    log_error(jrschema, __FILE__, __LINE__);
				}

				std::string response_class_name = path.response_class_name;
				jrschema = jp.parse_object(path.response_schema);
				if (jrschema.object() and not jrschema.error()) {
					jschemas.put_member(response_class_name, jrschema);
				}
				else if (jrschema.error() and path.response_schema.size() > 0) {
					log_error(jrschema, __FILE__, __LINE__);
				}
			}


//			this has way more problems being baked into here than this is possibly worth.
//			just a lot of stuff to control their process and we will need to develop
// 			some proxy capabilities.
// 
//			run_swagger();

			return jopenapi;
		}

		http_handler_function corona_test = [this](http_action_request _request)-> void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = get_banner();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_options = [this](http_action_request _request)-> void {
			json_parser jp;
            json fn_response = jp.create_object();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_login = [this](http_action_request _request)-> void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto fn_response = local_db->login_user(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};


		http_handler_function corona_classes_get = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			auto fn_response = local_db->get_classes(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_class_get = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			auto fn_response = local_db->get_class(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_classes_put = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->put_class(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_users_create = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			// there's not a token in create_user to allow anyone to onboard.
			json fn_response = local_db->create_user(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_users_send_confirm = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->send_user_confirm_code(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_users_confirm = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->user_confirm_user_code(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};


		http_handler_function corona_user_password = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->set_user_password(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_get = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->get_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_copy = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->copy_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_describe = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = get_openapi();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};


		http_handler_function corona_objects_query = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);

			json fn_response = local_db->query(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_create = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->create_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_put = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->put_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_delete = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->delete_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_edit = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member("Token", token);
			json fn_response = local_db->edit_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_run = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member("Token", token);
			json fn_response = local_db->run_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		json parse_request(http_request _request)
		{
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

		http_response check_parse_error(json _request)
		{ 
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

		http_response create_response(int _http_status_code, json _source)
		{
			http_response response;

			response.http_status_code = _http_status_code;
			response.content_type = "application/json";
			response.response_body = _source.to_buffer();
			response.content_length = response.response_body.get_size();
			response.server = "Corona 1.0";
			response.system_result = os_result(0);
			response.headers["Allow"]="POST";
			response.headers["Access-Control-Allow-Origin"] = "*";
			return response;
		}

		class api_definition
		{
		public:
			std::string path;
			std::string name;
			std::string description;
			std::string verb;
			std::string request_class_name;
			std::string response_class_name;
			std::string request_schema;
			std::string response_schema;

			api_definition() = default;
			api_definition(const api_definition& _src) = default;
			api_definition(api_definition&& _src) = default;
			api_definition &operator =(const api_definition& _src) = default;
			api_definition &operator =(api_definition&& _src) = default;
			~api_definition() = default;
		};

		std::vector<api_definition> api_paths;

		void bind_web_server(http_server& _server)
		{
			std::string root_path = listen_point;
			std::string base_path;

			try {

				base_path = root_path;

				if (root_path.starts_with("http://")) {
					base_path = base_path.substr(7);
				}
				else if (root_path.starts_with("https://")) {
					base_path = base_path.substr(8);
				}

				int idx = base_path.find('/');

				if (idx != std::string::npos) {
					base_path = base_path.substr(idx+1);
				}

				if (not base_path.ends_with('/')) {
					base_path += "/";
				}

				if (not root_path.ends_with('/')) {
					root_path += "/";
				}

				api_paths.clear();

                api_definition new_api;


/**************
Bind home
***************/
				new_api.path = "";
				new_api.name = "home";
				new_api.description = "Returns test handler for this server.";
				new_api.verb = "get";
				new_api.request_schema = {};
				new_api.response_schema = {};
				new_api.request_class_name = {};
				new_api.response_class_name = {};
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbGET, root_path, base_path, new_api.path, corona_test);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);

/**************
Bind test
***************/
				new_api.name = "test";
				new_api.description = "Returns test handler for this server.";
				new_api.path = "test/";
				new_api.verb = "get";
				new_api.request_schema = {};
				new_api.response_schema = {};
				new_api.request_class_name = {};
				new_api.response_class_name = {};
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbGET, root_path, base_path, new_api.path, corona_test);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);


/**************
Bind createuser
***************/
				new_api.name = "create_user";
				new_api.description = "Creates a user, sending, if possible, a sign on email for the new user.  The proposed user name will be adjusted to a new user name.  Can be used by users signing themselves up.";
				new_api.path = "login/createuser/";
				new_api.verb = "post";
				new_api.request_class_name = "sys_create_user_request";
				new_api.response_class_name = "sys_create_user_response";

				new_api.request_schema = R"({
  "type": "object",
  "properties": {
	"user_name": {
	  "type": "string",
	  "description": "The user name of the new user."
	},
	"email": {
	  "type": "string",
	  "description": "The user name of the new user.",
	  "format": "email"
	},
	"password1": {
	  "type": "string",
	  "description": "Password 1.",
	  "format": "password"
	},
	"password2": {
	  "type": "string",
	  "description": "Password 2.",
	  "format": "password"
	}
  }
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the user was created successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object",
	  "description": "Result object or objects."
	},
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_users_create);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);

/**************
Bind loginuser
***************/

				new_api.path = "login/loginuser/";
				new_api.verb = "post";
				new_api.name = "login_user";
				new_api.description = "Attempt to access the system.";
				new_api.request_schema = R"({
  "type": "object",
  "properties": {
	"user_name": {
	  "type": "string",
	  "description": "The user name of the new user."
	},
	"password": {
	  "type": "string",
	  "description": "Password",
	  "format": "password"
	}
  }
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the user was created successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object",
	  "description": "Result object or objects."
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				new_api.request_class_name = "sys_login_user_request";
				new_api.response_class_name = "sys_login_user_response";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_login);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);

/**************
Bind confirmuser
***************/
				new_api.path = "login/confirmuser/";
				new_api.verb = "post";
				new_api.name = "confirm_user";
				new_api.description = "Validate a code sent previously to a user's email.";
				new_api.request_class_name = "sys_confirm_user_request";
				new_api.response_class_name = "sys_confirm_user_response";
				new_api.request_schema = R"({
  "type": "object",
  "properties": {
	"user_name": {
	  "type": "string",
	  "description": "Your user name."
	},
	"validation_code": {
	  "type": "string",
	  "description": "The code you received via email."
	}
  }
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the user was created successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object",
	  "description": "Result object or objects."
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_users_confirm);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);

/**************
Bind SENDUSER
***************/

				new_api.path = "login/senduser/";
				new_api.verb = "post";
				new_api.name = "send_user";
				new_api.description = "Send a secret code to a user's email so they can login again or confirm a password change.";
				new_api.request_class_name = "sys_sendcode_user_request";
				new_api.response_class_name = "sys_sendcode_user_response";
				new_api.request_schema = R"({
  "type": "object",
  "properties": {
	"user_name": {
	  "type": "string",
	  "description": "Your user name."
	}
  }
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if sent successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object",
	  "description": "Result object or objects."
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";


				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_users_send_confirm);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);



/**************
Bind passworduser
***************/

				new_api.path = "login/passworduser/";
				new_api.verb = "post";
				new_api.name = "change_password";
				new_api.description = "Changes a user's password.";
				new_api.request_class_name = "sys_sendcode_user_request";
				new_api.response_class_name = "sys_sendcode_user_response";
				new_api.request_schema = R"({
  "type": "object",
  "required": [ "user_name", "password1", "password2" ],
  "properties": {
	"user_name": {
	  "type": "string",
	  "description": "Your user name."
	},
	"password1": {
	  "type": "string",
	  "description": "Code you received in email.",
	  "format": "password"
	},
	"password2": {
	  "type": "string",
	  "description": "Code you received in email.",
	  "format": "password"
	},
	"validation_code": {
	  "type": "string",
	  "description": "Code you received in email."
	}
  }
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if changed successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object",
	  "description": "Result object or objects."
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_user_password);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);

/**************
Bind get classes
***************/

				new_api.path = "classes/get/";
				new_api.verb = "post";
				new_api.name = "get_classes";
				new_api.description = "Retrieves all the classes on the server that the user can access.";
				new_api.request_class_name = "sys_get_classes_request";
				new_api.response_class_name = "sys_get_classes_response";
				new_api.request_schema = R"({
  "type": "object"
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True, so data should have results in it."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "array",
	  "description": "Result object or objects."
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_classes_get);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);


				/**************
				Bind get class details
				***************/

				new_api.path = "classes/get/details/";
				new_api.verb = "post";
				new_api.name = "get_class_details";
				new_api.description = "Retrieves details of classes on the server that the user can access.  This class provides a physical map over and above what just get_classes does, but the map really, isn't something you'll need that much.";
				new_api.request_class_name = "sys_get_class_details";
				new_api.response_class_name = "sys_get_class_response";
				new_api.request_schema = R"({
  "type": "object"

})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "true if successful."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object",
	  "description": "Result object or objects."
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_class_get);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);


				/**************
				Bind put_class 
				***************/

				new_api.path = "classes/put/";
				new_api.verb = "post";
				new_api.name = "put_class_details";
				new_api.description = "Updates a class definition and its data.  If a base class has new fields, they are added to the descedendants as well.";
				new_api.request_class_name = "sys_put_class_request";
				new_api.response_class_name = "sys_put_class_response";
				new_api.request_schema = R"({
  "type": "object",
  "properties": {
		"data": {
		  "type": "object",
		  "description": "The class definition to update."
		}
  }
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the class was updated successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object",
	  "description": "Result class."
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_classes_put);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);


				/**************
				Bind get_object
				***************/

				new_api.path = "objects/get/";
				new_api.verb = "post";
				new_api.name = "get_object";
				new_api.description = "Fetches an object by class_name and object_id.";
				new_api.request_class_name = "sys_get_object_request";
				new_api.response_class_name = "sys_get_object_response";
				new_api.request_schema = R"({
  "type": "object",
  "properties": {
		"class_name": {
		  "type": "string",
		  "description": "The class of the object to get."
		},
		"object_id": {
		  "type": "integer",
		  "description": "The id of the object to get."
		}
  }
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the user was created successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object",
	  "description": "Result object or objects."
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_objects_get);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);




				/**************
				Bind query objects
				***************/

				new_api.path = "objects/query/";
				new_api.verb = "post";
				new_api.name = "query_objects";
				new_api.description = "Returns a stream of objects based on a query composed of where, joins, and project.";
				new_api.request_class_name = "sys_query_request";
				new_api.response_class_name = "sys_query_response";
				new_api.request_schema = R"({
  "type": "object",
  "properties": {
		"from": {
		  "type": "array",
		  "description": "classes that contain data."
		},
		"stages": {
		  "type": "array",
		  "description": "stages such as filter, join, project."
		}
  }
})";

				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the query was successful."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "array",
	  "description": "Result objects.",
	  "items": {
			"type":"object"
	  }
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_objects_query);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);
				api_paths.push_back(new_api);

				/**************
				Bind create object
				***************/


				new_api.path = "objects/create/";
				new_api.verb = "post";
				new_api.name = "create_objects";
				new_api.description = "Constructs a new object of a given class.";
				new_api.request_class_name = "sys_create_request";
				new_api.response_class_name = "sys_create_response";
				new_api.request_schema = R"({
  "type": "object",
  "properties": {
		"data": {
		  "type": "object",
		  "description": "contains .",
		  "properties" : {
			"class_name" : {
				"type" : "string"
			}
		  }
		}
	}
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the object was created successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
		  "type": "object",
		  "description": "object that was created."		  
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_objects_create);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);


				/**************
				Bind put object
				***************/

				new_api.path = "objects/put/";
				new_api.verb = "post";
				new_api.name = "put_objects";
				new_api.description = "Updates one or more objects, validating each.";
				new_api.request_class_name = "sys_put_object_request";
				new_api.response_class_name = "sys_put_object_response";
				new_api.request_schema = R"({
  "type": "object",
  "data": {
		"type": "array",
		"description": "classes that contain data."
	}
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the object was saved successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object"
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_objects_put);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);


				/**************
				Bind delete object
				***************/

				new_api.path = "objects/delete/";
				new_api.verb = "post";
				new_api.name = "delete_object";
				new_api.description = "Delete an object by class name and id.";
				new_api.request_class_name = "sys_delete_object_request";
				new_api.response_class_name = "sys_delete_object_response";
				new_api.request_schema = R"({
  "type": "object",
	"properties": {
	  "class_name": {
			"type": "string",
			"description": "class of object to delete."
		},
	  "object_id": {
			"type": "number",
			"description": "id of object to delete."
		}
	}
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the object was deleted successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object"
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_objects_delete);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);


				/**************
				Bind edit 
				***************/

				new_api.path = "objects/edit/";
				new_api.verb = "post";
				new_api.name = "get_object";
				new_api.description = "Fetches an object by class_name and object_id, also returning its schema and edit options.";
				new_api.request_class_name = "sys_edit_object_request";
				new_api.response_class_name = "sys_edit_object_response";
				new_api.request_schema = R"({
  "type": "object",
	"properties": {
	  "class_name": {
			"type": "string",
			"description": "class of object to edit."
		},
	  "object_id": {
			"type": "number",
			"description": "id of object to edit."
		},
	  "include_children": {
			"type": "boolean",
			"description": "If true, include children of the object."
		}
	}
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the object was deleted successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object"
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				new_api.request_class_name = "sys_edit_object_request";
				new_api.response_class_name = "sys_edit_object_response";
				new_api.verb = "post";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_objects_edit);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);


				/**************
				Bind run
				***************/

				new_api.path = "objects/run/";
				new_api.verb = "post";
				new_api.name = "run_object";
				new_api.description = "Puts an object, validating and updating it.  Then, runs any query methods on the object, returning the resultant object.";
				new_api.request_class_name = R"(sys_run_object_request)";
				new_api.response_class_name = R"(sys_run_object_response)";
				new_api.request_schema = R"({
  "type": "object",
  "data": {
		"type": "object",
		"description": "object to run."
	},
   "include_children": "boolean"
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the object was deleted successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object"
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_objects_run);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);

				new_api.path = "objects/copy/";
				new_api.name = "copy_object";
				new_api.verb = "post";
				new_api.description = "Creates a copy of an object.  This actually can cast an object to something else as well.";
				new_api.request_class_name = R"(sys_copy_object_request)";
				new_api.response_class_name = R"(sys_copy_object_response)";
				new_api.request_schema = R"({
  "type": "object",
	"properties": {
	  "from": {
			"type": "object",
			"properties": {
				"class_name": {
					"type": "string",
					"description": "class of object to copy."
				},
				"object_id": {
					"type": "number",
					"description": "id of object to copy."
				},
				"path": {
					"type": "string",
					"description": "member of object to copy."
				}
			}
		},
		"to": {
			"type": "object",
			"properties": {
				"class_name": {
					"type": "string",
					"description": "class of destination."
				},
				"object_id": {
					"type": "number",
					"description": "id of destination."
				},
				"path": {
					"type": "string",
					"description": "member of object to copy."
				}
			}
		},
		"transform": {
			"type": "object",
			"properties": {
				"class_name": {
					"type": "string",
					"description": "Class to transform to.  Basically, do a memberwise copy of the source class into this one, matching fields updated."
				}
			}
		}
	}
})";
				new_api.response_schema = R"({
  "type": "object",
  "properties": {
	"success": {
	  "type": "boolean",
	  "description": "True if the object was deleted successfully."
	},
	"message": {
	  "type": "string",
	  "description": "Text of message."
	},
	"data": {
	  "type": "object"
	},
	"token": {
	  "type": "string",
	  "description": "Token for the user to use in subsequent requests."
	}
  }
})";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbPOST, root_path, base_path, new_api.path, corona_objects_copy);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);

				new_api.path = "describe/";
				new_api.verb = "get";
				new_api.name = "describe";
				new_api.description = "Returns this open api specification.";
				new_api.request_schema = {};
				new_api.response_schema = {};
				new_api.request_class_name = R"()";
				new_api.response_class_name = R"()";
				api_paths.push_back(new_api);
				_server.put_handler(HTTP_VERB::HttpVerbGET, root_path, base_path, new_api.path, corona_describe);
				_server.put_handler(HTTP_VERB::HttpVerbOPTIONS, root_path, base_path, new_api.path, corona_options);
			}
			catch (std::exception exc)
			{
				log_exception(exc, __FILE__, __LINE__);
			}

		}

		virtual void log_warning(std::string _message, const char* _file = nullptr, int _line = 0) override
		{
			std::string msg = _message;
			if (msg.empty())
				msg = "Warning";
            system_monitoring_interface::log_warning(msg, _file, _line);
			if (on_logged_error) {
				on_logged_error(msg, _file, _line);
			}
        }

		virtual void log_error(json j, const char* _file = nullptr, int _line = 0)
		{
			std::string msg = j["message"];
			if (msg.empty())
				msg = "Error";
			log_warning(msg, _file, _line);
			log_json<json>(j, 4);
			if (on_logged_error) {
				std::string temp = j.to_json();
				msg = msg + "\nThe JSON is:\n" + temp;
				on_logged_error(msg, _file, _line);
			}
		}

		double frame_seconds = 1.0 / 20.0; // 60 frames per second	
		double poll_seconds = 12.0;

		std::chrono::steady_clock::time_point last_frame_time = std::chrono::high_resolution_clock::now();
		std::chrono::steady_clock::time_point last_poll_time = std::chrono::high_resolution_clock::now();

		int64_t total_frames = 0;
		int64_t missed_frames = 0;

		void run_frame()
		{
			::Sleep(1);

			auto current_frame_time = std::chrono::high_resolution_clock::now();
			const std::chrono::duration<double> diff_frame = current_frame_time - last_frame_time;
            double elapsed_frames = diff_frame.count() / frame_seconds;
			int64_t efc = elapsed_frames;

			if (efc > 0) {
				total_frames += efc;
                missed_frames += efc - 1;
				last_frame_time = current_frame_time;
			}

			const std::chrono::duration<double> diff_poll = current_frame_time - last_poll_time;

			double elapsed_polls = diff_poll.count() / poll_seconds;
			int64_t efp = elapsed_polls;
			if (efp > 0) {
                poll_db();
                last_poll_time = current_frame_time;
            }
		}

		HANDLE swagger_process = nullptr;
		lockable swagger_lock;

		void run_swagger()
		{
			scope_lock lock_me(swagger_lock);

			std::string system_command;
			std::string describe_url = listen_point + "/describe";

			if (swagger_process != nullptr) 
			{
				// check to see if we are busy.  if so, then, cool, but if not, then, we need to close the handle and create another 
				// process.
				int msg = ::WaitForSingleObject(swagger_process, 50);

				if (msg == WAIT_OBJECT_0) {
					CloseHandle(swagger_process);
					swagger_process = nullptr;
				}
			}

			if (swagger_process == nullptr)
			{
                swagger_process = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				system_command = R"(docker run -p 6677:8080 -e SWAGGER_JSON_URL=)";
				system_command += describe_url;
				system_command += R"( -e BASE_URL=/api -e SWAGGER_ALLOW_ORIGIN=* swaggerapi/swagger-ui)";
				system_job* sj = new system_job(system_command, swagger_process);
				global_job_queue->submit_job(sj);
			}
		}

		void run(runnable _runnable)
		{
			general_job* gj = new general_job(_runnable);
			global_job_queue->submit_job(gj);
		}

		void run_http(runnable_http_request _runnable, runnable_http_response _ui_complete)
		{
			general_http_ui_job* guj = new general_http_ui_job(_runnable, _ui_complete);
			global_job_queue->submit_job(guj);
		}

		template <typename dest, typename item> void run_each(dest* _targets, std::vector<item>& _items, std::function<void(dest* _target, item& _src)> _on_each)
		{
			std::vector<HANDLE> events;

			int counter = 0;
			int bucket_size = _items.size() / (global_job_queue->getThreadCount() + 1);

			if (bucket_size < _items.size())
				bucket_size = _items.size();

			for (int idx = 0; idx < _items.size(); ) {
				HANDLE handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				events.push_back(handle);
				int end = idx + bucket_size;
				if (end > _items.size())
					end = _items.size();
				general_job* gj = new general_job([_targets, idx, end, _on_each, &_items]() -> void {
					for (int x = idx; x < end; x++)
					{
						item& itm = _items[x];
						dest* d = &_targets[x];
						_on_each(d, itm);
					}
					}, handle);
				global_job_queue->submit_job(gj);
				idx = end;
			}

			for (auto evt : events) {
				::WaitForSingleObject(evt, INFINITE);
				CloseHandle(evt);
			}
		}

		template <typename item> void run_each(std::vector<item>& _items, std::function<void(item& _item)> _on_each)
		{
			std::vector<HANDLE> events;

			int counter = 0;
			int bucket_size = _items.size() / (global_job_queue->getThreadCount() + 1);

			if (bucket_size < _items.size())
				bucket_size = _items.size();

			for (int idx = 0; idx < _items.size(); ) {
				HANDLE handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				events.push_back(handle);
				int end = idx + bucket_size;
				if (end > _items.size())
					end = _items.size();
				std::vector<item>* src_items = &_items;
				general_job* gj = new general_job([idx, end, _on_each, src_items]() -> void {
					for (int x = idx; x < end; x++)
					{
						item& itm = (*src_items)[x];
						_on_each(itm);
					}
					}, handle);
				global_job_queue->submit_job(gj);
				idx = end;
			}

			for (auto evt : events) {
				::WaitForSingleObject(evt, INFINITE);
				CloseHandle(evt);
			}
		}

		template <typename item> void run_each(std::vector<item>& _items, int _width, int _height, std::function<void(int _x, int _y, item& _item)> _on_each)
		{
			std::vector<HANDLE> events;

			int counter = 0;
			int bucket_size = _items.size() / (global_job_queue->getThreadCount() + 1);

			if (bucket_size < _items.size())
				bucket_size = _items.size();

			for (int idx = 0; idx < _items.size(); ) {
				HANDLE handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				events.push_back(handle);
				int end = idx + bucket_size;
				if (end > _items.size())
					end = _items.size();
				std::vector<item>* src_items = &_items;
				general_job* gj = new general_job([idx, _width, _height, end, _on_each, src_items]() -> void {
					for (int x = idx; x < end; x++)
					{
						int px = x % _width;
						int py = x / _width;
						item& itm = (*src_items)[x];	
						_on_each(px, py, itm);
					}
					}, handle);
				global_job_queue->submit_job(gj);
				idx = end;
			}

			for (auto evt : events) {
				::WaitForSingleObject(evt, INFINITE);
				CloseHandle(evt);
			}
		}



	};
}

#endif
