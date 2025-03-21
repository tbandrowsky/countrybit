#pragma once
#include "EditObjectResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct EditObjectResponse : EditObjectResponseT<EditObjectResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::coronawinrt::CoronaObject data;

        EditObjectResponse() = default;

        winrt::coronawinrt::CoronaObject Data();
        void Data(winrt::coronawinrt::CoronaObject const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct EditObjectResponse : EditObjectResponseT<EditObjectResponse, implementation::EditObjectResponse>
    {
    };
}
