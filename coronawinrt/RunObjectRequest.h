#pragma once
#include "RunObjectRequest.g.h"
#include "BaseRequest.h"

namespace winrt::coronawinrt::implementation
{
    struct RunObjectRequest : RunObjectRequestT<RunObjectRequest, coronawinrt::implementation::BaseRequest>
    {
        winrt::coronawinrt::CoronaObject data;

        RunObjectRequest() = default;

        winrt::coronawinrt::CoronaObject Data();
        void Data(winrt::coronawinrt::CoronaObject const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct RunObjectRequest : RunObjectRequestT<RunObjectRequest, implementation::RunObjectRequest>
    {
    };
}
