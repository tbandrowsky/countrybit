#pragma once
#include "coronawinrt.CopyObjectResponse.g.h"
#include "coronawinrt.BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct CopyObjectResponse : CopyObjectResponseT<CopyObjectResponse, coronawinrt::implementation::BaseResponse>
    {
        CopyObjectResponse() = default;

        winrt::coronawinrt::CoronaObject Data();
        void Data(winrt::coronawinrt::CoronaObject const& value);
    };
}
