#include "QtReconGateway.hpp"

namespace infrastructure {

QtReconGateway::QtReconGateway(ReconRunner* runner, QObject* parent)
    : QObject(parent), m_runner(runner) {}

void QtReconGateway::start(const domain::ReconOptions& options) {
    m_runner->start(QString::fromStdString(options.target), options.fastMode, options.skipNuclei,
                    options.useZapProxy);
}

void QtReconGateway::stop() {
    m_runner->stop();
}

std::string QtReconGateway::lastSummaryPath() const {
    return m_runner->lastSummaryPath().toStdString();
}

}  // namespace infrastructure
