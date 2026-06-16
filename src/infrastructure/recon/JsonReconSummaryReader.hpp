#pragma once

#include "domain/ports/ReconPorts.hpp"

namespace infrastructure {

struct NucleiFinding {
    std::string templateId;
    std::string severity;
    std::string url;
    std::string description;
};

class JsonReconSummaryReader : public domain::IReconSummaryReader {
public:
    domain::ReconSummaryData load(const std::string& summaryPath) const override;
    std::vector<std::string> loadUrls(const std::string& summaryPath) const override;
    std::vector<NucleiFinding> loadNucleiFindings(const std::string& summaryPath) const;
};

}  // namespace infrastructure
