#pragma once
#include "coronawinrt.BaseResponse.g.h"

namespace winrt::coronawinrt::implementation
{
    struct BaseResponse : BaseResponseT<BaseResponse>
    {
        hstring message;
        hstring token;
        bool success;
        double execution_time_seconds;

        BaseResponse() = default;

        hstring Message();
        void Message(hstring const& value);
        hstring Token();
        void Token(hstring const& value);
        bool Success();
        void Success(bool value);
        double ExecutionTimeSeconds();
        void ExecutionTimeSeconds(double value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct BaseResponse : BaseResponseT<BaseResponse, implementation::BaseResponse>
    {
    };
}
