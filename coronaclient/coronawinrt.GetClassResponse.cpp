#include "pch.h"
#include "coronawinrt.GetClassResponse.h"
#include "coronawinrt.GetClassResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> GetClassResponse::Classes()
    {
        throw hresult_not_implemented();
    }
    void GetClassResponse::Classes(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> const& value)
    {
        throw hresult_not_implemented();
    }
}
