#pragma once
#include "QueryConditionContains.g.h"
#include "QueryCondition.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryConditionContains : QueryConditionContainsT<QueryConditionContains, coronawinrt::implementation::QueryCondition>
    {
        hstring value_path;
        hstring value;

        QueryConditionContains() = default;

        hstring ValuePath();
        void ValuePath(hstring const& value);
        hstring Value();
        void Value(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryConditionContains : QueryConditionContainsT<QueryConditionContains, implementation::QueryConditionContains>
    {
    };
}
