#pragma once

#ifndef CORONA_COMM_BUS_COMMANDS_H
#define CORONA_COMM_BUS_COMMANDS_H

namespace corona
{

	class corona_create_object_command : public corona_bus_command
	{
	public:
		std::string	create_class_name;
		std::string form_name;

		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = bus->find_control(form_name);
			if (cb) {
				obj = co_await bus->create_object(create_class_name);
				cb->put_json(obj);
			}
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "create_object_command");
			_dest.put_member("create_class_name", create_class_name);
			_dest.put_member("form_name", form_name);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (!_src.has_members(missing, { "create_class_name", "form_name" })) {
				system_monitoring_interface::global_mon->log_warning("create_object_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			create_class_name = _src["create_class_name"];
			form_name = _src["form_name"];
		}

	};

	class corona_select_object_command : public corona_bus_command
	{
	public:
		std::string	table_name;
		std::string page_name;
		std::string form_name;

		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = bus->find_control(table_name);
			if (cb) {
				json key_data = cb->get_selected_object();
				if (key_data.object()) {
					obj = co_await bus->get_object(key_data);
					bus->select_page(page_name, form_name, obj);
				}
			}
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "select_object_command");
			_dest.put_member("table_name", table_name);
			_dest.put_member("form_name", form_name);
			_dest.put_member("page_name", page_name);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (!_src.has_members(missing, { "table_name", "form_name", "page_name"})) {
				system_monitoring_interface::global_mon->log_warning("select_object_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			table_name = _src["table_name"];
			form_name = _src["form_name"];
			page_name = _src["page_name"];
		}

	};

	class corona_preview_object_command : public corona_bus_command
	{
	public:
		std::string	table_name;
		std::string form_name;

		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = bus->find_control(table_name);
			if (cb) {
				json key_data = cb->get_selected_object();
				if (key_data.object()) {
					obj = co_await bus->get_object(key_data);
					control_base* cf = bus->find_control(form_name);
					if (cf) {
						cf->set_data(obj);
					}
				}
			}
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "preview_object_command");
			_dest.put_member("table_name", table_name);
			_dest.put_member("form_name", form_name);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (!_src.has_members(missing, { "table_name", "form_name" })) {
				system_monitoring_interface::global_mon->log_warning("preview_object_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			table_name = _src["table_name"];
			form_name = _src["form_name"];
		}

	};

	class corona_save_object_command : public corona_bus_command
	{
	public:

		std::string form_name;

		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = nullptr;

			if (!form_name.empty())
				cb = bus->find_control(form_name);

