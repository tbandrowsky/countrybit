#include "pch.h"
#include "QueryConditionNone.h"
#include "QueryConditionNone.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> QueryConditionNone::Conditions()
    {
        return conditions;
    }
    void QueryConditionNone::Conditions(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> const& value)
    {
        conditions = value;
    }
}
