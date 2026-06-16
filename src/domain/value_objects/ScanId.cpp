#include "ScanId.hpp"

#include <random>
#include <sstream>

namespace domain {

namespace {

std::string randomHex(std::size_t bytes) {
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, 255);
    std::ostringstream out;
    for (std::size_t i = 0; i < bytes; ++i) {
        out << std::hex << (dist(rng) & 0xFF);
    }
    return out.str();
}

}  // namespace

shared::Result<ScanId> ScanId::create(std::string raw) {
    if (raw.empty()) {
        return shared::Result<ScanId>::fail(shared::DomainError("ScanId must not be empty"));
    }
    return shared::Result<ScanId>::ok(ScanId(std::move(raw)));
}

ScanId ScanId::generate() {
    return ScanId(randomHex(16));
}

ScanId::ScanId(std::string value) : m_value(std::move(value)) {}

const std::string& ScanId::value() const {
    return m_value;
}

}  // namespace domain
