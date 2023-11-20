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

		json actor_options;
		json allowed_base_classes;
		json class_list;
		json field_list;

		std::string default_namespace;

		calico_client()
		{
			;
		}

		task<int> login(std::string _model, std::string _user_name, json& login_json)
		{
			int success = 0;

			json_parser jp;
			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json login_object = jp.create_object();

			login_object.put_member("userName", _user_name);
			login_object.put_member("modelName", _model);
			http_params login_result = co_await calico_client.post(calico_host, calico_port, "api/LoginActor", login_object);

			if (login_result.response.content_type.starts_with("application/json"))
			{
				login_json = jp.parse_object(login_result.response.response_body.get_ptr());
				// login_json["jwtToken"] will have the token
				success = 1;
			}
			+
			co_return success;
		}

		task<int> get_classes(json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			calico_request.copy_member("jwtToken", credentials);
			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/GetClassList", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
			}
			co_return success;
		}

		task<int> get_fields(json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			calico_request.copy_member("jwtToken", credentials);
			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/GetFieldList", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
			}
			co_return success;
		}

		task<int> create_class(json _source, json& credentials, json& calico_response)
		{
			int success = false;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json request(std::make_shared<json_object>());
			request.put_member("jwtToken", credentials);
			request.put_member("namespace", default_namespace);
			request.put_member("classFullName", _source);
			request.put_member("baseClassName", _source);
			request.put_member("relatedClassList", _source);
			request.put_member("delimitedClassFieldList", _source);

			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/PutClassEx", request);
			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
			}
			co_return success;
		}

		task<int> get_actor_options(json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			calico_request.copy_member("jwtToken", credentials);

			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/GetActorOptions", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				success = true;
			}
			co_return success;
		}

		task<int> create_object(json source, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			calico_request.copy_member("jwtToken", credentials);
			calico_request.put_member("objectJson", source);
			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/CreateObject", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				success = true;
			}
			co_return success;
		}

		task<int> put_object(json source, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			calico_request.copy_member("jwtToken", credentials);
			calico_request.put_member("objectJson", source);
			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/CreateObject", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				success = true;
			}
			co_return success;
		}

		task<int> select_object(json source, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json calico_request = jp.create_object();
			calico_request.copy_member("jwtToken", credentials);
			calico_request.copy_member("objectId", source);
			calico_request.copy_member("multiselect", source);

			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/SelectObject", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				success = true;
			}
			co_return success;
		}

		task<int> select_home(json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json calico_request = jp.create_object();
			calico_request.copy_member("jwtToken", credentials);

			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/SelectHome", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				success = true;
			}
			co_return success;
		}

		task<int> select_class(std::string className, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json calico_request = jp.create_object();

			calico_request.copy_member("jwtToken", credentials);
			calico_request.put_member("className", className);

			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/SelectClass", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				success = true;
			}
			co_return success;
		}

		task<int> delete_object(json source, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			calico_request.copy_member("jwtToken", credentials);
			calico_request.copy_member("objectId", source);
			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/DeleteObject", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				success = true;
			}
			co_return success;
		}

		task<int> put_object(json source, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();

			calico_request.copy_member("jwtToken", credentials);
			calico_request.copy_member("objectJSON", source);
			calico_request.copy_member("ruleName", source);
			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/DeleteObject", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				success = true;
			}
			co_return success;
		}

		task<int> query(json source, json& credentials, json& calico_response)
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			calico_request.copy_member("jwtToken", credentials);
			calico_request.copy_member("className", source);
			calico_request.copy_member("searchObjectJson", source);
			calico_request.copy_member("filterSelections", source);
			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/Query", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				success = true;
			}
			co_return success;
		}

	};
}

#endif
