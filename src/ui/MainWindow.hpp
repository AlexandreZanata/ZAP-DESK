#pragma once

#include <QMainWindow>
#include <QTimer>

#include "core/ZapClient.hpp"
#include "core/ZapDaemon.hpp"
#include "services/ReconBridge.hpp"
#include "services/ReconRunner.hpp"
#include "services/ZapUpdater.hpp"

class QCheckBox;
class QLineEdit;
class QPushButton;
class QLabel;
class QTableWidget;
class QTextEdit;
class QTabWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onStartZap();
    void onStopZap();
    void onCheckConnection();
    void onAjaxScan();
    void onActiveScan();
    void onStopScans();
    void onRefreshAlerts();
    void onPollStatus();
    void onStartRecon();
    void onStopRecon();
    void onFeedZapFromRecon();
    void onCheckZapUpdate();
    void onFullPipeline();

private:
    ZapClient m_client;
    ZapDaemon m_daemon;
    ReconRunner m_recon;
    ReconBridge m_bridge;
    ZapUpdater m_updater;
    QTimer m_pollTimer;

    QTabWidget* m_tabs{};
    QLabel* m_statusLabel{};
    QLabel* m_asciiBanner{};
    QLineEdit* m_urlInput{};
    QCheckBox* m_fastMode{};
    QCheckBox* m_skipNuclei{};
    QCheckBox* m_useProxy{};
    QCheckBox* m_authorized{};
    QPushButton* m_startZapBtn{};
    QPushButton* m_stopZapBtn{};
    QPushButton* m_ajaxBtn{};
    QPushButton* m_activeBtn{};
    QPushButton* m_stopScanBtn{};
    QPushButton* m_reconBtn{};
    QPushButton* m_stopReconBtn{};
    QPushButton* m_feedZapBtn{};
    QPushButton* m_pipelineBtn{};
    QPushButton* m_updateZapBtn{};
    QTableWidget* m_alertsTable{};
    QTextEdit* m_log{};

    void appendLog(const QString& message);
    void setConnectedUi(bool connected);
    void setupUi();
    void applyStyle();
    QString currentTarget() const;
};
