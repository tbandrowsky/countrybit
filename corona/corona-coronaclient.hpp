#ifndef CORONA_CORONACLIENT_H
#define CORONA_CORONACLIENT_H

namespace corona
{
	class corona_client
	{
	public:

		std::string host;
		int port;
		
		corona_client()
		{
			
		}

		corona_client(std::string _host, int _port)
		{
			host = _host;
			port = _port;
		}

		json create_request(json _credential, json _data)
		{
			json_parser jp;
			json request;
			request = jp.create_object();
			request.copy_member("Token", _credential);
			request.put_member("Data", _data);
			return request;
		}

		std::function<void(std::string _function_name, json& _credentials, json& _payload, json& _corona_response)> on_post_request;
		std::function<void(call_status _status, std::string _function_name, json& _credentials, json& _payload, json& _corona_response)> on_post_response;

		call_status general_post(std::string _function_name, json& _credentials, json& _payload, json& _corona_response)
		{
			call_status dfs;
			json_parser jp;
			
			if (on_post_request) {
				on_post_request(_function_name, _credentials, _payload, _corona_response);
			}

			http_client corona_client;
			const char* corona_host = host.c_str();
			int corona_port = port;
			json token_request = create_request(_credentials, _payload);
			http_params corona_http = corona_client.post(corona_host, corona_port, _function_name.c_str(), token_request);
			dfs.request = corona_http.request;
			dfs.response = corona_http.response;
			dfs.function_path = _function_name;

			if (corona_http.response.content_type.starts_with("application/json"))
			{
				_corona_response = jp.parse_object(corona_http.response.response_body.get_ptr());
				dfs.success = _corona_response["Success"];
				dfs.message = _corona_response["Message"];
			}

			if (on_post_response) {
				on_post_response(dfs, _function_name, _credentials, _payload, _corona_response);
			}

			return dfs;
		}

		call_status login(std::string _user_name, std::string _password, json& login_json)
		{
			int success = 0;

			call_status dfs;

			json_parser jp;
			http_client corona_client;
			const char* corona_host = host.c_str();
			int corona_port = port;
			json login_object = jp.create_object();

			login_object.put_member("Name", _user_name);
			login_object.put_member("Password", _password);
			http_params login_result = corona_client.post(corona_host, corona_port, "login/start", login_object);

			dfs.request = login_result.request;
			dfs.response = login_result.response;

			if (login_result.response.content_type.starts_with("application/json"))
			{
				login_json = jp.parse_object(login_result.response.response_body.get_ptr());
				std::cout << login_json.to_json() << std::endl;

				dfs.success = login_json["Success"];
				dfs.message = login_json["Message"];
			}
			return dfs;
		}

		call_status create_user(json& _credentials, json& _user_definition, json& corona_response)
		{
			return general_post("login/create", _credentials, _user_definition, corona_response);
		}

		call_status send_login_code(json& _credentials, std::string _name, json& corona_response)
		{
			json_parser jp;

			json corona_request = jp.create_object();
			corona_request.put_member("Name", _name);
			return general_post("login/sendcode", _credentials, corona_request, corona_response);
		}

		call_status confirm_login_code(json& _credentials, std::string _name, json& corona_response)
		{
			json_parser jp;

			json corona_request = jp.create_object();
			corona_request.put_member("Name", _name);
			return general_post("login/confirmcode", _credentials, corona_request, corona_response);
		}

		call_status set_login_password(json& _credentials, std::string _name, std::string _password1, std::string _password2, json& corona_response)
		{
			json_parser jp;
			json corona_request = jp.create_object();
			corona_request.put_member("Name", _name);
			corona_request.put_member("Password1", _password1);
			corona_request.put_member("Password2", _password2);
			return general_post("login/passwordset", _credentials, corona_request, corona_response);
		}

		call_status get_classes(json& _credentials, json& corona_response)
		{
			json_parser jp;
			json corona_request = jp.create_object();
			return general_post("classes/get", _credentials, corona_request, corona_response);
		}

		call_status put_class(json& _credentials, json& _class_definition, json& corona_response)
		{
			return general_post("classes/put", _credentials, _class_definition, corona_response);
		}

		call_status create_object(json& _credentials, std::string _class_name, json& corona_response)
		{
			json_parser jp;
			json corona_request = jp.create_object();
			corona_request.put_member("ClassName", _class_name);
			return general_post("objects/create", _credentials, corona_request, corona_response);
		}

		call_status put_object(json& _credentials, json _object_definition, json& corona_response)
		{
			return general_post("objects/put", _credentials, _object_definition, corona_response);
		}

		call_status get_object(json& _credentials, std::string _class_name, std::string _object_name, json& corona_response)
		{
			json_parser jp;
			json corona_request = jp.create_object();
			corona_request.put_member("ClassName", _class_name);
			corona_request.put_member("Name", _object_name);
			return general_post("objects/get", _credentials, corona_request, corona_response);
		}

		call_status get_object(json& _credentials, int64_t _object_id, json& corona_response)
		{
			json_parser jp;
			json corona_request = jp.create_object();
			corona_request.put_member_i64("ObjectId", _object_id);
			return general_post("objects/get", _credentials, corona_request, corona_response);
		}

		call_status delete_object(json& _credentials, db_object_id_type _object_id, json& corona_response)
		{
			json_parser jp;
			json corona_request = jp.create_object();
			corona_request.put_member_i64("ObjectId", _object_id);
			return general_post("objects/delete", _credentials, corona_request, corona_response);
		}

		call_status edit_object(json& _credentials, db_object_id_type _object_id, json& corona_response)
		{
			json_parser jp;
			json corona_request = jp.create_object();
			corona_request.put_member_i64("ObjectId", _object_id);
			return general_post("objects/edit", _credentials, corona_request, corona_response);
		}

		call_status query_objects(json& _credentials, json _query_body, json& corona_response)
		{
			json_parser jp;
			return general_post("objects/query", _credentials, _query_body, corona_response);
		}
	};
}

#endif
