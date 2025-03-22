#pragma once
#include "LocalCoronaClient.g.h"

namespace winrt::coronawinrt::implementation
{
    struct LocalCoronaClient : LocalCoronaClientT<LocalCoronaClient>
    {
        corona::json get_local_token();

        std::string database_schema_filename;
        std::string database_config_filename;

        corona::json_file_watcher database_schema_mon;
        corona::json_file_watcher database_config_mon;

        std::string database_filename;
        std::string user_file_name;

        bool ready_for_polling;

        corona::json system_proof;

        corona::json								local_db_config;

        std::shared_ptr<corona::corona_database>	local_db;
        std::shared_ptr<corona::application>		app;
        std::shared_ptr<corona::file>				db_file;
        corona::json								abbreviations;
        corona::json                                server_config;

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
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::RunObjectRequest> RunObject(winrt::coronawinrt::RunObjectRequest request);
        winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CopyObjectRequest> CopyObject(winrt::coronawinrt::CopyObjectRequest request);

        corona::json get_local_token();
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct LocalCoronaClient : LocalCoronaClientT<LocalCoronaClient, implementation::LocalCoronaClient>
    {
    };
}
