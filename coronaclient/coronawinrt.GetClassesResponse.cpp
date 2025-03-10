#include "pch.h"
#include "coronawinrt.GetClassesResponse.h"
#include "coronawinrt.GetClassesResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> GetClassesResponse::Classes()
    {
        return classes;
    }
    void GetClassesResponse::Classes(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> const& value)
    {
        classes = value;
    }
}
