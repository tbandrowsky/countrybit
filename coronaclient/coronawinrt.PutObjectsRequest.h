#pragma once
#include "coronawinrt.PutObjectsRequest.g.h"
#include "coronawinrt.BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct PutObjectsRequest : PutObjectsRequestT<PutObjectsRequest, coronawinrt::implementation::BaseRequest>
    {
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> data;

        PutObjectsRequest() = default;

        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> Data();
        void Data(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct PutObjectsRequest : PutObjectsRequestT<PutObjectsRequest, implementation::PutObjectsRequest>
    {
    };
}
