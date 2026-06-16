#pragma once

#include "shared/errors/DomainError.hpp"
#include "shared/result/Result.hpp"

#include <string>

namespace domain {

class ScanId {
public:
    static shared::Result<ScanId> create(std::string raw);
    static ScanId generate();

    const std::string& value() const;

private:
    explicit ScanId(std::string value);
    std::string m_value;
};

}  // namespace domain
