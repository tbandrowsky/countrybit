#include "pch.h"
#include "LocalCoronaClient.h"
#include "LocalCoronaClient.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring LocalCoronaClient::ApplicationName()
    {
        return application_name;
    }

    void LocalCoronaClient::ApplicationName(hstring const& value)
    {
        application_name = value;
    }

    hstring LocalCoronaClient::ApplicationFolderName()
    {
        return application_name;
    }

    void LocalCoronaClient::ApplicationFolderName(hstring const& value)
    {
        application_name = value;
    }

    hstring LocalCoronaClient::ConfigFileNameBase()
    {
        return application_name;
    }

    void LocalCoronaClient::ConfigFileNameBase(hstring const& value)
    {
        application_name = value;
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::BaseResponse> LocalCoronaClient::Open()
    {
        using namespace corona;

        system_monitoring_interface::start(); // this will create the global log queue.
        timer tx;
        date_time t = corona::date_time::now();
        json_parser jp;

        ready_for_polling = false;

        database_config_filename = winrt::to_string(config_file_name_base);
        database_config_mon.filename = database_config_filename;

        auto result = database_config_mon.poll_contents(app.get(), local_db_config);
        if (result == null_row) {
            system_monitoring_interface::global_mon->log_warning(std::format("config file {0} not found", database_config_filename), __FILE__, __LINE__);
            throw std::invalid_argument("config file not found");
        }
        else {
            system_monitoring_interface::global_mon->log_information(std::format("using config file {0}", database_config_filename), __FILE__, __LINE__);
        }

        server_config = local_db_config["Server"];

        database_filename = server_config["database_filename"];
        database_schema_filename = server_config["schema_filename"];

        if (database_filename.empty() or database_schema_filename.empty())
        {
            throw std::logic_error("database file or schema file not specified");
        }

        database_schema_mon.filename = database_schema_filename;

        app = std::make_shared<application>();
        app->application_name = server_config["application_name"];
        listen_point = server_config["listen_point"];

        if (app->application_name.empty())
        {
            throw std::logic_error("application_name not specified");
        }

        if (listen_point.empty())
        {
            throw std::logic_error("listen_point not specified");
        }

        log_information("Self test.");
        prove_system();

        log_information("Startup user name " + app->get_user_display_name());

        if (true or not app->file_exists(database_filename))
        {
            db_file = app->open_file_ptr(database_filename, file_open_types::create_always);
            local_db = std::make_shared<corona_database>(db_file);
            local_db->apply_config(local_db_config);

            json create_database_response = local_db->create_database();

            bool success = (bool)create_database_response[success_field];
            if (!success) {
                log_json(create_database_response);
                throw std::exception("Coult not create database");
            }

            relative_ptr_type result = database_config_mon.poll(app.get());
            ready_for_polling = true;
        }
        else
        {
            db_file = app->open_file_ptr(database_filename, file_open_types::open_existing);
            local_db = std::make_shared<corona_database>(db_file);
            local_db->apply_config(local_db_config);

            local_db->open_database(0);

            relative_ptr_type result = database_config_mon.poll(app.get());
            ready_for_polling = true;
        }

        bind_web_server(db_api_server);
        db_api_server.start();

        log_information("listening on :" + listen_point, __FILE__, __LINE__);
        for (auto path : api_paths) {
            log_information(path, __FILE__, __LINE__);
        }
        log_command_stop("comm_service_bus", "startup complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);


    }

    corona::json LocalCoronaClient::get_local_token()
    {
        using namespace corona;
        json_parser jp;
        json login_request = jp.create_object();
        json server_config = local_db_config["Server"];
        std::string user_name = server_config[sys_user_name_field];
        std::string password = server_config[sys_user_password_field];
        login_request.put_member(sys_user_name_field, user_name);
        login_request.put_member(sys_user_password_field, password);
        corona::json result = local_db->login_user(login_request);
        return result[token_field];
    }


    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::BaseResponse> LocalCoronaClient::Close()
    {

    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CreateUserResponse> LocalCoronaClient::CreateUser(winrt::coronawinrt::CreateUserRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::LoginUserResponse> LocalCoronaClient::LoginUser(winrt::coronawinrt::LoginUserRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::ConfirmUserResponse> LocalCoronaClient::ConfirmUser(winrt::coronawinrt::ConfirmUserRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::SendUserResponse> LocalCoronaClient::SendUser(winrt::coronawinrt::SendUserRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PasswordUserResponse> LocalCoronaClient::PasswordUser(winrt::coronawinrt::PasswordUserRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetClassResponse> LocalCoronaClient::GetClass(winrt::coronawinrt::GetClassRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetClassesResponse> LocalCoronaClient::GetClasses(winrt::coronawinrt::GetClassesRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PutClassesResponse> LocalCoronaClient::PutClasses(winrt::coronawinrt::PutClassesRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetObjectResponse> LocalCoronaClient::GetObjects(winrt::coronawinrt::GetObjectRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PutObjectsResponse> LocalCoronaClient::PutObjects(winrt::coronawinrt::PutObjectsRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::QueryObjectsResponse> LocalCoronaClient::QueryObjects(winrt::coronawinrt::QueryObjectsRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::DeleteObjectsResponse> LocalCoronaClient::DeleteObjects(winrt::coronawinrt::DeleteObjectsRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CreateObjectResponse> LocalCoronaClient::CreateObject(winrt::coronawinrt::CreateObjectRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::EditObjectResponse> LocalCoronaClient::EditObject(winrt::coronawinrt::EditObjectRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::RunObjectRequest> LocalCoronaClient::RunObject(winrt::coronawinrt::RunObjectRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CopyObjectRequest> LocalCoronaClient::CopyObject(winrt::coronawinrt::CopyObjectRequest request)
    {
        throw hresult_not_implemented();
    }
    hstring LocalCoronaClient::FileName()
    {
        throw hresult_not_implemented();
    }
    void LocalCoronaClient::FileName(hstring const& value)
    {
        throw hresult_not_implemented();
    }
}
