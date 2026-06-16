#include "SettingsDialog.hpp"

#include "UiKit.hpp"
#include "config/AppConfig.hpp"
#include "components/AppTheme.hpp"
#include "security/CredentialStore.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>

namespace components {

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("ZAP-DESK Settings");
    resize(560, 520);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(UiKit::kOuterMargin, UiKit::kOuterMargin, UiKit::kOuterMargin,
                             UiKit::kOuterMargin);
    root->setSpacing(UiKit::kSectionGap);

    auto pathsSection = UiKit::createSection("Paths & Network", QString(), this);
    auto* pathsForm = new QFormLayout;
    m_zapHome = new QLineEdit;
    m_zapPort = new QSpinBox;
    m_zapPort->setRange(1024, 65535);
    m_resultsDir = new QLineEdit;
    m_reconnerDir = new QLineEdit;
    pathsForm->addRow("ZAP home:", m_zapHome);
    pathsForm->addRow("ZAP API port:", m_zapPort);
    pathsForm->addRow("Results directory:", m_resultsDir);
    pathsForm->addRow("Reconner directory:", m_reconnerDir);
    pathsSection.layout->addLayout(pathsForm);
    root->addWidget(pathsSection.frame);

    auto securitySection = UiKit::createSection("Security", QString(), this);
    auto* securityForm = new QFormLayout;
    m_zapApiKey = new QLineEdit;
    m_zapApiKey->setEchoMode(QLineEdit::Password);
    m_zapApiKey->setPlaceholderText("Leave empty to use ZAP_API_KEY env or dev mode");
    m_zapDevMode = new QCheckBox("Dev mode: disable ZAP API key check (localhost only)");
    m_useKeyring = new QCheckBox("Store API key in system keyring (libsecret)");
    m_reconRateLimit = new QSpinBox;
    m_reconRateLimit->setRange(0, 3600);
    m_reconRateLimit->setSuffix(" s");
    securityForm->addRow("ZAP API key:", m_zapApiKey);
    securityForm->addRow(m_useKeyring);
    securityForm->addRow(m_zapDevMode);
    securityForm->addRow("Recon rate limit:", m_reconRateLimit);
    securitySection.layout->addLayout(securityForm);
    root->addWidget(securitySection.frame);

    auto appearanceSection = UiKit::createSection("Appearance", QString(), this);
    auto* appearanceForm = new QFormLayout;
    m_theme = new QComboBox;
    m_theme->addItem(AppTheme::displayName(AppThemeMode::Light), AppTheme::toString(AppThemeMode::Light));
    m_theme->addItem(AppTheme::displayName(AppThemeMode::Dark), AppTheme::toString(AppThemeMode::Dark));
    m_theme->addItem(AppTheme::displayName(AppThemeMode::Hacker), AppTheme::toString(AppThemeMode::Hacker));
    m_crtOverlay = new QCheckBox("Enable CRT scanline overlay (Hacker theme only)");
    m_crtOverlay->setChecked(true);
    appearanceForm->addRow("Theme:", m_theme);
    appearanceForm->addRow(m_crtOverlay);
    appearanceSection.layout->addLayout(appearanceForm);
    root->addWidget(appearanceSection.frame);

    connect(m_theme, &QComboBox::currentIndexChanged, this, [this]() {
        const auto mode = AppTheme::fromString(m_theme->currentData().toString());
        m_crtOverlay->setEnabled(AppTheme::supportsCrtOverlay(mode));
    });

    auto* hint = new QLabel(
        "Production: disable dev mode and set an API key.\n"
        "Changes apply after restart or next ZAP boot.");
    hint->setObjectName("hint");
    root->addWidget(hint);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
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
    m_useKeyring->setChecked(security::CredentialStore::useKeyring());
    m_useKeyring->setEnabled(security::CredentialStore::keyringAvailable());
    if (!security::CredentialStore::keyringAvailable()) {
        m_useKeyring->setToolTip("Install libsecret-tools (secret-tool) to enable keyring storage.");
    }
    m_zapDevMode->setChecked(settings.value("security/devMode", false).toBool());
    m_reconRateLimit->setValue(settings.value("security/reconRateLimit", 30).toInt());

    const QString theme = settings.value("ui/theme", cfg.uiTheme()).toString();
    const int themeIndex = m_theme->findData(theme);
    m_theme->setCurrentIndex(themeIndex >= 0 ? themeIndex : m_theme->findData("hacker"));
    m_crtOverlay->setChecked(settings.value("ui/crtOverlay", true).toBool());
    m_crtOverlay->setEnabled(
        AppTheme::supportsCrtOverlay(AppTheme::fromString(m_theme->currentData().toString())));
}

void SettingsDialog::saveToConfig() {
    QSettings settings;
    settings.setValue("zap/home", m_zapHome->text().trimmed());
    settings.setValue("zap/port", m_zapPort->value());
    settings.setValue("paths/results", m_resultsDir->text().trimmed());
    settings.setValue("paths/reconner", m_reconnerDir->text().trimmed());
    settings.setValue("security/devMode", m_zapDevMode->isChecked());
    settings.setValue("security/reconRateLimit", m_reconRateLimit->value());
    settings.setValue("ui/theme", m_theme->currentData().toString());
    settings.setValue("ui/crtOverlay", m_crtOverlay->isChecked());
    settings.sync();

    security::CredentialStore::setUseKeyring(m_useKeyring->isChecked());
    security::CredentialStore::setZapApiKey(m_zapApiKey->text().trimmed());
    AppConfig::instance().reload();
}

}  // namespace components
