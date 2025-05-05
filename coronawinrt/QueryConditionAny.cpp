#include "pch.h"
#include "QueryConditionAny.h"
#include "QueryConditionAny.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryConditionAny::ValuePath()
    {
        return value_path;
    }
    void QueryConditionAny::ValuePath(hstring const& value)
    {
        value_path = value;
    }
    hstring QueryConditionAny::Value()
    {
        return value;
    }
    void QueryConditionAny::Value(hstring const& value)
    {
        this->value = value;
    }
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> QueryConditionAny::Conditions()
    {
        return conditions;
    }
    void QueryConditionAny::Conditions(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> const& value)
    {
        conditions = value;
    }
}
