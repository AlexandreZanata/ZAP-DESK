#include "SettingsDialog.hpp"

#include "config/AppConfig.hpp"

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
    resize(520, 320);

    auto* root = new QVBoxLayout(this);

    auto* pathsBox = new QGroupBox(">> PATHS & NETWORK");
    auto* form = new QFormLayout(pathsBox);

    m_zapHome = new QLineEdit;
    m_zapPort = new QSpinBox;
    m_zapPort->setRange(1024, 65535);
    m_resultsDir = new QLineEdit;
    m_reconnerDir = new QLineEdit;

    form->addRow("ZAP home:", m_zapHome);
    form->addRow("ZAP API port:", m_zapPort);
    form->addRow("Results directory:", m_resultsDir);
    form->addRow("Reconner directory:", m_reconnerDir);

    m_crtOverlay = new QCheckBox("Enable CRT scanline overlay");
    m_crtOverlay->setChecked(true);

    auto* hint = new QLabel("Changes apply after restart or next ZAP boot.");
    hint->setObjectName("hint");

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    root->addWidget(pathsBox);
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
    m_crtOverlay->setChecked(settings.value("ui/crtOverlay", true).toBool());
}

void SettingsDialog::saveToConfig() {
    QSettings settings;
    settings.setValue("zap/home", m_zapHome->text().trimmed());
    settings.setValue("zap/port", m_zapPort->value());
    settings.setValue("paths/results", m_resultsDir->text().trimmed());
    settings.setValue("paths/reconner", m_reconnerDir->text().trimmed());
    settings.setValue("ui/crtOverlay", m_crtOverlay->isChecked());
    settings.sync();
    AppConfig::instance().reload();
}

}  // namespace components
