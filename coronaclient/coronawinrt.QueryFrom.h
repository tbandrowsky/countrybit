#pragma once
#include "coronawinrt.QueryFrom.g.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryFrom : QueryFromT<QueryFrom>
    {
        QueryFrom() = default;

        hstring ClassName();
        void ClassName(hstring const& value);
        hstring Name();
        void Name(hstring const& value);
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::BasicFilter> Filters();
        void Filters(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::BasicFilter> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryFrom : QueryFromT<QueryFrom, implementation::QueryFrom>
    {
    };
}
