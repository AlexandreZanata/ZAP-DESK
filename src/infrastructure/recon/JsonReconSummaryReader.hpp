#pragma once

#include "domain/ports/ReconPorts.hpp"

namespace infrastructure {

class JsonReconSummaryReader : public domain::IReconSummaryReader {
public:
    domain::ReconSummaryData load(const std::string& summaryPath) const override;
    std::vector<std::string> loadUrls(const std::string& summaryPath) const override;
};

}  // namespace infrastructure
