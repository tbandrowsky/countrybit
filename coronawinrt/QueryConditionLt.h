#pragma once
#include "QueryConditionLt.g.h"
#include "QueryCondition.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryConditionLt : QueryConditionLtT<QueryConditionLt, coronawinrt::implementation::QueryCondition>
    {
        hstring value_path;
        hstring value;

        QueryConditionLt() = default;

        hstring ValuePath();
        void ValuePath(hstring const& value);
        hstring Value();
        void Value(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryConditionLt : QueryConditionLtT<QueryConditionLt, implementation::QueryConditionLt>
    {
    };
}
