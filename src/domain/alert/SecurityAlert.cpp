#include "SecurityAlert.hpp"

namespace domain {

SecurityAlert::SecurityAlert(RiskLevel risk, std::string name, std::string url, std::string description)
    : m_risk(risk),
      m_name(std::move(name)),
      m_url(std::move(url)),
      m_description(std::move(description)) {}

RiskLevel SecurityAlert::risk() const { return m_risk; }
const std::string& SecurityAlert::name() const { return m_name; }
const std::string& SecurityAlert::url() const { return m_url; }
const std::string& SecurityAlert::description() const { return m_description; }

}  // namespace domain
