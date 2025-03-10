#pragma once
#include "coronawinrt.LoginUserResponse.g.h"
#include "coronawinrt.BaseResponse.h"

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
