#pragma once
#include "coronawinrt.QueryObjectsRequest.g.h"
#include "coronawinrt.BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryObjectsRequest : QueryObjectsRequestT<QueryObjectsRequest, coronawinrt::implementation::BaseRequest>
    {
        QueryObjectsRequest() = default;

        winrt::coronawinrt::Query QueryBody();
        void QueryBody(winrt::coronawinrt::Query const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryObjectsRequest : QueryObjectsRequestT<QueryObjectsRequest, implementation::QueryObjectsRequest>
    {
    };
}
