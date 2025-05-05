#include "pch.h"
#include "CoronaBaseObject.h"
#include "CoronaBaseObject.g.cpp"

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
