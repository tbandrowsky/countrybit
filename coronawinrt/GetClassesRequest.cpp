#include "pch.h"
#include "GetClassesRequest.h"
#include "GetClassesRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring GetClassesRequest::ClassName()
    {
        return class_name;
    }
    void GetClassesRequest::ClassName(hstring const& value)
    {
        class_name = value;
    }
}
