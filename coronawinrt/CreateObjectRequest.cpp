#include "pch.h"
#include "CreateObjectRequest.h"
#include "CreateObjectRequest.g.cpp"


namespace winrt::coronawinrt::implementation
{
    hstring CreateObjectRequest::ClassName()
    {
        return class_name;
    }
    void CreateObjectRequest::ClassName(hstring const& value)
    {
        class_name = value;
    }
}
