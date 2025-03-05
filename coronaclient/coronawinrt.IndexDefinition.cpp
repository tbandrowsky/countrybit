#include "pch.h"
#include "coronawinrt.IndexDefinition.h"
#include "coronawinrt.IndexDefinition.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring IndexDefinition::IndexName()
    {
        throw hresult_not_implemented();
    }
    void IndexDefinition::IndexName(hstring const& value)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::Collections::IVector<hstring> IndexDefinition::IndexKeys()
    {
        throw hresult_not_implemented();
    }
    void IndexDefinition::IndexKeys(winrt::Windows::Foundation::Collections::IVector<hstring> const& value)
    {
        throw hresult_not_implemented();
    }
}
