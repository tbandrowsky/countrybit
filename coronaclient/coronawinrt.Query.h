#pragma once
#include "coronawinrt.Query.g.h"

namespace winrt::coronawinrt::implementation
{
    struct Query : QueryT<Query>
    {
        winrt::coronawinrt::QueryFrom from;
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryStage> stages;

        Query() = default;

        winrt::coronawinrt::QueryFrom From();
        void From(winrt::coronawinrt::QueryFrom const& value);
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryStage> Stages();
        void Stages(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::QueryStage> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct Query : QueryT<Query, implementation::Query>
    {
    };
}
