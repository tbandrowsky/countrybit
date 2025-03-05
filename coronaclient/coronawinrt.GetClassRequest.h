#pragma once
#include "coronawinrt.GetClassRequest.g.h"
#include "coronawinrt.BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct GetClassRequest : GetClassRequestT<GetClassRequest, coronawinrt::implementation::BaseRequest>
    {
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
