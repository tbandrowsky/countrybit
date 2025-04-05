#pragma once

/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
This is the application bus. This is the root point that ties it together
for corona desktop applications

Notes

For Future Consideration
*/


#ifndef CORONA_COMM_APP_BUS_H
#define CORONA_COMM_APP_BUS_H

namespace corona
{

	class comm_app_bus : public comm_bus_app_interface
	{
	protected:

		std::multimap<UINT, windows_event_waiter> windows_waiters;
		std::multimap<std::string, topic_event_waiter> topic_waiters;

		void check_windows_queue(MSG* _msg)
		{
			auto waiters = windows_waiters.find(_msg->message);
			while (waiters != std::end(windows_waiters))
			{
				auto temp_waiter = waiters;
				auto& waiter = *waiters;
				SetEvent(waiter.second.hevent);
				waiters++;
			}
		}

		void check_topic(std::string _topic)
		{
			auto waiters = topic_waiters.find(_topic);
			while (waiters != std::end(topic_waiters))
			{
				auto& waiter = *waiters;
				SetEvent(waiter.second.hevent);
				waiters++;
			}
			topic_waiters.erase(_topic);
		}

		relative_ptr_type read_json(std::string _file_name, json& item)
		{
			relative_ptr_type pt = null_row;
			if (app->file_exists(_file_name)) {
				try {
					std::string file_contents = read_all_string(_file_name);
					json_parser jp;
					item = jp.parse_object(file_contents);
					pt = 1;
				}
				catch (std::exception exc)
				{
					std::cerr << "Error:" << __FILE__ << " " << __LINE__ << " polling " << _file_name << " failed:" << exc.what() << std::endl;
				}
				return pt;
			}
		}

		json								local_db_config;

	public:

		std::shared_ptr<corona_database>	local_db;
		std::shared_ptr<application>		app;
		std::shared_ptr<file>				db_file;
		json								abbreviations;

		corona_client						client;

		// the adapter, created first.  It is the graphics card on the machine
		std::shared_ptr<directXAdapter> factory;

		// This creates the application window (and children), handling the mapping from the window and the events
		// back through the presentation.
		std::shared_ptr<directApplicationWin32> app_ui;

		// create a win32 set of windows that can use this factory
		// to make windows with a d2d / d3d image
		std::shared_ptr<menu_item> app_menu;

		// create the presentation - this holds the data of what is on screen, for various pages.
		std::shared_ptr<presentation> presentation_layer;

		int application_icon_id;

		std::string database_schema_filename;
		std::string database_config_filename;
		std::string styles_config_filename;
		std::string pages_config_filename;

		directory_checker checker;

		std::string database_filename;
		std::string user_file_name;

		bool ready_for_polling;

		json system_proof;

