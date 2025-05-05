#pragma once
#include "ConfirmUserResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct ConfirmUserResponse : ConfirmUserResponseT<ConfirmUserResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::coronawinrt::CoronaUser data;

        ConfirmUserResponse() = default;

        winrt::coronawinrt::CoronaUser Data();
        void Data(winrt::coronawinrt::CoronaUser const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct ConfirmUserResponse : ConfirmUserResponseT<ConfirmUserResponse, implementation::ConfirmUserResponse>
    {
    };
}
