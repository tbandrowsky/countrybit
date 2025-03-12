#include "pch.h"
#include "coronawinrt.QueryFieldOptions.h"
#include "coronawinrt.QueryFieldOptions.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::Query QueryFieldOptions::QueryBody()
    {
        return query_body;
    }
    void QueryFieldOptions::QueryBody(winrt::coronawinrt::Query const& value)
    {
        query_body = value;
    }
}
