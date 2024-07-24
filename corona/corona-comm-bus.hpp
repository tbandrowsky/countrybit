#pragma once

#ifndef CORONA_COMM_BUS_H
#define CORONA_COMM_BUS_H

namespace corona
{

	class comm_bus : public comm_bus_interface
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

		json								admin_user;
		buffer								io_buffer;

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

		comm_bus(std::string _application_name, 
			std::string _application_folder_name)
		{
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

			database_schema_file_name = "schema.json";
			database_config_file_name = "config.json";
			pages_config_file_name = "pages.json";
			styles_config_file_name = "styles.json";
			database_file_name = app->get_data_filename("corona.cdb");
			user_file_name = app->get_data_filename("user.json");

			database_schema_mon.file_name = database_schema_file_name;
			database_config_mon.file_name = database_config_file_name;
			pages_config_mon.file_name = pages_config_file_name;
			styles_config_mon.file_name = styles_config_file_name;

			if (!app->file_exists(database_file_name)) 
			{
				db_file = app->open_file_ptr(database_file_name, file_open_types::create_always);
				local_db = std::make_shared<corona_database>(db_file);

				file_transaction<relative_ptr_type> result = database_config_mon.poll(app.get());

				auto admin_user_transaction = local_db->create_database();
				admin_user = admin_user_transaction.wait();
				if (admin_user.is_object()) {
					std::string suser_json = admin_user.to_json();
					io_buffer.set_buffer(suser_json);
					file user_file = app->open_file(user_file_name, file_open_types::create_always);
					file_task task = user_file.write(0, io_buffer.get_ptr(), io_buffer.get_size());
					task.initiate();
				}
			} 
			else 
			{
				db_file = app->open_file_ptr(database_file_name, file_open_types::open_existing);
				local_db = std::make_shared<corona_database>(db_file);
				json_file_watcher user_file_watcher;
				user_file_watcher.file_name = user_file_name;
				user_file_watcher.poll_contents(app.get(), admin_user);
			}
		}

		void poll_db()
		{
			json_parser jp;
			json temp;
			if (database_schema_mon.poll_contents(app.get(), temp) != null_row) {
				auto tempo = local_db->apply_schema(temp);
				tempo.wait();
			}
			if (database_config_mon.poll_contents(app.get(), temp) != null_row) {
				local_db->apply_config(temp);
			}
		}

		void poll_pages(bool _select_default_page)
		{
			json_parser			jp;
			json				pages_json;
			json				styles_json;
			relative_ptr_type	pages_changed = pages_config_mon.poll_contents(app.get(), pages_json);
			relative_ptr_type	styles_changed = styles_config_mon.poll_contents(app.get(), styles_json);

			if (pages_changed != null_row || 
				styles_changed != null_row)
			{
				// to do, at some point create a merge method in json proper.
				json combined;
				if (styles_json.is_object() && pages_json.is_object())
				{
					combined = styles_json.clone();
					json jsrcstyles = pages_json["styles"].clone();
					json jdststyles = combined["styles"];

					if (jsrcstyles.is_array() && jdststyles.is_array()) 
					{
						jdststyles.append_array(jsrcstyles);
					}
					else if (jsrcstyles.is_array())
					{
						combined.put_member_array("styles", jsrcstyles);
					}

					json jsrcpages = pages_json["pages"].clone();
					json jdstpages = combined["pages"];

					if (jsrcpages.is_array() && jdstpages.is_array())
					{
						jdstpages.append_array(jsrcpages);
					}
					else if (jsrcpages.is_array())
					{
						combined.put_member_array("pages", jsrcpages);
					}

					load_pages(combined, _select_default_page);
				}
			}
		}

		virtual void poll(bool _select_default_page)
		{
			poll_db();
			poll_pages(_select_default_page);
		}

		virtual comm_bus_transaction<json>  create_user(json user_information)
		{
			json_parser jp;
			json request = jp.create_object();
			request.put_member("Token", admin_user);
			request.put_member("Data", user_information);
			json j = co_await local_db->create_user(request);
			co_return j;
		}

