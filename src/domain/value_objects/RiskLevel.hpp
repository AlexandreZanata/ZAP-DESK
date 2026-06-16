#pragma once

#include <string>

namespace domain {

enum class RiskLevel { Unknown, Informational, Low, Medium, High, Critical };

RiskLevel riskLevelFromString(const std::string& raw);
std::string riskLevelToString(RiskLevel level);

}  // namespace domain
