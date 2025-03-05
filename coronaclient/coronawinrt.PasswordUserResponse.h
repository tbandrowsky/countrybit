#pragma once
#include "coronawinrt.PasswordUserResponse.g.h"
#include "coronawinrt.BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct PasswordUserResponse : PasswordUserResponseT<PasswordUserResponse, coronawinrt::implementation::BaseResponse>
    {
        PasswordUserResponse() = default;

        winrt::coronawinrt::CoronaUser Data();
        void Data(winrt::coronawinrt::CoronaUser const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct PasswordUserResponse : PasswordUserResponseT<PasswordUserResponse, implementation::PasswordUserResponse>
    {
    };
}
