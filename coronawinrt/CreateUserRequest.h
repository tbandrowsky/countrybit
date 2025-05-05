#pragma once
#include "CreateUserRequest.g.h"
#include "BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct CreateUserRequest : CreateUserRequestT<CreateUserRequest, coronawinrt::implementation::BaseRequest>
    {
        hstring username;
        hstring email;
        hstring address1;
        hstring address2;
        hstring city;
        hstring state;
        hstring zip;
        hstring password1;
        hstring password2;

        CreateUserRequest() = default;

        hstring Username();
        void Username(hstring const& value);
        hstring Email();
        void Email(hstring const& value);
        hstring Address1();
        void Address1(hstring const& value);
        hstring Address2();
        void Address2(hstring const& value);
        hstring City();
        void City(hstring const& value);
        hstring State();
        void State(hstring const& value);
        hstring Zip();
        void Zip(hstring const& value);
        hstring Password1();
        void Password1(hstring const& value);
        hstring Password2();
        void Password2(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct CreateUserRequest : CreateUserRequestT<CreateUserRequest, implementation::CreateUserRequest>
    {
    };
}
