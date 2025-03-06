#pragma once
#include "coronawinrt.ConfirmUserRequest.g.h"
#include "coronawinrt.BaseRequest.h"


namespace winrt::coronawinrt::implementation
{
    struct ConfirmUserRequest : ConfirmUserRequestT<ConfirmUserRequest, coronawinrt::implementation::BaseRequest>
    {
        ConfirmUserRequest() = default;

        hstring email;
        hstring validation_code;

        hstring Email();
        void Email(hstring const& value);
        hstring ValidationCode();
        void ValidationCode(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct ConfirmUserRequest : ConfirmUserRequestT<ConfirmUserRequest, implementation::ConfirmUserRequest>
    {
    };
}
