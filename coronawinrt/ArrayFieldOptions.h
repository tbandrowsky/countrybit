#pragma once
#include "ArrayFieldOptions.g.h"
#include "FieldOptions.h"

namespace winrtf = winrt::Windows::Foundation::Collections;

namespace winrt::coronawinrt::implementation
{
    struct ArrayFieldOptions : ArrayFieldOptionsT<ArrayFieldOptions, coronawinrt::implementation::FieldOptions>
    {
        winrt::coronawinrt::FieldTypes                            fundamental_type;
        winrtf::IVector<hstring> allowed_classes{ winrt::single_threaded_observable_vector<hstring>() };
        winrtf::IVector<hstring> allowed_base_classes{ winrt::single_threaded_observable_vector<hstring>() };

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
