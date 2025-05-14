#pragma once

/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
This is the application commands. commands can be sprinkled throughout the ui
to provide a declarative way to handle certain ui use cases. generally 
attached to buttons but can be wired up to just about anything.
conceptually similar to winui commands, but a tad more flexible.
thread / ui thread syncing handled for you.  most of the time.

Notes

For Future Consideration
*/

#ifndef CORONA_COMM_BUS_COMMANDS_H
#define CORONA_COMM_BUS_COMMANDS_H

namespace corona
{

	void put_json(std::shared_ptr<corona_bus_command>& _dest, json _src);




	class corona_form_command : public corona_bus_command
	{
	public:
		std::string form_name;

		corona_client_response response;

		std::shared_ptr<corona_bus_command> on_start;
		std::shared_ptr<corona_bus_command> on_success;
		std::shared_ptr<corona_bus_command> on_fail;

		corona_form_command() 
		{
			;
		}

		virtual corona_client_response invoke(json _form_data, comm_bus_app_interface* bus)
		{
			return response;
		}

		virtual corona_client_response& set_message(corona_client_response& _src, comm_bus_app_interface* _bus);

		virtual json execute(json context, comm_bus_app_interface* bus)
		{
			json obj = bus->get_form_data(form_name);

			if (obj.object()) {
                if (on_start) {
                    on_start->execute(context, bus);
                }	
				auto response = invoke(obj, bus);
				set_message(response, bus);
				if (response.success) {
					if (on_success) {
						context.put_member("value", response.message);
						on_success->execute(context, bus);
					}
					obj = response.data;
				}
				else if (on_fail)
				{
					context.put_member("value", response.message);
					on_fail->execute(context, bus);
				}
			}
			else {
				log_warning("Could not find form '" + form_name + "'");
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			json_parser jp;
			if (on_start) {
				json jon_start = jp.create_object();
				on_start->get_json(jon_start);
				_dest.put_member("on_start", jon_start);
			}
			if (on_success) {
				json jon_login_success = jp.create_object();
				on_success->get_json(jon_login_success);
				_dest.put_member("on_success", jon_login_success);
			}
			if (on_fail) {
				json jon_login_fail = jp.create_object();
				on_fail->get_json(jon_login_fail);
				_dest.put_member("on_fail", jon_login_fail);
			}
			_dest.put_member("form_name", form_name);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, {  })) {
				system_monitoring_interface::global_mon->log_warning("corona_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			on_start = nullptr;

			json jon_start = _src["on_start"];
			if (jon_start.object()) {
				corona::put_json(on_start, jon_start);
			}
			json jon_login_success = _src["on_success"];
			if (jon_login_success.object()) {
				corona::put_json(on_success, jon_login_success);
			}
			json jon_login_fail = _src["on_fail"];
			if (jon_login_fail.object()) {
				corona::put_json(on_fail, jon_login_fail);
			}
			form_name = _src["form_name"];
		}


	};

	class  corona_register_user_command : public corona_form_command
	{
	public:

		std::string user_name_field = "";
		std::string email_field = "";
		std::string password1_field = "";
		std::string password2_field = "";
		std::string first_name_field = "";
		std::string last_name_field = "";
		std::string street_field = "";
		std::string city_field = "";
		std::string state_field = "";
		std::string zip_field = "";
		std::string phone_field = "";

		corona_register_user_command()
		{
			;
		}


		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			json_parser jp;
			json user_obj = jp.create_object();

			user_obj.put_member("user_name", (std::string)obj[user_name_field]);
			user_obj.put_member("email", (std::string)obj[email_field]);
			user_obj.put_member("password1", (std::string)obj[password1_field]);
			user_obj.put_member("password2", (std::string)obj[password2_field]);
			user_obj.put_member("first_name", (std::string)obj[first_name_field]);
			user_obj.put_member("street", (std::string)obj[street_field]);
			user_obj.put_member("city", (std::string)obj[city_field]);
			user_obj.put_member("state", (std::string)obj[state_field]);
			user_obj.put_member("phone", (std::string)obj[phone_field]);
			response = bus->remote_register_user(user_obj);
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			corona_form_command::get_json(_dest);

