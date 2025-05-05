#pragma once
#include "CopyObjectResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct CopyObjectResponse : CopyObjectResponseT<CopyObjectResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::coronawinrt::CoronaObject data;

        CopyObjectResponse() = default;

        winrt::coronawinrt::CoronaObject Data();
        void Data(winrt::coronawinrt::CoronaObject const& value);
    };
}
