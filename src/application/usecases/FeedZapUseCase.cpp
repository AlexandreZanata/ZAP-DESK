#include "FeedZapUseCase.hpp"

#include "shared/errors/AppError.hpp"

namespace application {

shared::Result<std::string> FeedZapUseCase::validate(const FeedZapRequest& request) {
    if (request.summaryPath.empty()) {
        return shared::Result<std::string>::fail(
            shared::AppError(shared::ErrorCode::NotFound, "summary.json path is empty"));
    }
    if (!request.zapRunning) {
        return shared::Result<std::string>::fail(
            shared::AppError(shared::ErrorCode::ZapOffline, "ZAP must be online before feeding URLs"));
    }
    return shared::Result<std::string>::ok(request.summaryPath);
}

}  // namespace application
