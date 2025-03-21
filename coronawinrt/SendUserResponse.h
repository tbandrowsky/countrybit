#pragma once
#include "SendUserResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct SendUserResponse : SendUserResponseT<SendUserResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::coronawinrt::CoronaUser data;

        SendUserResponse() = default;

        winrt::coronawinrt::CoronaUser Data();
        void Data(winrt::coronawinrt::CoronaUser const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct SendUserResponse : SendUserResponseT<SendUserResponse, implementation::SendUserResponse>
    {
    };
}
