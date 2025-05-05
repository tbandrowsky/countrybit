#include "pch.h"
#include "QueryConditionAll.h"
#include "QueryConditionAll.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryConditionAll::ValuePath()
    {
        return value_path;
    }
    void QueryConditionAll::ValuePath(hstring const& value)
    {
        value_path = value;
    }
    hstring QueryConditionAll::Value()
    {
        return value;
    }
    void QueryConditionAll::Value(hstring const& value)
    {
        this->value = value;
    }
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> QueryConditionAll::Conditions()
    {
        return conditions;
    }
    void QueryConditionAll::Conditions(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> const& value)
    {
        conditions = value;
    }
}
