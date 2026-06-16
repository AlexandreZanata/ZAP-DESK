#pragma once

#include "domain/ports/ReconPorts.hpp"
#include "services/ReconRunner.hpp"

#include <QObject>

namespace infrastructure {

class QtReconGateway : public QObject, public domain::IReconGateway {
    Q_OBJECT

public:
    explicit QtReconGateway(ReconRunner* runner, QObject* parent = nullptr);

    void start(const domain::ReconOptions& options) override;
    void stop() override;
    std::string lastSummaryPath() const override;

private:
    ReconRunner* m_runner;
};

}  // namespace infrastructure
