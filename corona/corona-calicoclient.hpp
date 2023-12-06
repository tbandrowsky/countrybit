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

		calico_client()
		{
			;
		}

		calico_client(std::string _host, int _port)
		{
			host = _host;
			port = _port;
		}

		int login(std::string _model, std::string _user_name, json& login_json)
		{
			int success = 0;

			json_parser jp;
			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json login_object = jp.create_object();

			login_object.put_member("UserName", _user_name);
			login_object.put_member("ModelName", _model);
			http_params login_result = calico_client.post(calico_host, calico_port, "api/LoginActor", login_object);

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
					success = false;
				}
				else 
				{
					success = true;
				}
			}
			return success;
		}

		int get_field_options(json& credentials, std::string _field_name, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.put_member("FieldName", _field_name);
				http_params calico_http = calico_client.get(calico_host, calico_port, "api/GetFieldOptions", calico_request);

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					std::cout << calico_response.to_json() << std::endl;
					success = true;
				}
			}
			return success;
		}


		int get_classes(json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				http_params calico_http = calico_client.get(calico_host, calico_port, "api/GetClassList", calico_request);

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					std::cout << calico_response.to_json() << std::endl;
					success = true;
				}
			}
			return success;
		}

		int get_fields(json& credentials, json& calico_response)
		{
			int success = 0;

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
					success = true;
				}
			}
			return success;
		}

		int create_class(json _source, json& credentials, json& calico_response)
		{
			int success = false;

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
				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				}
			}
			return success;
		}

		int get_actor_options(json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.put_member("SearchObjectJson", "{}");

				http_params calico_http = calico_client.get(calico_host, calico_port, "api/GetActorOptions", calico_request);

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					success = true;
				}
			}
			return success;
		}

		int create_object(json source, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.put_member("ObjectJson", source);
				http_params calico_http = calico_client.post(calico_host, calico_port, "api/CreateObject", calico_request);

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					success = true;
				}
			}
			return success;
		}

		int put_object(json source, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.put_member("ObjectJson", source);
				http_params calico_http = calico_client.post(calico_host, calico_port, "api/PutObject", calico_request);

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					success = true;
				}
			}
			return success;
		}

		int select_object(json source, json& credentials, json& calico_response)
		{
			int success = 0;

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

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					success = true;
				}
			}
			return success;
		}

		int select_home(json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);

				http_params calico_http = calico_client.post(calico_host, calico_port, "api/SelectHome", calico_request);

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					success = true;
				}
			}
			return success;
		}

		int select_class(std::string className, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json calico_request = jp.create_object();

			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.put_member("ClassName", className);

				http_params calico_http = calico_client.post(calico_host, calico_port, "api/SelectClass", calico_request);

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					success = true;
				}
			}
			return success;
		}

		int delete_object(json source, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			if (credentials.has_member("JwtToken")) {
				calico_request.copy_member("JwtToken", credentials);
				calico_request.copy_member("ObjectId", source);
				http_params calico_http = calico_client.post(calico_host, calico_port, "api/DeleteObject", calico_request);

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					success = true;
				}
			}
			return success;
		}

		int query(json source, json& credentials, json& calico_response)
		{
			int success = 0;

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

				if (calico_http.response.content_type.starts_with("application/json"))
				{
					calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
					success = true;
				}
			}
			return success;
		}

	};
}

#endif
