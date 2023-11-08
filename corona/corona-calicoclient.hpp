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

		std::string user_token;

		std::string last_message;
		std::string last_success_string;
		std::string last_execution_time;

		json actor_options;
		json allowed_base_classes;

		std::string default_namespace;

		calico_client()
		{
			;
		}

		task<int> login(std::string _model, std::string _user_name)
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
				json login_json = jp.parse_object(login_result.response.response_body.get_ptr());
				if (login_json.has_member("jwtToken")) 
				{
					user_token = login_json.get_member("jwtToken");
				}
				else 
				{
					user_token = "";
				}
				last_message = login_json.get_member("message");
				last_success_string = login_json.get_member("success");
				if (last_success_string == "true") {
					success = 1;
				}
			}

			co_return success;
		}

		task<int> get_classes()
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			calico_request.put_member("JwtToken", user_token);
			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/GetClassList", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				json calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				auto classList = calico_response["calicoClassList"]["classes"];
			}
			co_return success;
		}

		task<int> get_fields()
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			calico_request.put_member("JwtToken", user_token);
			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/GetFieldList", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				json calico_response = jp.parse_object(calico_http.response.response_body.get_ptr());
				auto fieldList = calico_response["fieldList"]["fields"];
			}
			co_return success;
		}

		task<int> get_actor_options()
		{
			int success = 0;

			json_parser jp;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;
			json calico_request = jp.create_object();
			calico_request.put_member("JwtToken", user_token);
			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/GetFieldList", calico_request);

			if (calico_http.response.content_type.starts_with("application/json"))
			{
				actor_options = jp.parse_object(calico_http.response.response_body.get_ptr());
				allowed_base_classes = actor_options["actor_options"]["create_options"].map([](json& _item) {
					json new_item = _item->clone();
					return new_item;
					});
				success = true;
			}
			co_return success;
		}

		task<int> create_class( json _source )
		{
			int success = false;

			http_client calico_client;
			const char* calico_host = host.c_str();
			int calico_port = port;

			json request(std::make_shared<json_object>());
			request.put_member("jwtToken", user_token);
			request.put_member("namespace", default_namespace);
			request.put_member("classFullName", _source);
			request.put_member("baseClassName", _source);
			request.put_member("relatedClassList", _source);
			request.put_member("delimitedClassFieldList", _source);

			http_params calico_http = co_await calico_client.post(calico_host, calico_port, "api/PutClassEx", request);
			if (calico_http.response.content_type.starts_with("application/json"))
			{
				success = true;
			}
			co_return success;
		}

	};
}

#endif
