#pragma once
#include "QueryConditionEq.g.h"
#include "QueryCondition.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryConditionEq : QueryConditionEqT<QueryConditionEq, coronawinrt::implementation::QueryCondition>
    {
        hstring value_path;
        hstring value;

        QueryConditionEq() = default;

        hstring ValuePath();
        void ValuePath(hstring const& value);
        hstring Value();
        void Value(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryConditionEq : QueryConditionEqT<QueryConditionEq, implementation::QueryConditionEq>
    {
    };
}
