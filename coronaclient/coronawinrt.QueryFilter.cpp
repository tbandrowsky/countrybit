#include "pch.h"
#include "coronawinrt.QueryFilter.h"
#include "coronawinrt.QueryFilter.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::QueryCondition QueryFilter::Condition()
    {
        throw hresult_not_implemented();
    }
    void QueryFilter::Condition(winrt::coronawinrt::QueryCondition const& value)
    {
        throw hresult_not_implemented();
    }
}
