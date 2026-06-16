#pragma once

#include "domain/recon/ReconTypes.hpp"

#include <functional>
#include <string>
#include <vector>

namespace domain {

class IReconGateway {
public:
    virtual ~IReconGateway() = default;

    virtual void start(const ReconOptions& options) = 0;
    virtual void stop() = 0;
    virtual std::string lastSummaryPath() const = 0;
};

class IPreflightGateway {
public:
    virtual ~IPreflightGateway() = default;
    virtual void check(std::function<void(PreflightReport)> callback) = 0;
};

class IReconSummaryReader {
public:
    virtual ~IReconSummaryReader() = default;
    virtual ReconSummaryData load(const std::string& summaryPath) const = 0;
    virtual std::vector<std::string> loadUrls(const std::string& summaryPath) const = 0;
};

}  // namespace domain
