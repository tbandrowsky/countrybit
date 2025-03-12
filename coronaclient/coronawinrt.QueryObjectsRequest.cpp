#include "pch.h"
#include "coronawinrt.QueryObjectsRequest.h"
#include "coronawinrt.QueryObjectsRequest.g.cpp"

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