			_dest.put_member("class_name", "register_user"sv);
			_dest.put_member("user_name_field", user_name_field);
			_dest.put_member("email_field", email_field);
			_dest.put_member("password1_field", password1_field);
			_dest.put_member("password2_field", password2_field);
			_dest.put_member("first_name_field", first_name_field);
			_dest.put_member("last_name_field", last_name_field);
			_dest.put_member("street_field", street_field);
			_dest.put_member("zip_field", zip_field);
			_dest.put_member("city_field", city_field);
			_dest.put_member("phone_field", phone_field);
		}

		virtual void put_json(json& _src)
		{
			corona_form_command::put_json(_src);

			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "user_name_field", "email_field", "password1_field", "password2_field"})) {
				system_monitoring_interface::global_mon->log_warning("register_user_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			user_name_field = _src["user_name_field"];
			email_field = _src["email_field"];
			password1_field = _src["password1_field"];
			password2_field = _src["password2_field"];
			first_name_field = _src["first_name_field"];
			last_name_field = _src["last_name_field"];
			street_field = _src["password2_field"];
			city_field = _src["password2_field"];

		}

	};

	class  corona_confirm_user_command : public corona_form_command
	{
	public:

		std::string user_name_field = "";
		std::string validation_code_field = "";

		corona_confirm_user_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			std::string user_name = obj[user_name_field];
			std::string validation_field = obj[validation_code_field];
			response = bus->remote_confirm_user(user_name, validation_field);
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

            corona_form_command::get_json(_dest);

			_dest.put_member("class_name", "confirm_user"sv);
			_dest.put_member("user_name_field", user_name_field);
			_dest.put_member("validation_code_field", validation_code_field);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

            corona_form_command::put_json(_src);

			if (not _src.has_members(missing, { "user_name_field", "validation_code_field" })) {
				system_monitoring_interface::global_mon->log_warning("confirm_user_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			user_name_field = _src["user_name_field"];
			validation_code_field = _src["validation_code"];
		}

	};

	class  corona_send_user_command : public corona_form_command
	{
	public:

		std::string user_name_field = "";

		corona_send_user_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			std::string user_name = obj[user_name_field];
			response = bus->remote_send_user(user_name);
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			corona_form_command::get_json(_dest);

			_dest.put_member("class_name", "send_user"sv);
			_dest.put_member("user_name_field", user_name_field);
		}

		virtual void put_json(json& _src)
		{

			corona_form_command::put_json(_src);

			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "user_name_field"})) {
				system_monitoring_interface::global_mon->log_warning("send_user_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			user_name_field = _src["user_name_field"];	
		}

	};

	class  corona_password_user_command : public corona_form_command
	{
	public:

		std::string user_name_field = "";
        std::string validation_code_field = "";
		std::string password1_field = "";
		std::string password2_field = "";

		corona_password_user_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			std::string user_name = obj[user_name_field];
			std::string validation_code = obj[validation_code_field];
			std::string password1 = obj[password1_field];
			std::string password2= obj[password2_field];
			response = bus->remote_set_password(user_name, validation_code, password1, password2);
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			corona_form_command::get_json(_dest);

			_dest.put_member("class_name", "password_user"sv);
			_dest.put_member("user_name_field", user_name_field);
			_dest.put_member("validation_code_field", validation_code_field);
			_dest.put_member("password1_field", password1_field);
			_dest.put_member("password2_field", password2_field);
		}

		virtual void put_json(json& _src)
		{

			corona_form_command::put_json(_src);

			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "user_name_field", "validation_code_field", "password1_field", "password2_field"})) {
				system_monitoring_interface::global_mon->log_warning("set_user_password missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			user_name_field = _src["user_name_field"];
			validation_code_field = _src["validation_code_field"];
			password1_field = _src["password1_field"];
			password2_field = _src["password2_field"];
		}

	};

	class  corona_login_command : public corona_form_command
	{
	public:

		std::string user_name_field = "";
		std::string user_password_field = "";

		corona_login_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			std::string user_name = obj[user_name_field];
			std::string password = obj[user_password_field];
			response = bus->remote_login(user_name, password);
			return response;
		}


		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			corona_form_command::get_json(_dest);

			_dest.put_member("class_name", "login_user"sv);
			_dest.put_member("user_name_field", user_name_field);
			_dest.put_member("user_password_field", user_password_field);

		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			corona_form_command::put_json(_src);

			if (not _src.has_members(missing, { "form_name", "user_name_field", "user_password_field" })) {
				system_monitoring_interface::global_mon->log_warning("login_user missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			user_name_field = _src["user_name_field"];
			user_password_field = _src["user_password_field"];

		}

	};

	class  corona_get_classes_command : public corona_form_command
	{
	public:
		std::string			table_name = "";

		corona_get_classes_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			json_parser jp;
			control_base* cb_table = {};

			if (not table_name.empty())
				cb_table = bus->find_control(table_name);

			if (not cb_table) {
				comm_bus_app_interface::global_bus->log_warning(std::format("{0} table for search command not found", table_name), __FILE__, __LINE__);
			}

			if (cb_table)
			{
				response = bus->remote_get_classes();
				if (response.success) {
					json results = jp.create_array();
					if (response.data.array()) {
						for (auto cls : response.data) {
							json item = jp.create_object();
							std::string cls_name = cls[class_name_field];
							std::string cls_description = cls["class_description"];
							std::string cls_base = cls[base_class_name_field];
							item.put_member(base_class_name_field, cls_base);
							item.put_member(class_name_field, cls_name);
							item.put_member("class_description", cls_description);
							results.push_back(item);
						}
					}
					cb_table->set_items(results);
				}
			}

			return response;
		}


		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "get_classes"sv);
			_dest.put_member("table_name", table_name);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { })) {
				system_monitoring_interface::global_mon->log_warning("get_classes_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			table_name = _src["table_name"];
		}
	};

	class  corona_get_class_command : public corona_form_command
	{
	public:

		corona_get_class_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			return response;
		}
	};

	class  corona_put_class_command : public corona_form_command
	{
	public:

		corona_put_class_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			return response;
		}
	};

	class  corona_set_password_command : public corona_form_command
	{
	public:
		std::string user_name_field = "";
		std::string validation_code_field = "";
		std::string password1_field = "";
		std::string password2_field = "";

		corona_set_password_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			std::string user_name = obj[user_name_field];
			std::string validation_code = obj[validation_code_field];
			std::string password1 = obj[password1_field];
			std::string password2 = obj[password2_field];
			response = bus->remote_set_password(user_name, validation_code, password1, password2);
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "set_password"sv);
			_dest.put_member("user_name_field", user_name_field);
			_dest.put_member("validation_code_field", validation_code_field);
			_dest.put_member("password1_field", password1_field);
			_dest.put_member("password2_field", password2_field);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "user_name_ctl", "validation_code_ctl", "password1_ctl", "password2_ctl" })) {
				system_monitoring_interface::global_mon->log_warning("set_password_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			user_name_field = _src["user_name_field"];
			validation_code_field = _src["validation_code_field"];
			password1_field = _src["password1_field"];
			password2_field = _src["password2_field"];
		}

	};

	class corona_create_object_command : public corona_form_command
	{
	public:
		std::string	create_class_name = "";
		corona_instance instance;

		corona_create_object_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			response = bus->create_object(instance, create_class_name);
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "create_object"sv);
			_dest.put_member("create_class_name", create_class_name);
			_dest.put_member("form_name", form_name);

			_dest.put_member_i64("instance", (int64_t)instance);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "create_class_name", "form_name" })) {
				system_monitoring_interface::global_mon->log_warning("create_object_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			create_class_name = _src["create_class_name"];
			instance = (corona_instance)((int64_t)_src["instance"]);
		}

	};

	class corona_create_object_frame_command : public corona_form_command
	{
	public:
		std::string	create_class_name = "";
		std::string	page_to_select = "";
		std::string	frame_to_load = "";
		std::string	frame_contents_page = "";
		std::string	form_to_load = "";
		corona_instance instance;
		corona_client_response response;


		corona_create_object_frame_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			response = bus->create_object(instance, create_class_name);
			if (response.success and response.data.object()) {
				bus->select_page(page_to_select, frame_to_load, frame_contents_page, form_to_load, response.data);
			}
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "create_object_frame"sv);
			_dest.put_member("create_class_name", create_class_name);
			_dest.put_member("page_to_select", page_to_select);
			_dest.put_member("frame_to_load", frame_to_load);
			_dest.put_member("frame_contents_page", frame_contents_page);
			_dest.put_member("form_to_load", form_to_load);

			_dest.put_member_i64("instance", (int64_t)instance);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "create_class_name", "form_name" })) {
				system_monitoring_interface::global_mon->log_warning("create_object_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			create_class_name = _src["create_class_name"];
			instance = (corona_instance)((int64_t)_src["instance"]);
			page_to_select = _src["page_to_select"];
			frame_to_load = _src["frame_to_load"];
			frame_contents_page = _src["frame_contents_page"];
			form_to_load = _src["form_to_load"];

		}

	};

	class corona_select_object_frame_command : public corona_form_command
	{
	public:
		std::string		table_name = "";
		std::string		page_to_select = "";
		std::string		frame_to_load = "";
		std::string		frame_contents_page = "";
		std::string		form_to_load = "";
		corona_instance instance;

		corona_select_object_frame_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			control_base* cb = bus->find_control(table_name);
			if (cb) {
				json key_data = cb->get_selected_object();
				if (key_data.object()) {
					response = bus->edit_object(instance, key_data);
                    if (response.success) {
                        bus->select_page(page_to_select, frame_to_load, frame_contents_page, form_to_load, response.data);
                    }
				}
			}
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "select_object_frame"sv);
			_dest.put_member("table_name", table_name);
			_dest.put_member("page_to_select", page_to_select);
			_dest.put_member("frame_to_load", frame_to_load);
			_dest.put_member("frame_contents_page", frame_contents_page);
			_dest.put_member("form_to_load", form_to_load);
			_dest.put_member_i64("instance", (int64_t)instance);

		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "frame_contents_page", "frame_to_load" })) {
				system_monitoring_interface::global_mon->log_warning("select_object_frame_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			table_name = _src["table_name"];
			page_to_select = _src["page_to_select"];
			frame_to_load = _src["frame_to_load"];
			frame_contents_page = _src["frame_contents_page"];
			form_to_load = _src["form_to_load"];
			instance = (corona_instance)((int64_t)_src["instance"]);
		}
	};

	class corona_select_object_page_command : public corona_form_command
	{
	public:
		std::string	table_name = "";
		std::string page_name = "";
		corona_instance instance;

		corona_select_object_page_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			control_base* cb = bus->find_control(table_name);
			if (cb) {
				json key_data = cb->get_selected_object();
				if (key_data.object()) {
					response = bus->edit_object(instance, key_data);
					if (response.success) {
						bus->select_page(page_name, form_name, response.data);
					}
				}
			}
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "select_object_page"sv);
			_dest.put_member("table_name", table_name);
			_dest.put_member("form_name", form_name);
			_dest.put_member("page_name", page_name);
			_dest.put_member_i64("instance", (int64_t)instance);

		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "table_name", "form_name", "page_name"})) {
				system_monitoring_interface::global_mon->log_warning("select_object_page_command missing:");
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
			instance = (corona_instance)((int64_t)_src["instance"]);
		}

	};

	class corona_preview_object_command : public corona_form_command
	{
	public:
		std::string	table_name = "";
		corona_instance instance;

		corona_preview_object_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			control_base* cb = bus->find_control(table_name);
			if (cb) {
				json key_data = cb->get_selected_object();
				if (key_data.object()) {
					response = bus->get_object(instance, key_data);
					if (response.success) {
						control_base* cf = bus->find_control(form_name);
						if (cf) {
							cf->set_data(response.data);
						}
					}
				}
			}
			return response;
		}


		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "preview_object"sv);
			_dest.put_member("table_name", table_name);
			_dest.put_member("form_name", form_name);
			_dest.put_member_i64("instance", (int64_t)instance);

		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "table_name", "form_name" })) {
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
			instance = (corona_instance)((int64_t)_src["instance"]);
		}

	};

	class corona_save_object_command : public corona_form_command
	{
	public:

		corona_instance instance;

		corona_save_object_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override 
		{
			control_base* cb = nullptr;

			if (not form_name.empty())
				cb = bus->find_control(form_name);

			if (cb) {
				json object_data = cb->get_data();
				if (object_data.object()) {
					response =  bus->put_object(instance, object_data);
				}
			}
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			corona_form_command::get_json(_dest);

			_dest.put_member("class_name", "save_object"sv);
			_dest.put_member("form_name", form_name);
			_dest.put_member_i64("instance", (int64_t)instance);

		}

		virtual void put_json(json& _src)
		{

			corona_form_command::put_json(_src);

			form_name = _src["form_name"];
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "form_name" })) {
				system_monitoring_interface::global_mon->log_warning("save_object_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}
			instance = (corona_instance)((int64_t)_src["instance"]);
		}

	};

	class corona_load_object_command : public corona_form_command
	{
	public:
		std::string control_name = "";
		json		object_data;
		corona_instance instance;

		corona_load_object_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override 
		{
			control_base* cb = nullptr;
			
			cb = bus->find_control(control_name);

			if (cb) {
				if (object_data.object()) {
					response =  bus->put_object(instance, object_data);
				}
			}
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "load_object"sv);
			_dest.put_member("control_name", control_name);
			_dest.put_member("data", object_data);
			_dest.put_member_i64("instance", (int64_t)instance);

		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "control_name", "data" })) {
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
			instance = (corona_instance)((int64_t)_src["instance"]);
		}

	};

	class corona_delete_object_command : public corona_form_command
	{
	public:
		std::string		control_name = "";
		corona_instance instance;

		corona_delete_object_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override 
		{
			control_base* cb = bus->find_control(control_name);
			if (cb) {
				json obj_data = cb->get_data();
				if (obj_data.object()) {
					response =  bus->delete_object(instance, obj_data);
				}
			}
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;
			_dest.put_member("class_name", "delete_object"sv);
			_dest.put_member("control_name", control_name);
			_dest.put_member_i64("instance", (int64_t)instance);

		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "class_name", "control_name" })) {
				system_monitoring_interface::global_mon->log_warning("delete_object_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			control_name = _src["control_name"];
			instance = (corona_instance)((int64_t)_src["instance"]);
		}

	};

	class corona_run_object_command : public corona_form_command
	{
	public:
		std::string			search_class_name = "";
		std::string			form_name = "";
		std::string			table_name = "";
		query_context		qctx;
		corona_instance instance;

		corona_run_object_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			json_parser jp;
			control_base* cb_form = {};
			control_base* cb_table = {};

			if (not form_name.empty())
				cb_form = bus->find_control(form_name);

			if (not table_name.empty())
				cb_table = bus->find_control(table_name);

			if (not cb_form) {
				comm_bus_app_interface::global_bus->log_warning(std::format("{0} form for run command not found", form_name), __FILE__, __LINE__);
			}

			if (not cb_table) {
				comm_bus_app_interface::global_bus->log_warning(std::format("{0} table for run command not found", table_name), __FILE__, __LINE__);
			}
			if (cb_form and cb_table)
			{
				json search_class_filters = jp.create_object();
				search_class_filters.put_member("class_name", search_class_name);
				json object_data = cb_form->get_data();
				if (object_data.object()) {
					json search_class = jp.create_object();
					search_class.put_member("class_name", search_class_name);
					search_class_filters.put_member("filter", object_data);
					response = bus->query_objects(instance, search_class_filters);
					qctx.set_data_source(form_name, object_data);
					qctx.set_data_source(search_class_name, obj);
					json results = qctx.run();
					if (cb_table) {
						cb_table->set_items(results);
					}
				}
			}

			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "search_objects"sv);
			_dest.put_member("search_class_name", search_class_name);
			_dest.put_member("form_name", form_name);
			_dest.put_member("table_name", table_name);
			json_parser jp;
			json jctx = jp.create_object();
			qctx.get_json(jctx);
			_dest.put_member("query", jctx);
			_dest.put_member_i64("instance", (int64_t)instance);

		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, { "form_name", "table_name", "search_class_name" })) {
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
			instance = (corona_instance)((int64_t)_src["instance"]);
		}

	};

	class corona_search_objects_command : public corona_form_command
	{
	public:
		std::string			search_class_name = "";
		std::string			form_name = "";
		std::string			table_name = "";
		query_context		qctx;
		corona_instance instance;

		corona_search_objects_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			json_parser jp;
			control_base* cb_form = {};
			control_base* cb_table = {};

			if (not form_name.empty())
				cb_form = bus->find_control(form_name);

			if (not table_name.empty())
				cb_table = bus->find_control(table_name);

			if (not cb_form) {
				comm_bus_app_interface::global_bus->log_warning(std::format("{0} form for search command not found", form_name), __FILE__, __LINE__);
			}

			if (not cb_table) {
				comm_bus_app_interface::global_bus->log_warning(std::format("{0} table for search command not found", table_name), __FILE__, __LINE__);
			}

			if (cb_form and cb_table) 
			{
				json search_class_filters = jp.create_object();
				search_class_filters.put_member("class_name", search_class_name);
				json object_data = cb_form->get_data();
				if (object_data.object()) {
					json search_class = jp.create_object();
					search_class.put_member("class_name", search_class_name);
					search_class_filters.put_member("filter", object_data);
					response =  bus->query_objects(instance, search_class_filters);
					qctx.set_data_source(form_name, object_data);
					qctx.set_data_source(search_class_name, response.data);
					json results = qctx.run();
					if (cb_table) {
						cb_table->set_items(results);
					}
				}
			}

			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "search_objects"sv);
			_dest.put_member("search_class_name", search_class_name);
			_dest.put_member("form_name", form_name);
			_dest.put_member("table_name", table_name);
			json_parser jp;
			json jctx = jp.create_object();
			qctx.get_json(jctx);
			_dest.put_member("query", jctx);
			_dest.put_member_i64("instance", (int64_t)instance);

		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, { "form_name", "table_name", "search_class_name"})) {
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
			instance = (corona_instance)((int64_t)_src["instance"]);
		}

	};

	class corona_select_page_command : public corona_form_command
	{
	public:
		std::string		page_name;
		std::string		form_name;
		std::string		source_name;
		corona_instance instance;

		corona_select_page_command()
		{
			;
		}

		virtual corona_client_response invoke(json obj, comm_bus_app_interface* bus) override
		{
			control_base* cb = {};
			if (not source_name.empty())
				cb = bus->find_control(source_name);
			json data;
			if (cb) {
				data = cb->get_data();
			}
			bus->select_page(page_name, form_name, data);
			response.data = data;
			response.success = true;
			return response;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "select_page"sv);
			_dest.put_member("page_name", page_name);
			_dest.put_member("form_name", form_name);
			_dest.put_member("source_name", source_name);
			_dest.put_member_i64("instance", (int64_t)instance);

		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, { "page_name" })) {
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
				if (not _src.has_members(missing, { "form_name", "source_name" })) {
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
			instance = (corona_instance)((int64_t)_src["instance"]);

		}
	};

	class corona_script_command: public corona_form_command
	{
	public:
		std::string		control_name = "";
		std::vector<std::shared_ptr<corona_bus_command>> commands;

		corona_script_command()
		{
			;
		}

		virtual json execute(json context,  comm_bus_app_interface* bus)
		{
			json obj;
			for (auto comm : commands) {
				obj = comm->execute(context, bus);
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "script"sv);
			json_parser jp;
			json jcommand_array = jp.create_array();

			for (auto comm : commands)
			{
				json jcomm = jp.create_object();
				comm->get_json(jcomm);
				jcommand_array.push_back(jcomm);
			}

			_dest.put_member("control_name", control_name);
			_dest.put_member("commands", jcommand_array);

		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, { "control_name", "commands" })) {
				system_monitoring_interface::global_mon->log_warning("script missing:");

				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			control_name = _src["control_name"];
			commands.clear();

			json_parser jp;
			json jcommand_array = _src["commands"];
			if (jcommand_array.array()) {
				for (auto jcomm : jcommand_array)
				{
					std::shared_ptr<corona_bus_command> comm;
					corona::put_json(comm, jcomm);
					if (not comm)
						break;
					commands.push_back(comm);
				}
			}
		}
	};

	class corona_set_property_command : public corona_form_command
	{
	public:
		std::string		control_name;
		std::string		property_name;
		std::string		value;

		// this is defined in corona-presentation-builder.  
		// should have done this more cleanly with interfaces, but
		// this gets the job done.

		corona_set_property_command()
		{
			;
		}

		virtual json execute(json context,  comm_bus_app_interface* bus);
		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "set_property"sv);
			_dest.put_member("control_name", control_name);
			_dest.put_member("property_name", property_name);
			_dest.put_member("value", value);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, { "control_name", "property_name"})) {
				system_monitoring_interface::global_mon->log_warning("set_property missing:");

				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			control_name = _src["control_name"];
			property_name = _src["property_name"];
			value = _src["value"];
		}
	};

	class corona_select_frame_command : public corona_form_command
	{
	public:
		std::string		form_to_read;
		std::string		page_to_select;
		std::string		frame_to_load;
		std::string		frame_contents_page;
		corona_instance instance;

		corona_select_frame_command()
		{
			;
		}

		virtual json execute(json context,  comm_bus_app_interface* bus)
		{
			json obj;
			control_base* cb = {};
			// if form_to_read isn't empty
			// then 
			if (not form_to_read.empty())
				cb = bus->find_control(form_to_read);
			json data;
			if (cb) {
				data = cb->get_data();
			}
			std::string empty_form;
			bus->select_page(page_to_select, frame_to_load, frame_contents_page, empty_form, data);
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "select_frame"sv);
			_dest.put_member("form_to_read", form_to_read);
			_dest.put_member("page_to_select", page_to_select);
			_dest.put_member("frame_to_load", frame_to_load);
			_dest.put_member("frame_contents_page", frame_contents_page);
			_dest.put_member_i64("instance", (int64_t)instance);

		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "frame_contents_page", "frame_to_load" })) {
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
			instance = (corona_instance)((int64_t)_src["instance"]);
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

		if (not _src.has_members(missing, { "class_name" })) {
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

			if (class_name == "script")
			{
				_dest = std::make_shared<corona_script_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "set_property")
			{
				_dest = std::make_shared<corona_set_property_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "register_user")
			{
				_dest = std::make_shared<corona_register_user_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "send_user")
			{
				_dest = std::make_shared<corona_send_user_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "password_user")
			{
				_dest = std::make_shared<corona_password_user_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "confirm_user")
			{
				_dest = std::make_shared<corona_confirm_user_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "login_user")
			{
				_dest = std::make_shared<corona_login_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "get_classes")
			{
				_dest = std::make_shared<corona_get_classes_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "get_class")
			{
				_dest = std::make_shared<corona_get_class_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "put_class")
			{
				_dest = std::make_shared<corona_put_class_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "set_password")
			{
				_dest = std::make_shared<corona_set_password_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "create_object")
			{
				_dest = std::make_shared<corona_create_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "create_object_frame")
			{
				_dest = std::make_shared<corona_create_object_frame_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "select_object_page")
			{
				_dest = std::make_shared<corona_select_object_page_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "select_object_frame")
			{
				_dest = std::make_shared<corona_select_object_frame_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "preview_object")
			{
				_dest = std::make_shared<corona_preview_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "save_object")
			{
				_dest = std::make_shared<corona_save_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "load_object")
			{
				_dest = std::make_shared<corona_load_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "delete_object")
			{
				_dest = std::make_shared<corona_delete_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "search_objects")
			{
				_dest = std::make_shared<corona_search_objects_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "select_page")
			{
				_dest = std::make_shared<corona_select_page_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "select_frame")
			{
				_dest = std::make_shared<corona_select_frame_command>();
				_dest->put_json(_src);
			}
		}
	}

	corona_client_response& corona_form_command::set_message(corona_client_response& _src, comm_bus_app_interface* _bus)
	{
		auto dest = std::make_shared<corona_set_property_command>();

		dest->control_name = "status_message";
		dest->property_name = "text";
		dest->value = _src.message;
		dest->execute(_src.data, _bus);

		return _src;
	}

}

#endif
