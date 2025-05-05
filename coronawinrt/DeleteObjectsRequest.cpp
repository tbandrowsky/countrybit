#include "pch.h"
#include "DeleteObjectsRequest.h"
#include "DeleteObjectsRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> DeleteObjectsRequest::Objects()
    {
        return objects;
    }
    void DeleteObjectsRequest::Objects(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> const& value)
    {
        objects = value;
    }
}
