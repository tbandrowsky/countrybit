#pragma once
#include "QueryConditionGtEq.g.h"
#include "QueryCondition.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryConditionGtEq : QueryConditionGtEqT<QueryConditionGtEq, coronawinrt::implementation::QueryCondition>
    {
        hstring value_path;
        hstring value;

        QueryConditionGtEq() = default;

        hstring ValuePath();
        void ValuePath(hstring const& value);
        hstring Value();
        void Value(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryConditionGtEq : QueryConditionGtEqT<QueryConditionGtEq, implementation::QueryConditionGtEq>
    {
    };
}
