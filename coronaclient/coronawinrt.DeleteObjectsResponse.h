#pragma once
#include "coronawinrt.DeleteObjectsResponse.g.h"
#include "coronawinrt.BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct DeleteObjectsResponse : DeleteObjectsResponseT<DeleteObjectsResponse, coronawinrt::implementation::BaseResponse>
    {
        DeleteObjectsResponse() = default;

        winrt::coronawinrt::CoronaObject Data();
        void Data(winrt::coronawinrt::CoronaObject const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct DeleteObjectsResponse : DeleteObjectsResponseT<DeleteObjectsResponse, implementation::DeleteObjectsResponse>
    {
    };
}
