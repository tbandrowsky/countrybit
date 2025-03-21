#include "pch.h"
#include "DoubleFieldOptions.h"
#include "DoubleFieldOptions.g.cpp"

namespace winrt::coronawinrt::implementation
{
    double DoubleFieldOptions::MinValue()
    {
        return min_value;
    }
    void DoubleFieldOptions::MinValue(double value)
    {
        min_value = value;
    }
    double DoubleFieldOptions::MaxValue()
    {
        return max_value;
    }
    void DoubleFieldOptions::MaxValue(double value)
    {
        max_value = value;
    }
    hstring DoubleFieldOptions::Message()
    {
        return message;
    }
    void DoubleFieldOptions::Message(hstring const& value)
    {
        message = value;
    }
}
