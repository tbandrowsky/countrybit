#pragma once
#include "coronawinrt.IndexDefinition.g.h"

namespace winrt::coronawinrt::implementation
{
    struct IndexDefinition : IndexDefinitionT<IndexDefinition>
    {
        IndexDefinition() = default;

        hstring IndexName();
        void IndexName(hstring const& value);
        winrt::Windows::Foundation::Collections::IVector<hstring> IndexKeys();
        void IndexKeys(winrt::Windows::Foundation::Collections::IVector<hstring> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct IndexDefinition : IndexDefinitionT<IndexDefinition, implementation::IndexDefinition>
    {
    };
}
