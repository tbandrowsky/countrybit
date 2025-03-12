#include "pch.h"
#include "coronawinrt.QueryConditionEq.h"
#include "coronawinrt.QueryConditionEq.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryConditionEq::ValuePath()
    {
        return value_path;
    }
    void QueryConditionEq::ValuePath(hstring const& value)
    {
        value_path = value;
    }
    hstring QueryConditionEq::Value()
    {
        return value;
    }
    void QueryConditionEq::Value(hstring const& value)
    {
        this->value = value;
    }
}
