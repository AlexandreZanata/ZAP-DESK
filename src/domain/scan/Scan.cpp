#include "Scan.hpp"

#include <chrono>

namespace domain {

namespace {

std::int64_t nowEpoch() {
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

}  // namespace

shared::Result<Scan> Scan::create(ScanId id, TargetUrl target) {
    if (id.value().empty()) {
        return shared::Result<Scan>::fail(shared::DomainError("Scan requires a valid id"));
    }
    return shared::Result<Scan>::ok(Scan(std::move(id), std::move(target), nowEpoch()));
}

Scan::Scan(ScanId id, TargetUrl target, std::int64_t createdAt)
    : m_id(std::move(id)), m_target(std::move(target)), m_createdAt(createdAt), m_updatedAt(createdAt) {}

const ScanId& Scan::id() const { return m_id; }
const TargetUrl& Scan::target() const { return m_target; }
ScanStatus Scan::status() const { return m_status; }
std::int64_t Scan::createdAt() const { return m_createdAt; }
std::int64_t Scan::updatedAt() const { return m_updatedAt; }
int Scan::version() const { return m_version; }

void Scan::markRunning() {
    m_status = ScanStatus::Running;
    touch();
}

void Scan::markCompleted() {
    m_status = ScanStatus::Completed;
    touch();
}

void Scan::markFailed() {
    m_status = ScanStatus::Failed;
    touch();
}

void Scan::markCancelled() {
    m_status = ScanStatus::Cancelled;
    touch();
}

void Scan::touch() {
    m_updatedAt = nowEpoch();
    ++m_version;
}

}  // namespace domain
