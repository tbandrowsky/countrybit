#include "pch.h"
#include "coronawinrt.QueryConditionNone.h"
#include "coronawinrt.QueryConditionNone.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> QueryConditionNone::Conditions()
    {
        throw hresult_not_implemented();
    }
    void QueryConditionNone::Conditions(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryCondition> const& value)
    {
        throw hresult_not_implemented();
    }
}
