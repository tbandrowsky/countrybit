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
			_dest.put_member("class_name", "select_object_command");
			_dest.put_member("table_name", table_name);
			_dest.put_member("form_name", form_name);
		}

		virtual void put_json(json& _src)
		{
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
			control_name = _src["control_name`"];
		}

	};

	class corona_search_objects_command : public corona_bus_command
	{
	public:
		std::string			search_class_name;
		std::string			form_name;
		std::string			table_name;

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

			if (cb_form && cb_table) {
				json search_class_filters = jp.create_object();
				search_class_filters.put_member("ClassName", search_class_name);
				json object_data = cb_form->get_data();
				if (object_data.object()) {
					search_class_filters.put_member("Filter", object_data);
					obj = co_await bus->query_objects(search_class_filters);
					if (cb_table) {
						cb_table->set_items(obj);
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
		}

		virtual void put_json(json& _src)
		{
			form_name = _src["form_name"];
			table_name = _src["table_name"];
			search_class_name = _src["search_class_name"];
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
			page_name = _src["page_name"];
			form_name = _src["form_name"];
			source_name = _src["source_name"];
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
		}
	}
}

#endif
