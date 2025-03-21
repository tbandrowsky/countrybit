#pragma once
#include "PutObjectsResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct PutObjectsResponse : PutObjectsResponseT<PutObjectsResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> data;

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
