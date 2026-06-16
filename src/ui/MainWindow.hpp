#pragma once

#include <QMainWindow>
#include <QTimer>

#include "presentation/ApplicationFacade.hpp"

class QCheckBox;
class QLineEdit;
class QPushButton;
class QResizeEvent;
class QTabWidget;

namespace components {
class CrtOverlay;
class FindingsTable;
class LogConsole;
class ReconSummaryPanel;
class ScanHistorySidebar;
class ScanProgressBar;
class SettingsDialog;
class StatusBanner;
}  // namespace components

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onStartZap();
    void onStopZap();
    void onAjaxScan();
    void onActiveScan();
    void onStopScans();
    void onRefreshFindings();
    void onPollStatus();
    void onStartRecon();
    void onStopRecon();
    void onFeedZapFromRecon();
    void onCheckZapUpdate();
    void onFullPipeline();
    void onOpenSettings();

private:
    presentation::ApplicationFacade m_app;
    QTimer m_pollTimer;

    components::StatusBanner* m_statusBanner{};
    components::ScanHistorySidebar* m_history{};
    components::FindingsTable* m_findings{};
    components::LogConsole* m_logConsole{};
    components::ScanProgressBar* m_reconProgress{};
    components::ReconSummaryPanel* m_reconSummary{};
    components::CrtOverlay* m_crtOverlay{};

    QTabWidget* m_tabs{};
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

    void appendLog(const QString& message);
    void setConnectedUi(bool connected);
    void setupUi();
    void applyStyle();
    void wireFacade();
    void applyCrtOverlaySetting();
    QString currentTarget() const;
};
