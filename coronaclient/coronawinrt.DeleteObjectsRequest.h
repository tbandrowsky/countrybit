#pragma once
#include "coronawinrt.DeleteObjectsRequest.g.h"
#include "coronawinrt.BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct DeleteObjectsRequest : DeleteObjectsRequestT<DeleteObjectsRequest, coronawinrt::implementation::BaseRequest>
    {
        DeleteObjectsRequest() = default;

        winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> Objects();
        void Objects(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct DeleteObjectsRequest : DeleteObjectsRequestT<DeleteObjectsRequest, implementation::DeleteObjectsRequest>
    {
    };
}
