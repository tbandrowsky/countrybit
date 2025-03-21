#pragma once
#include "QueryStage.g.h"

namespace winrt::coronawinrt::implementation
{
    struct QueryStage : QueryStageT<QueryStage>
    {
        hstring stage_name;

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
