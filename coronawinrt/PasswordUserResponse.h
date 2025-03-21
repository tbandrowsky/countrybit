#pragma once
#include "PasswordUserResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct PasswordUserResponse : PasswordUserResponseT<PasswordUserResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::coronawinrt::CoronaUser data;

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
