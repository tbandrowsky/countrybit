#include "pch.h"
#include "ObjectFieldOptions.h"
#include "ObjectFieldOptions.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::FieldTypes ObjectFieldOptions::FundamentalType()
    {
        return fundamental_type;
    }
    void ObjectFieldOptions::FundamentalType(winrt::coronawinrt::FieldTypes const& value)
    {
        fundamental_type = value;
    }
    winrt::Windows::Foundation::Collections::IVector<hstring> ObjectFieldOptions::AllowedBaseClasses()
    {
        return allowed_base_classes;
    }
    void ObjectFieldOptions::AllowedBaseClasses(winrt::Windows::Foundation::Collections::IVector<hstring> const& value)
    {
        allowed_base_classes = value;
    }
    winrt::Windows::Foundation::Collections::IVector<hstring> ObjectFieldOptions::AllowedClasses()
    {
        return allowed_classes;
    }
    void ObjectFieldOptions::AllowedClasses(winrt::Windows::Foundation::Collections::IVector<hstring> const& value)
    {
        allowed_classes = value;
    }
}
