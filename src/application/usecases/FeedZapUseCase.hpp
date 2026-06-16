#pragma once

#include "shared/result/Result.hpp"

#include <string>

namespace application {

struct FeedZapRequest {
    std::string summaryPath;
    bool zapRunning{false};
};

class FeedZapUseCase {
public:
    static shared::Result<std::string> validate(const FeedZapRequest& request);
};

}  // namespace application
