#include "MainWindow.hpp"

#include "components/CrtOverlay.hpp"
#include "components/FindingsTable.hpp"
#include "components/HackerTheme.hpp"
#include "components/LogConsole.hpp"
#include "components/ReconSummaryPanel.hpp"
#include "components/ScanHistorySidebar.hpp"
#include "components/ScanProgressBar.hpp"
#include "components/SettingsDialog.hpp"
#include "components/StatusBanner.hpp"
#include "config/AppConfig.hpp"

#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QTabWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    applyStyle();
    wireFacade();

    connect(&m_pollTimer, &QTimer::timeout, this, &MainWindow::onPollStatus);
    m_pollTimer.start(2000);

    appendLog("ZAP-DESK v0.6.0 — security terminal online");
    appendLog("Reconner path: " + AppConfig::instance().reconnerDir());
    if (AppConfig::instance().zapApiKey().isEmpty() && !AppConfig::instance().zapDevMode()) {
        appendLog(">> Security: enable dev mode in Settings for localhost without API key.");
    }
    setConnectedUi(m_app.isZapRunning());
    if (m_app.isZapRunning()) m_app.checkZapConnection();
}

void MainWindow::wireFacade() {
    connect(&m_app, &presentation::ApplicationFacade::logMessage, this, &MainWindow::appendLog);
    connect(&m_app, &presentation::ApplicationFacade::operationFailed, this, [this](const QString& msg) {
        appendLog(">> ERROR: " + msg);
        if (!m_reconBtn->isEnabled()) {
            m_reconBtn->setEnabled(true);
            m_stopReconBtn->setEnabled(false);
        }
    });

    connect(&m_app, &presentation::ApplicationFacade::zapDaemonStateChanged, this,
            [this](bool running) {
                setConnectedUi(running);
                if (running) m_app.checkZapConnection();
            });

    connect(&m_app, &presentation::ApplicationFacade::zapConnectionChecked, this,
            [this](bool ok, const QString& version) {
                if (!ok) {
                    m_statusBanner->setStatusText(">> STATUS: ZAP BOOTING...");
                    m_statusBanner->setStatusColor("#ffb000");
                    return;
                }
                m_statusBanner->setStatusText(QString(">> STATUS: ONLINE — ZAP %1").arg(version));
                m_statusBanner->setStatusColor("#00ff41");
                setConnectedUi(true);
                appendLog(QString(">> ZAP connected: v%1").arg(version));
            });

    connect(&m_app, &presentation::ApplicationFacade::reconPhaseChanged, this,
            [this](int step, int total, const QString& phase, const QString& message) {
                m_reconProgress->setPhase(step, total, phase, message);
            });

    connect(&m_app, &presentation::ApplicationFacade::reconFinished, this,
            [this](bool ok, const QString& summaryPath, const QString& summaryText) {
                m_reconBtn->setEnabled(true);
                m_stopReconBtn->setEnabled(false);
                m_feedZapBtn->setEnabled(ok && !summaryPath.isEmpty());
                m_history->addEntry(currentTarget(), ok);
                if (ok) {
                    m_reconSummary->setSummaryText(summaryText);
                    appendLog(">> RECON COMPLETE — " + summaryPath);
                    m_app.refreshFindings();
                } else {
                    m_reconProgress->setFailed();
                    m_reconSummary->clear();
                    appendLog(">> RECON FAILED");
                }
            });

    connect(&m_app, &presentation::ApplicationFacade::preflightFailed, this,
            [this](const QString& details) {
                m_reconBtn->setEnabled(true);
                m_stopReconBtn->setEnabled(false);
                appendLog(">> PREFLIGHT FAILED: " + details);
                QMessageBox::warning(this, "Preflight failed",
                                     "Recon prerequisites not met.\n\n" + details +
                                         "\n\nSee docs/ZAP-INSTALL-LINUX.md and reconner/install-tools.sh");
            });

    connect(&m_app, &presentation::ApplicationFacade::findingsReady, this,
            [this](const QVector<presentation::FindingView>& findings) {
                QVector<components::FindingRow> rows;
                rows.reserve(findings.size());
                for (const auto& finding : findings) {
                    rows.push_back({finding.source, finding.risk, finding.name, finding.url,
                                    finding.description});
                }
                m_findings->setFindings(rows);
            });

    connect(&m_app, &presentation::ApplicationFacade::scanStatusUpdated, this,
            [this](const QString& text, const QString& colorHex) {
                m_statusBanner->setStatusText(text);
                m_statusBanner->setStatusColor(colorHex);
            });

    connect(&m_app, &presentation::ApplicationFacade::zapUpdateChecked, this,
            [this](bool updateAvailable, const QString& localVersion, const QString& latestVersion,
                   const QString& releaseUrl) {
                if (updateAvailable) {
                    appendLog(QString(">> UPDATE AVAILABLE: v%1 → v%2").arg(localVersion, latestVersion));
                    appendLog(">> Release: " + releaseUrl);
                    const auto answer = QMessageBox::question(
                        this, "ZAP Update",
                        QString("New version %1 available (local: %2).\nRun scripts/update-zap.sh?")
                            .arg(latestVersion, localVersion));
                    if (answer == QMessageBox::Yes) {
                        m_app.runZapUpdate();
                    }
                } else {
                    appendLog(QString(">> ZAP is up to date (v%1).").arg(localVersion));
                }
            });
}

