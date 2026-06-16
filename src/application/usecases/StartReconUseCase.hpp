#pragma once

#include "domain/recon/ReconTypes.hpp"
#include "domain/scan/Scan.hpp"
#include "shared/result/Result.hpp"

#include <string>

namespace application {

struct StartReconRequest {
    std::string targetUrl;
    bool authorized{false};
    bool fastMode{false};
    bool skipNuclei{false};
    bool useZapProxy{true};
};

struct ValidatedReconStart {
    domain::Scan scan;
    domain::ReconOptions options;
};

class StartReconUseCase {
public:
    static shared::Result<ValidatedReconStart> validate(const StartReconRequest& request);
};

}  // namespace application
