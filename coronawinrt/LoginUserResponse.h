#pragma once
#include "LoginUserResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct LoginUserResponse : LoginUserResponseT<LoginUserResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::coronawinrt::CoronaUser data;

        LoginUserResponse() = default;

        winrt::coronawinrt::CoronaUser Data();
        void Data(winrt::coronawinrt::CoronaUser const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct LoginUserResponse : LoginUserResponseT<LoginUserResponse, implementation::LoginUserResponse>
    {
    };
}
