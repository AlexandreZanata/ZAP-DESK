#pragma once

#include "core/ZapClient.hpp"
#include "core/ZapDaemon.hpp"
#include "domain/alert/SecurityAlert.hpp"
#include "application/usecases/StartReconUseCase.hpp"
#include "domain/scan/Scan.hpp"
#include "infrastructure/recon/JsonReconSummaryReader.hpp"
#include "infrastructure/recon/QtPreflightGateway.hpp"
#include "infrastructure/recon/QtReconGateway.hpp"
#include "infrastructure/zap/QtZapGateway.hpp"
#include "services/ReconPreflight.hpp"
#include "services/ReconRunner.hpp"
#include "services/ZapUpdater.hpp"

#include "security/AuditLogger.hpp"
#include "security/ReconRateLimiter.hpp"

#include <QObject>
#include <optional>

namespace presentation {

struct FindingView {
    QString source;
    QString risk;
    QString name;
    QString url;
    QString description;
};

class ApplicationFacade : public QObject {
    Q_OBJECT

public:
    explicit ApplicationFacade(QObject* parent = nullptr);

    void bootZap();
    void shutdownZap();
    void checkZapConnection();

    void startAjaxScan(const QString& targetUrl);
    void startActiveScan(const QString& targetUrl);
    void stopScans();
    void refreshFindings();
    void pollScanStatus();

    void startRecon(const QString& targetUrl, bool authorized, bool fastMode, bool skipNuclei,
                    bool useZapProxy);
    void stopRecon();
    void feedZapFromLastRecon();
    void checkZapUpdate();
    void runZapUpdate();
    void runFullPipeline(const QString& targetUrl, bool authorized, bool fastMode, bool skipNuclei,
                         bool useZapProxy);

    void reloadFromConfig();
    bool isZapRunning() const;

signals:
    void logMessage(const QString& message);
    void zapDaemonStateChanged(bool running);
    void zapConnectionChecked(bool ok, const QString& version);
    void reconPhaseChanged(int step, int total, const QString& phase, const QString& message);
    void reconFinished(bool ok, const QString& summaryPath, const QString& summaryText);
    void preflightFailed(const QString& details);
    void findingsReady(const QVector<FindingView>& findings);
    void zapUpdateChecked(bool updateAvailable, const QString& localVersion,
                          const QString& latestVersion, const QString& releaseUrl);
    void operationFailed(const QString& message);
    void scanStatusUpdated(const QString& text, const QString& colorHex);

private:
    void wireServices();
    void applyClientSecurity();
    void launchValidatedRecon(const application::ValidatedReconStart& validated);
    QString formatSummary(const std::string& summaryPath) const;
    void feedZapAndScan(const std::string& summaryPath);

    ZapClient m_client;
    ZapDaemon m_daemon;
    ReconRunner m_recon;
    ReconPreflight m_preflight;
    ZapUpdater m_updater;

    infrastructure::JsonReconSummaryReader m_summaryReader;
    infrastructure::QtReconGateway m_reconGateway;
    infrastructure::QtPreflightGateway m_preflightGateway;
    infrastructure::QtZapGateway m_zapGateway;

    security::AuditLogger m_audit;
    security::ReconRateLimiter m_reconLimiter;

    std::optional<domain::Scan> m_activeScan;
    bool m_fullPipelinePending{false};
    QString m_lastSummaryPath;
};

}  // namespace presentation
