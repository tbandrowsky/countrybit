#include "pch.h"
#include "coronawinrt.EditObjectRequest.h"
#include "coronawinrt.EditObjectRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring EditObjectRequest::ClassName()
    {
        return class_name;
    }
    void EditObjectRequest::ClassName(hstring const& value)
    {
        class_name = value;
    }
    int64_t EditObjectRequest::ObjectId()
    {
        return object_id;
    }
    void EditObjectRequest::ObjectId(int64_t value)
    {
        object_id = value;
    }
}
