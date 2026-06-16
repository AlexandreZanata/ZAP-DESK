#pragma once

#include <string>

namespace shared {

enum class ErrorCode {
    Validation,
    NotFound,
    Unauthorized,
    PreflightFailed,
    ZapOffline,
    Infrastructure,
    Domain,
};

class AppError {
public:
    AppError(ErrorCode code, std::string message);

    ErrorCode code() const;
    const std::string& message() const;

private:
    ErrorCode m_code;
    std::string m_message;
};

}  // namespace shared
