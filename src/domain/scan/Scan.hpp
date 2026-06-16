#pragma once

#include "ScanStatus.hpp"
#include "domain/value_objects/ScanId.hpp"
#include "domain/value_objects/TargetUrl.hpp"
#include "shared/errors/DomainError.hpp"
#include "shared/result/Result.hpp"

#include <cstdint>
#include <string>

namespace domain {

class Scan {
public:
    static shared::Result<Scan> create(ScanId id, TargetUrl target);

    const ScanId& id() const;
    const TargetUrl& target() const;
    ScanStatus status() const;
    std::int64_t createdAt() const;
    std::int64_t updatedAt() const;
    int version() const;

    void markRunning();
    void markCompleted();
    void markFailed();
    void markCancelled();

private:
    Scan(ScanId id, TargetUrl target, std::int64_t createdAt);

    void touch();

    ScanId m_id;
    TargetUrl m_target;
    ScanStatus m_status{ScanStatus::Pending};
    std::int64_t m_createdAt{0};
    std::int64_t m_updatedAt{0};
    int m_version{0};
};

}  // namespace domain
