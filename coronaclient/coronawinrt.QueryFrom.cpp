#include "pch.h"
#include "coronawinrt.QueryFrom.h"
#include "coronawinrt.QueryFrom.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryFrom::ClassName()
    {
        return class_name;
    }
    void QueryFrom::ClassName(hstring const& value)
    {
        class_name = value;
    }
    hstring QueryFrom::Name()
    {
        return name;
    }
    void QueryFrom::Name(hstring const& value)
    {
        name = value;
    }
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::BasicFilter> QueryFrom::Filters()
    {
        return filters;
    }
    void QueryFrom::Filters(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::BasicFilter> const& value)
    {
        filters = value;
    }
}
