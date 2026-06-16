#pragma once

#include "shared/result/Result.hpp"

namespace application {

struct FullPipelineRequest {
    std::string targetUrl;
    bool authorized{false};
    bool fastMode{false};
    bool skipNuclei{false};
    bool useZapProxy{true};
};

class RunFullPipelineUseCase {
public:
    static shared::Result<FullPipelineRequest> validate(const FullPipelineRequest& request);
};

}  // namespace application
