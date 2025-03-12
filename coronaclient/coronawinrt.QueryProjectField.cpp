#include "pch.h"
#include "coronawinrt.QueryProjectField.h"
#include "coronawinrt.QueryProjectField.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryProjectField::FieldName()
    {
        return field_name;
    }
    void QueryProjectField::FieldName(hstring const& value)
    {
        field_name = value;
    }
    hstring QueryProjectField::ValuePath()
    {
        return value_path;
    }
    void QueryProjectField::ValuePath(hstring const& value)
    {
        value_path = value;
    }
}
