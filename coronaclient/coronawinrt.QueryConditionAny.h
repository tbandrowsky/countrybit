#pragma once
#include "coronawinrt.QueryConditionAny.g.h"
#include "coronawinrt.QueryCondition.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryConditionAny : QueryConditionAnyT<QueryConditionAny, coronawinrt::implementation::QueryCondition>
    {
        hstring value_path;
        hstring value;
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> conditions;

        QueryConditionAny() = default;

        hstring ValuePath();
        void ValuePath(hstring const& value);
        hstring Value();
        void Value(hstring const& value);
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> Conditions();
        void Conditions(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryConditionAny : QueryConditionAnyT<QueryConditionAny, implementation::QueryConditionAny>
    {
    };
}
