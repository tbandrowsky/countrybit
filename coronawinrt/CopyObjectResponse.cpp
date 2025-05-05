#include "pch.h"
#include "CopyObjectResponse.h"
#include "CopyObjectResponse.g.cpp"

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
