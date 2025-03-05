#pragma once
#include "coronawinrt.CreateUserResponse.g.h"
#include "coronawinrt.BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct CreateUserResponse : CreateUserResponseT<CreateUserResponse, coronawinrt::implementation::BaseResponse>
    {
        CreateUserResponse() = default;

        winrt::coronawinrt::CoronaUser Data();
        void Data(winrt::coronawinrt::CoronaUser const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct CreateUserResponse : CreateUserResponseT<CreateUserResponse, implementation::CreateUserResponse>
    {
    };
}
