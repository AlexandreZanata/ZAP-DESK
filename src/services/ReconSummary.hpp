#pragma once

#include <QString>

struct ReconSummaryStats {
    int subdomains{0};
    int liveHosts{0};
    int openPorts{0};
    int paths{0};
    int vulnerabilities{0};
    int errors{0};
    bool valid{false};
};

class ReconSummary {
public:
    static ReconSummaryStats parse(const QString& summaryPath);
    static QString format(const ReconSummaryStats& stats);
};
