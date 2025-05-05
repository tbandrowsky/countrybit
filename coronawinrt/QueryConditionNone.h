#pragma once
#include "QueryConditionNone.g.h"
#include "QueryCondition.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryConditionNone : QueryConditionNoneT<QueryConditionNone, coronawinrt::implementation::QueryCondition>
    {
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> conditions;

        QueryConditionNone() = default;

        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> Conditions();
        void Conditions(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryConditionNone : QueryConditionNoneT<QueryConditionNone, implementation::QueryConditionNone>
    {
    };
}
