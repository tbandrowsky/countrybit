#include "pch.h"
#include "coronawinrt.ArrayFieldOptions.h"
#include "coronawinrt.ArrayFieldOptions.g.cpp"


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

void* winrt_make_coronawinrt_ArrayFieldOptions()
{
    return winrt::detach_abi(winrt::make<winrt::coronawinrt::factory_implementation::ArrayFieldOptions>());
}
WINRT_EXPORT namespace winrt::coronawinrt
{
    ArrayFieldOptions::ArrayFieldOptions() :
        ArrayFieldOptions(make<coronawinrt::implementation::ArrayFieldOptions>())
    {
    }
}
