#include "QtPreflightGateway.hpp"

namespace infrastructure {

QtPreflightGateway::QtPreflightGateway(ReconPreflight* preflight, QObject* parent)
    : QObject(parent), m_preflight(preflight) {}

void QtPreflightGateway::check(std::function<void(domain::PreflightReport)> callback) {
    m_preflight->check([callback](const ReconPreflight::Result& result) {
        domain::PreflightReport report;
        report.ok = result.ok;
        report.error = result.error.toStdString();
        for (const QString& tool : result.missingTools) {
            report.missingTools.push_back(tool.toStdString());
        }
        callback(report);
    });
}

}  // namespace infrastructure
