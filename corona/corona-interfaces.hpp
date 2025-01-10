#pragma once

#ifndef CORONA_INTERFACES_H

namespace corona
{
	class corona_client_response
	{
	public:
		bool			success;
		std::string		message;
		double			execution_time;
		json			data;

		corona_client_response& operator = (http_params& _params)
		{
			json_parser jp;
			json response = jp.parse_object(_params.response.server.c_str());
			data = response[data_field];
			execution_time = response["execution_time_seconds"];
			success = (bool)response[success_field];
			message = (bool)response[message_field];
			return *this;
		}
	};


	class corona_client_interface
	{
	public:

		virtual corona_client_response register_user(std::string user_name, std::string email, std::string password1, std::string password2) = 0;
		virtual corona_client_response confirm_user(std::string user_name, std::string confirmation_code) = 0;
		virtual corona_client_response send_user(std::string user_name) = 0;
		virtual corona_client_response login(std::string _user_name, std::string _password) = 0;
		virtual corona_client_response login() = 0;
		virtual corona_client_response set_password(std::string user_name, std::string validation_code, std::string password1, std::string password2) = 0;
		virtual corona_client_response get_classes() = 0;
		virtual corona_client_response get_class(std::string class_name) = 0;
		virtual corona_client_response put_class(json _class_definition) = 0;
		virtual corona_client_response create_object(std::string _class_name) = 0;
		virtual corona_client_response edit_object(std::string _class_name, int64_t _object_id, bool _object_id_field) = 0;
		virtual corona_client_response get_object(std::string _class_name, int64_t _object_id, bool _include_children) = 0;
		virtual corona_client_response put_object(json _object) = 0;
		virtual corona_client_response run_object(json _object) = 0;
		virtual corona_client_response delete_object(std::string _class_name, int64_t _object_id) = 0;
		virtual corona_client_response query_objects(json _query) = 0;
	};
}

#endif
