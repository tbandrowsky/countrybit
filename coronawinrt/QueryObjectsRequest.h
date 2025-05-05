#pragma once
#include "QueryObjectsRequest.g.h"
#include "BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryObjectsRequest : QueryObjectsRequestT<QueryObjectsRequest, coronawinrt::implementation::BaseRequest>
    {
        winrt::coronawinrt::Query query_body;

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
