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

	class  corona_register_user_command : public corona_bus_command
	{
	public:

		std::string user_name_ctl;
		std::string email_ctl;
		std::string password1_ctl;
		std::string password2_ctl;

		corona_client_response response;

		virtual json execute()
		{
			json obj;

			auto cuser_name = bus->find_control(user_name_ctl);
			auto cemail_ctl = bus->find_control(email_ctl);
			auto cpassword1 = bus->find_control(password1_ctl);
			auto cpassword2 = bus->find_control(password2_ctl);

			if (cuser_name and cpassword1 and cpassword2) {
				std::string user_name = cuser_name->get_data();
				std::string email = cemail_ctl->get_data();
				std::string password1 = cpassword1->get_data();
				std::string password2 = cpassword2->get_data();
				response = bus->remote_register_user(user_name, email, password1, password2);
				obj = response.data;
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "register_user_command"sv);
			_dest.put_member("user_name_ctl", user_name_ctl);
			_dest.put_member("email_ctl", email_ctl);
			_dest.put_member("password1_ctl", password1_ctl);
			_dest.put_member("password2_ctl", password2_ctl);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "user_name_ctl", "email_ctl", "password1_ctl", "password2_ctl"})) {
				system_monitoring_interface::global_mon->log_warning("register_user_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			user_name_ctl = _src["user_name_ctl"];
			email_ctl = _src["email_ctl"];
			password1_ctl = _src["password1_ctl"];
			password2_ctl = _src["password2_ctl"];
		}

	};

	class  corona_send_user_command : public corona_bus_command
	{
	public:
		std::string user_name_ctl;
		corona_client_response response;

		virtual json execute()
		{
			json obj;

			auto cuser_name = bus->find_control(user_name_ctl);

			if (cuser_name) {
				std::string user_name = cuser_name->get_data();
				response = bus->remote_send_user(user_name_ctl);
				obj = response.data;
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "send_user_command"sv);
			_dest.put_member("user_name_ctl", user_name_ctl);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "user_name_ctl" })) {
				system_monitoring_interface::global_mon->log_warning("send_user_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			user_name_ctl = _src["user_name_ctl"];	
		}

	};

	class  corona_login_command : public corona_bus_command
	{
	public:
		std::string user_name_ctl;
		std::string user_password_ctl;
		std::shared_ptr<corona_bus_command> on_login_success;
		std::shared_ptr<corona_bus_command> on_login_fail;

		corona_client_response response;

		virtual json execute()
		{
			json obj;

			auto cuser_name = bus->find_control(user_name_ctl);
			auto cuser_password = bus->find_control(user_password_ctl);

			if (cuser_name and cuser_password) {
				std::string user_name = cuser_name->get_data();
				std::string password = cuser_password->get_data();
				response = bus->remote_login(user_name, password);

				if (response.success) {
					if (on_login_success) {
						on_login_success->execute();
					}
				}
				else if (on_login_fail)
				{
					on_login_fail->execute();
				}

				obj = response.data;
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "login_command"sv);
			_dest.put_member("username_ctl", user_name_ctl);
			_dest.put_member("password_ctl", user_password_ctl);

			json_parser jp;
			if (on_login_success) {
				json jon_login_success = jp.create_object();
				on_login_success->get_json(jon_login_success);
				_dest.put_member("on_login_success", jon_login_success);
			}
			if (on_login_fail) {
				json jon_login_fail = jp.create_object();
				on_login_fail->get_json(jon_login_fail);
				_dest.put_member("on_login_fail", jon_login_fail);
			}
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "username_ctl", "password_ctl" })) {
				system_monitoring_interface::global_mon->log_warning("login_command missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			user_name_ctl = _src["username_ctl"];
			user_password_ctl = _src["password_ctl"];

			json jon_login_success = _src["on_login_success"];
			if (jon_login_success.object()) {
				corona::put_json(on_login_success, jon_login_success);
			}
			json jon_login_fail = _src["on_login_fail"];
			if (jon_login_fail.object()) {
				corona::put_json(on_login_fail, jon_login_fail);
			}
		}

	};

	class  corona_get_classes_command : public corona_bus_command
	{
	public:
		std::string			table_name;
		corona_client_response response;

		virtual json execute()
		{
			json obj;
			json_parser jp;
			control_base* cb_table = {};

			if (not table_name.empty())
				cb_table = bus->find_control(table_name);

			if (not cb_table) {
				comm_bus_app_interface::global_bus->log_warning(std::format("{0} table for search command not found", table_name), __FILE__, __LINE__);
			}

			if (cb_table)
			{
				corona_client_response classes = bus->remote_get_classes();
				if (classes.success) {
					json results = jp.create_array();
					if (classes.data.array()) {
						for (auto cls : classes.data) {
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

			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "get_classes_command"sv);
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
		}
	};

	class  corona_get_class_command : public corona_bus_command
	{
	public:
		virtual json execute()
		{
			json obj;
			return obj;
		}
	};

	class  corona_put_class_command : public corona_bus_command
	{
	public:
		virtual json execute()
		{
			json obj;
			return obj;
		}
	};

	class  corona_set_password_command : public corona_bus_command
	{
	public:
		std::string user_name_ctl;
		std::string validation_code_ctl;
		std::string password1_ctl;
		std::string password2_ctl;
		corona_client_response response;

		virtual json execute()
		{
			json obj;

			auto cuser_name = bus->find_control(user_name_ctl);
			auto cvalidation_code_ctl = bus->find_control(validation_code_ctl);
			auto cpassword1 = bus->find_control(password1_ctl);
			auto cpassword2 = bus->find_control(password2_ctl);

			if (cuser_name and cpassword1 and cpassword2) {
				std::string user_name = cuser_name->get_data();
				std::string validation_code = cvalidation_code_ctl->get_data();
				std::string password1 = cpassword1->get_data();
				std::string password2 = cpassword2->get_data();
				response = bus->remote_set_password(user_name, validation_code, password1, password2);
				obj = response.data;
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "set_password_command"sv);
			_dest.put_member("user_name_ctl", user_name_ctl);
			_dest.put_member("validation_code_ctl", validation_code_ctl);
			_dest.put_member("password1_ctl", password1_ctl);
			_dest.put_member("password2_ctl", password2_ctl);
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

			user_name_ctl = _src["user_name_ctl"];
			validation_code_ctl = _src["validation_code_ctl"];
			password1_ctl = _src["password1_ctl"];
			password2_ctl = _src["password2_ctl"];
		}

	};

	class corona_create_object_command : public corona_bus_command
	{
	public:
		std::string	create_class_name;
		std::string form_name;
		corona_instance instance;
		corona_client_response response;

		virtual json execute()
		{
			json obj;
			control_base* cb = bus->find_control(form_name);
			if (cb) {
				obj =  bus->create_object(instance, create_class_name);
				cb->put_json(obj);
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "create_object_command"sv);
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
			form_name = _src["form_name"];
			instance = (corona_instance)((int64_t)_src["instance"]);
		}

	};

	class corona_create_object_frame_command : public corona_bus_command
	{
	public:
		std::string	create_class_name;
		std::string	page_to_select;
		std::string	frame_to_load;
		std::string	frame_contents_page;
		std::string	form_to_load;
		corona_instance instance;
		corona_client_response response;

		virtual json execute()
		{
			json obj;
			obj = bus->create_object(instance, create_class_name);
			if (not obj.empty()) {
				bus->select_page(page_to_select, frame_to_load, frame_contents_page, form_to_load, obj);
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "create_object_frame_command"sv);
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

	class corona_select_object_frame_command : public corona_bus_command
	{
	public:
		std::string		table_name;
		std::string		page_to_select;
		std::string		frame_to_load;
		std::string		frame_contents_page;
		std::string		form_to_load;
		corona_instance instance;

		virtual json execute()
		{
			json obj;
			control_base* cb = bus->find_control(table_name);
			if (cb) {
				json key_data = cb->get_selected_object();
				if (key_data.object()) {
					obj = bus->edit_object(instance, key_data);
					bus->select_page(page_to_select, frame_to_load, frame_contents_page, form_to_load, obj);
				}
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "select_object_frame_command"sv);
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

	class corona_select_object_page_command : public corona_bus_command
	{
	public:
		std::string	table_name;
		std::string page_name;
		std::string form_name;
		corona_instance instance;

		virtual json execute()
		{
			json obj;
			control_base* cb = bus->find_control(table_name);
			if (cb) {
				json key_data = cb->get_selected_object();
				if (key_data.object()) {
					obj =  bus->edit_object(instance, key_data);
					bus->select_page(page_name, form_name, obj);
				}
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "select_object_page_command"sv);
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

	class corona_preview_object_command : public corona_bus_command
	{
	public:
		std::string	table_name;
		std::string form_name;
		corona_instance instance;

		virtual json execute()
		{
			json obj;
			control_base* cb = bus->find_control(table_name);
			if (cb) {
				json key_data = cb->get_selected_object();
				if (key_data.object()) {
					obj =  bus->get_object(instance, key_data);
					control_base* cf = bus->find_control(form_name);
					if (cf) {
						cf->set_data(obj);
					}
				}
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "preview_object_command"sv);
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

	class corona_save_object_command : public corona_bus_command
	{
	public:

		std::string form_name;
		corona_instance instance;

		virtual json execute()
		{
			json obj;
			control_base* cb = nullptr;

			if (not form_name.empty())
				cb = bus->find_control(form_name);

			if (cb) {
				json object_data = cb->get_data();
				if (object_data.object()) {
					obj =  bus->put_object(instance, object_data);
				}
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "save_object_command"sv);
			_dest.put_member("form_name", form_name);
			_dest.put_member_i64("instance", (int64_t)instance);

		}

		virtual void put_json(json& _src)
		{
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

	class corona_load_object_command : public corona_bus_command
	{
	public:
		std::string control_name;
		json		object_data;
		corona_instance instance;

		virtual json execute()
		{
			json obj;
			control_base* cb = nullptr;
			
			cb = bus->find_control(control_name);

			if (cb) {
				if (object_data.object()) {
					obj =  bus->put_object(instance, object_data);
				}
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "load_object_command"sv);
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

	class corona_delete_object_command : public corona_bus_command
	{
	public:
		std::string		control_name;
		corona_instance instance;

		virtual json execute()
		{
			json obj;
			control_base* cb = bus->find_control(control_name);
			if (cb) {
				json object_data = cb->get_data();
				if (object_data.object()) {
					obj =  bus->delete_object(instance, object_data);
				}
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;
			_dest.put_member("class_name", "delete_object_command"sv);
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

	class corona_run_object_command : public corona_bus_command
	{
	public:
		std::string			search_class_name;
		std::string			form_name;
		std::string			table_name;
		query_context		qctx;
		corona_instance instance;

		virtual json execute()
		{
			json_parser jp;
			json obj;
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
					obj = bus->query_objects(instance, search_class_filters);
					qctx.set_data_source(form_name, object_data);
					qctx.set_data_source(search_class_name, obj);
					json results = qctx.run();
					if (cb_table) {
						cb_table->set_items(results);
					}
				}
			}

			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "search_objects_command"sv);
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

	class corona_search_objects_command : public corona_bus_command
	{
	public:
		std::string			search_class_name;
		std::string			form_name;
		std::string			table_name;
		query_context		qctx;
		corona_instance instance;

		virtual json execute()
		{
			json_parser jp;
			json obj;
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
					search_class_filters.put_member("Filter", object_data);
					obj =  bus->query_objects(instance, search_class_filters);
					qctx.set_data_source(form_name, object_data);
					qctx.set_data_source(search_class_name, obj);
					json results = qctx.run();
					if (cb_table) {
						cb_table->set_items(results);
					}
				}
			}

			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "search_objects_command"sv);
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

	class corona_select_page_command : public corona_bus_command
	{
	public:
		std::string		page_name;
		std::string		form_name;
		std::string		source_name;
		corona_instance instance;

		virtual json execute()
		{
			json obj;
			control_base* cb = {};
			if (not source_name.empty())
				cb = bus->find_control(source_name);
			json data;
			if (cb) {
				data = cb->get_data();
			}
			bus->select_page(page_name, form_name, data);
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "select_page_command"sv);
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

	class corona_script_command: public corona_bus_command
	{
	public:
		std::string		control_name;
		std::vector<std::shared_ptr<corona_bus_command>> commands;

		virtual json execute()
		{
			json obj;
			for (auto comm : commands) {
				obj = comm->execute();
			}
			return obj;
		}

		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "script_command"sv);
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

	class corona_set_text_command : public corona_bus_command
	{
	public:
		std::string		control_name;
		std::string		text_to_set;

		// this is defined in corona-presentation-builder.  
		// should have done this more cleanly with interfaces, but
		// this gets the job done.

		virtual json execute();
		virtual void get_json(json& _dest)
		{
			using namespace std::literals;

			_dest.put_member("class_name", "set_text_command"sv);
			_dest.put_member("control_name", control_name);
			_dest.put_member("text", text_to_set);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, { "control_name", "text" })) {
				system_monitoring_interface::global_mon->log_warning("corona_set_text_command missing:");

				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			control_name = _src["control_name"];
			text_to_set = _src["text"];
		}
	};

	class corona_select_frame_command : public corona_bus_command
	{
	public:
		std::string		form_to_read;
		std::string		page_to_select;
		std::string		frame_to_load;
		std::string		frame_contents_page;
		corona_instance instance;

		virtual json execute()
		{
			json obj;
			control_base* cb = {};
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

			_dest.put_member("class_name", "select_frame_command"sv);
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

			if (class_name == "script_command")
			{
				_dest = std::make_shared<corona_script_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "set_text_command")
			{
				_dest = std::make_shared<corona_set_text_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "register_user_command")
			{
				_dest = std::make_shared<corona_register_user_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "send_user_command")
			{
				_dest = std::make_shared<corona_send_user_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "login_command")
			{
				_dest = std::make_shared<corona_login_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "get_classes_command")
			{
				_dest = std::make_shared<corona_get_classes_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "get_class_command")
			{
				_dest = std::make_shared<corona_get_class_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "put_class_command")
			{
				_dest = std::make_shared<corona_put_class_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "set_password_command")
			{
				_dest = std::make_shared<corona_set_password_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "create_object_command")
			{
				_dest = std::make_shared<corona_create_object_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "create_object_frame_command")
			{
				_dest = std::make_shared<corona_create_object_frame_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "select_object_page_command")
			{
				_dest = std::make_shared<corona_select_object_page_command>();
				_dest->put_json(_src);
			}
			else if (class_name == "select_object_frame_command")
			{
				_dest = std::make_shared<corona_select_object_frame_command>();
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
