#include "pch.h"
#include "coronawinrt.BaseRequest.h"
#include "coronawinrt.BaseRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring BaseRequest::Token()
    {
        throw hresult_not_implemented();
    }
    void BaseRequest::Token(hstring const& value)
    {
        throw hresult_not_implemented();
    }
}
