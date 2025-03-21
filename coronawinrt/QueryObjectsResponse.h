#pragma once
#include "QueryObjectsResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryObjectsResponse : QueryObjectsResponseT<QueryObjectsResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaBaseObject> data;

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
