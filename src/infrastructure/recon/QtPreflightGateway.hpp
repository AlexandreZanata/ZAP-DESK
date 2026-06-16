#pragma once

#include "domain/ports/ReconPorts.hpp"
#include "services/ReconPreflight.hpp"

#include <QObject>

namespace infrastructure {

class QtPreflightGateway : public QObject, public domain::IPreflightGateway {
    Q_OBJECT

public:
    explicit QtPreflightGateway(ReconPreflight* preflight, QObject* parent = nullptr);

    void check(std::function<void(domain::PreflightReport)> callback) override;

private:
    ReconPreflight* m_preflight;
};

}  // namespace infrastructure
