#include "pch.h"
#include "BasicFilter.h"
#include "BasicFilter.g.cpp"


namespace winrt::coronawinrt::implementation
{
    hstring BasicFilter::FieldName()
    {
        return field_name;
    }
    void BasicFilter::FieldName(hstring const& value)
    {
        field_name = value;
    }
    hstring BasicFilter::FieldValue()
    {
        return field_value;
    }
    void BasicFilter::FieldValue(hstring const& value)
    {
        field_value = value;
    }
}
