#include "pch.h"
#include "CoronaObject.h"
#include "CoronaObject.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring CoronaObject::ClassName()
    {
        return class_name;
    }
    void CoronaObject::ClassName(hstring const& value)
    {
        class_name = value;
    }
    int64_t CoronaObject::ObjectId()
    {
        return object_id;
    }
    void CoronaObject::ObjectId(int64_t value)
    {
        object_id = value;
    }
    winrt::Windows::Foundation::DateTime CoronaObject::Created()
    {
        return created;
    }
    void CoronaObject::Created(winrt::Windows::Foundation::DateTime const& value)
    {
        created = value;
    }
    hstring CoronaObject::CreatedBy()
    {
        return created_by;
    }
    void CoronaObject::CreatedBy(hstring const& value)
    {
        created_by = value;
    }
    winrt::Windows::Foundation::DateTime CoronaObject::Updated()
    {
        return updated;
    }
    void CoronaObject::Updated(winrt::Windows::Foundation::DateTime const& value)
    {
        updated = value;
    }
    hstring CoronaObject::UpdatedBy()
    {
        return updated_by;
    }
    void CoronaObject::UpdatedBy(hstring const& value)
    {
        updated_by = value;
    }
}
