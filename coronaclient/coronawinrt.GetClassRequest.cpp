#include "pch.h"
#include "coronawinrt.GetClassRequest.h"
#include "coronawinrt.GetClassRequest.g.cpp"

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
