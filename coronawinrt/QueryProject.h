#pragma once
#include "QueryProject.g.h"


namespace winrt::coronawinrt::implementation
{
    struct QueryProject : QueryProjectT<QueryProject>
    {
        hstring source_name;
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryProjectField> projection;

        QueryProject() = default;

        hstring SourceName();
        void SourceName(hstring const& value);
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryProjectField> Projection();
        void Projection(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryProjectField> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryProject : QueryProjectT<QueryProject, implementation::QueryProject>
    {
    };
}
