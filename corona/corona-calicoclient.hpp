#ifndef CORONA_CALICOCLIENT_H
#define CORONA_CALICOCLIENT_H

#include "corona-httpclient.hpp"
#include "corona-function.hpp"
#include "corona-json.hpp"

namespace corona
{
	class calico_client
	{
	public:

		std::string host;
		int port;
		std::string default_namespace;
		std::shared_ptr<data_lake> app_lake;

		calico_client()
		{
			
		}

		calico_client(std::shared_ptr<data_lake> _app_lake, std::string _host, int _port, std::string _model_name, std::string _user_name )
		{
			app_lake = _app_lake;
			host = _host;
			port = _port;
			bind(_app_lake, _model_name, _user_name);
		}

		call_status login(std::string _model, std::string _user_name, json& login_json)
		{
			int success = 0;

			call_status dfs;

			json_parser jp;
			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json login_object = jp.create_object();

			login_object.put_member("UserName", _user_name);
			login_object.put_member("ModelName", _model);
			http_params login_result = calico_client.post(calico_host, calico_port, "api/LoginActor", login_object);

			dfs.request = login_result.request;
			dfs.response = login_result.response;

			if (login_result.response.content_type.starts_with("application/json"))
			{
				login_json = jp.parse_object(login_result.response.response_body.get_ptr());
				std::cout << login_json.to_json() << std::endl;

				// put a bogus token in there so downstream things will fail silently.
				if (!login_json.has_member("JwtToken")) {
					login_json.put_member("JwtToken", "this won't work");
				}

				dfs.success = login_json["Success"];
				dfs.message = login_json["Message"];
			}
			return dfs;
		}

