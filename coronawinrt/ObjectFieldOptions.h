#pragma once
#include "ObjectFieldOptions.g.h"
#include "FieldOptions.h"

namespace winrtf = winrt::Windows::Foundation::Collections;

namespace winrt::coronawinrt::implementation
{
    struct ObjectFieldOptions : ObjectFieldOptionsT<ObjectFieldOptions, coronawinrt::implementation::FieldOptions>
    {
        ObjectFieldOptions() = default;

        winrt::coronawinrt::FieldTypes                            fundamental_type;
        winrtf::IVector<hstring> allowed_classes{ winrt::single_threaded_observable_vector<hstring>() };
        winrtf::IVector<hstring> allowed_base_classes{ winrt::single_threaded_observable_vector<hstring>() };

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
