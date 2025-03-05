#pragma once
#include "coronawinrt.SendUserRequest.g.h"
#include "coronawinrt.BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct SendUserRequest : SendUserRequestT<SendUserRequest, coronawinrt::implementation::BaseRequest>
    {
        SendUserRequest() = default;

        hstring Email();
        void Email(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct SendUserRequest : SendUserRequestT<SendUserRequest, implementation::SendUserRequest>
    {
    };
}
