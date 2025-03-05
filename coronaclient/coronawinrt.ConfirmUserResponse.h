#pragma once
#include "coronawinrt.ConfirmUserResponse.g.h"
#include "coronawinrt.BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct ConfirmUserResponse : ConfirmUserResponseT<ConfirmUserResponse, coronawinrt::implementation::BaseResponse>
    {
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
