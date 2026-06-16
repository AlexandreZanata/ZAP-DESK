#pragma once

#include <string>
#include <vector>

namespace domain {

struct ReconSummaryData {
    int subdomains{0};
    int liveHosts{0};
    int openPorts{0};
    int paths{0};
    int vulnerabilities{0};
    int errors{0};
    bool valid{false};
};

struct ReconOptions {
    std::string target;
    bool fastMode{false};
    bool skipNuclei{false};
    bool useZapProxy{true};
};

struct PreflightReport {
    bool ok{false};
    std::vector<std::string> missingTools;
    std::string error;
};

}  // namespace domain
