#pragma once
#include "coronawinrt.CreateObjectRequest.g.h"
#include "coronawinrt.BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct CreateObjectRequest : CreateObjectRequestT<CreateObjectRequest, coronawinrt::implementation::BaseRequest>
    {
        hstring class_name;

        CreateObjectRequest() = default;

        hstring ClassName();
        void ClassName(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct CreateObjectRequest : CreateObjectRequestT<CreateObjectRequest, implementation::CreateObjectRequest>
    {
    };
}
