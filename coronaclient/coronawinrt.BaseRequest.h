#pragma once
#include "coronawinrt.BaseRequest.g.h"
namespace winrt::coronawinrt::implementation
{
    struct BaseRequest : BaseRequestT<BaseRequest>
    {
        BaseRequest() = default;

        hstring Token();
        void Token(hstring const& value);

        hstring token;
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct BaseRequest : BaseRequestT<BaseRequest, implementation::BaseRequest>
    {
    };
}
