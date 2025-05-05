#pragma once
#include "DeleteObjectsResponse.g.h"
#include "BaseResponse.h"

namespace winrt::coronawinrt::implementation
{
    struct DeleteObjectsResponse : DeleteObjectsResponseT<DeleteObjectsResponse, coronawinrt::implementation::BaseResponse>
    {
        winrt::coronawinrt::CoronaObject data;

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
