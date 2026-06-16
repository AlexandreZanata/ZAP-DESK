#include "MainWindow.hpp"

#include "components/AppTheme.hpp"
#include "components/CrtOverlay.hpp"
#include "components/KdeTheme.hpp"
#include "components/UiKit.hpp"
#include "components/FindingsTable.hpp"
#include "components/LogConsole.hpp"
#include "components/ReconSummaryPanel.hpp"
#include "components/ScanHistorySidebar.hpp"
#include "components/ScanProgressBar.hpp"
#include "components/SettingsDialog.hpp"
#include "components/StatusBanner.hpp"
#include "config/AppConfig.hpp"

#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    const QIcon appIcon(":/icons/zap.svg");
    if (!appIcon.isNull()) {
        setWindowIcon(appIcon);
    }

    setupUi();
    applyStyle();
    wireFacade();

    connect(&m_pollTimer, &QTimer::timeout, this, &MainWindow::onPollStatus);
    m_pollTimer.start(2000);

    appendLog("ZAP-DESK v0.8.0 — security terminal online");
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
                    m_statusBanner->setStatusText("STATUS: ZAP BOOTING...");
                    m_statusBanner->setStatusColor(statusColor("warning"));
                    return;
                }
                m_statusBanner->setStatusText(QString("STATUS: ONLINE — ZAP %1").arg(version));
                m_statusBanner->setStatusColor(statusColor("online"));
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
            [this](const QString& text, const QString& /*colorHex*/) {
                m_statusBanner->setStatusText(text);
                const QString role = text.contains("ACTIVE SCAN") ? "warning" : "scan";
                m_statusBanner->setStatusColor(statusColor(role));
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
    auto* fileMenu = menuBar()->addMenu("File");
    auto* settingsAction = fileMenu->addAction("Settings...");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onOpenSettings);

    auto* viewMenu = menuBar()->addMenu("View");
    auto* themeMenu = viewMenu->addMenu("Theme");

    auto* themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);

    auto addThemeAction = [&](const QString& label, components::AppThemeMode mode) {
        auto* action = themeMenu->addAction(label);
        action->setCheckable(true);
        action->setData(components::AppTheme::toString(mode));
        themeGroup->addAction(action);
        connect(action, &QAction::triggered, this, [this, mode]() { setThemeMode(mode); });
    };

    addThemeAction("Light", components::AppThemeMode::Light);
    addThemeAction("Dark", components::AppThemeMode::Dark);
    addThemeAction("Hacker (CRT)", components::AppThemeMode::Hacker);

    auto* central = new QWidget(this);
    central->setObjectName("centralRoot");
    auto* centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    auto* outer = new QHBoxLayout;
    outer->setContentsMargins(components::UiKit::kOuterMargin, components::UiKit::kOuterMargin,
                              components::UiKit::kOuterMargin, components::UiKit::kOuterMargin);
    outer->setSpacing(components::UiKit::kColumnGap);

    m_history = new components::ScanHistorySidebar;
    outer->addWidget(m_history, 0);

    auto* mainPanel = new QWidget;
    mainPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* mainColumn = new QVBoxLayout(mainPanel);
    mainColumn->setContentsMargins(0, 0, 0, 0);
    mainColumn->setSpacing(components::UiKit::kSectionGap);

    m_statusBanner = new components::StatusBanner("0.8.0");
    m_themeBtn = m_statusBanner->themeButton();
    connect(m_themeBtn, &QPushButton::clicked, this, &MainWindow::onCycleTheme);
    mainColumn->addWidget(m_statusBanner, 0);

    m_tabs = new QTabWidget;
    m_tabs->setObjectName("mainTabs");
    m_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tabs->setMinimumHeight(0);
    m_tabs->tabBar()->setExpanding(true);
    m_tabs->tabBar()->setDrawBase(false);

    auto* zapTab = new QWidget;
    zapTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    zapTab->setMinimumHeight(0);
    auto* zapTabLayout = new QVBoxLayout(zapTab);
    zapTabLayout->setContentsMargins(0, 0, 0, 0);
    zapTabLayout->setSpacing(0);
    QWidget* zapPage{};
    auto* zapScroll = components::UiKit::createTabScrollPage(zapPage, zapTab);
    zapScroll->setMinimumHeight(0);
    zapTabLayout->addWidget(zapScroll, 1);

    auto* reconTab = new QWidget;
    reconTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    reconTab->setMinimumHeight(0);
    auto* reconTabLayout = new QVBoxLayout(reconTab);
    reconTabLayout->setContentsMargins(0, 0, 0, 0);
    reconTabLayout->setSpacing(0);
    QWidget* reconPage{};
    auto* reconScroll = components::UiKit::createTabScrollPage(reconPage, reconTab);
    reconScroll->setMinimumHeight(0);
    reconTabLayout->addWidget(reconScroll, 1);

    m_tabs->addTab(zapTab, "ZAP Scanner");
    m_tabs->addTab(reconTab, "Recon Pipeline");

    m_logConsole = new components::LogConsole;
    m_logConsole->setMinimumHeight(components::UiKit::kLogMinHeight + 72);

    auto* workSplitter = new QSplitter(Qt::Vertical, mainPanel);
    workSplitter->setObjectName("workSplitter");
    workSplitter->setChildrenCollapsible(false);
    workSplitter->setHandleWidth(5);
    workSplitter->addWidget(m_tabs);
    workSplitter->addWidget(m_logConsole);
    workSplitter->setStretchFactor(0, 4);
    workSplitter->setStretchFactor(1, 1);
    workSplitter->setSizes({620, 150});

    mainColumn->addWidget(workSplitter, 1);

    outer->addWidget(mainPanel, 1);

    auto* zapLayout = components::UiKit::createPageLayout(zapPage);

    auto targetSection = components::UiKit::createSection(
        "Target URL", "Authorized target for ZAP scans and spider.", zapPage);
    m_urlInput = new QLineEdit(zapPage);
    m_urlInput->setText("https://");
    m_urlInput->setPlaceholderText("https://authorized-target.com");
    m_urlInput->setClearButtonEnabled(true);
    targetSection.layout->addWidget(m_urlInput);
    zapLayout->addWidget(targetSection.frame);

    auto actionsSection = components::UiKit::createSection(
        "Scanner Actions", "Boot ZAP, run scans, and refresh findings.", zapPage);
    auto* zapActions = components::UiKit::createActionGrid();

    m_startZapBtn = new QPushButton("Boot ZAP");
    m_stopZapBtn = new QPushButton("Stop ZAP");
    m_ajaxBtn = new QPushButton("Ajax Spider");
    m_activeBtn = new QPushButton("Active Scan");
    m_stopScanBtn = new QPushButton("Abort Scan");
    m_updateZapBtn = new QPushButton("Check ZAP Update");
    auto* refreshBtn = new QPushButton("Refresh Findings");

    components::UiKit::stylePrimaryButton(m_startZapBtn);
    components::UiKit::styleDangerButton(m_stopZapBtn);
    components::UiKit::tuneActionButton(m_ajaxBtn);
    components::UiKit::stylePrimaryButton(m_activeBtn);
    components::UiKit::styleDangerButton(m_stopScanBtn);
    components::UiKit::tuneActionButton(m_updateZapBtn);
    components::UiKit::tuneActionButton(refreshBtn);

    zapActions->addWidget(m_startZapBtn, 0, 0);
    zapActions->addWidget(m_stopZapBtn, 0, 1);
    zapActions->addWidget(m_ajaxBtn, 1, 0);
    zapActions->addWidget(m_activeBtn, 1, 1);
    zapActions->addWidget(m_stopScanBtn, 2, 0);
    zapActions->addWidget(refreshBtn, 2, 1);
    zapActions->addWidget(m_updateZapBtn, 3, 0, 1, 2);
    zapActions->setColumnStretch(0, 1);
    zapActions->setColumnStretch(1, 1);
    for (int row = 0; row < 4; ++row) {
        zapActions->setRowMinimumHeight(row, components::UiKit::kButtonMinHeight);
    }
    actionsSection.layout->addLayout(zapActions);
    zapLayout->addWidget(actionsSection.frame);

    m_findings = new components::FindingsTable(zapPage);
    zapLayout->addWidget(m_findings, 0);

    auto* reconLayout = components::UiKit::createPageLayout(reconPage);

    m_authorized = new QCheckBox("I confirm written authorization to test this target", reconPage);
    m_fastMode = new QCheckBox("Fast mode (--fast)", reconPage);
    m_skipNuclei = new QCheckBox("Skip nuclei (--skip-nuclei)", reconPage);
    m_useProxy = new QCheckBox("Route via ZAP proxy (127.0.0.1:8080)", reconPage);
    m_useProxy->setChecked(true);

    auto optionsSection = components::UiKit::createSection(
        "Recon Options", "Configure pipeline behavior before starting.", reconPage);
    optionsSection.layout->setSpacing(8);
    optionsSection.layout->addWidget(m_authorized);
    optionsSection.layout->addWidget(m_fastMode);
    optionsSection.layout->addWidget(m_skipNuclei);
    optionsSection.layout->addWidget(m_useProxy);
    reconLayout->addWidget(optionsSection.frame);

    auto reconActionsSection = components::UiKit::createSection("Pipeline Actions", QString(), reconPage);
    auto* reconActions = components::UiKit::createActionGrid();

    m_reconBtn = new QPushButton("Run Recon");
    m_stopReconBtn = new QPushButton("Stop Recon");
    m_feedZapBtn = new QPushButton("Feed ZAP");
    m_pipelineBtn = new QPushButton("Full Pipeline");
    m_stopReconBtn->setEnabled(false);
    m_feedZapBtn->setEnabled(false);

    components::UiKit::stylePrimaryButton(m_reconBtn);
    components::UiKit::styleDangerButton(m_stopReconBtn);
    components::UiKit::tuneActionButton(m_feedZapBtn);
    components::UiKit::stylePrimaryButton(m_pipelineBtn);

    reconActions->addWidget(m_reconBtn, 0, 0);
    reconActions->addWidget(m_stopReconBtn, 0, 1);
    reconActions->addWidget(m_feedZapBtn, 1, 0);
    reconActions->addWidget(m_pipelineBtn, 1, 1);
    reconActions->setColumnStretch(0, 1);
    reconActions->setColumnStretch(1, 1);
    for (int row = 0; row < 2; ++row) {
        reconActions->setRowMinimumHeight(row, components::UiKit::kButtonMinHeight);
    }
    reconActionsSection.layout->addLayout(reconActions);
    reconLayout->addWidget(reconActionsSection.frame);

    m_reconProgress = new components::ScanProgressBar(reconPage);
    m_reconSummary = new components::ReconSummaryPanel(reconPage);
    reconLayout->addWidget(m_reconProgress, 0);
    reconLayout->addWidget(m_reconSummary, 0);

    centralLayout->addLayout(outer, 1);

    setCentralWidget(central);
    components::AppTheme::applyWindowBackground(central, AppConfig::instance().uiThemeMode());
    setMinimumSize(1024, 720);
    resize(1320, 860);
    setWindowTitle("ZAP-DESK");

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

    updateThemeUi();
}

