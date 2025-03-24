#pragma once
#include "LocalCoronaClient.g.h"

using namespace corona;

namespace winrt::coronawinrt::implementation
{
    struct LocalCoronaClient : LocalCoronaClientT<LocalCoronaClient>
    {

		std::vector<std::string> api_paths;

        json get_local_token();
        json get_banner();
        void poll_db();
		json parse_request(http_request _request);
		http_response check_parse_error(json _request);
		http_response create_response(int _http_status_code, json _source);
		void bind_web_server(corona::http_server& _server);
		std::string get_token(http_action_request& _request);
		http_server db_api_server;

        std::string database_schema_filename;
        std::string database_config_filename;

        json_file_watcher database_schema_mon;
        json_file_watcher database_config_mon;

        std::string database_filename;
        std::string user_file_name;

        bool ready_for_polling;

        json system_proof;
        json								local_db_config;

        std::shared_ptr<corona_database>	local_db;
        std::shared_ptr<application>		app;
        std::shared_ptr<file>				db_file;
        json								abbreviations;
        json                                server_config;
        std::string                         listen_point;

        LocalCoronaClient() = default;

        hstring application_name;
        hstring application_folder_name;
        hstring config_file_name_base;
        hstring file_name;

        hstring ApplicationName();
        void ApplicationName(hstring const& value);
        hstring ApplicationFolderName();
        void ApplicationFolderName(hstring const& value);
        hstring ConfigFileNameBase();
        void ConfigFileNameBase(hstring const& value);
        hstring FileName();
        void FileName(hstring const& value);

        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::BaseResponse> Open();
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::BaseResponse> Close();
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CreateUserResponse> CreateUser(winrt::coronawinrt::CreateUserRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::LoginUserResponse> LoginUser(winrt::coronawinrt::LoginUserRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::ConfirmUserResponse> ConfirmUser(winrt::coronawinrt::ConfirmUserRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::SendUserResponse> SendUser(winrt::coronawinrt::SendUserRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PasswordUserResponse> PasswordUser(winrt::coronawinrt::PasswordUserRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetClassResponse> GetClass(winrt::coronawinrt::GetClassRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetClassesResponse> GetClasses(winrt::coronawinrt::GetClassesRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PutClassesResponse> PutClasses(winrt::coronawinrt::PutClassesRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetObjectResponse> GetObjects(winrt::coronawinrt::GetObjectRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PutObjectsResponse> PutObjects(winrt::coronawinrt::PutObjectsRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::QueryObjectsResponse> QueryObjects(winrt::coronawinrt::QueryObjectsRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::DeleteObjectsResponse> DeleteObjects(winrt::coronawinrt::DeleteObjectsRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CreateObjectResponse> CreateObject(winrt::coronawinrt::CreateObjectRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::EditObjectResponse> EditObject(winrt::coronawinrt::EditObjectRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::RunObjectResponse> RunObject(winrt::coronawinrt::RunObjectRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CopyObjectResponse> CopyObject(winrt::coronawinrt::CopyObjectRequest request);

		http_handler_function corona_test = [this](http_action_request _request)-> void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			json fn_response = get_banner();
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_login = [this](http_action_request _request)-> void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			auto fn_response = local_db->login_user(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};


		http_handler_function corona_classes_get = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			auto fn_response = local_db->get_classes(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_class_get = [this](http_action_request _request)->void {

			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			auto fn_response = local_db->get_class(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_classes_put = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->put_class(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_users_create = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			// there's not a token in create_user to allow anyone to onboard.
			json fn_response = local_db->create_user(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_users_send_confirm = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->send_user_confirm_code(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_users_confirm = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->user_confirm_user_code(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};


		http_handler_function corona_user_password = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->set_user_password(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_get = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->get_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_copy = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->copy_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_query = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->query(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_create = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->create_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_put = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->put_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_delete = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member(token_field, token);
			json fn_response = local_db->delete_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_edit = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member("Token", token);
			json fn_response = local_db->edit_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};

		http_handler_function corona_objects_run = [this](http_action_request _request)->void {
			json parsed_request = parse_request(_request.request);
			if (parsed_request.error()) {
				http_response error_response = create_response(500, parsed_request);
				_request.send_response(500, "Parse error", parsed_request);
			}
			std::string token = get_token(_request);
			parsed_request.put_member("Token", token);
			json fn_response = local_db->run_object(parsed_request);
			http_response response = create_response(200, fn_response);
			_request.send_response(200, "Ok", fn_response);
			};
    };


}
namespace winrt::coronawinrt::factory_implementation
{
    struct LocalCoronaClient : LocalCoronaClientT<LocalCoronaClient, implementation::LocalCoronaClient>
    {
    };
}
