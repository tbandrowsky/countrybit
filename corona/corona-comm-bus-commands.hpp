#pragma once

#ifndef CORONA_COMM_BUS_COMMANDS_H
#define CORONA_COMM_BUS_COMMANDS_H

namespace corona
{

	class corona_select_object_command : public corona_bus_command
	{
	public:
		int			table_control_id;
		int			target_control_id;
		std::string page_name;

		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = bus->find_control(table_control_id);
			if (cb) {
				json key_data = cb->get_selected_object();
				if (key_data.is_object()) {
					obj = co_await bus->get_object(key_data);
					bus->select_page(page_name, target_control_id, obj);
				}
			}
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("table_control_id", table_control_id);
			_dest.put_member("target_control_id", target_control_id);
			_dest.put_member("page_name", page_name);
		}

		virtual void put_json(json& _src)
		{
			table_control_id = _src["table_control_id"];
			target_control_id = _src["target_control_id"];
			page_name = _src["page_name"];
		}

	};

	class corona_save_object_command : public corona_bus_command
	{
	public:

		int			form_control_id;

		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = bus->find_control(form_control_id);
			if (cb) {
				json object_data = cb->get_data();
				if (object_data.is_object()) {
					obj = co_await bus->put_object(object_data);
				}
			}
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("form_control_id", form_control_id);
		}

		virtual void put_json(json& _src)
		{
			form_control_id = _src["form_control_id"];
		}

	};

	class corona_load_object_command : public corona_bus_command
	{
	public:
		int			form_control_id;
		json		object_data;

		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = bus->find_control(form_control_id);
			if (cb) {
				if (object_data.is_object()) {
					obj = co_await bus->put_object(object_data);
				}
			}
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("form_control_id", form_control_id);
			_dest.put_member("data", object_data);
		}

		virtual void put_json(json& _src)
		{
			form_control_id = _src["form_control_id"];
			object_data = _src["data"];
		}

	};

	class corona_delete_object_command : public corona_bus_command
	{
	public:
		int			form_control_id;
		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = bus->find_control(form_control_id);
			if (cb) {
				json object_data = cb->get_data();
				if (object_data.is_object()) {
					obj = co_await bus->delete_object(object_data);
				}
			}
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("form_control_id", form_control_id);
		}

		virtual void put_json(json& _src)
		{
			form_control_id = _src["form_control_id"];
		}

	};

	class corona_search_objects_command : public corona_bus_command
	{
	public:
		int			form_control_id;
		int			table_control_id;
		virtual comm_bus_transaction<json> execute()
		{
			json obj;
			control_base* cb = bus->find_control(form_control_id);
			if (cb) {
				json object_data = cb->get_data();
				if (object_data.is_object()) {
					obj = co_await bus->query_objects(object_data);
					control_base* cbt = bus->find_control(table_control_id);
					if (cbt) {
						cbt->set_data(object_data);
					}
				}
			}
			co_return obj;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("form_control_id", form_control_id);
			_dest.put_member("table_control_id", table_control_id);
		}

		virtual void put_json(json& _src)
		{
			form_control_id = _src["form_control_id"];
			table_control_id = _src["table_control_id"];
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
		if (_src.has_member("ClassName"))
		{
			std::string class_name = _src["ClassName"];
			if (class_name == "SelectObjectCommand")
			{
				_dest = std::make_shared<corona_select_object_command>();
			}
			else if (class_name == "SaveObjectCommand")
			{
				_dest = std::make_shared<corona_save_object_command>();
			}
			else if (class_name == "LoadObjectCommand")
			{
				_dest = std::make_shared<corona_load_object_command>();
			}
			else if (class_name == "DeleteObjectCommand")
			{
				_dest = std::make_shared<corona_delete_object_command>();
			}
			else if (class_name == "SearchObjectsCommand")
			{
				_dest = std::make_shared<corona_search_objects_command>();
			}
		}
	}

}

#endif

