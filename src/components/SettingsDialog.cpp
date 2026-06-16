#include "SettingsDialog.hpp"

#include "config/AppConfig.hpp"
#include "security/CredentialStore.hpp"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

namespace components {

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("ZAP-DESK Settings");
    resize(560, 480);

    auto* root = new QVBoxLayout(this);

    auto* pathsBox = new QGroupBox(">> PATHS & NETWORK");
    auto* pathsForm = new QFormLayout(pathsBox);

    m_zapHome = new QLineEdit;
    m_zapPort = new QSpinBox;
    m_zapPort->setRange(1024, 65535);
    m_resultsDir = new QLineEdit;
    m_reconnerDir = new QLineEdit;

    pathsForm->addRow("ZAP home:", m_zapHome);
    pathsForm->addRow("ZAP API port:", m_zapPort);
    pathsForm->addRow("Results directory:", m_resultsDir);
    pathsForm->addRow("Reconner directory:", m_reconnerDir);

    auto* securityBox = new QGroupBox(">> SECURITY");
    auto* securityForm = new QFormLayout(securityBox);

    m_zapApiKey = new QLineEdit;
    m_zapApiKey->setEchoMode(QLineEdit::Password);
    m_zapApiKey->setPlaceholderText("Leave empty to use ZAP_API_KEY env or dev mode");

    m_zapDevMode = new QCheckBox("Dev mode: disable ZAP API key check (localhost only)");
    m_reconRateLimit = new QSpinBox;
    m_reconRateLimit->setRange(0, 3600);
    m_reconRateLimit->setSuffix(" s");

    securityForm->addRow("ZAP API key:", m_zapApiKey);
    securityForm->addRow(m_zapDevMode);
    securityForm->addRow("Recon rate limit:", m_reconRateLimit);

    m_crtOverlay = new QCheckBox("Enable CRT scanline overlay");
    m_crtOverlay->setChecked(true);

    auto* hint = new QLabel(
        "Production: disable dev mode and set an API key.\n"
        "Changes apply after restart or next ZAP boot.");
    hint->setObjectName("hint");

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    root->addWidget(pathsBox);
    root->addWidget(securityBox);
    root->addWidget(m_crtOverlay);
    root->addWidget(hint);
    root->addWidget(buttons);

    loadFromConfig();
}

void SettingsDialog::loadFromConfig() {
    const auto& cfg = AppConfig::instance();
    QSettings settings;

    m_zapHome->setText(settings.value("zap/home", cfg.zapHome()).toString());
    m_zapPort->setValue(settings.value("zap/port", cfg.zapApiPort()).toInt());
    m_resultsDir->setText(settings.value("paths/results", cfg.resultsDir()).toString());
    m_reconnerDir->setText(settings.value("paths/reconner", cfg.reconnerDir()).toString());
    m_zapApiKey->setText(security::CredentialStore::zapApiKey());
    m_zapDevMode->setChecked(settings.value("security/devMode", false).toBool());
    m_reconRateLimit->setValue(settings.value("security/reconRateLimit", 30).toInt());
    m_crtOverlay->setChecked(settings.value("ui/crtOverlay", true).toBool());
}

void SettingsDialog::saveToConfig() {
    QSettings settings;
    settings.setValue("zap/home", m_zapHome->text().trimmed());
    settings.setValue("zap/port", m_zapPort->value());
    settings.setValue("paths/results", m_resultsDir->text().trimmed());
    settings.setValue("paths/reconner", m_reconnerDir->text().trimmed());
    settings.setValue("security/devMode", m_zapDevMode->isChecked());
    settings.setValue("security/reconRateLimit", m_reconRateLimit->value());
    settings.setValue("ui/crtOverlay", m_crtOverlay->isChecked());
    settings.sync();

    security::CredentialStore::setZapApiKey(m_zapApiKey->text().trimmed());
    AppConfig::instance().reload();
}

}  // namespace components
