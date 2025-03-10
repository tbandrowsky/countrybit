#pragma once
#include "coronawinrt.Int64FieldOptions.g.h"
#include "coronawinrt.FieldOptions.h"

namespace winrt::coronawinrt::implementation
{
    struct Int64FieldOptions : Int64FieldOptionsT<Int64FieldOptions, coronawinrt::implementation::FieldOptions>
    {
        int64_t min_value;
        int64_t max_value;
        hstring message;

        Int64FieldOptions() = default;

        int64_t MinValue();
        void MinValue(int64_t value);
        int64_t MaxValue();
        void MaxValue(int64_t value);
        hstring Message();
        void Message(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct Int64FieldOptions : Int64FieldOptionsT<Int64FieldOptions, implementation::Int64FieldOptions>
    {
    };
}
