#pragma once
#include "coronawinrt.LocalCoronaClient.g.h"

namespace winrt::coronawinrt::implementation
{
    struct LocalCoronaClient : LocalCoronaClientT<LocalCoronaClient>
    {
        LocalCoronaClient() = default;

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
        hstring FileName();
        void FileName(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct LocalCoronaClient : LocalCoronaClientT<LocalCoronaClient, implementation::LocalCoronaClient>
    {
    };
}
