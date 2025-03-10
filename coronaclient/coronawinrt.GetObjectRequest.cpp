#include "pch.h"
#include "coronawinrt.GetObjectRequest.h"
#include "coronawinrt.GetObjectRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring GetObjectRequest::ClassName()
    {
        return class_name;
    }
    void GetObjectRequest::ClassName(hstring const& value)
    {
        class_name = value;
    }
    int64_t GetObjectRequest::ObjectId()
    {
        return object_id;
    }
    void GetObjectRequest::ObjectId(int64_t value)
    {
        object_id = value;
    }
}