void MainWindow::setThemeMode(components::AppThemeMode mode) {
    AppConfig::instance().setUiTheme(components::AppTheme::toString(mode));
    applyTheme();
    updateThemeUi();

    if (m_app.isZapRunning()) {
        m_statusBanner->setStatusColor(statusColor("online"));
    } else {
        setConnectedUi(false);
    }

    appendLog(">> Theme: " + components::AppTheme::displayName(mode));
}

void MainWindow::updateThemeUi() {
    const auto mode = AppConfig::instance().uiThemeMode();
    if (m_themeBtn) {
        m_themeBtn->setText("THEME: " + components::AppTheme::displayName(mode));
    }

    const auto actions = findChildren<QAction*>();
    const QString current = components::AppTheme::toString(mode);
    for (QAction* action : actions) {
        if (action->actionGroup() && action->data().isValid()) {
            action->setChecked(action->data().toString() == current);
        }
    }
}

void MainWindow::onCycleTheme() {
    const auto mode = AppConfig::instance().uiThemeMode();
    switch (mode) {
        case components::AppThemeMode::Light:
            setThemeMode(components::AppThemeMode::Dark);
            break;
        case components::AppThemeMode::Dark:
            setThemeMode(components::AppThemeMode::Hacker);
            break;
        case components::AppThemeMode::Hacker:
        default:
            setThemeMode(components::AppThemeMode::Light);
            break;
    }
}

void MainWindow::applyStyle() {
    applyTheme();
}

void MainWindow::applyTheme() {
    const auto mode = AppConfig::instance().uiThemeMode();
    components::KdeTheme::apply(mode);
    if (centralWidget()) {
        components::AppTheme::refreshSurfaces(centralWidget(), mode);
    }
    if (m_statusBanner) {
        m_statusBanner->applyTheme(mode);
    }
    if (m_history) {
        m_history->applyTheme(mode);
    }
    if (m_findings) {
        m_findings->applyTheme(mode);
    }
    applyCrtOverlaySetting();
}

QString MainWindow::statusColor(const QString& role) const {
    return components::AppTheme::statusColor(AppConfig::instance().uiThemeMode(), role);
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
        m_statusBanner->setStatusText("STATUS: OFFLINE");
        m_statusBanner->setStatusColor(statusColor("offline"));
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
    applyTheme();
    updateThemeUi();
    appendLog(">> Settings saved — restart ZAP if port or home changed.");
}
