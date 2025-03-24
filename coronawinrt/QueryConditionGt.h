#pragma once
#include "QueryConditionGt.g.h"
#include "QueryCondition.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryConditionGt : QueryConditionGtT<QueryConditionGt, coronawinrt::implementation::QueryCondition>
    {
        hstring value_path;
        hstring value;

        QueryConditionGt() = default;

        hstring ValuePath();
        void ValuePath(hstring const& value);
        hstring Value();
        void Value(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryConditionGt : QueryConditionGtT<QueryConditionGt, implementation::QueryConditionGt>
    {
    };
}
