#pragma once

#include "domain/value_objects/RiskLevel.hpp"

#include <string>

namespace domain {

class SecurityAlert {
public:
    SecurityAlert(RiskLevel risk, std::string name, std::string url, std::string description);

    RiskLevel risk() const;
    const std::string& name() const;
    const std::string& url() const;
    const std::string& description() const;

private:
    RiskLevel m_risk;
    std::string m_name;
    std::string m_url;
    std::string m_description;
};

}  // namespace domain
