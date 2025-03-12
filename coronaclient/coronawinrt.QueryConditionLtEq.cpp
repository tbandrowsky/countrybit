#include "pch.h"
#include "coronawinrt.QueryConditionLtEq.h"
#include "coronawinrt.QueryConditionLtEq.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryConditionLtEq::ValuePath()
    {
        return value_path;
    }
    void QueryConditionLtEq::ValuePath(hstring const& value)
    {
        value_path = value;
    }
    hstring QueryConditionLtEq::Value()
    {
        return value;
    }
    void QueryConditionLtEq::Value(hstring const& value)
    {
        this->value = value;
    }
}
