#include "pch.h"
#include "FieldDefinition.h"
#include "FieldDefinition.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::FieldTypes FieldDefinition::FieldType()
    {
        return field_type;
    }
    void FieldDefinition::FieldType(winrt::coronawinrt::FieldTypes const& value)
    {
        field_type = value;
    }
    hstring FieldDefinition::FieldName()
    {
        return field_name;
    }
    void FieldDefinition::FieldName(hstring const& value)
    {
        field_name = value;
    }
    hstring FieldDefinition::FieldDescription()
    {
        return field_description;
    }
    void FieldDefinition::FieldDescription(hstring const& value)
    {
        field_description = value;
    }
    winrt::coronawinrt::FieldOptions FieldDefinition::FieldOptions()
    {
        return field_options;
    }
    void FieldDefinition::FieldOptions(winrt::coronawinrt::FieldOptions const& value)
    {
        field_options = value; 
    }
}
