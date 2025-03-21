#include "pch.h"
#include "GetClassResponse.h"
#include "GetClassResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> GetClassResponse::Classes()
    {
        return classes;
    }
    void GetClassResponse::Classes(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> const& value)
    {
        classes = value;
    }
}
