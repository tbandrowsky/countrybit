#pragma once
#include "BasicFilter.g.h"

namespace winrt::coronawinrt::implementation
{
    struct BasicFilter : BasicFilterT<BasicFilter>
    {
        hstring field_name;
        hstring field_value;

        BasicFilter() = default;

        hstring FieldName();
        void FieldName(hstring const& value);
        hstring FieldValue();
        void FieldValue(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct BasicFilter : BasicFilterT<BasicFilter, implementation::BasicFilter>
    {
    };
}
