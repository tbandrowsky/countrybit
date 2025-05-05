#pragma once
#include "QueryFilter.g.h"
#include "QueryStage.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryFilter : QueryFilterT<QueryFilter, coronawinrt::implementation::QueryStage>
    {
        winrt::coronawinrt::QueryCondition condition;

        QueryFilter() = default;

        winrt::coronawinrt::QueryCondition Condition();
        void Condition(winrt::coronawinrt::QueryCondition const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryFilter : QueryFilterT<QueryFilter, implementation::QueryFilter>
    {
    };
}