			if (cb) {
				json object_data = cb->get_data();
				if (object_data.object()) {
					obj = co_await bus->put_object(object_data);
				}
			}
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "save_object_command");
			_dest.put_member("form_name", form_name);
		}

		virtual void put_json(json& _src)
		{
			form_name = _src["form_name"];
			std::vector<std::string> missing;
			if (!_src.has_members(missing, { "form_name" })) {
				system_monitoring_interface::global_mon->log_warning("save_object_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}


		}

	};

	class corona_load_object_command : public corona_bus_command
	{
	public:
		std::string control_name;
		json		object_data;

		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = nullptr;
			
			cb = bus->find_control(control_name);

			if (cb) {
				if (object_data.object()) {
					obj = co_await bus->put_object(object_data);
				}
			}
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "load_object_command");
			_dest.put_member("control_name", control_name);
			_dest.put_member("data", object_data);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (!_src.has_members(missing, { "control_name", "data" })) {
				system_monitoring_interface::global_mon->log_warning("load_object_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}
			control_name = _src["control_name"];
			object_data = _src["data"];
		}

	};

	class corona_delete_object_command : public corona_bus_command
	{
	public:
		std::string		control_name;

		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = bus->find_control(control_name);
			if (cb) {
				json object_data = cb->get_data();
				if (object_data.object()) {
					obj = co_await bus->delete_object(object_data);
				}
			}
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "delete_object_command");
			_dest.put_member("control_name", control_name);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (!_src.has_members(missing, { "class_name", "control_name" })) {
				system_monitoring_interface::global_mon->log_warning("delete_object_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			control_name = _src["control_name"];
		}

	};

	class corona_search_objects_command : public corona_bus_command
	{
	public:
		std::string			search_class_name;
		std::string			form_name;
		std::string			table_name;
		query_context		qctx;

		virtual comm_bus_transaction<json> execute()
		{
			json_parser jp;
			json obj;
			control_base* cb_form = {};
			control_base* cb_table = {};

			if (!form_name.empty())
				cb_form = bus->find_control(form_name);

			if (!table_name.empty())
				cb_table = bus->find_control(table_name);

			if (cb_form && cb_table) 
			{
				json search_class_filters = jp.create_object();
				search_class_filters.put_member("ClassName", search_class_name);
				json object_data = cb_form->get_data();
				if (object_data.object()) {
					json search_class = jp.create_object();
					search_class.put_member("ClassName", search_class_name);
					search_class_filters.put_member("Filter", object_data);
					obj = co_await bus->query_objects(search_class_filters);
					qctx.set_data_source(form_name, object_data);
					qctx.set_data_source(search_class_name, obj);
					json results = qctx.run();
					if (cb_table) {
						cb_table->set_items(results);
					}
				}
			}

			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "search_objects_command");
			_dest.put_member("search_class_name", search_class_name);
			_dest.put_member("form_name", form_name);
			_dest.put_member("table_name", table_name);
			json_parser jp;
			json jctx = jp.create_object();
			qctx.get_json(jctx);
			_dest.put_member("query", jctx);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (!_src.has_members(missing, { "form_name", "table_name", "search_class_name"})) {
				system_monitoring_interface::global_mon->log_warning("search_objects_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			form_name = _src["form_name"];
			table_name = _src["table_name"];
			search_class_name = _src["search_class_name"];
			json jctx = _src["query"];
			qctx.put_json(jctx);
		}

	};

	class corona_select_page_command : public corona_bus_command
	{
	public:
		std::string		page_name;
		std::string		form_name;
		std::string		source_name;

		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = {};
			if (!source_name.empty())
				cb = bus->find_control(source_name);
			json data;
			if (cb) {
				data = cb->get_data();
			}
			bus->select_page(page_name, form_name, data);
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "select_page_command");
			_dest.put_member("page_name", page_name);
			_dest.put_member("form_name", form_name);
			_dest.put_member("source_name", source_name);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (!_src.has_members(missing, { "page_name" })) {
				system_monitoring_interface::global_mon->log_warning("select_page_command missing:");

				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			if (system_monitoring_interface::global_mon->enable_options_display) {
				missing.clear();
				if (!_src.has_members(missing, { "form_name", "source_name" })) {
					system_monitoring_interface::global_mon->log_warning("select_object_command has options:");
					std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
						system_monitoring_interface::global_mon->log_warning(s);
						});
					system_monitoring_interface::global_mon->log_information("the source json is:");
					system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				}
			}
			page_name = _src["page_name"];
			form_name = _src["form_name"];
			source_name = _src["source_name"];
		}
	};

	class corona_select_frame_command : public corona_bus_command
	{
	public:
		std::string		form_to_read;
		std::string		page_to_select;
		std::string		frame_to_load;
		std::string		frame_contents_page;

		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = {};
			if (!form_to_read.empty())
				cb = bus->find_control(form_to_read);
			json data;
			if (cb) {
				data = cb->get_data();
			}
			bus->select_page(page_to_select, frame_to_load, frame_contents_page, data);
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "select_frame_command");
			_dest.put_member("form_to_read", form_to_read);
			_dest.put_member("page_to_select", page_to_select);
			_dest.put_member("frame_to_load", frame_to_load);
			_dest.put_member("frame_contents_page", frame_contents_page);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (!_src.has_members(missing, { "frame_contents_page", "frame_to_load" })) {
				system_monitoring_interface::global_mon->log_warning("select_frame_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}
	
			form_to_read = _src["form_to_read"];
			page_to_select = _src["page_to_select"];
			frame_to_load = _src["frame_to_load"];
			frame_contents_page = _src["frame_contents_page"];
		}
	};

	void get_json(json& _dest, std::shared_ptr<corona_bus_command>& _src)
	{
		json_parser jp;
		_dest = jp.create_object();
		_src->get_json(_dest);
	}

	void put_json(std::shared_ptr<corona_bus_command>& _dest, json _src)
	{
		std::vector<std::string> missing;

		if (_src.empty())
			return;

		if (!_src.has_members(missing, { "class_name" })) {
			system_monitoring_interface::global_mon->log_warning("command object missing class_name.");
			if (_src.size()) {
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
			}
			return;
		}

		if (_src.has_member("class_name"))
		{
			std::string class_name = _src["class_name"];

			if (class_name == "create_object_command")
			{
				_dest = std::make_shared<corona_create_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "select_object_command")
			{
				_dest = std::make_shared<corona_select_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "preview_object_command")
			{
				_dest = std::make_shared<corona_preview_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "save_object_command")
			{
				_dest = std::make_shared<corona_save_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "load_object_command")
			{
				_dest = std::make_shared<corona_load_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "delete_object_command")
			{
				_dest = std::make_shared<corona_delete_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "search_objects_command")
			{
				_dest = std::make_shared<corona_search_objects_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "select_page_command")
			{
				_dest = std::make_shared<corona_select_page_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "select_frame_command")
			{
				_dest = std::make_shared<corona_select_frame_command>();
				_dest->put_json(_src);
			}
		}
	}
}

#endif
