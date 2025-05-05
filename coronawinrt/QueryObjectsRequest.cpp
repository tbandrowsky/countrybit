#include "pch.h"
#include "QueryObjectsRequest.h"
#include "QueryObjectsRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::Query QueryObjectsRequest::QueryBody()
    {
        return query_body;
    }
    void QueryObjectsRequest::QueryBody(winrt::coronawinrt::Query const& value)
    {
        query_body = value;
    }
}
