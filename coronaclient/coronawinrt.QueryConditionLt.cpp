#include "pch.h"
#include "coronawinrt.QueryConditionLt.h"
#include "coronawinrt.QueryConditionLt.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryConditionLt::ValuePath()
    {
        return value_path;
    }
    void QueryConditionLt::ValuePath(hstring const& value)
    {
        value_path = value;
    }
    hstring QueryConditionLt::Value()
    {
        return value;
    }
    void QueryConditionLt::Value(hstring const& value)
    {
        this->value = value;
    }
}
