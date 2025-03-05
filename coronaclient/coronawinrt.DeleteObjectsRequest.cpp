#include "pch.h"
#include "coronawinrt.DeleteObjectsRequest.h"
#include "coronawinrt.DeleteObjectsRequest.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> DeleteObjectsRequest::Objects()
    {
        throw hresult_not_implemented();
    }
    void DeleteObjectsRequest::Objects(winrt::Windows::Foundation::Collections::IVector<winrt::coronawinrt::CoronaObject> const& value)
    {
        throw hresult_not_implemented();
    }
}
