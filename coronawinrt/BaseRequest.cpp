#include "pch.h"
#include "BaseRequest.h"
#include "BaseRequest.g.cpp"

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
