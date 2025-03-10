#include "pch.h"
#include "coronawinrt.GetClassesRequest.h"
#include "coronawinrt.GetClassesRequest.g.cpp"

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
