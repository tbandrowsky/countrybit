#pragma once
#include "coronawinrt.LoginUserRequest.g.h"
#include "coronawinrt.BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct LoginUserRequest : LoginUserRequestT<LoginUserRequest, coronawinrt::implementation::BaseRequest>
    {
        LoginUserRequest() = default;

        hstring UserName();
        void UserName(hstring const& value);
        hstring Password();
        void Password(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct LoginUserRequest : LoginUserRequestT<LoginUserRequest, implementation::LoginUserRequest>
    {
    };
}
