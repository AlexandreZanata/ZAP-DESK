#include "MainWindow.hpp"

#include "config/AppConfig.hpp"

#include <QCheckBox>
#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    applyStyle();
    wireFacade();

    connect(&m_pollTimer, &QTimer::timeout, this, &MainWindow::onPollStatus);
    m_pollTimer.start(2000);

    appendLog("ZAP-DESK v0.3.0 — security terminal online");
    appendLog("Reconner path: " + AppConfig::instance().reconnerDir());
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
                    m_statusLabel->setText(">> STATUS: ZAP BOOTING...");
                    m_statusLabel->setStyleSheet("color: #ffb000;");
                    return;
                }
                m_statusLabel->setText(QString(">> STATUS: ONLINE — ZAP %1").arg(version));
                m_statusLabel->setStyleSheet("color: #00ff41;");
                setConnectedUi(true);
                appendLog(QString(">> ZAP connected: v%1").arg(version));
            });

    connect(&m_app, &presentation::ApplicationFacade::reconPhaseChanged, this,
            [this](int step, int total, const QString& phase, const QString& message) {
                m_reconProgressLabel->setText(
                    QString(">> RECON [%1/%2] %3 — %4").arg(step).arg(total).arg(phase, message));
            });

    connect(&m_app, &presentation::ApplicationFacade::reconFinished, this,
            [this](bool ok, const QString& summaryPath, const QString& summaryText) {
                m_reconBtn->setEnabled(true);
                m_stopReconBtn->setEnabled(false);
                m_feedZapBtn->setEnabled(ok && !summaryPath.isEmpty());
                if (ok) {
                    m_reconSummaryLabel->setText(summaryText);
                    appendLog(">> RECON COMPLETE — " + summaryPath);
                } else {
                    m_reconProgressLabel->setText(">> RECON FAILED");
                    m_reconSummaryLabel->setText(">> RECON SUMMARY: —");
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

    connect(&m_app, &presentation::ApplicationFacade::alertsReady, this,
            [this](const QVector<presentation::AlertView>& alerts) {
                m_alertsTable->setRowCount(alerts.size());
                for (int i = 0; i < alerts.size(); ++i) {
                    const auto& a = alerts[i];
                    m_alertsTable->setItem(i, 0, new QTableWidgetItem(a.risk));
                    m_alertsTable->setItem(i, 1, new QTableWidgetItem(a.name));
                    m_alertsTable->setItem(i, 2, new QTableWidgetItem(a.url));
                    m_alertsTable->setItem(i, 3, new QTableWidgetItem(a.description));
                }
            });

    connect(&m_app, &presentation::ApplicationFacade::scanStatusUpdated, this,
            [this](const QString& text, const QString& colorHex) {
                m_statusLabel->setText(text);
                m_statusLabel->setStyleSheet(QString("color: %1;").arg(colorHex));
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
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);

    m_asciiBanner = new QLabel(
        "╔══════════════════════════════════════════════════════════╗\n"
        "║  ZAP-DESK // RECONNER — SECURITY TERMINAL v0.3.0         ║\n"
        "║  [ OWASP ZAP + RECON PIPELINE ]  :: LINUX EDITION ::    ║\n"
        "╚══════════════════════════════════════════════════════════╝");
    m_asciiBanner->setObjectName("banner");
    root->addWidget(m_asciiBanner);

    m_statusLabel = new QLabel(">> STATUS: OFFLINE");
    m_statusLabel->setObjectName("status");
    root->addWidget(m_statusLabel);

    m_tabs = new QTabWidget;
    auto* zapTab = new QWidget;
    auto* reconTab = new QWidget;
    m_tabs->addTab(zapTab, "[ ZAP ]");
    m_tabs->addTab(reconTab, "[ RECON ]");
    root->addWidget(m_tabs, 1);

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
    auto* refreshBtn = new QPushButton("REFRESH ALERTS");

    zapActions->addWidget(m_startZapBtn, 0, 0);
    zapActions->addWidget(m_stopZapBtn, 0, 1);
    zapActions->addWidget(m_ajaxBtn, 1, 0);
    zapActions->addWidget(m_activeBtn, 1, 1);
    zapActions->addWidget(m_stopScanBtn, 2, 0);
    zapActions->addWidget(refreshBtn, 2, 1);
    zapActions->addWidget(m_updateZapBtn, 3, 0, 1, 2);
    zapLayout->addLayout(zapActions);

    m_alertsTable = new QTableWidget(0, 4);
    m_alertsTable->setHorizontalHeaderLabels({"RISK", "ALERT", "URL", "DESC"});
    m_alertsTable->horizontalHeader()->setStretchLastSection(true);
    m_alertsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_alertsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_alertsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    zapLayout->addWidget(new QLabel(">> ALERTS"));
    zapLayout->addWidget(m_alertsTable, 1);

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

    m_reconProgressLabel = new QLabel(">> RECON: idle");
    m_reconProgressLabel->setObjectName("hint");
    reconLayout->addWidget(m_reconProgressLabel);

    m_reconSummaryLabel = new QLabel(">> RECON SUMMARY: —");
    m_reconSummaryLabel->setObjectName("hint");
    reconLayout->addWidget(m_reconSummaryLabel);

    auto* reconInfo = new QLabel(
        "Pipeline: subfinder → httpx → nmap → whatweb → gobuster → nuclei\n"
        "Results dir: " + AppConfig::instance().resultsDir());
    reconInfo->setObjectName("hint");
    reconLayout->addWidget(reconInfo);
    reconLayout->addStretch();

    m_log = new QTextEdit;
    m_log->setReadOnly(true);
    m_log->setMaximumHeight(180);
    root->addWidget(new QLabel(">> SYSTEM LOG"));
    root->addWidget(m_log);

    setCentralWidget(central);
    resize(1180, 780);
    setWindowTitle("ZAP-DESK // RECONNER");

    connect(m_startZapBtn, &QPushButton::clicked, this, &MainWindow::onStartZap);
    connect(m_stopZapBtn, &QPushButton::clicked, this, &MainWindow::onStopZap);
    connect(m_ajaxBtn, &QPushButton::clicked, this, &MainWindow::onAjaxScan);
    connect(m_activeBtn, &QPushButton::clicked, this, &MainWindow::onActiveScan);
    connect(m_stopScanBtn, &QPushButton::clicked, this, &MainWindow::onStopScans);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshAlerts);
    connect(m_updateZapBtn, &QPushButton::clicked, this, &MainWindow::onCheckZapUpdate);
    connect(m_reconBtn, &QPushButton::clicked, this, &MainWindow::onStartRecon);
    connect(m_stopReconBtn, &QPushButton::clicked, this, &MainWindow::onStopRecon);
    connect(m_feedZapBtn, &QPushButton::clicked, this, &MainWindow::onFeedZapFromRecon);
    connect(m_pipelineBtn, &QPushButton::clicked, this, &MainWindow::onFullPipeline);
}

void MainWindow::applyStyle() {
    setStyleSheet(R"(
        * { font-family: "Courier New", "DejaVu Sans Mono", "Liberation Mono", monospace; }
        QMainWindow, QWidget { background: #0a0a0a; color: #00ff41; font-size: 13px; }
        #banner { color: #00ff41; font-size: 12px; padding: 6px; background: #050505;
                  border: 1px solid #00ff41; border-radius: 0; }
        #status { color: #ffb000; font-weight: bold; padding: 4px 0; }
        #hint { color: #00cccc; font-size: 12px; }
        QTabWidget::pane { border: 1px solid #00ff41; background: #0a0a0a; }
        QTabBar::tab { background: #111; color: #00ff41; padding: 8px 16px;
                       border: 1px solid #004400; margin-right: 2px; }
        QTabBar::tab:selected { background: #002200; color: #00ff41; border-color: #00ff41; }
        QGroupBox { border: 1px solid #004400; border-radius: 0; margin-top: 14px; padding-top: 14px;
                    color: #00cccc; font-weight: bold; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }
        QLineEdit { background: #000; color: #00ff41; border: 1px solid #00ff41;
                    border-radius: 0; padding: 8px; selection-background-color: #004400; }
        QPushButton { background: #001a00; color: #00ff41; border: 1px solid #00ff41;
                       border-radius: 0; padding: 10px 14px; font-weight: bold; }
        QPushButton:hover { background: #003300; color: #ffffff; }
        QPushButton:disabled { background: #111; color: #336633; border-color: #223322; }
        QCheckBox { color: #00cccc; spacing: 8px; }
        QCheckBox::indicator { width: 14px; height: 14px; border: 1px solid #00ff41; background: #000; }
        QCheckBox::indicator:checked { background: #00ff41; }
        QTableWidget { background: #000; color: #00ff41; gridline-color: #004400;
                       border: 1px solid #00ff41; border-radius: 0; }
        QHeaderView::section { background: #001100; color: #00ff41; padding: 6px; border: none;
                               border-bottom: 1px solid #00ff41; }
        QTextEdit { background: #000; color: #00ff41; border: 1px solid #004400; border-radius: 0; }
        QScrollBar:vertical { background: #0a0a0a; width: 12px; }
        QScrollBar::handle:vertical { background: #004400; min-height: 20px; }
    )");
}

void MainWindow::appendLog(const QString& message) {
    const auto ts = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_log->append(QString("[%1] %2").arg(ts, message));
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
        m_statusLabel->setText(">> STATUS: OFFLINE");
        m_statusLabel->setStyleSheet("color: #ff3333;");
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
void MainWindow::onRefreshAlerts() { m_app.refreshAlerts(); }
void MainWindow::onPollStatus() { m_app.pollScanStatus(); }

void MainWindow::onStartRecon() {
    m_reconBtn->setEnabled(false);
    m_stopReconBtn->setEnabled(true);
    m_feedZapBtn->setEnabled(false);
    m_reconProgressLabel->setText(">> RECON: starting...");
    m_reconSummaryLabel->setText(">> RECON SUMMARY: —");

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
