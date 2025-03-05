#pragma once
#include "coronawinrt.CreateObjectResponse.g.h"
#include "coronawinrt.BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct CreateObjectResponse : CreateObjectResponseT<CreateObjectResponse, coronawinrt::implementation::BaseResponse>
    {
        CreateObjectResponse() = default;

        winrt::coronawinrt::CoronaObject Data();
        void Data(winrt::coronawinrt::CoronaObject const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct CreateObjectResponse : CreateObjectResponseT<CreateObjectResponse, implementation::CreateObjectResponse>
    {
    };
}
