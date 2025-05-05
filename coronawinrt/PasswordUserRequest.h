#pragma once
#include "PasswordUserRequest.g.h"
#include "BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct PasswordUserRequest : PasswordUserRequestT<PasswordUserRequest, coronawinrt::implementation::BaseRequest>
    {
        PasswordUserRequest() = default;

        hstring user_name;
        hstring validation_code;
        hstring password1;
        hstring password2;

        hstring UserName();
        void UserName(hstring const& value);
        hstring ValidationCode();
        void ValidationCode(hstring const& value);
        hstring Password1();
        void Password1(hstring const& value);
        hstring Password2();
        void Password2(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct PasswordUserRequest : PasswordUserRequestT<PasswordUserRequest, implementation::PasswordUserRequest>
    {
    };
}
