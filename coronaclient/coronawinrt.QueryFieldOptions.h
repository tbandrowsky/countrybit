#pragma once
#include "coronawinrt.QueryFieldOptions.g.h"
#include "coronawinrt.FieldOptions.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryFieldOptions : QueryFieldOptionsT<QueryFieldOptions, coronawinrt::implementation::FieldOptions>
    {

        winrt::coronawinrt::Query query_body;

        QueryFieldOptions() = default;

        winrt::coronawinrt::Query QueryBody();
        void QueryBody(winrt::coronawinrt::Query const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryFieldOptions : QueryFieldOptionsT<QueryFieldOptions, implementation::QueryFieldOptions>
    {
    };
}
