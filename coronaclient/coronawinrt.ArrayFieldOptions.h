#pragma once
#include "coronawinrt.ArrayFieldOptions.g.h"
#include "coronawinrt.FieldOptions.h"

namespace winrt::coronawinrt::implementation
{
    struct ArrayFieldOptions : ArrayFieldOptionsT<ArrayFieldOptions, coronawinrt::implementation::FieldOptions>
    {
        ArrayFieldOptions() = default;

        winrt::coronawinrt::FieldTypes FundamentalType();
        void FundamentalType(winrt::coronawinrt::FieldTypes const& value);
        winrt::Windows::Foundation::Collections::IVector<hstring> AllowedBaseClasses();
        void AllowedBaseClasses(winrt::Windows::Foundation::Collections::IVector<hstring> const& value);
        winrt::Windows::Foundation::Collections::IVector<hstring> AllowedClasses();
        void AllowedClasses(winrt::Windows::Foundation::Collections::IVector<hstring> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct ArrayFieldOptions : ArrayFieldOptionsT<ArrayFieldOptions, implementation::ArrayFieldOptions>
    {
    };
}
