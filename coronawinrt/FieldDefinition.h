#pragma once
#include "FieldDefinition.g.h"

namespace winrt::coronawinrt::implementation
{
    struct FieldDefinition : FieldDefinitionT<FieldDefinition>
    {
        FieldDefinition() = default;

        winrt::coronawinrt::FieldTypes field_type;
        hstring field_name;
        hstring field_description;
        winrt::coronawinrt::FieldOptions field_options;

        winrt::coronawinrt::FieldTypes FieldType();
        void FieldType(winrt::coronawinrt::FieldTypes const& value);
        hstring FieldName();
        void FieldName(hstring const& value);
        hstring FieldDescription();
        void FieldDescription(hstring const& value);
        winrt::coronawinrt::FieldOptions FieldOptions();
        void FieldOptions(winrt::coronawinrt::FieldOptions const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct FieldDefinition : FieldDefinitionT<FieldDefinition, implementation::FieldDefinition>
    {
    };
}
