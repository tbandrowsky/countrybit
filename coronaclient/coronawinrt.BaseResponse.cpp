#include "pch.h"
#include "coronawinrt.BaseResponse.h"
#include "coronawinrt.BaseResponse.g.cpp"


namespace winrt::coronawinrt::implementation
{
    hstring BaseResponse::Message()
    {
        return message;
    }
    void BaseResponse::Message(hstring const& value)
    {
        message = value;
    }
    hstring BaseResponse::Token()
    {
        return token;
    }
    void BaseResponse::Token(hstring const& value)
    {
        token = value;
    }
    bool BaseResponse::Success()
    {
        return success;
    }
    void BaseResponse::Success(bool value)
    {
        success = value;
    }
    double BaseResponse::ExecutionTimeSeconds()
    {
        return execution_time_seconds;
    }
    void BaseResponse::ExecutionTimeSeconds(double value)
    {
        execution_time_seconds = value;
    }
}
