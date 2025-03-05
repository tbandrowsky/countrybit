#include "pch.h"
#include "coronawinrt.GetClassesResponse.h"
#include "coronawinrt.GetClassesResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> GetClassesResponse::Classes()
    {
        throw hresult_not_implemented();
    }
    void GetClassesResponse::Classes(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> const& value)
    {
        throw hresult_not_implemented();
    }
}
