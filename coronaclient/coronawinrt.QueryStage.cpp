#include "pch.h"
#include "coronawinrt.QueryStage.h"
#include "coronawinrt.QueryStage.g.cpp"

namespace winrt::coronawinrt::implementation
{
    hstring QueryStage::StageName()
    {
        return stage_name;
    }
    void QueryStage::StageName(hstring const& value)
    {
        stage_name = value;
    }
}
