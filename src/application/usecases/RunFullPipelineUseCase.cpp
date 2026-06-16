#include "RunFullPipelineUseCase.hpp"

#include "application/usecases/StartReconUseCase.hpp"

namespace application {

shared::Result<FullPipelineRequest> RunFullPipelineUseCase::validate(
    const FullPipelineRequest& request) {
    StartReconRequest reconRequest{
        request.targetUrl,
        request.authorized,
        request.fastMode,
        request.skipNuclei,
        request.useZapProxy,
    };

    const auto validated = StartReconUseCase::validate(reconRequest);
    if (!validated.isOk()) {
        return shared::Result<FullPipelineRequest>::fail(validated.error());
    }

    return shared::Result<FullPipelineRequest>::ok(request);
}

}  // namespace application