void MainWindow::setupUi() {
    auto* menu = menuBar()->addMenu("File");
    auto* settingsAction = menu->addAction("Settings...");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onOpenSettings);

    auto* central = new QWidget(this);
    auto* outer = new QHBoxLayout(central);

    m_history = new components::ScanHistorySidebar;
    outer->addWidget(m_history);

    auto* mainColumn = new QVBoxLayout;
    m_statusBanner = new components::StatusBanner("0.6.0");
    mainColumn->addWidget(m_statusBanner);

    m_tabs = new QTabWidget;
    auto* zapTab = new QWidget;
    auto* reconTab = new QWidget;
    m_tabs->addTab(zapTab, "[ ZAP ]");
    m_tabs->addTab(reconTab, "[ RECON ]");
    mainColumn->addWidget(m_tabs, 1);

    auto* zapLayout = new QVBoxLayout(zapTab);

    auto* targetBox = new QGroupBox(">> TARGET");
    auto* targetLayout = new QHBoxLayout(targetBox);
    m_urlInput = new QLineEdit("https://");
    m_urlInput->setPlaceholderText("https://authorized-target.com");
    targetLayout->addWidget(m_urlInput);
    zapLayout->addWidget(targetBox);

    auto* zapActions = new QGridLayout;
    m_startZapBtn = new QPushButton("BOOT ZAP");
    m_stopZapBtn = new QPushButton("KILL ZAP");
    m_ajaxBtn = new QPushButton("AJAX SPIDER");
    m_activeBtn = new QPushButton("ACTIVE SCAN");
    m_stopScanBtn = new QPushButton("ABORT SCAN");
    m_updateZapBtn = new QPushButton("CHECK ZAP UPDATE");
    auto* refreshBtn = new QPushButton("REFRESH FINDINGS");

    zapActions->addWidget(m_startZapBtn, 0, 0);
    zapActions->addWidget(m_stopZapBtn, 0, 1);
    zapActions->addWidget(m_ajaxBtn, 1, 0);
    zapActions->addWidget(m_activeBtn, 1, 1);
    zapActions->addWidget(m_stopScanBtn, 2, 0);
    zapActions->addWidget(refreshBtn, 2, 1);
    zapActions->addWidget(m_updateZapBtn, 3, 0, 1, 2);
    zapLayout->addLayout(zapActions);

    m_findings = new components::FindingsTable;
    zapLayout->addWidget(m_findings, 1);

    auto* reconLayout = new QVBoxLayout(reconTab);

    m_authorized = new QCheckBox("I confirm written authorization to test this target");
    m_fastMode = new QCheckBox("Fast mode (--fast)");
    m_skipNuclei = new QCheckBox("Skip nuclei (--skip-nuclei)");
    m_useProxy = new QCheckBox("Route via ZAP proxy (127.0.0.1:8080)");
    m_useProxy->setChecked(true);

    reconLayout->addWidget(m_authorized);
    reconLayout->addWidget(m_fastMode);
    reconLayout->addWidget(m_skipNuclei);
    reconLayout->addWidget(m_useProxy);

    auto* reconActions = new QHBoxLayout;
    m_reconBtn = new QPushButton("RUN RECON");
    m_stopReconBtn = new QPushButton("STOP RECON");
    m_feedZapBtn = new QPushButton("FEED ZAP");
    m_pipelineBtn = new QPushButton("FULL PIPELINE");
    m_stopReconBtn->setEnabled(false);
    m_feedZapBtn->setEnabled(false);

    reconActions->addWidget(m_reconBtn);
    reconActions->addWidget(m_stopReconBtn);
    reconActions->addWidget(m_feedZapBtn);
    reconActions->addWidget(m_pipelineBtn);
    reconLayout->addLayout(reconActions);

    m_reconProgress = new components::ScanProgressBar;
    m_reconSummary = new components::ReconSummaryPanel;
    reconLayout->addWidget(m_reconProgress);
    reconLayout->addWidget(m_reconSummary);

    auto* reconInfo = new QLabel(
        "Pipeline: subfinder → httpx → nmap → whatweb → gobuster → nuclei\n"
        "Results dir: " + AppConfig::instance().resultsDir());
    reconInfo->setObjectName("hint");
    reconLayout->addWidget(reconInfo);
    reconLayout->addStretch();

    m_logConsole = new components::LogConsole;
    mainColumn->addWidget(m_logConsole);

    outer->addLayout(mainColumn, 1);

    setCentralWidget(central);
    resize(1280, 820);
    setWindowTitle("ZAP-DESK // RECONNER");

    m_crtOverlay = new components::CrtOverlay(central);
    applyCrtOverlaySetting();

    connect(m_startZapBtn, &QPushButton::clicked, this, &MainWindow::onStartZap);
    connect(m_stopZapBtn, &QPushButton::clicked, this, &MainWindow::onStopZap);
    connect(m_ajaxBtn, &QPushButton::clicked, this, &MainWindow::onAjaxScan);
    connect(m_activeBtn, &QPushButton::clicked, this, &MainWindow::onActiveScan);
    connect(m_stopScanBtn, &QPushButton::clicked, this, &MainWindow::onStopScans);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshFindings);
    connect(m_updateZapBtn, &QPushButton::clicked, this, &MainWindow::onCheckZapUpdate);
    connect(m_reconBtn, &QPushButton::clicked, this, &MainWindow::onStartRecon);
    connect(m_stopReconBtn, &QPushButton::clicked, this, &MainWindow::onStopRecon);
    connect(m_feedZapBtn, &QPushButton::clicked, this, &MainWindow::onFeedZapFromRecon);
    connect(m_pipelineBtn, &QPushButton::clicked, this, &MainWindow::onFullPipeline);
}

