#include "pch.h"
#include "coronawinrt.CopyObjectResponse.h"
#include "coronawinrt.CopyObjectResponse.g.cpp"

namespace winrt::coronawinrt::implementation
{
    winrt::coronawinrt::CoronaObject CopyObjectResponse::Data()
    {
        return data;
    }
    void CopyObjectResponse::Data(winrt::coronawinrt::CoronaObject const& value)
    {
        data = value;
    }
}
