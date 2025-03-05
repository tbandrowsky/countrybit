#pragma once
#include "coronawinrt.QueryConditionGtEq.g.h"
#include "coronawinrt.QueryCondition.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryConditionGtEq : QueryConditionGtEqT<QueryConditionGtEq, coronawinrt::implementation::QueryCondition>
    {
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
