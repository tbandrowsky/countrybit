#pragma once
#include "SendUserRequest.g.h"
#include "BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct SendUserRequest : SendUserRequestT<SendUserRequest, coronawinrt::implementation::BaseRequest>
    {
        hstring email;

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
