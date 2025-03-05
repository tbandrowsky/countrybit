#pragma once
#include "coronawinrt.QueryStage.g.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryStage : QueryStageT<QueryStage>
    {
        QueryStage() = default;

        hstring StageName();
        void StageName(hstring const& value);
    };
}
namespace winrt::coronawinrt::factory_implementation
{
    struct QueryStage : QueryStageT<QueryStage, implementation::QueryStage>
    {
    };
}
