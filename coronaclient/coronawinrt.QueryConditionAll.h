#pragma once
#include "coronawinrt.QueryConditionAll.g.h"
#include "coronawinrt.QueryCondition.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryConditionAll : QueryConditionAllT<QueryConditionAll, coronawinrt::implementation::QueryCondition>
    {
        hstring value_path;
        hstring value;
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> conditions;

        QueryConditionAll() = default;

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
    struct QueryConditionAll : QueryConditionAllT<QueryConditionAll, implementation::QueryConditionAll>
    {
    };
}
