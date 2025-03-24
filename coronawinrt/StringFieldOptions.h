#pragma once
#include "StringFieldOptions.g.h"
#include "FieldOptions.h"

namespace winrt::coronawinrt::implementation
{
    struct StringFieldOptions : StringFieldOptionsT<StringFieldOptions, coronawinrt::implementation::FieldOptions>
    {
        StringFieldOptions() = default;

        int64_t MinLength();
        void MinLength(int64_t value);
        int64_t MaxLength();
        void MaxLength(int64_t value);
        hstring Message();
        void Message(hstring const& value);
        hstring RegularExpression();
        void RegularExpression(hstring const& value);
        winrt::Windows::Foundation::Collections::IVector<hstring> AllowedValues();
        void AllowedValues(winrt::Windows::Foundation::Collections::IVector<hstring> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct StringFieldOptions : StringFieldOptionsT<StringFieldOptions, implementation::StringFieldOptions>
    {
    };
}
