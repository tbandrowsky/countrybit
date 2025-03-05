#pragma once
#include "coronawinrt.QueryObjectsResponse.g.h"
#include "coronawinrt.BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryObjectsResponse : QueryObjectsResponseT<QueryObjectsResponse, coronawinrt::implementation::BaseResponse>
    {
        QueryObjectsResponse() = default;

        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaBaseObject> Data();
        void Data(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaBaseObject> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryObjectsResponse : QueryObjectsResponseT<QueryObjectsResponse, implementation::QueryObjectsResponse>
    {
    };
}
