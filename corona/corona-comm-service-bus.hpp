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

		std::string database_schema_filename;
		std::string database_config_filename;

		json_file_watcher database_schema_mon;
		json_file_watcher database_config_mon;

		json local_db_config;

		std::string database_filename;

		bool ready_for_polling;

		json system_proof;
		json server_config;

		http_server db_api_server;

		std::string listen_point;

		comm_bus_service(std::string _config_filename)
		{
			system_monitoring_interface::start(); // this will create the global log queue.
			timer tx;
			date_time t = date_time::now();
			json_parser jp;

			log_command_start("comm_service_bus", "startup", t);

			log_information("Country Video Games Corona Database startup");

			ready_for_polling = false;

			database_config_filename = _config_filename;

			database_config_mon.filename = database_config_filename;
			auto result = database_config_mon.poll_contents(app.get(), local_db_config);
			if (result == null_row) {
				system_monitoring_interface::global_mon->log_warning(std::format("config file {0} not found", database_config_filename), __FILE__, __LINE__);
				throw std::invalid_argument("config file not found");
			}
			else {
				system_monitoring_interface::global_mon->log_information(std::format("using config file {0}", database_config_filename), __FILE__, __LINE__);
			}

			server_config = local_db_config["Server"];

			database_filename = server_config["database_filename"];
			database_schema_filename = server_config["schema_filename"];

			if (database_filename.empty() or database_schema_filename.empty())
			{
				throw std::logic_error("database file or schema file not specified");
			}

			database_schema_mon.filename = database_schema_filename;

			app = std::make_shared<application>();
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

			log_information("Self test.");
			prove_system();

			log_information("Startup user name " + app->get_user_display_name());

			if (true or not app->file_exists(database_filename))
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

			bind_web_server(db_api_server);
			db_api_server.start();

			log_information("listening on :" + listen_point, __FILE__, __LINE__);
			for (auto path : api_paths) {
				log_information(path, __FILE__, __LINE__);
			}
			log_command_stop("comm_service_bus", "startup complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
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

			dependencies = {"xbranch"};
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
			login_request.put_member(sys_user_password_field, sa_password);
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
			"name" : "datax"
		}
	],
	"stages": [
		{
			"class_name" : "filter",
			"stage_input_name" : "datax"			
		}
	]
}
)"_jobject;
			

			request.put_member("token", token);
			json from = request["from"].get_element(0);
			from.put_member(class_name_field, _class_name);
			result = local_db->get_classes(request);

			return result;
		}

		void poll_db()
		{
			if (ready_for_polling) {
				timer tx;
				json_parser jp;
				json temp;
				date_time start_time = date_time::now();

				bool show_listen = false;

				if (database_schema_mon.poll_contents(app.get(), temp) != null_row) {
					log_command_start("poll_db", "apply schema", start_time, __FILE__, __LINE__);
					auto tempo = local_db->apply_schema(temp);
					log_command_stop("poll_db", "schema applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					show_listen = true;
				}
				timer tx2;
				if (database_config_mon.poll_contents(app.get(), temp) != null_row) {
					log_command_start("poll_db", "apply config", start_time, __FILE__, __LINE__);
					local_db->apply_config(temp);
					log_command_stop("poll_db", "config applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					show_listen = true;
				}

				if (show_listen) {
					log_information("listening on :" + listen_point, __FILE__, __LINE__);
					for (auto path : api_paths) {
						log_information(path, __FILE__, __LINE__);
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
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->create_user(parsed_request);
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
			json fn_response = local_db->confirm_user(parsed_request);
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
			return response;
		}

		std::vector<std::string> api_paths;

		void bind_web_server(http_server& _server)
		{
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

			path = _root_path + "objects/copy/";
			api_paths.push_back(path);
			_server.put_handler(HTTP_VERB::HttpVerbPOST, path, corona_objects_copy);

		}

		virtual void log_error(json j, const char* _file = nullptr, int _line = 0)
		{
			std::string msg = j["message"];
			if (msg.empty())
				msg = "Error";
			log_warning(msg, _file, _line);
			log_json<json>(j, 4);
		}

		void run(runnable _runnable)
		{
			general_job* gj = new general_job(_runnable);
			global_job_queue->add_job(gj);
		}

		void run_http(runnable_http_request _runnable, runnable_http_response _ui_complete)
		{
			general_http_ui_job* guj = new general_http_ui_job(_runnable, _ui_complete);
			global_job_queue->add_job(guj);
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
				global_job_queue->add_job(gj);
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
				global_job_queue->add_job(gj);
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
				global_job_queue->add_job(gj);
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
