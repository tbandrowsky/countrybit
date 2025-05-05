#include "pch.h"
#include "Int64FieldOptions.h"
#include "Int64FieldOptions.g.cpp"

namespace winrt::coronawinrt::implementation
{
    int64_t Int64FieldOptions::MinValue()
    {
        return min_value;
    }
    void Int64FieldOptions::MinValue(int64_t value)
    {
        min_value = value;
    }
    int64_t Int64FieldOptions::MaxValue()
    {
        return max_value;
    }
    void Int64FieldOptions::MaxValue(int64_t value)
    {
        min_value = value;
    }
    hstring Int64FieldOptions::Message()
    {
        return message;
    }
    void Int64FieldOptions::Message(hstring const& value)
    {
        message = value;
    }
}
