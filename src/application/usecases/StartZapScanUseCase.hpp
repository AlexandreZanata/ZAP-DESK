#pragma once

#include "shared/result/Result.hpp"

#include <string>

namespace application {

class StartZapScanUseCase {
public:
    static shared::Result<std::string> validateTarget(const std::string& rawUrl);
};

}  // namespace application
