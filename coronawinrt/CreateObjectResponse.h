#pragma once
#include "CreateObjectResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct CreateObjectResponse : CreateObjectResponseT<CreateObjectResponse, coronawinrt::implementation::BaseResponse>
    {
        CoronaObject data;

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
