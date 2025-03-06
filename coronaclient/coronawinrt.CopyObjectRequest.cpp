#include "pch.h"
#include "coronawinrt.CopyObjectRequest.h"
#include "coronawinrt.CopyObjectRequest.g.cpp"


namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CopyObjectPath CopyObjectRequest::Source()
    {
        return source;
    }
    void CopyObjectRequest::Source(winrt::coronawinrt::CopyObjectPath const& value)
    {
        source = value;
    }
    winrt::coronawinrt::CopyObjectPath CopyObjectRequest::Dest()
    {
        return dest;
    }
    void CopyObjectRequest::Dest(winrt::coronawinrt::CopyObjectPath const& value)
    {
        dest = value;
    }
    winrt::coronawinrt::CopyObjectPath CopyObjectRequest::Transform()
    {
        return dest;
    }
    void CopyObjectRequest::Transform(winrt::coronawinrt::CopyObjectPath const& value)
    {
        dest = value;
    }
}
