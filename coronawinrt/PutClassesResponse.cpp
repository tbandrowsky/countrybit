#include "pch.h"
#include "PutClassesResponse.h"
#include "PutClassesResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaObject PutClassesResponse::Data()
    {
        return data;
    }
    void PutClassesResponse::Data(winrt::coronawinrt::CoronaObject const& value)
    {
        data = value;
    }
}
