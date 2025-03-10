#pragma once
#include "coronawinrt.GetObjectResponse.g.h"
#include "coronawinrt.BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct GetObjectResponse : GetObjectResponseT<GetObjectResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::coronawinrt::CoronaObject data;

        GetObjectResponse() = default;

        winrt::coronawinrt::CoronaObject Data();
        void Data(winrt::coronawinrt::CoronaObject const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct GetObjectResponse : GetObjectResponseT<GetObjectResponse, implementation::GetObjectResponse>
    {
    };
}
