#include "pch.h"
#include "coronawinrt.QueryCondition.h"
#include "coronawinrt.QueryCondition.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryCondition::ClassName()
    {
        return class_name;
    }
    void QueryCondition::ClassName(hstring const& value)
    {
        class_name = value;
    }
}
