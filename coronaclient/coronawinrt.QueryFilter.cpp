#include "pch.h"
#include "coronawinrt.QueryFilter.h"
#include "coronawinrt.QueryFilter.g.cpp"

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
