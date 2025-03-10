#include "pch.h"
#include "coronawinrt.Query.h"
#include "coronawinrt.Query.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::QueryFrom Query::From()
    {
        return from;
    }
    void Query::From(winrt::coronawinrt::QueryFrom const& value)
    {
        from = value;
    }
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryStage> Query::Stages()
    {
        return stages;
    }
    void Query::Stages(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryStage> const& value)
    {
        throw stages = value;
    }
}
