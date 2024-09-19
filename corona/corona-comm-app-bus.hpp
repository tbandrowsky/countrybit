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

		json								local_db_config;

	public:

		std::shared_ptr<corona_database>	local_db;
		std::shared_ptr<application>		app;
		std::shared_ptr<file>				db_file;

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

		std::string database_schema_file_name;
		std::string database_config_file_name;
		std::string styles_config_file_name;
		std::string pages_config_file_name;

		json_file_watcher database_schema_mon;
		json_file_watcher database_config_mon;
		json_file_watcher pages_config_mon;
		json_file_watcher styles_config_mon;

		std::string database_file_name;
		std::string user_file_name;

		bool ready_for_polling;

		json system_proof;

		comm_app_bus(std::string _application_name,
			std::string _application_folder_name,
			std::string _config_file_name_base)
		{
			system_monitoring_interface::start(); // this will create the global log queue.
			timer tx;
			date_time t = date_time::now();
			json_parser jp;

			log_command_start("comm_app_bus", "startup", t);

			log_information("Thank you for flying with Country Video Games");
			log_information("Country Video Games Corona is going to do a formal verification of itself");
			log_information("and this environment.");

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

			prove_system();

			log_information("Welcome to the Revolution " + app->get_user_display_name());
			log_information("Your first super secret order:");
			log_information("remember to drink your ovaltine");

			database_schema_file_name = _config_file_name_base + "schema.json";
			database_config_file_name = "config.json";
			pages_config_file_name = _config_file_name_base + "pages.json";
			styles_config_file_name = _config_file_name_base + "styles.json";
			database_file_name = app->get_data_filename("corona.cdb");

			database_schema_mon.file_name = database_schema_file_name;
			database_config_mon.file_name = database_config_file_name;
			pages_config_mon.file_name = pages_config_file_name;
			styles_config_mon.file_name = styles_config_file_name;

			if (not app->file_exists(database_file_name)) 
			{
				db_file = app->open_file_ptr(database_file_name, file_open_types::create_always);
				local_db = std::make_shared<corona_database>(this, db_file);

				if (database_config_mon.poll_contents(app.get(), local_db_config) != null_row) {
					local_db->apply_config(local_db_config);
				}

				json create_database_response = local_db->create_database();

				relative_ptr_type result = database_config_mon.poll(app.get());
				ready_for_polling = true;

				json token = get_local_token();
			} 
			else 
			{
				db_file = app->open_file_ptr(database_file_name, file_open_types::open_existing);
				local_db = std::make_shared<corona_database>(db_file);

				if (database_config_mon.poll_contents(app.get(), local_db_config) != null_row) {
					local_db->apply_config(local_db_config);
				}

				local_db->open_database(0);

				json token = get_local_token();

				relative_ptr_type result = database_config_mon.poll(app.get());
				ready_for_polling = true;
			}

			log_command_stop("comm_app_bus", "startup complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			log_information("This is an immensely modable system.");
			log_information("config.json - for networking stuff in the next release.");
			log_information("revolution_schema.json - the database configuration is here.");
			log_information("			   you can add your own data sources.");
			log_information("			   add new fields, all while the system");
			log_information("			   is running.");
			log_information("revolution_styles.json -  similar to CSS, lets you change colors and fonts.");
			log_information("revolution_pages.json -  forms, plus the commands back to the engine.");
			log_information("                         edit this guy to change the way the app works.");
			log_information("the database is in");
			log_information("username/appdata/roaming/appname");
			log_information("The schema, styles and pages files can be edited live.");
			log_information("We are cleared for departure.");
		}

		void prove_system()
		{
			timer tx;
			date_time t = date_time::now();

			json_parser jp;
			json system_proof = jp.create_object();

			log_job_start("verification", "verification start", t, __FILE__, __LINE__);

			test_object(system_proof, app);
			test_file_block(system_proof, app);
			test_file(system_proof, app);
			test_data_block(system_proof, app);
			test_json_node(system_proof, app);
			test_json_table(system_proof, app);

			bool system_works = system_proof.prove_member("is_true");
			if (not system_works) {
				json top_level = system_proof["table"];
				json reason = failure_analysis(system_proof, top_level, "is_true");
				log_warning("This system does not work because:", __FILE__, __LINE__);
				log_json(reason);
				log_job_stop("verification", "verification failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else 
			{
				log_job_stop("verification", "verification complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
		}

		// if the 

		json failure_analysis(json _root_proof, json _current, std::string _proposition)
		{
			json_parser jp;
			json reason;

			reason = jp.create_object();

			if (_current.object()) {
				bool is_true = (bool)_current[_proposition];
				if (is_true) 
				{
					json reason = jp.from_integer(1);
					return reason;
				}
				else 
				{
					json dependencies = _current[ "dependencies" ];
					if (dependencies.object()) {
						auto depmembers = dependencies.get_members();
						for (auto depm : depmembers) {
							std::string assertion_name = depm.first;
							bool is_true = (bool)_current[assertion_name];
							if (not is_true) {
								json underlying = depm.second;
								if (underlying.array()) {
									for (auto path : underlying) {
										json source_chumpy = _root_proof.query(path);
										if (source_chumpy.object()) {
											json fail_reason = failure_analysis(_root_proof,
												source_chumpy["object"],
												source_chumpy["name"]
											);
											if (fail_reason.object())
											{
												return fail_reason;
											}
										}
										std::string test_name = _current["test_name"];
										json fail_reason = jp.create_object();
										fail_reason.put_member("object", assertion_name);
										fail_reason.put_member("name", test_name);
										return fail_reason;
									}
								}
							}
						}
					}
				}
			}
			reason = jp.from_integer(1);
			return reason;
		}

		void poll_db()
		{
			if (ready_for_polling) {
				timer tx;
				json_parser jp;
				json temp;
				date_time start_time = date_time::now();
				if (database_schema_mon.poll_contents(app.get(), temp) != null_row) {
					log_command_start("poll_db", "apply schema", start_time, __FILE__, __LINE__);
					auto tempo = local_db->apply_schema(temp);
					log_command_stop("poll_db", "schema applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
				timer tx2;
				if (database_config_mon.poll_contents(app.get(), temp) != null_row) {
					log_command_start("poll_db", "apply config", start_time, __FILE__, __LINE__);
					local_db->apply_config(temp);
					log_command_stop("poll_db", "config applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
			}
		}

		void poll_pages(bool _select_default_page)
		{
			json_parser			jp;
			json				pages_json;
			json				styles_json;
			relative_ptr_type	pages_changed = pages_config_mon.poll_contents(app.get(), pages_json);
			relative_ptr_type	styles_changed = styles_config_mon.poll_contents(app.get(), styles_json);

			if (pages_changed != null_row or 
				styles_changed != null_row)
			{
				timer tx;
				date_time t = date_time::now();

				log_job_start("poll_pages", "apply pages", t, __FILE__, __LINE__);

				// to do, at some point create a merge method in json proper.
				json combined;
				if (styles_json.object() and pages_json.object())
				{
					combined = styles_json.clone();
					json jsrcstyles = pages_json["styles"].clone();
					json jdststyles = combined["styles"];

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

					if (jsrcpages.array() and jdstpages.array())
					{
						jdstpages.append_array(jsrcpages);
					}
					else if (jsrcpages.array())
					{
						combined.put_member_array("pages", jsrcpages);
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
		}

		virtual void poll(bool _select_default_page)
		{
			poll_db();
			poll_pages(_select_default_page);
		}

		virtual void error(json _error)
		{
			log_error(_error);
		}

		virtual json  create_user(json user_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("create_user", user_information["Name"], dt);
			timer tx;
			json_parser jp;
			json request = jp.create_object();
			json token = get_local_token();
			request.put_member("Token", token);
			request.put_member("Data", user_information);
			json j =  local_db->create_user(request);
			if (j.error())
				log_error(j, __FILE__, __LINE__);
			log_command_stop("create_user", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return j;
		}

		virtual json  login_user(json login_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("login_user", login_information["Name"], dt);
			timer tx;
			json response =  local_db->login_user(login_information);
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

		virtual json  create_object(std::string class_name)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("create_object", class_name, dt);

			json token = get_local_token();
			timer tx;
			json_parser jp;
			json request = jp.create_object();
			request.put_member("Token", token);
			request.put_member("ClassName", class_name);
			json response =  local_db->create_object(request);
			log_command_stop("create_object", response["Message"], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			response = response["Data"];
			return response;
		}

		virtual json  put_object(json object_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("put_object", object_information["ClassName"], dt);
			timer tx;
			json token = get_local_token();

			json_parser jp;
			json request = object_information.clone();
			request.put_member("Token", token);
			request.put_member("Data", object_information);
			json response =  local_db->put_object(request);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			response = response["Data"];
			log_command_stop("put_object", response["Message"], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		virtual json  get_object(json object_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("get_object", object_information["ClassName"], dt);
			timer tx;
			json_parser jp;
			json request = object_information.clone();
			json token = get_local_token();
			request.put_member("Token", token);
			json response =  local_db->get_object(request);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			log_command_stop("get_object", response["Message"], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			response = response["Data"];
			return response;
		}

		virtual json  delete_object(json object_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("delete_object", object_information["ClassName"], dt);
			timer tx;
			json request = object_information.clone();
			json token = get_local_token();
			request.put_member("Token", token);
			json response =  local_db->put_object(request);
			if (response.error())
				log_error(response, __FILE__, __LINE__);

			log_command_stop("delete_object", response["Message"], tx.get_elapsed_seconds(), __FILE__, __LINE__);
			response = response["Data"];
			return response;
		}


		virtual json  query_objects(json query_information)
		{
			date_time dt;
			dt = date_time::now();
			log_command_start("query_objects", query_information["ClassName"], dt);
			timer tx;
			json_parser jp;
			json request = jp.create_object();
			json token = get_local_token();
			query_information.put_member("Token", token);
			json response =  local_db->query(request);
			if (response.error())
				log_error(response, __FILE__, __LINE__);
			response = response["Data"];
			if (response.array()) {
				std::string rr = std::format("{0} items", response.size());
				log_information(rr);
			}
			log_command_stop("query_objects", response["Message"], tx.get_elapsed_seconds(), __FILE__, __LINE__);
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

				json jcommands = _pages.get_member("startup");

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
