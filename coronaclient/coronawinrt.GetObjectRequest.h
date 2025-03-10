#pragma once
#include "coronawinrt.GetObjectRequest.g.h"
#include "coronawinrt.BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct GetObjectRequest : GetObjectRequestT<GetObjectRequest, coronawinrt::implementation::BaseRequest>
    {
        GetObjectRequest() = default;

        hstring class_name;
        int64_t object_id;

        hstring ClassName();
        void ClassName(hstring const& value);
        int64_t ObjectId();
        void ObjectId(int64_t value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct GetObjectRequest : GetObjectRequestT<GetObjectRequest, implementation::GetObjectRequest>
    {
    };
}
