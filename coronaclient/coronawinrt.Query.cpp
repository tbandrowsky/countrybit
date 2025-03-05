#include "pch.h"
#include "coronawinrt.Query.h"
#include "coronawinrt.Query.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::QueryFrom Query::From()
    {
        throw hresult_not_implemented();
    }
    void Query::From(winrt::coronawinrt::QueryFrom const& value)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryStage> Query::Stages()
    {
        throw hresult_not_implemented();
    }
    void Query::Stages(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryStage> const& value)
    {
        throw hresult_not_implemented();
    }
}
