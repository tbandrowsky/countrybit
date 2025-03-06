#include "pch.h"
#include "coronawinrt.BaseRequest.h"
#include "coronawinrt.BaseRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring BaseRequest::Token()
    {
        return token;
    }
    void BaseRequest::Token(hstring const& value)
    {
        token = value;
    }
}
