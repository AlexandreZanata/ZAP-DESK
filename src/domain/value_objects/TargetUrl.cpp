#include "TargetUrl.hpp"

namespace domain {

namespace {

bool hasScheme(const std::string& url) {
    return url.rfind("https://", 0) == 0 || url.rfind("http://", 0) == 0;
}

}  // namespace

shared::Result<TargetUrl> TargetUrl::create(std::string raw) {
    if (raw.empty()) {
        return shared::Result<TargetUrl>::fail(
            shared::DomainError("Target URL must not be empty"));
    }
    if (!hasScheme(raw)) {
        return shared::Result<TargetUrl>::fail(
            shared::DomainError("Target URL must start with http:// or https://"));
    }
    if (raw == "https://" || raw == "http://") {
        return shared::Result<TargetUrl>::fail(shared::DomainError("Target URL host is missing"));
    }
    return shared::Result<TargetUrl>::ok(TargetUrl(std::move(raw)));
}

TargetUrl::TargetUrl(std::string value) : m_value(std::move(value)) {}

const std::string& TargetUrl::value() const {
    return m_value;
}

}  // namespace domain
