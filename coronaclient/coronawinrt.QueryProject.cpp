#include "pch.h"
#include "coronawinrt.QueryProject.h"
#include "coronawinrt.QueryProject.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryProject::SourceName()
    {
        return source_name;
    }
    void QueryProject::SourceName(hstring const& value)
    {
        source_name = value;
    }
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryProjectField> QueryProject::Projection()
    {
        return projection;
    }
    void QueryProject::Projection(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryProjectField> const& value)
    {
        projection = value;
    }
}
