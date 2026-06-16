#include "RiskLevel.hpp"

#include <algorithm>
#include <cctype>

namespace domain {

namespace {

std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

}  // namespace

RiskLevel riskLevelFromString(const std::string& raw) {
    const std::string v = lower(raw);
    if (v == "high") return RiskLevel::High;
    if (v == "medium") return RiskLevel::Medium;
    if (v == "low") return RiskLevel::Low;
    if (v == "informational" || v == "info") return RiskLevel::Informational;
    if (v == "critical") return RiskLevel::Critical;
    return RiskLevel::Unknown;
}

std::string riskLevelToString(RiskLevel level) {
    switch (level) {
        case RiskLevel::High:
            return "High";
        case RiskLevel::Medium:
            return "Medium";
        case RiskLevel::Low:
            return "Low";
        case RiskLevel::Informational:
            return "Informational";
        case RiskLevel::Critical:
            return "Critical";
        default:
            return "Unknown";
    }
}

}  // namespace domain
