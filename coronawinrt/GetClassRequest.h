#pragma once
#include "GetClassRequest.g.h"
#include "BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct GetClassRequest : GetClassRequestT<GetClassRequest, coronawinrt::implementation::BaseRequest>
    {
        hstring class_name;

        GetClassRequest() = default;

        hstring ClassName();
        void ClassName(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct GetClassRequest : GetClassRequestT<GetClassRequest, implementation::GetClassRequest>
    {
    };
}