void MainWindow::applyStyle() {
    setStyleSheet(components::HackerTheme::stylesheet());
}

void MainWindow::applyCrtOverlaySetting() {
    const bool enabled = AppConfig::instance().crtOverlayEnabled();
    m_crtOverlay->setVisible(enabled);
    if (enabled && centralWidget()) {
        m_crtOverlay->setGeometry(centralWidget()->rect());
        m_crtOverlay->raise();
    }
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    if (m_crtOverlay && centralWidget()) {
        m_crtOverlay->setGeometry(centralWidget()->rect());
    }
}

void MainWindow::appendLog(const QString& message) {
    m_logConsole->append(message);
}

QString MainWindow::currentTarget() const {
    return m_urlInput->text().trimmed();
}

void MainWindow::setConnectedUi(bool connected) {
    m_startZapBtn->setEnabled(!connected);
    m_stopZapBtn->setEnabled(connected);
    m_ajaxBtn->setEnabled(connected);
    m_activeBtn->setEnabled(connected);
    m_stopScanBtn->setEnabled(connected);

    if (!connected) {
        m_statusBanner->setStatusText(">> STATUS: OFFLINE");
        m_statusBanner->setStatusColor("#ff3333");
    }
}

void MainWindow::onStartZap() { m_app.bootZap(); }
void MainWindow::onStopZap() {
    m_app.shutdownZap();
    setConnectedUi(false);
}
void MainWindow::onAjaxScan() { m_app.startAjaxScan(currentTarget()); }
void MainWindow::onActiveScan() { m_app.startActiveScan(currentTarget()); }
void MainWindow::onStopScans() { m_app.stopScans(); }
void MainWindow::onRefreshFindings() { m_app.refreshFindings(); }
void MainWindow::onPollStatus() { m_app.pollScanStatus(); }

void MainWindow::onStartRecon() {
    m_reconBtn->setEnabled(false);
    m_stopReconBtn->setEnabled(true);
    m_feedZapBtn->setEnabled(false);
    m_reconProgress->setPhase(0, 1, "init", "starting...");
    m_reconSummary->clear();

    m_app.startRecon(currentTarget(), m_authorized->isChecked(), m_fastMode->isChecked(),
                     m_skipNuclei->isChecked(), m_useProxy->isChecked());
}

void MainWindow::onStopRecon() {
    m_app.stopRecon();
    m_reconBtn->setEnabled(true);
    m_stopReconBtn->setEnabled(false);
}

void MainWindow::onFeedZapFromRecon() { m_app.feedZapFromLastRecon(); }
void MainWindow::onCheckZapUpdate() { m_app.checkZapUpdate(); }

void MainWindow::onFullPipeline() {
    m_app.runFullPipeline(currentTarget(), m_authorized->isChecked(), m_fastMode->isChecked(),
                          m_skipNuclei->isChecked(), m_useProxy->isChecked());
}

void MainWindow::onOpenSettings() {
    components::SettingsDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) return;

    dialog.saveToConfig();
    m_app.reloadFromConfig();
    applyCrtOverlaySetting();
    appendLog(">> Settings saved — restart ZAP if port or home changed.");
}
