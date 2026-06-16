#include "ScanHistorySidebar.hpp"

#include "AppTheme.hpp"
#include "UiKit.hpp"
#include "config/AppConfig.hpp"

#include <QDateTime>
#include <QLabel>
#include <QListWidget>
#include <QSizePolicy>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace components {

ScanHistorySidebar::ScanHistorySidebar(QWidget* parent) : QWidget(parent) {
    setObjectName("sidebar");
    setFixedWidth(UiKit::kSidebarWidth);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setMinimumHeight(200);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(UiKit::kCardPadding, UiKit::kCardPadTop, UiKit::kCardPadding,
                                UiKit::kCardPadding);
    layout->setSpacing(UiKit::kCardSpacing);

    layout->addWidget(UiKit::createSectionTitle("Scan History", this));
    layout->addWidget(
        UiKit::createSectionHint("Recent recon runs on this machine.", this));

    m_list = new QListWidget(this);
    m_list->setSpacing(2);
    layout->addWidget(m_list, 1);

    AppTheme::applySidebar(this, AppConfig::instance().uiThemeMode());
    applyTheme(AppConfig::instance().uiThemeMode());
}

void ScanHistorySidebar::applyTheme(AppThemeMode mode) {
    AppTheme::applySidebar(this, mode);
    AppTheme::applyItemViewPalette(m_list, mode);
}

void ScanHistorySidebar::addEntry(const QString& target, bool success) {
    const auto ts = QDateTime::currentDateTime().toString("MMM dd · HH:mm");
    const QString status = success ? "✓" : "✗";
    m_list->insertItem(0, QString("%1  %2\n%3").arg(status, target, ts));
}

}  // namespace components
