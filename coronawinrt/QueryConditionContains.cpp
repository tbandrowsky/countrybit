#include "pch.h"
#include "QueryConditionContains.h"
#include "QueryConditionContains.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryConditionContains::ValuePath()
    {
        return value_path;
    }
    void QueryConditionContains::ValuePath(hstring const& value)
    {
        value_path = value;
    }
    hstring QueryConditionContains::Value()
    {
        return value;
    }
    void QueryConditionContains::Value(hstring const& value)
    {
        this->value = value;
    }
}
