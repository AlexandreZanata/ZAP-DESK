#include "StartZapScanUseCase.hpp"

#include "domain/value_objects/TargetUrl.hpp"

namespace application {

shared::Result<std::string> StartZapScanUseCase::validateTarget(const std::string& rawUrl) {
    auto target = domain::TargetUrl::create(rawUrl);
    if (!target.isOk()) {
        return shared::Result<std::string>::fail(target.error());
    }
    return shared::Result<std::string>::ok(target.value().value());
}

}  // namespace application
