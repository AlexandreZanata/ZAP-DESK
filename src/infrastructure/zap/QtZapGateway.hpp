#pragma once

#include "domain/ports/ZapPorts.hpp"

#include <QObject>

class ZapClient;
class ZapDaemon;
class ZapUpdater;

namespace infrastructure {

class QtZapGateway : public QObject, public domain::IZapGateway {
    Q_OBJECT

public:
    QtZapGateway(ZapClient* client, ZapDaemon* daemon, ZapUpdater* updater, QObject* parent = nullptr);

    bool isDaemonRunning() const override;
    void boot() override;
    void shutdown() override;

    void checkConnection(std::function<void(bool ok, std::string version)> callback) override;
    void accessUrl(const std::string& url, std::function<void(bool ok, std::string error)> callback) override;
    void startAjaxSpider(const std::string& url,
                         std::function<void(bool ok, std::string message)> callback) override;
    void startActiveScan(const std::string& url,
                         std::function<void(bool ok, std::string message)> callback) override;
    void stopScans() override;
    void ajaxSpiderStatus(std::function<void(std::string status)> callback) override;
    void activeScanStatus(std::function<void(std::string status)> callback) override;
    void fetchAlerts(
        std::function<void(std::vector<domain::SecurityAlert>, std::string error)> callback) override;
    void checkForUpdates(std::function<void(domain::ZapUpdateInfo)> callback) override;
    void runUpdateScript(std::function<void(bool ok, std::string error)> callback) override;

private:
    ZapClient* m_client;
    ZapDaemon* m_daemon;
    ZapUpdater* m_updater;
};

}  // namespace infrastructure
