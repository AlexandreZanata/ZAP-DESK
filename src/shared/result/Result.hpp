#pragma once

#include "shared/errors/AppError.hpp"

#include <optional>
#include <utility>

namespace shared {

template <typename T, typename E = AppError>
class Result {
public:
    static Result ok(T value) {
        Result r;
        r.m_ok = true;
        r.m_value.emplace(std::move(value));
        return r;
    }

    static Result fail(E error) {
        Result r;
        r.m_ok = false;
        r.m_error = std::move(error);
        return r;
    }

    bool isOk() const { return m_ok; }
    const T& value() const { return m_value.value(); }
    const E& error() const { return m_error; }

private:
    bool m_ok{false};
    std::optional<T> m_value;
    E m_error{ErrorCode::Infrastructure, ""};
};

template <typename E>
class Result<void, E> {
public:
    static Result ok() {
        Result r;
        r.m_ok = true;
        return r;
    }

    static Result fail(E error) {
        Result r;
        r.m_ok = false;
        r.m_error = std::move(error);
        return r;
    }

    bool isOk() const { return m_ok; }
    const E& error() const { return m_error; }

private:
    bool m_ok{false};
    E m_error{ErrorCode::Infrastructure, ""};
};

}  // namespace shared
