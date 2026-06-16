#include "ApplicationFacade.hpp"

#include "application/usecases/FeedZapUseCase.hpp"
#include "application/usecases/RunFullPipelineUseCase.hpp"
#include "application/usecases/StartReconUseCase.hpp"
#include "application/usecases/StartZapScanUseCase.hpp"
#include "config/AppConfig.hpp"

#include <QStringList>

namespace presentation {

ApplicationFacade::ApplicationFacade(QObject* parent)
    : QObject(parent),
      m_reconGateway(&m_recon, this),
      m_preflightGateway(&m_preflight, this),
      m_zapGateway(&m_client, &m_daemon, &m_updater, this),
      m_audit(AppConfig::instance().auditLogPath()),
      m_reconLimiter(AppConfig::instance().reconRateLimitSeconds()) {
    applyClientSecurity();
    wireServices();
}

void ApplicationFacade::applyClientSecurity() {
    m_client.setBaseUrl(AppConfig::instance().zapApiUrl());
    m_client.setApiKey(AppConfig::instance().zapApiKey());
    m_reconLimiter.setMinIntervalSeconds(AppConfig::instance().reconRateLimitSeconds());
}

void ApplicationFacade::wireServices() {
    connect(&m_daemon, &ZapDaemon::logMessage, this, &ApplicationFacade::logMessage);
    connect(&m_daemon, &ZapDaemon::stateChanged, this, &ApplicationFacade::zapDaemonStateChanged);

    connect(&m_recon, &ReconRunner::logLine, this, &ApplicationFacade::logMessage);
    connect(&m_recon, &ReconRunner::phaseChanged, this, &ApplicationFacade::reconPhaseChanged);
    connect(&m_recon, &ReconRunner::finished, this,
            [this](bool ok, const QString& summaryPath) {
                if (m_activeScan) {
                    ok ? m_activeScan->markCompleted() : m_activeScan->markFailed();
                }

                const QString summaryText =
                    ok ? formatSummary(summaryPath.toStdString()) : QString(">> RECON SUMMARY: —");
                if (ok) {
                    m_lastSummaryPath = summaryPath;
                }
                emit reconFinished(ok, summaryPath, summaryText);

                if (ok && m_fullPipelinePending) {
                    m_fullPipelinePending = false;
                    feedZapAndScan(summaryPath.toStdString());
                }
            });

    connect(&m_updater, &ZapUpdater::logMessage, this, &ApplicationFacade::logMessage);
}

bool ApplicationFacade::isZapRunning() const {
    return m_zapGateway.isDaemonRunning();
}

void ApplicationFacade::bootZap() {
    m_audit.log("BOOT_ZAP");
    emit logMessage(">> Starting ZAP engine...");
    m_zapGateway.boot();
}

void ApplicationFacade::shutdownZap() {
    m_audit.log("SHUTDOWN_ZAP");
    m_zapGateway.shutdown();
}

void ApplicationFacade::checkZapConnection() {
    m_zapGateway.checkConnection([this](bool ok, const std::string& version) {
        emit zapConnectionChecked(ok, QString::fromStdString(version));
    });
}

void ApplicationFacade::startAjaxScan(const QString& targetUrl) {
    const auto validated = application::StartZapScanUseCase::validateTarget(targetUrl.toStdString());
    if (!validated.isOk()) {
        emit operationFailed(QString::fromStdString(validated.error().message()));
        return;
    }

    const std::string url = validated.value();
    m_audit.log("AJAX_SCAN", QString::fromStdString(url));
    emit logMessage(QString(">> AJAX SPIDER → %1").arg(QString::fromStdString(url)));
    m_zapGateway.accessUrl(url, [this, url](bool ok, const std::string& err) {
        if (!ok) {
            emit operationFailed(QString("FAIL accessUrl: ") + QString::fromStdString(err));
            return;
        }
        m_zapGateway.startAjaxSpider(url, [this](bool started, const std::string& msg) {
            emit logMessage(started ? ">> AJAX SPIDER running..."
                                    : QString(">> ERROR: ") + QString::fromStdString(msg));
        });
    });
}

void ApplicationFacade::startActiveScan(const QString& targetUrl) {
    const auto validated = application::StartZapScanUseCase::validateTarget(targetUrl.toStdString());
    if (!validated.isOk()) {
        emit operationFailed(QString::fromStdString(validated.error().message()));
        return;
    }

    const std::string url = validated.value();
    m_audit.log("ACTIVE_SCAN", QString::fromStdString(url));
    emit logMessage(QString(">> ACTIVE SCAN → %1").arg(QString::fromStdString(url)));
    m_zapGateway.startActiveScan(url, [this](bool ok, const std::string& msg) {
        emit logMessage(ok ? ">> ACTIVE SCAN started."
                           : QString(">> ERROR: ") + QString::fromStdString(msg));
    });
}

void ApplicationFacade::stopScans() {
    m_audit.log("STOP_SCANS");
    m_zapGateway.stopScans();
    emit logMessage(">> AJAX SPIDER aborted.");
    emit logMessage(">> ACTIVE SCAN aborted.");
}

void ApplicationFacade::refreshFindings() {
    m_zapGateway.fetchAlerts([this](const std::vector<domain::SecurityAlert>& alerts,
                                    const std::string& error) {
        if (!error.empty()) {
            emit operationFailed(QString::fromStdString(error));
            return;
        }

        QVector<FindingView> rows;
        rows.reserve(static_cast<int>(alerts.size()));

        for (const auto& alert : alerts) {
            rows.push_back({"ZAP", QString::fromStdString(domain::riskLevelToString(alert.risk())),
                            QString::fromStdString(alert.name()),
                            QString::fromStdString(alert.url()),
                            QString::fromStdString(alert.description())});
        }

        if (!m_lastSummaryPath.isEmpty()) {
            const auto nuclei =
                m_summaryReader.loadNucleiFindings(m_lastSummaryPath.toStdString());
            for (const auto& item : nuclei) {
                rows.push_back(
                    {"nuclei", QString::fromStdString(item.severity),
                     QString::fromStdString(item.templateId), QString::fromStdString(item.url),
                     QString::fromStdString(item.description)});
            }
        }

        emit findingsReady(rows);
        emit logMessage(QString(">> %1 finding(s) loaded (ZAP + nuclei).").arg(rows.size()));
    });
}

void ApplicationFacade::pollScanStatus() {
    if (!m_zapGateway.isDaemonRunning()) return;

    m_zapGateway.ajaxSpiderStatus([this](const std::string& ajax) {
        if (ajax == "running") {
            emit scanStatusUpdated(">> AJAX SPIDER RUNNING...", "#00ffff");
        }
    });

    m_zapGateway.activeScanStatus([this](const std::string& status) {
        bool ok = false;
        const int pct = QString::fromStdString(status).toInt(&ok);
        if (ok && pct >= 0 && pct < 100) {
            emit scanStatusUpdated(QString(">> ACTIVE SCAN: %1%").arg(pct), "#ffb000");
        }
    });
}

QString ApplicationFacade::formatSummary(const std::string& summaryPath) const {
    const auto stats = m_summaryReader.load(summaryPath);
    if (!stats.valid) return ">> RECON SUMMARY: no data";
    return QString(">> RECON SUMMARY: %1 subdomains | %2 live hosts | %3 open ports | %4 paths | %5 vulns")
        .arg(stats.subdomains)
        .arg(stats.liveHosts)
        .arg(stats.openPorts)
        .arg(stats.paths)
        .arg(stats.vulnerabilities);
}

void ApplicationFacade::launchValidatedRecon(const application::ValidatedReconStart& validated) {
    m_reconLimiter.recordStart();
    m_activeScan = validated.scan;
    m_activeScan->markRunning();
    m_reconGateway.start(validated.options);
}

void ApplicationFacade::startRecon(const QString& targetUrl, bool authorized, bool fastMode,
                                   bool skipNuclei, bool useZapProxy) {
    QString rateReason;
    if (!m_reconLimiter.canStart(&rateReason)) {
        emit operationFailed(rateReason);
        return;
    }

    application::StartReconRequest request{
        targetUrl.toStdString(), authorized, fastMode, skipNuclei, useZapProxy};

    const auto validated = application::StartReconUseCase::validate(request);
    if (!validated.isOk()) {
        emit operationFailed(QString::fromStdString(validated.error().message()));
        return;
    }

    m_audit.log("START_RECON", targetUrl);
    emit logMessage(">> Running preflight checks...");
    m_preflightGateway.check([this, validated](const domain::PreflightReport& report) {
        if (!report.ok) {
            QString details = QString::fromStdString(report.error);
            if (!report.missingTools.empty()) {
                QStringList tools;
                for (const auto& t : report.missingTools) tools << QString::fromStdString(t);
                details = "Missing: " + tools.join(", ");
            }
            emit preflightFailed(details);
            return;
        }

        emit logMessage(">> Preflight OK — all tools available.");
        launchValidatedRecon(validated.value());
    });
}

void ApplicationFacade::stopRecon() {
    m_audit.log("STOP_RECON");
    m_reconGateway.stop();
    if (m_activeScan) m_activeScan->markCancelled();
}

void ApplicationFacade::feedZapAndScan(const std::string& summaryPath) {
    application::FeedZapRequest request{summaryPath, m_zapGateway.isDaemonRunning()};
    const auto validated = application::FeedZapUseCase::validate(request);
    if (!validated.isOk()) {
        emit operationFailed(QString::fromStdString(validated.error().message()));
        return;
    }

    const auto urls = m_summaryReader.loadUrls(summaryPath);
    if (urls.empty()) {
        emit operationFailed("No URLs found in summary.json");
        return;
    }

    m_audit.log("FEED_ZAP", QString::fromStdString(summaryPath));
    emit logMessage(">> Feeding ZAP with recon URLs...");

    struct FeedState {
        int seeded = 0;
        int pending = 0;
        QString error;
        std::vector<std::string> urls;
    };

    auto* state = new FeedState;
    state->urls = urls;
    state->pending = static_cast<int>(urls.size());

    for (const std::string& url : urls) {
        m_zapGateway.accessUrl(url, [this, state](bool ok, const std::string& err) {
            if (ok) {
                ++state->seeded;
            } else if (state->error.isEmpty()) {
                state->error = QString::fromStdString(err);
            }

            --state->pending;
            if (state->pending == 0) {
                emit logMessage(QString(">> %1 URL(s) sent to ZAP.").arg(state->seeded));
                if (!state->error.isEmpty()) {
                    emit logMessage(">> Warning: " + state->error);
                }
                if (!state->urls.empty()) {
                    startActiveScan(QString::fromStdString(state->urls.front()));
                }
                delete state;
            }
        });
    }
}

void ApplicationFacade::feedZapFromLastRecon() {
    feedZapAndScan(m_reconGateway.lastSummaryPath());
}

void ApplicationFacade::checkZapUpdate() {
    m_audit.log("CHECK_ZAP_UPDATE");
    emit logMessage(">> Checking OWASP ZAP updates...");
    m_zapGateway.checkForUpdates([this](const domain::ZapUpdateInfo& info) {
        if (info.latestVersion.empty()) {
            emit operationFailed("ZAP offline or could not fetch GitHub releases.");
            return;
        }
        emit zapUpdateChecked(info.updateAvailable, QString::fromStdString(info.localVersion),
                              QString::fromStdString(info.latestVersion),
                              QString::fromStdString(info.releaseUrl));
    });
}

void ApplicationFacade::runZapUpdate() {
    m_audit.log("RUN_ZAP_UPDATE");
    m_zapGateway.runUpdateScript([this](bool ok, const std::string& err) {
        emit logMessage(ok ? ">> ZAP updated successfully." : QString(">> ") + QString::fromStdString(err));
    });
}

void ApplicationFacade::runFullPipeline(const QString& targetUrl, bool authorized, bool fastMode,
                                        bool skipNuclei, bool useZapProxy) {
    application::FullPipelineRequest request{
        targetUrl.toStdString(), authorized, fastMode, skipNuclei, useZapProxy};

    const auto validated = application::RunFullPipelineUseCase::validate(request);
    if (!validated.isOk()) {
        emit operationFailed(QString::fromStdString(validated.error().message()));
        return;
    }

    m_audit.log("FULL_PIPELINE", targetUrl);
    emit logMessage(">> FULL PIPELINE: ZAP BOOT → RECON → FEED → ACTIVE SCAN");
    m_fullPipelinePending = true;

    auto beginRecon = [this, targetUrl, authorized, fastMode, skipNuclei, useZapProxy]() {
        startRecon(targetUrl, authorized, fastMode, skipNuclei, useZapProxy);
    };

    if (!m_zapGateway.isDaemonRunning()) {
        connect(&m_daemon, &ZapDaemon::stateChanged, this,
                [this, beginRecon](bool running) {
                    if (!running) return;
                    disconnect(&m_daemon, &ZapDaemon::stateChanged, this, nullptr);
                    beginRecon();
                },
                Qt::SingleShotConnection);
        bootZap();
    } else {
        beginRecon();
    }
}

void ApplicationFacade::reloadFromConfig() {
    AppConfig::instance().reload();
    applyClientSecurity();
}

}  // namespace presentation
