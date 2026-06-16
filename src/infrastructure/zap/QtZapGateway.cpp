#include "QtZapGateway.hpp"

#include "core/ZapClient.hpp"
#include "core/ZapDaemon.hpp"
#include "domain/value_objects/RiskLevel.hpp"
#include "services/ZapUpdater.hpp"

infrastructure::QtZapGateway::QtZapGateway(ZapClient* client, ZapDaemon* daemon, ZapUpdater* updater,
                                           QObject* parent)
    : QObject(parent), m_client(client), m_daemon(daemon), m_updater(updater) {}

bool infrastructure::QtZapGateway::isDaemonRunning() const {
    return m_daemon->isRunning();
}

void infrastructure::QtZapGateway::boot() {
    m_daemon->start();
}

void infrastructure::QtZapGateway::shutdown() {
    m_daemon->stop();
}

void infrastructure::QtZapGateway::checkConnection(
    std::function<void(bool ok, std::string version)> callback) {
    m_client->checkConnection([callback](bool ok, const QString& version) {
        callback(ok, version.toStdString());
    });
}

void infrastructure::QtZapGateway::accessUrl(const std::string& url,
                                             std::function<void(bool ok, std::string error)> callback) {
    m_client->accessUrl(QString::fromStdString(url), [callback](bool ok, const QString& err) {
        callback(ok, err.toStdString());
    });
}

void infrastructure::QtZapGateway::startAjaxSpider(
    const std::string& url, std::function<void(bool ok, std::string message)> callback) {
    m_client->startAjaxSpider(QString::fromStdString(url),
                             [callback](bool ok, const QString& msg) { callback(ok, msg.toStdString()); });
}

void infrastructure::QtZapGateway::startActiveScan(
    const std::string& url, std::function<void(bool ok, std::string message)> callback) {
    m_client->startActiveScan(QString::fromStdString(url),
                              [callback](bool ok, const QString& msg) { callback(ok, msg.toStdString()); });
}

void infrastructure::QtZapGateway::stopScans() {
    m_client->stopAjaxSpider([](bool) {});
    m_client->stopActiveScan([](bool) {});
}

void infrastructure::QtZapGateway::ajaxSpiderStatus(std::function<void(std::string status)> callback) {
    m_client->ajaxSpiderStatus([callback](const QString& status) { callback(status.toStdString()); });
}

void infrastructure::QtZapGateway::activeScanStatus(std::function<void(std::string status)> callback) {
    m_client->activeScanStatus([callback](const QString& status) { callback(status.toStdString()); });
}

void infrastructure::QtZapGateway::fetchAlerts(
    std::function<void(std::vector<domain::SecurityAlert>, std::string error)> callback) {
    m_client->fetchAlerts([callback](const QVector<ZapAlert>& alerts, const QString& err) {
        if (!err.isEmpty()) {
            callback({}, err.toStdString());
            return;
        }

        std::vector<domain::SecurityAlert> mapped;
        mapped.reserve(alerts.size());
        for (const auto& alert : alerts) {
            mapped.emplace_back(domain::riskLevelFromString(alert.risk.toStdString()),
                                alert.name.toStdString(), alert.url.toStdString(),
                                alert.description.toStdString());
        }
        callback(mapped, {});
    });
}

void infrastructure::QtZapGateway::checkForUpdates(std::function<void(domain::ZapUpdateInfo)> callback) {
    m_client->checkConnection([this, callback](bool ok, const QString& version) {
        if (!ok) {
            callback({});
            return;
        }

        m_updater->checkForUpdates(version, [callback](const ::ZapUpdateInfo& info) {
            domain::ZapUpdateInfo mapped;
            mapped.localVersion = info.localVersion.toStdString();
            mapped.latestVersion = info.latestVersion.toStdString();
            mapped.releaseUrl = info.releaseUrl.toStdString();
            mapped.updateAvailable = info.updateAvailable;
            callback(mapped);
        });
    });
}

void infrastructure::QtZapGateway::runUpdateScript(
    std::function<void(bool ok, std::string error)> callback) {
    m_updater->launchUpdateScript([callback](bool ok, const QString& err) {
        callback(ok, err.toStdString());
    });
}
