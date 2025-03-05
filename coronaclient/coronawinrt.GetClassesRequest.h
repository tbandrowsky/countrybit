#pragma once
#include "coronawinrt.GetClassesRequest.g.h"
#include "coronawinrt.BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct GetClassesRequest : GetClassesRequestT<GetClassesRequest, coronawinrt::implementation::BaseRequest>
    {
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