		comm_app_bus(std::string _application_name,
			std::string _application_folder_name,
			std::string _config_filename_base)
		{
			system_monitoring_interface::start(); // this will create the global log queue.
			timer tx;
			date_time t = date_time::now();
			json_parser jp;

			log_command_start("comm_app_bus", "startup", t);

			ready_for_polling = false;

			app = std::make_shared<application>();
			app->application_folder_name = _application_folder_name;
			app->application_name = _application_name;

			MFStartup(MF_VERSION);

			factory = std::make_shared<directXAdapter>();

			// build the factory initially.  we may have occasion to call this 
			// in the future in the event of a system setting change or something.
			// otherwise.... I hate doing big things in constructors.
			factory->refresh();
			// create a win32 set of windows that can use this factory
			// to make windows with a d2d / d3d image
			app_ui = std::make_shared<directApplicationWin32>(this, factory);

			// create the presentation - this holds the data of what is on screen, for various pages.
			presentation_layer = std::make_shared<presentation>(this, app_ui);

			app_menu = std::make_shared<menu_item>();

			log_information("Default user:" + app->get_user_display_name());

			database_schema_filename = _config_filename_base + "schema.json";
			database_config_filename = "config.json";
			pages_config_filename = _config_filename_base + "pages.json";
			styles_config_filename = _config_filename_base + "styles.json";
			database_filename = app->get_data_filename("corona.cdb");

			char path[MAX_PATH + 16] = {};
			GetModuleFileNameA(NULL, path, MAX_PATH);
			PathRemoveFileSpecA(path);

			checker.path = path;

			if (not app->file_exists(database_filename))
			{
				db_file = app->open_file_ptr(database_filename, file_open_types::create_always);
				local_db = std::make_shared<corona_database>(db_file);

				if (read_json(database_config_filename, local_db_config) != null_row) {
					local_db->apply_config(local_db_config);
				}

				json create_database_response = local_db->create_database();

			}
			else
			{
				db_file = app->open_file_ptr(database_filename, file_open_types::open_existing);
				local_db = std::make_shared<corona_database>(db_file);

				if (read_json(database_config_filename, local_db_config) != null_row) {
					local_db->apply_config(local_db_config);
				}

				local_db->open_database(0);
			}

			json token = get_local_token();

			ready_for_polling = true;

			log_command_stop("comm_app_bus", "startup complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
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

			dependencies = { "rw locks", "json node" };
			testo = tm.create_test_set("json table", dependencies);
			test_json_table(testo, app);

			bool system_works = tm.prove("json table");
			if (not system_works) {
				log_job_stop("verification", "verification failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else
			{
				log_job_stop("verification", "verification complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
		}


		void poll_db()
		{
			timer tx;
			json_parser jp;
			json temp;
			date_time start_time = date_time::now();
			if (read_json(database_schema_filename, temp) != null_row) {
				log_command_start("poll_db", "apply schema", start_time, __FILE__, __LINE__);
				auto tempo = local_db->apply_schema(temp);
				log_command_stop("poll_db", "schema applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			timer tx2;
			if (read_json(database_config_filename, temp) != null_row) {
				log_command_start("poll_db", "apply config", start_time, __FILE__, __LINE__);
				local_db->apply_config(temp);
				log_command_stop("poll_db", "config applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
		}

		void poll_pages(bool _select_default_page)
		{
			json_parser			jp;
			json				pages_json;
			json				styles_json;

			read_json(pages_config_filename, pages_json);
			read_json(styles_config_filename, styles_json);

			timer tx;
			date_time t = date_time::now();

			log_job_start("poll_pages", "apply pages", t, __FILE__, __LINE__);

			abbreviations = jp.create_object();

			// to do, at some point create a merge method in json proper.
			json combined;
			if (styles_json.object() and pages_json.object())
			{
				combined = styles_json.clone();

				json jsrcstyles = pages_json["styles"].clone();
				json jdststyles = combined["styles"];

				json jabbreviations = pages_json["abbreviations"].clone();
				if (jabbreviations.object()) {
					auto jams = jabbreviations.get_members();
					for (auto m : jams) {
						abbreviations.put_member(m.first, m.second);
					}
				}

				jabbreviations = combined["abbreviations"].clone();
				if (jabbreviations.object()) {
					auto jams = jabbreviations.get_members();
					for (auto m : jams) {
						abbreviations.put_member(m.first, m.second);
					}
				}

				if (jsrcstyles.array() and jdststyles.array())
				{
					jdststyles.append_array(jsrcstyles);
				}
				else if (jsrcstyles.array())
				{
					combined.put_member_array("styles", jsrcstyles);
				}

				json jsrcpages = pages_json["pages"].clone();
				json jdstpages = combined["pages"];

				if (jsrcpages.array())
				{
					json jpages_expanded_array = jp.create_array();

					for (auto jpage : jsrcpages)
					{
						if (jpage.object()) {
							jpage.apply_abbreviations(abbreviations);
							std::string class_name = jpage[class_name_field];
							std::string file_name = jpage["file_name"];
							if (class_name == "import") 
							{
								json_parser jpx;
								std::string src_page = read_all_string(file_name);
								json expanded_page = jpx.parse_object(src_page);

								if (expanded_page.error()) 
								{
									auto errs = jpx.get_errors();
									log_error(errs, __FILE__, __LINE__);
								}
								else
								{
									expanded_page.apply_abbreviations(abbreviations);
									jpages_expanded_array.append_element(expanded_page);
									std::string story = std::format("imported page {0}", file_name);
									log_information(story, __FILE__, __LINE__);
								}
							}
							else
							{
								jpages_expanded_array.append_element(jpage);
							}
						}
					}

					if (jdstpages.array())
					{
						jdstpages.append_array(jpages_expanded_array);
					}
					else
					{
						combined.put_member_array("pages", jpages_expanded_array);
					}
				}

				json jsrcstartup = pages_json["startup"].clone();
				json jdststartup = combined["startup"];

				if (jsrcstartup.array() and jdststartup.array())
				{
					jdststartup.append_array(jsrcstartup);
				}
				else if (jsrcstartup.array())
				{
					combined.put_member_array("startup", jsrcstartup);
				}

				load_pages(combined, _select_default_page);
			}
			log_job_stop("poll_pages", "pages updated", tx.get_elapsed_seconds(), __FILE__, __LINE__);
		}

		virtual void poll(bool _select_default_page)
		{
			directory_checker::check_options options;

			options.files_to_ignore.insert_or_assign(database_filename, true);

			if (checker.check_changes( options)) {
				poll_db();
				poll_pages(_select_default_page);
			}
		}

		virtual void error(json _error)
		{
			log_error(_error);
		}

		virtual corona_client_response remote_register_user(std::string _user_name, std::string _email, std::string _password1, std::string _password2)
		{
			return client.register_user(_user_name, _email, _password1, _password2);
		}

		virtual corona_client_response remote_confirm_user(std::string _user_name, std::string _confirmation_code)
		{
			return client.confirm_user(_user_name, _confirmation_code);
		}

		virtual corona_client_response remote_send_user(std::string _user_name)
		{
			return client.send_user(_user_name);
		}

		virtual corona_client_response remote_login(std::string _user_name, std::string _password)
		{
			return client.login(_user_name, _password);
		}

		virtual corona_client_response remote_login()
		{
			return client.login();
		}

		virtual corona_client_response remote_set_password(std::string user_name, std::string validation_code, std::string password1, std::string password2)
		{
			return client.set_password(user_name, validation_code, password1, password2);
		}

		virtual corona_client_response remote_get_classes()
		{
			return client.get_classes();
		}

		virtual corona_client_response remote_get_class(std::string class_name)
		{
			return client.get_class(class_name);
		}
		virtual corona_client_response remote_put_class(json _class_definition)
		{
			return client.put_class(_class_definition);
		}
		virtual corona_client_response remote_create_object(std::string class_name)
		{
			return client.create_object(class_name);
		}
		virtual corona_client_response remote_edit_object(std::string _class_name, int64_t _object_id, bool _include_children)
		{
			return client.edit_object(_class_name, _object_id, _include_children);
		}
		virtual corona_client_response remote_get_object(std::string _class_name, int64_t _object_id, bool _include_children)
		{
			return client.get_object(_class_name, _object_id, _include_children);
		}
		virtual corona_client_response remote_put_object(json _object)
		{
			return client.put_object(_object);
		}
		virtual corona_client_response remote_run_object(json _object)
		{
			return client.run_object(_object);
		}
		virtual corona_client_response remote_delete_object(std::string _class_name, int64_t _object_id)
		{
			return client.delete_object(_class_name, _object_id);
		}
		virtual corona_client_response remote_query_objects(json _query)
		{
			return client.query_objects(_query);
		}

		virtual json  local_create_user(json user_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("create_user", user_information["Name"], dt);
			timer tx;
			json_parser jp;
			json request = jp.create_object();
			json token = get_local_token();
			request.put_member(token_field, token);
			request.put_member("Data", user_information);
			json j = local_db->create_user(request);
			if (j.error())
				log_error(j, __FILE__, __LINE__);
			log_command_stop("create_user", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return j;
		}

		virtual json  local_login_user(json login_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("login_user", login_information["Name"], dt);
			timer tx;
			json response = local_db->login_user(login_information);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			log_command_stop("login", response["Message"], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		json get_local_token()
		{
			json_parser jp;
			json login_request = jp.create_object();
			json server_config = local_db_config["Server"];
			std::string user_name = server_config[sys_user_name_field];
			std::string password = server_config[sys_user_password_field];
			login_request.put_member(sys_user_name_field, user_name);
			login_request.put_member(sys_user_password_field, password);
			json result = local_db->login_user(login_request);
			return result[token_field];
		}

		virtual json  local_create_object(std::string class_name)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("create_object", class_name, dt);

			json token = get_local_token();
			timer tx;
			json_parser jp;
			json request = jp.create_object();
			request.put_member(token_field, token);
			request.put_member(class_name_field, class_name);
			json response = local_db->create_object(request);
			log_command_stop("create_object", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			response = response[data_field];
			return response;
		}

		virtual json  local_run_object(json object_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("run_object", object_information[class_name_field], dt);
			timer tx;
			json token = get_local_token();

			json_parser jp;
			json request = object_information.clone();
			request.put_member(token_field, token);
			request.put_member(data_field, object_information);
			json response = local_db->put_object(request);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			response = response[data_field];
			log_command_stop("put_object", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		virtual json  local_put_object(json object_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("put_object", object_information[class_name_field], dt);
			timer tx;
			json token = get_local_token();

			json_parser jp;
			json request = object_information.clone();
			request.put_member(token_field, token);
			request.put_member(data_field, object_information);
			json response = local_db->put_object(request);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			response = response[data_field];
			log_command_stop("put_object", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}


		virtual json  local_get_object(json object_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("get_object", object_information[class_name_field], dt);
			timer tx;
			json_parser jp;
			json request = object_information.clone();
			json token = get_local_token();
			request.put_member(token_field, token);
			json response = local_db->get_object(request);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			log_command_stop("get_object", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			response = response[data_field];
			return response;
		}

		virtual json  local_edit_object(std::string _class_name, int64_t _object_id)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("edit_object", _class_name, dt);
			timer tx;
			json_parser jp;
			json request = jp.create_object();
			json token = get_local_token();
			request.put_member(token_field, token);
			json response = local_db->edit_object(request);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			log_command_stop("edit_object", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			response = response[data_field];
			return response;
		}

		virtual json  local_delete_object(json object_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("delete_object", object_information[class_name_field], dt);
			timer tx;
			json request = object_information.clone();
			json token = get_local_token();
			request.put_member(token_field, token);
			json response = local_db->put_object(request);
			if (response.error())
				log_error(response, __FILE__, __LINE__);

			log_command_stop("delete_object", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			response = response[data_field];
			return response;
		}


		virtual json  local_query_objects(json query_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("query_objects", query_information[class_name_field], dt);
			timer tx;
			json_parser jp;
			json request = jp.create_object();
			json token = get_local_token();
			query_information.put_member(token_field, token);
			json response = local_db->query(request);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			response = response[data_field];
			if (response.array()) {
				std::string rr = std::format("{0} items", response.size());
				log_information(rr);
			}
			log_command_stop("query_objects", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		virtual json  create_object(corona_instance _instance, std::string class_name)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("create_object", class_name, dt);
			json response;
			timer tx;

			if (_instance == corona_instance::local)
			{
				auto result = local_create_object(class_name);
				response = result;
			}
			else
			{
				auto result = remote_create_object(class_name);
				if (!result.success) {
					log_error(result, __FILE__, __LINE__);
				}
				response = result.data;
			}

			log_command_stop("create_object", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		virtual json  put_object(corona_instance _instance, json object_information)
		{
			date_time dt;
			dt = date_time::now();
			timer tx;
			log_command_start("put_object", object_information[class_name_field], dt);
			json response;

			if (_instance == corona_instance::local)
			{
				return local_put_object(object_information);
			}
			else
			{
				auto result = remote_put_object(object_information);
				if (!result.success) {
					log_error(result, __FILE__, __LINE__);
				}
				response = result.data;
			}

			log_command_stop("put_object", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		virtual json  run_object(corona_instance _instance, json object_information)
		{
			date_time dt;
			dt = date_time::now();
			timer tx;
			log_command_start("run_object", object_information[class_name_field], dt);
			json response;

			if (_instance == corona_instance::local)
			{
				return local_run_object(object_information);
			}
			else
			{
				auto result = remote_run_object(object_information);
				if (!result.success) {
					log_error(result, __FILE__, __LINE__);
				}
				response = result.data;
			}

			log_command_stop("run_object", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		virtual json  edit_object(corona_instance _instance, json object_information)
		{
			date_time dt;
			dt = date_time::now();
			timer tx;
			log_command_start("edit_object", object_information[class_name_field], dt);
			json response;

			std::string class_name = object_information[class_name_field];
			int64_t object_id = (int64_t)object_information[object_id_field];

			if (_instance == corona_instance::local)
			{
				return local_edit_object(class_name, object_id);
			}
			else
			{
				auto result = remote_edit_object(class_name, object_id, true);
				if (!result.success) {
					log_error(result, __FILE__, __LINE__);
				}
				response = result.data;
			}

			log_command_stop("edit_object", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		virtual json  get_object(corona_instance _instance, json object_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("get_object", object_information[class_name_field], dt);
			json response;
			
			if (_instance == corona_instance::local)
			{
				return local_get_object(object_information);
			}
			else
			{
				std::string class_name = object_information[class_name_field];
				int64_t object_id = (int64_t)object_information[object_id_field];
				auto result = remote_get_object(class_name, object_id, false);
				if (!result.success) {
					log_error(result, __FILE__, __LINE__);
				}
				response = result.data;
			}

			return response;
		}

		virtual json  delete_object(corona_instance _instance, json object_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("delete_object", object_information[class_name_field], dt);
			json response;

			if (_instance == corona_instance::local)
			{
				return local_delete_object(object_information);
			}
			else
			{
				std::string class_name = object_information[class_name_field];
				int64_t object_id = (int64_t)object_information[object_id_field];

				auto result = remote_delete_object(class_name, object_id);
				if (!result.success) {
					log_error(result, __FILE__, __LINE__);
				}
				response = result.data;
			}
			return response;
		}


		virtual json  query_objects(corona_instance _instance, json query_information)
		{
			if (_instance == corona_instance::local)
			{
				return local_query_objects(query_information);
			}

			date_time dt;
			dt = date_time::now();
			log_command_start("query_objects", query_information[class_name_field], dt);
			timer tx;
			json_parser jp;
			json request = jp.create_object();
			json token = get_local_token();
			query_information.put_member(token_field, token);
			json response =  local_db->query(request);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			response = response[data_field];
			if (response.array()) {
				std::string rr = std::format("{0} items", response.size());
				log_information(rr);
			}
			log_command_stop("query_objects", response[message_field], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		virtual control_base* find_control(std::string _name)
		{
			return presentation_layer->find_ptr<control_base>(_name);
		}

		virtual control_base* find_control(int _id)
		{
			return presentation_layer->find_ptr<control_base>(_id);
		}

		void load_pages(json _pages, bool _select_default)
		{
			date_time dt = date_time::now();
			log_function_start("load_pages", "", dt);
			run_ui([this, _pages, _select_default]() ->void {

				timer tx;
				date_time dt = date_time::now();
				std::stringstream page_message;
				std::string default_page = presentation_layer->setPresentation(_pages);
				if (_select_default and !default_page.empty()) {
					presentation_layer->select_page(default_page);
					page_message << "Pages loaded, default page: " << default_page;
				}
				else {
					page_message << "Pages loaded:" << default_page;
				}
				log_function_stop("load_pages", page_message.str(), tx.get_elapsed_seconds(), __FILE__, __LINE__);

				json jcommands = _pages["startup"];

				log_function_start("startup commands", "", dt);
				log_json<json>(jcommands);

				if (jcommands.array())
				{
					for (auto cmd : jcommands) {
						std::shared_ptr<corona_bus_command> command;
						corona::put_json(command, cmd);
						if (command) {
							run_system_command(command);
						}
					}
				}
				log_function_stop("startup commands", page_message.str(), tx.get_elapsed_seconds(), __FILE__, __LINE__);

			});
		}

		virtual void select_page(std::string _page, std::string _target_control, json _obj)
		{
			date_time dt = date_time::now();
			log_function_start("select_page", _page, dt);

			run_ui([this, _page, _target_control, _obj]() ->void {
				timer tx;
				presentation_layer->select_page(_page);
				if (not _target_control.empty()) {
					control_base* cb = find_control(_target_control);
					if (cb) {
						if (not cb->set_items(_obj)) {
							cb->set_data(_obj);
						}
					}
				}
				log_function_start("select_page", _page, tx.get_elapsed_seconds());
			});
		}

		virtual void select_page(std::string _page, std::string _target_frame, std::string _frame_contents_page, std::string _form_to_load, json _obj)
		{
			date_time dt = date_time::now();
			log_command_start("select_page", _page, dt);

			run_ui([this, _page, _target_frame, _frame_contents_page, _form_to_load, _obj]() ->void {
				timer tx;
				if (not presentation_layer->is_current_page(_page)) {
					presentation_layer->select_page(_page);
				}
				if (not _target_frame.empty()) {
					control_base* cb = find_control(_target_frame);
					if (cb) {
						frame_layout* fl = dynamic_cast<frame_layout*>(cb);
						if (fl) {
							if (presentation_layer->pages.contains(_frame_contents_page)) {
								auto pg_src = presentation_layer->pages[_frame_contents_page];
								auto pg_master = presentation_layer->get_current_page();
								fl->set_contents(presentation_layer.get(), pg_master, pg_src.get());
							}
						}
						if (not cb->set_items(_obj)) {
							cb->set_data(_obj);
						}
						if (not _form_to_load.empty()) {
							control_base* formx = find_control(_form_to_load);
							if (formx) {
								formx->set_data(_obj);
							}
						}
						presentation_layer->onResize();
					}
				}
				log_command_stop("select_page", _page, tx.get_elapsed_seconds());
			});
		}

		void when(UINT topic, std::function<void()> _runnable)
		{
			windows_event_waiter evt;
			evt.msg = topic;
			std::pair<UINT,windows_event_waiter> pair;
			pair.first = topic;
			pair.second = evt;
			windows_waiters.insert(pair);
			::WaitForSingleObject(evt.hevent, INFINITE);
			_runnable();
		}

		void when(std::string _topic, std::function<void()> _runnable)
		{
			topic_event_waiter evt;
			evt.topic = _topic;
			std::pair<std::string, topic_event_waiter> pair;
			pair.first = _topic;
			pair.second = evt;
			topic_waiters.insert(pair);
			::WaitForSingleObject(evt.hevent, INFINITE);
			_runnable();
		}

		virtual void run_app_ui(HINSTANCE hInstance, LPSTR command_line, bool fullScreen)
		{
			app_ui->runDialog(hInstance, app->application_name.c_str(), application_icon_id, fullScreen, presentation_layer);
		}

		virtual void run_system_command(std::shared_ptr<corona_bus_command> _command)
		{
			if (_command) {
				json_parser jp;
				json jcommand = jp.create_object();
				corona::get_json(jcommand, _command);
				date_time start_time = date_time::now();
				log_function_start("run_command", jcommand["class_name"], start_time, __FILE__, __LINE__);
				this->run_ui([this, jcommand]() {
					timer tx;
					log_json<json>(jcommand);
					std::shared_ptr<corona_bus_command> command;
					corona::put_json(command, jcommand);
					if (command) {
						command->bus = this;
						auto tranny = command->execute();
					}
					log_function_stop("run_command", jcommand["class_name"], tx.get_elapsed_seconds(), __FILE__, __LINE__);
					});
			}
		}

		virtual void run_command(std::shared_ptr<corona_bus_command> _command)
		{
			if (_command) {
				json_parser jp;
				json jcommand = jp.create_object();
				corona::get_json(jcommand, _command);
				date_time start_time = date_time::now();
				log_user_command_start("run_command", jcommand["class_name"], start_time, __FILE__, __LINE__);
				this->run_ui([this, jcommand]() {
					timer tx;
					log_json<json>(jcommand);
					std::shared_ptr<corona_bus_command> command;
					corona::put_json(command, jcommand);
					if (command) {
						command->bus = this;
						auto tranny = command->execute();
					}
					log_user_command_stop("run_command", jcommand["class_name"], tx.get_elapsed_seconds(), __FILE__, __LINE__);
				});
			}
		}

	};
}

#endif
