#include "AppError.hpp"

namespace shared {

AppError::AppError(ErrorCode code, std::string message) : m_code(code), m_message(std::move(message)) {}

ErrorCode AppError::code() const {
    return m_code;
}

const std::string& AppError::message() const {
    return m_message;
}

}  // namespace shared
