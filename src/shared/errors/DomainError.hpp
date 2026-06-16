#pragma once

#include "AppError.hpp"

namespace shared {

class DomainError : public AppError {
public:
    explicit DomainError(std::string message)
        : AppError(ErrorCode::Domain, std::move(message)) {}
};

}  // namespace shared
