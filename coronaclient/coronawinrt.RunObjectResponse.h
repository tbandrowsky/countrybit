#pragma once
#include "coronawinrt.RunObjectResponse.g.h"
#include "coronawinrt.BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct RunObjectResponse : RunObjectResponseT<RunObjectResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::coronawinrt::CoronaObject data;

        RunObjectResponse() = default;

        winrt::coronawinrt::CoronaObject Data();
        void Data(winrt::coronawinrt::CoronaObject const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct RunObjectResponse : RunObjectResponseT<RunObjectResponse, implementation::RunObjectResponse>
    {
    };
}
