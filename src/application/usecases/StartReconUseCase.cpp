#include "StartReconUseCase.hpp"

#include "domain/value_objects/ScanId.hpp"
#include "domain/value_objects/TargetUrl.hpp"
#include "shared/errors/DomainError.hpp"

namespace application {

shared::Result<ValidatedReconStart> StartReconUseCase::validate(const StartReconRequest& request) {
    if (!request.authorized) {
        return shared::Result<ValidatedReconStart>::fail(
            shared::DomainError("Written authorization is required before running recon"));
    }

    auto target = domain::TargetUrl::create(request.targetUrl);
    if (!target.isOk()) {
        return shared::Result<ValidatedReconStart>::fail(target.error());
    }

    auto scan = domain::Scan::create(domain::ScanId::generate(), target.value());
    if (!scan.isOk()) {
        return shared::Result<ValidatedReconStart>::fail(scan.error());
    }

    ValidatedReconStart out;
    out.scan = scan.value();
    out.options = domain::ReconOptions{
        target.value().value(),
        request.fastMode,
        request.skipNuclei,
        request.useZapProxy,
    };
    return shared::Result<ValidatedReconStart>::ok(std::move(out));
}

}  // namespace application