		call_status get_field_options(json& credentials, std::string _field_name, json& calico_response)
		{
			call_status dfs;
			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();

			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.put_member("FieldName", _field_name);
				http_params calico_http = calico_client.get(calico_host, calico_port, "api/GetFieldOptions", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}


		call_status get_classes(json& credentials, json& calico_response)
		{
			call_status dfs;
			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				http_params calico_http = calico_client.get(calico_host, calico_port, "api/GetClassList", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}

		call_status get_fields(json& credentials, json& calico_response)
		{
			call_status dfs;
			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				http_params calico_http = calico_client.get(calico_host, calico_port, "api/GetFieldList", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}

		call_status create_class(json _source, json& credentials, json& calico_response)
		{

			call_status dfs;
			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json request(std::make_shared<json_object>());
			if (credentials.has_member("JwtToken")) {
				request.put_member("JwtToken", credentials);
				request.put_member("Namespace", default_namespace);
				request.put_member("ClassFullName", _source);
				request.put_member("BaseClassName", _source);
				request.put_member("RelatedClassList", _source);
				request.put_member("DelimitedClassFieldList", _source);

				http_params calico_http = calico_client.post(calico_host, calico_port, "api/PutClassEx", request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;
				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}

		call_status get_class(json _source, json& credentials, json& calico_response)
		{

			call_status dfs;
			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json request(std::make_shared<json_object>());

			if (credentials.has_member("JwtToken")) {
				request.copy_member("JwtToken", credentials);
				request.copy_member("ClassName", _source);

				http_params calico_http = calico_client.get(calico_host, calico_port, "api/GetClassByName", request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;
				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}

		call_status get_actor_options(json& credentials, json& calico_response)
		{
			call_status dfs;
			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.put_member("SearchObjectJson", "{}");

				http_params calico_http = calico_client.get(calico_host, calico_port, "api/GetActorOptions", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}

		call_status create_object(json source, json& credentials, json& calico_response)
		{
			call_status dfs;
			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				std::string object_json = source.to_json_string();
				calico_request.put_member("ObjectJson", object_json);

				http_params calico_http = calico_client.post(calico_host, calico_port, "api/CreateObject", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}

		call_status put_object(json source, json& credentials, json& calico_response)
		{
			call_status dfs;
			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.put_member("ObjectJson", source);
				http_params calico_http = calico_client.post(calico_host, calico_port, "api/PutObject", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					if (calico_http.response.content_type.starts_with("application/json"))
					{
						calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
						dfs.success = calico_response["Success"];
						dfs.message = calico_response["Message"];
					}
				}
			}
			return dfs;
		}

		call_status get_object(json source, json& credentials, json& calico_response)
		{
			call_status dfs;
			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.copy_member("ObjectId", source);

				http_params calico_http = calico_client.get(calico_host, calico_port, "api/GetObject", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					if (calico_http.response.content_type.starts_with("application/json"))
					{
						calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
						dfs.success = calico_response["Success"];
						dfs.message = calico_response["Message"];
					}
				}
			}
			return dfs;
		}

		call_status select_object(json source, json& credentials, json& calico_response)
		{
			call_status dfs;
			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.copy_member("ObjectId", source);
				calico_request.copy_member("Multiselect", source);

				http_params calico_http = calico_client.post(calico_host, calico_port, "api/SelectObject", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}

		call_status select_home(json& credentials, json& calico_response)
		{
			call_status dfs;
			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);

				http_params calico_http = calico_client.post(calico_host, calico_port, "api/SelectHome", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}

		call_status select_class(std::string className, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			call_status dfs;
			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json calico_request = jp.create_object();

			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.put_member("ClassName", className);

				http_params calico_http = calico_client.post(calico_host, calico_port, "api/SelectClass", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}

		call_status delete_object(json source, json& credentials, json& calico_response)
		{
			call_status dfs;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.copy_member("ObjectId", source);
				http_params calico_http = calico_client.post(calico_host, calico_port, "api/DeleteObject", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}

		call_status query(json source, json& credentials, json& calico_response)
		{
			call_status dfs;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.copy_member("ClassName", source);
				calico_request.copy_member("SearchObjectJson", source);
				calico_request.copy_member("FilterSelections", source);
				http_params calico_http = calico_client.post(calico_host, calico_port, "api/Query", calico_request);
				dfs.request = calico_http.request;
				dfs.response = calico_http.response;

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = calico_response["Success"];
					dfs.message = calico_response["Message"];
				}
			}
			return dfs;
		}

		gui_method on_get_credentials;
		gui_method on_get_classes;
		gui_method on_get_class;
		gui_method on_get_fields;
		gui_method on_update;
		gui_method on_get_state;
		gui_method on_get_ui;

		void bind(std::shared_ptr<data_lake> shared_plane, std::string model_name, std::string user_name)
		{
			json_parser jp;
			json user_model = jp.create_object();

			shared_plane->put_api("calico", "calico api", "");

			user_model.put_member("ModelName", model_name );
			user_model.put_member("UserName", user_name);
			shared_plane->put_function("calico", "context", user_model); 

			on_get_credentials = [](json _params, data_lake* _lake, data_function* _set) {
				_lake->call_function("calico", "fields");
				return 1;
				};

			shared_plane->put_function("calico", "credentials",
				[this, user_name](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					std::cout << "Logging into Calico thread " << ::GetCurrentThreadId() << std::endl;

					auto result = _lake->get_result("calico", "context");

					// get our login credentials from the data set.
					call_status temp = login(result["ModelName"], result["UserName"], _set->data);
					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				on_get_credentials,
				0);
			 
			shared_plane->put_function("calico", "classes",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					call_status login_status = _lake->get_status("calico", "credentials");

					if (login_status.success) {

						// get our login credentials from the data set.
						json credentials = _lake->get_result("calico", "credentials");

						// call our application
						call_status temp = get_classes(credentials, _set->data);

						// and, while we are it now, we can update our actor options, to show our created object
						return temp;

					}
					else 
					{
						return login_status;
					}
				},
				on_get_classes,
				0);

			shared_plane->put_function("calico", "fields",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					json_parser jp;

					call_status login_status = _lake->get_status("calico", "credentials");

					if (login_status.success) {
						// get our login credentials from the data set.
						json credentials = _lake->get_result("calico", "credentials");

						json fields_response;
						json fields_by_fieldname;

						// call our application
						call_status temp = get_fields(credentials, fields_response);

						auto grouped_fields = fields_response["FieldList"]["Fields"]
							.group([](json& _item) {
							std::string fieldName = _item["FieldName"];
							return fieldName;
								});

						fields_by_fieldname = jp.create_object();

						auto members = grouped_fields.get_members();
						for (auto m : members) {
							json jm(m.second);
							if (jm.is_array()) {
								auto je = jm.get_element(0);
								fields_by_fieldname.put_member(m.first, je);
							}
							else
							{
								fields_by_fieldname.put_member(m.first, jm);
							}
						}

						fields_response.put_member("FieldsByName", fields_by_fieldname);

						_set->data = fields_response;

						// and, while we are it now, we can update our actor options, to show our created object
						return temp;
					}
					else 
					{
						return login_status;
					}
				},
				on_get_fields,
				0);

			shared_plane->put_function("calico", "get_state",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					// get our login credentials from the data set.
					call_status login_status = _lake->get_status("calico", "credentials");

					if (login_status.success) {
						json credentials = _lake->get_result("calico", "credentials");

						// call our application
						call_status temp = get_actor_options(credentials, _set->data);
						// and, while we are it now, we can update our actor options, to show our created object
						return temp;
					}
					else
					{
						return login_status;
					}
				},
				on_get_state,
				0);

			shared_plane->put_function("calico", "get_class",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					// get our login credentials from the data set.
					json credentials = _lake->get_result("calico", "credentials");

					// call our application
					call_status temp = get_class(_params, credentials, _set->data);
					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				on_get_class,
				0);

			shared_plane->put_function("calico", "get_ui",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					json_parser jp;

					// get our login credentials from the data set.
					json credentials = _lake->get_result("calico", "credentials");

					// call our application
					json actor_options_response;
					json actor_options;
					json class_detail;
					json object_detail;

					call_status temp1 = get_actor_options(credentials, actor_options_response);
					call_status temp2;
					call_status temp3;

					if (temp1.success) {

						auto actor_options = actor_options_response["ActorOptions"];
						json selected_objects = actor_options["SelectedObjects"];
						int64_t current_edited_object_id = -1;
						std::string current_edited_class_name;
						std::string current_edited_class_description;

						// and now we go through our selected objects....
						for (int i = 0; i < selected_objects.size(); i++)
						{
							// pull our current object out of this json array
							auto selected_object = selected_objects.get_element(i);

							std::string class_name = selected_object["ClassName"].get_string();
							std::string class_description = selected_object["ClassDescription"].get_string();
							int64_t class_id = selected_object["ClassId"].get_double();
							int64_t object_id = selected_object["ObjectId"].get_double();
							std::string object_id_string = selected_object["ObjectId"];

							if (current_edited_object_id < object_id)
							{
								current_edited_object_id = object_id;
								current_edited_class_name = class_name;
								current_edited_class_description = class_description;
							}
						}

						json class_parameters;
						class_parameters = jp.create_object();
						class_parameters.put_member("ClassName", current_edited_class_name);
						call_status temp2 = get_class(class_parameters, credentials, class_detail);

						if (temp2.success) {


							// extract our class data
							auto class_obj = class_detail["CalicoClass"];
							std::string classDescription = class_obj["ClassDescription"];
							std::string classFullName = class_obj["ClassFullName"];
							json relatedClassList = class_obj["RelatedClassList"];
							json classFieldList = class_obj["ClassFieldList"];
							std::string primaryKeyField = class_obj["PrimaryKeyField"];
							std::string sqlTableName = class_obj["SqlTableName"];
							std::string sqlViewName = class_obj["SqlViewName"];

							json object_parameters;
							object_parameters = jp.create_object();
							object_parameters.put_member("ObjectId", current_edited_object_id);
							temp3 = get_object(object_parameters, credentials, object_detail);

							json ui_result = jp.create_object();

							// now get the fields for this thing
							json all_fields = _lake->get_result("calico", "fields");

							auto fields = all_fields["FieldList"]["Fields"];

							json* selected_class_fields = &classFieldList;
							json class_field_list;
							class_field_list = fields.filter(
								[selected_class_fields](json& _field)
								{
									bool result = false;
									std::string fieldName = _field["FieldName"];
									result = selected_class_fields->any([fieldName](json& _item)
										{
											bool result = false;
											std::string class_field = _item;
											if (class_field == fieldName) {
												result = true;
											}
											return result;
										});
									return result;
								});

							ui_result.put_member("current_state", actor_options_response);
							ui_result.put_member("current_class", class_detail);
							ui_result.put_member("current_object", object_detail);
							ui_result.put_member("current_fields", class_field_list);

							_set->data = ui_result;
						}
					}

					// and, while we are it now, we can update our actor options, to show our created object
					return temp3;
				},
				on_get_ui,
				0);

			shared_plane->put_function("calico", "select_object",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					// get our login credentials from the data set.
					json credentials = _lake->get_result("calico", "credentials");

					// call our application
					call_status temp = select_object(_params, credentials, _set->data);
					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				on_update,
				0);

			shared_plane->put_function("calico", "create_object",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					// get our login credentials from the data set.
					json credentials = _lake->get_result("calico", "credentials");

					// call our application
					call_status temp = create_object(_params, credentials, _set->data);
					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				on_update,
				0);

			shared_plane->put_function("calico", "delete_object",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					// get our login credentials from the data set.
					json credentials = _lake->get_result("calico", "credentials");

					// call our application
					call_status temp = delete_object(_params, credentials, _set->data);
					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				on_update,
				0);

			shared_plane->put_function("calico", "put_object",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					// get our login credentials from the data set.
					json credentials = _lake->get_result("calico", "credentials");

					// call our application
					call_status temp = put_object(_params, credentials, _set->data);
					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				on_update,
				0);
		}

	};
}

#endif
