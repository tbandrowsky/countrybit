#include "pch.h"
#include "GetClassRequest.h"
#include "GetClassRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring GetClassRequest::ClassName()
    {
        return class_name;
    }
    void GetClassRequest::ClassName(hstring const& value)
    {
        class_name = value;
    }
}
