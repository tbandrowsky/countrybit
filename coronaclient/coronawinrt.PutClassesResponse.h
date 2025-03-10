#pragma once
#include "coronawinrt.PutClassesResponse.g.h"
#include "coronawinrt.BaseResponse.h"


namespace winrt::coronawinrt::implementation
{
    struct PutClassesResponse : PutClassesResponseT<PutClassesResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::coronawinrt::CoronaObject data;

        PutClassesResponse() = default;

        winrt::coronawinrt::CoronaObject Data();
        void Data(winrt::coronawinrt::CoronaObject const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct PutClassesResponse : PutClassesResponseT<PutClassesResponse, implementation::PutClassesResponse>
    {
    };
}
