#pragma once
#include "GetClassesRequest.g.h"
#include "BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct GetClassesRequest : GetClassesRequestT<GetClassesRequest, coronawinrt::implementation::BaseRequest>
    {
        hstring class_name;

        GetClassesRequest() = default;

        hstring ClassName();
        void ClassName(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct GetClassesRequest : GetClassesRequestT<GetClassesRequest, implementation::GetClassesRequest>
    {
    };
}
