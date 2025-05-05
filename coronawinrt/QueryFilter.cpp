#include "pch.h"
#include "QueryFilter.h"
#include "QueryFilter.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::QueryCondition QueryFilter::Condition()
    {
        return condition;
    }
    void QueryFilter::Condition(winrt::coronawinrt::QueryCondition const& value)
    {
        condition = value;
    }
}
