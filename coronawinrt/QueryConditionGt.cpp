#include "pch.h"
#include "QueryConditionGt.h"
#include "QueryConditionGt.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryConditionGt::ValuePath()
    {
        return value_path;
    }
    void QueryConditionGt::ValuePath(hstring const& value)
    {
        value_path = value;
    }
    hstring QueryConditionGt::Value()
    {
        return value;;
    }
    void QueryConditionGt::Value(hstring const& value)
    {
        this->value = value;        
    }
}
