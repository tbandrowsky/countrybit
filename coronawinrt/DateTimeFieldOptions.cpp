#include "pch.h"
#include "DateTimeFieldOptions.h"
#include "DateTimeFieldOptions.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::DateTime DateTimeFieldOptions::MinValue()
    {
        return min_value;
    }
    void DateTimeFieldOptions::MinValue(winrt::Windows::Foundation::DateTime const& value)
    {
        min_value = value;
    }
    winrt::Windows::Foundation::DateTime DateTimeFieldOptions::MaxValue()
    {
        return max_value;
    }
    void DateTimeFieldOptions::MaxValue(winrt::Windows::Foundation::DateTime const& value)
    {
        max_value = value;
    }
    hstring DateTimeFieldOptions::Message()
    {
        return message;
    }
    void DateTimeFieldOptions::Message(hstring const& value)
    {
        message = value;
    }
}
