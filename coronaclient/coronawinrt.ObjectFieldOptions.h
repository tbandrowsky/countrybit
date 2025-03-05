#pragma once
#include "coronawinrt.ObjectFieldOptions.g.h"
#include "coronawinrt.FieldOptions.h"

namespace winrt::coronawinrt::implementation
{
    struct ObjectFieldOptions : ObjectFieldOptionsT<ObjectFieldOptions, coronawinrt::implementation::FieldOptions>
    {
        ObjectFieldOptions() = default;

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
    struct ObjectFieldOptions : ObjectFieldOptionsT<ObjectFieldOptions, implementation::ObjectFieldOptions>
    {
    };
}
