#include "StatusBanner.hpp"

#include <QLabel>
#include <QVBoxLayout>

namespace components {

StatusBanner::StatusBanner(const QString& version, QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_banner = new QLabel(
        "╔══════════════════════════════════════════════════════════╗\n"
        "║  ZAP-DESK // RECONNER — SECURITY TERMINAL v" +
        version +
        "         ║\n"
        "║  [ OWASP ZAP + RECON PIPELINE ]  :: LINUX EDITION ::    ║\n"
        "╚══════════════════════════════════════════════════════════╝");
    m_banner->setObjectName("banner");

    m_status = new QLabel(">> STATUS: OFFLINE");
    m_status->setObjectName("status");

    layout->addWidget(m_banner);
    layout->addWidget(m_status);
}

void StatusBanner::setStatusText(const QString& text) {
    m_status->setText(text);
}

void StatusBanner::setStatusColor(const QString& colorHex) {
    m_status->setStyleSheet(QString("color: %1;").arg(colorHex));
}

}  // namespace components
