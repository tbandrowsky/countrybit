#include "pch.h"
#include "coronawinrt.CoronaBaseObject.h"
#include "coronawinrt.CoronaBaseObject.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring CoronaBaseObject::RawJson()
    {
        return raw_json;
    }
    void CoronaBaseObject::RawJson(hstring const& value)
    {
        raw_json = value;
    }
}
