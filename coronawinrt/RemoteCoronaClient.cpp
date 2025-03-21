#include "pch.h"
#include "RemoteCoronaClient.h"
#include "RemoteCoronaClient.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring RemoteCoronaClient::ServerHostName()
    {
        return server_host_name;
    }
    void RemoteCoronaClient::ServerHostName(hstring const& value)
    {
        server_host_name = value;
    }
    int32_t RemoteCoronaClient::ServerPort()
    {
        return server_port;
    }
    void RemoteCoronaClient::ServerPort(int32_t value)
    {
        server_port = value;
    }
    hstring RemoteCoronaClient::ServerUrl()
    {
        return server_url;
    }
    void RemoteCoronaClient::ServerUrl(hstring const& value)
    {
        server_url = value;
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CreateUserResponse> RemoteCoronaClient::CreateUser(winrt::coronawinrt::CreateUserRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::LoginUserResponse> RemoteCoronaClient::LoginUser(winrt::coronawinrt::LoginUserRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::ConfirmUserResponse> RemoteCoronaClient::ConfirmUser(winrt::coronawinrt::ConfirmUserRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::SendUserResponse> RemoteCoronaClient::SendUser(winrt::coronawinrt::SendUserRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PasswordUserResponse> RemoteCoronaClient::PasswordUser(winrt::coronawinrt::PasswordUserRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetClassResponse> RemoteCoronaClient::GetClass(winrt::coronawinrt::GetClassRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetClassesResponse> RemoteCoronaClient::GetClasses(winrt::coronawinrt::GetClassesRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PutClassesResponse> RemoteCoronaClient::PutClasses(winrt::coronawinrt::PutClassesRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::GetObjectResponse> RemoteCoronaClient::GetObjects(winrt::coronawinrt::GetObjectRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::PutObjectsResponse> RemoteCoronaClient::PutObjects(winrt::coronawinrt::PutObjectsRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::QueryObjectsResponse> RemoteCoronaClient::QueryObjects(winrt::coronawinrt::QueryObjectsRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::DeleteObjectsResponse> RemoteCoronaClient::DeleteObjects(winrt::coronawinrt::DeleteObjectsRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CreateObjectResponse> RemoteCoronaClient::CreateObject(winrt::coronawinrt::CreateObjectRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::EditObjectResponse> RemoteCoronaClient::EditObject(winrt::coronawinrt::EditObjectRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::RunObjectRequest> RemoteCoronaClient::RunObject(winrt::coronawinrt::RunObjectRequest request)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncOperation<winrt::coronawinrt::CopyObjectRequest> RemoteCoronaClient::CopyObject(winrt::coronawinrt::CopyObjectRequest request)
    {
        throw hresult_not_implemented();
    }
}