		virtual comm_bus_transaction<json>  login_user(json login_information)
		{
			json j = co_await local_db->login_user(login_information);
			co_return j;
		}

		virtual comm_bus_transaction<json>  create_object(std::string class_name)
		{
			json_parser jp;
			json request = jp.create_object();
			request.put_member("Token", admin_user);
			json data = jp.create_object();
			data.put_member("ClassName", class_name);
			request.put_member("Data", data);
			json response = co_await local_db->create_object(request);
			response = response["Data"];
			co_return response;
		}

		virtual comm_bus_transaction<json>  put_object(json object_information)
		{
			json_parser jp;
			json request = jp.create_object();
			request.put_member("Token", admin_user);
			request.put_member("Data", object_information);
			json response = co_await local_db->put_object(request);
			response = response["Data"];
			co_return response;
		}

		virtual comm_bus_transaction<json>  get_object(json object_information)
		{
			json_parser jp;
			json request = jp.create_object();
			request.put_member("Token", admin_user);
			request.put_member("Data", object_information);
			json response = co_await local_db->put_object(request);
			response = response["Data"];
			co_return response;
		}

		virtual comm_bus_transaction<json>  delete_object(json object_information)
		{
			json_parser jp;
			json request = jp.create_object();
			request.put_member("Token", admin_user);
			request.put_member("Data", object_information);
			json response = co_await local_db->put_object(request);
			response = response["Data"];
			co_return response;
		}

		virtual comm_bus_transaction<json>  pop_object(json user_information)
		{
			json_parser jp;
			json request = jp.create_object();
			request.put_member("Token", admin_user);
			request.put_member("Data", user_information);
			json response = co_await local_db->put_object(request);
			response = response["Data"];
			co_return response;
		}

		virtual comm_bus_transaction<json>  query_objects(json query_information)
		{
			json_parser jp;
			json request = jp.create_object();
			request.put_member("Token", admin_user);
			request.put_member("Data", query_information);
			json response = co_await local_db->query_class(request);
			response = response["Data"];
			co_return response;
		}

		virtual comm_bus_transaction<table_data>  query_objects_as_table(json query_information)
		{
			table_data td;
			json_parser jp;
			json request = jp.create_object();
			request.put_member("Token", admin_user);
			request.copy_member("Filter", query_information);
			json table = request["Table"];
			td.put_json(table);			
			json response = co_await local_db->query_class(request);
			json response_items = response["Data"];
			td.items = response_items;
			co_return td;
		}

		virtual comm_bus_transaction<list_data>  query_objects_as_list(json query_information)
		{
			list_data ld;
			json_parser jp;
			json request = jp.create_object();
			request.put_member("Token", admin_user);
			request.copy_member("Filter", query_information);
			json lst = request["List"];
			ld.put_json(lst);
			json response = co_await local_db->query_class(request);
			json response_items = response["Data"];
			ld.items = response_items;
			co_return ld;
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
			run_ui([this, _pages, _select_default]() ->void {
				std::string default_page = presentation_layer->setPresentation(_pages);
				if (_select_default && !default_page.empty()) {
					presentation_layer->select_page(default_page);
					std::cout << "Pages loaded, default page: " << default_page << std::endl;
				}
				else {
					std::cout << "Pages loaded:" << default_page << std::endl;
				}
				
				});
		}

		virtual void select_page(std::string _page, std::string _target_control, json _obj)
		{
			run_ui([this, _page, _target_control, _obj]() ->void {
				presentation_layer->select_page(_page);
				if (!_target_control.empty()) {
					control_base* cb = find_control(_target_control);
					if (cb) {
						if (!cb->set_items(_obj)) {
							cb->set_data(_obj);
						}
					}
				}
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
			if (strstr(command_line, "-console")) {
				EnableGuiStdOuts();
				std::cout << "Revolution Started" << std::endl;
			}

			app_ui->runDialog(hInstance, app->application_name.c_str(), application_icon_id, fullScreen, presentation_layer);
		}

	};
}

#endif
