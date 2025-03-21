#pragma once
#include "GetClassesResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct GetClassesResponse : GetClassesResponseT<GetClassesResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> classes;

        GetClassesResponse() = default;

        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> Classes();
        void Classes(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::ClassDefinition> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct GetClassesResponse : GetClassesResponseT<GetClassesResponse, implementation::GetClassesResponse>
    {
    };
}
