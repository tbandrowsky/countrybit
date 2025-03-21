#include "pch.h"
#include "QueryCondition.h"
#include "QueryCondition.g.cpp"

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
