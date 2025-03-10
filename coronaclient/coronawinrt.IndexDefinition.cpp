#include "pch.h"
#include "coronawinrt.IndexDefinition.h"
#include "coronawinrt.IndexDefinition.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring IndexDefinition::IndexName()
    {
        return index_name;
    }
    void IndexDefinition::IndexName(hstring const& value)
    {
        index_name = value;
    }
    winrt::Windows::Foundation::Collections::IVector<hstring> IndexDefinition::IndexKeys()
    {
        return index_keys;
    }
    void IndexDefinition::IndexKeys(winrt::Windows::Foundation::Collections::IVector<hstring> const& value)
    {
        index_keys = value;
    }
}
