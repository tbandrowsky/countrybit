#pragma once
#include "coronawinrt.DateTimeFieldOptions.g.h"
#include "coronawinrt.FieldOptions.h"

namespace winrt::coronawinrt::implementation
{
    struct DateTimeFieldOptions : DateTimeFieldOptionsT<DateTimeFieldOptions, coronawinrt::implementation::FieldOptions>
    {

        winrt::Windows::Foundation::DateTime min_value;
        winrt::Windows::Foundation::DateTime max_value;
        hstring message;

        DateTimeFieldOptions() = default;

        winrt::Windows::Foundation::DateTime MinValue();
        void MinValue(winrt::Windows::Foundation::DateTime const& value);
        winrt::Windows::Foundation::DateTime MaxValue();
        void MaxValue(winrt::Windows::Foundation::DateTime const& value);
        hstring Message();
        void Message(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct DateTimeFieldOptions : DateTimeFieldOptionsT<DateTimeFieldOptions, implementation::DateTimeFieldOptions>
    {
    };
}
