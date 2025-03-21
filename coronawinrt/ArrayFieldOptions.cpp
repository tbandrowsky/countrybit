#include "pch.h"
#include "ArrayFieldOptions.h"
#include "ArrayFieldOptions.g.cpp"


namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::FieldTypes ArrayFieldOptions::FundamentalType()
    {
        return fundamental_type;
    }
    void ArrayFieldOptions::FundamentalType(winrt::coronawinrt::FieldTypes const& value)
    {
        fundamental_type = value;
    }
    winrt::Windows::Foundation::Collections::IVector<hstring> ArrayFieldOptions::AllowedBaseClasses()
    {
        return allowed_base_classes;
    }
    void ArrayFieldOptions::AllowedBaseClasses(winrt::Windows::Foundation::Collections::IVector<hstring> const& value)
    {
        allowed_base_classes = value;
    }
    winrt::Windows::Foundation::Collections::IVector<hstring> ArrayFieldOptions::AllowedClasses()
    {
        return allowed_classes;
    }
    void ArrayFieldOptions::AllowedClasses(winrt::Windows::Foundation::Collections::IVector<hstring> const& value)
    {
        allowed_classes = value;
    }
}
