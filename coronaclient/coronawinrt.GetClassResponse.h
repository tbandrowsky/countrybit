#pragma once
#include "coronawinrt.GetClassResponse.g.h"
#include "coronawinrt.BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct GetClassResponse : GetClassResponseT<GetClassResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> classes;

        GetClassResponse() = default;

        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> Classes();
        void Classes(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct GetClassResponse : GetClassResponseT<GetClassResponse, implementation::GetClassResponse>
    {
    };
}
