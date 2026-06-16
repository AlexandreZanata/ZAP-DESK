#pragma once

#include "shared/errors/DomainError.hpp"
#include "shared/result/Result.hpp"

#include <string>

namespace domain {

class TargetUrl {
public:
    static shared::Result<TargetUrl> create(std::string raw);

    const std::string& value() const;

private:
    explicit TargetUrl(std::string value);
    std::string m_value;
};

}  // namespace domain
