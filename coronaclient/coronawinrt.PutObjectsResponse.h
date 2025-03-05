#pragma once
#include "coronawinrt.PutObjectsResponse.g.h"
#include "coronawinrt.BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct PutObjectsResponse : PutObjectsResponseT<PutObjectsResponse, coronawinrt::implementation::BaseResponse>
    {
        PutObjectsResponse() = default;

        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> Data();
        void Data(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct PutObjectsResponse : PutObjectsResponseT<PutObjectsResponse, implementation::PutObjectsResponse>
    {
    };
}
