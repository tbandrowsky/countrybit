#include "pch.h"
#include "QueryConditionGtEq.h"
#include "QueryConditionGtEq.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryConditionGtEq::ValuePath()
    {
        return value_path;
    }
    void QueryConditionGtEq::ValuePath(hstring const& value)
    {
        value_path = value;
    }
    hstring QueryConditionGtEq::Value()
    {
        return value;
    }
    void QueryConditionGtEq::Value(hstring const& value)
    {
        this->value = value;
    }
}
