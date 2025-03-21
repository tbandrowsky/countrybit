#pragma once
#include "EditObjectRequest.g.h"
#include "BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct EditObjectRequest : EditObjectRequestT<EditObjectRequest, coronawinrt::implementation::BaseRequest>
    {
        hstring class_name;
        int64_t object_id;

        EditObjectRequest() = default;

        hstring ClassName();
        void ClassName(hstring const& value);
        int64_t ObjectId();
        void ObjectId(int64_t value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct EditObjectRequest : EditObjectRequestT<EditObjectRequest, implementation::EditObjectRequest>
    {
    };
}
