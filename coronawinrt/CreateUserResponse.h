#pragma once
#include "CreateUserResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct CreateUserResponse : CreateUserResponseT<CreateUserResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::coronawinrt::CoronaUser data;

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
