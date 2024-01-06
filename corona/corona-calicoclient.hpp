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

		calico_client(std::shared_ptr<data_lake> _app_lake, std::string _host, int _port)
		{
			app_lake = _app_lake;
			host = _host;
			port = _port;
			bind(_app_lake);
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

				if (login_json.has_member("Errors"))
				{
					dfs.success = false;
				}
				else 
				{
					dfs.success = true;
				}
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
					std::cout << calico_response.to_json() << std::endl;
					dfs.success = true;
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
					std::cout << calico_response.to_json() << std::endl;
					dfs.success = true;
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

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					dfs.success = true;
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
					dfs.success = true;
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
					dfs.success = true;
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
					dfs.success = true;
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
					dfs.success = true;
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
					dfs.success = true;
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
					dfs.success = true;
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
					dfs.success = true;
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
					dfs.success = true;
				}
			}
			return dfs;
		}

		gui_method on_get_credentials;
		gui_method on_get_classes;
		gui_method on_get_fields;
		gui_method on_update;
		gui_method on_get_state;

		void bind(std::shared_ptr<data_lake> shared_plane)
		{
			std::string user_name;

			shared_plane->put_api("calico", "calico api", "");

			shared_plane->put_function("calico", "credentials",
				[this, user_name](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					// get our login credentials from the data set.
					json classes_json;
					json fields_json;
					call_status temp = login("Property", user_name, _set->data);
					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				on_get_credentials,
				0);

			shared_plane->put_function("calico", "classes",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					// get our login credentials from the data set.
					json credentials = _lake->get_result("calico", "credentials");

					// call our application
					call_status temp = get_classes(credentials, _set->data);
					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				on_get_classes,
				0);

			shared_plane->put_function("calico", "fields",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					// get our login credentials from the data set.
					json credentials = _lake->get_result("calico", "credentials");

					// call our application
					call_status temp = get_fields(credentials, _set->data);
					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				on_get_fields,
				0);

			shared_plane->put_function("calico", "get_state",
				[this](json _params, data_lake* _lake, data_function* _set) -> call_status
				{
					// get our login credentials from the data set.
					json credentials = _lake->get_result("calico", "credentials");

					// call our application
					call_status temp = get_actor_options(credentials, _set->data);
					// and, while we are it now, we can update our actor options, to show our created object
					return temp;
				},
				on_get_state,
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
