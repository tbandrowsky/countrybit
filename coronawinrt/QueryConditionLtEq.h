#pragma once
#include "QueryConditionLtEq.g.h"
#include "QueryCondition.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryConditionLtEq : QueryConditionLtEqT<QueryConditionLtEq, coronawinrt::implementation::QueryCondition>
    {
        hstring value_path;
        hstring value;

        QueryConditionLtEq() = default;

        hstring ValuePath();
        void ValuePath(hstring const& value);
        hstring Value();
        void Value(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryConditionLtEq : QueryConditionLtEqT<QueryConditionLtEq, implementation::QueryConditionLtEq>
    {
    };
}
