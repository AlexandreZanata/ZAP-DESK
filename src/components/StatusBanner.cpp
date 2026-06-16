#include "StatusBanner.hpp"

#include "UiKit.hpp"
#include "config/AppConfig.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace components {

StatusBanner::StatusBanner(const QString& version, QWidget* parent) : QWidget(parent) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    m_panel = UiKit::createPanel(this);
    m_panel->setObjectName("headerPanel");
    AppTheme::applyPanelFrame(m_panel, AppConfig::instance().uiThemeMode());

    auto* panelLayout = new QVBoxLayout(m_panel);
    panelLayout->setContentsMargins(UiKit::kCardPadding, UiKit::kCardPadTop, UiKit::kCardPadding,
                                    UiKit::kCardPadding);
    panelLayout->setSpacing(10);

    auto* topRow = new QHBoxLayout;
    topRow->setSpacing(16);

    auto* brandCol = new QVBoxLayout;
    brandCol->setSpacing(4);
    m_title = new QLabel("ZAP-DESK", m_panel);
    QFont titleFont = m_title->font();
    titleFont.setPointSize(titleFont.pointSize() + 6);
    titleFont.setWeight(QFont::Bold);
    m_title->setFont(titleFont);

    m_subtitle = new QLabel("Security Terminal  ·  OWASP ZAP + Recon Pipeline", m_panel);
    QFont subFont = m_subtitle->font();
    subFont.setPointSize(subFont.pointSize() - 1);
    m_subtitle->setFont(subFont);

    brandCol->addWidget(m_title);
    brandCol->addWidget(m_subtitle);

    auto* actionsCol = new QHBoxLayout;
    actionsCol->setSpacing(10);
    actionsCol->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_version = new QLabel(QString("v%1").arg(version), m_panel);
    m_version->setAlignment(Qt::AlignCenter);

    m_themeBtn = new QPushButton("THEME: Light", m_panel);
    m_themeBtn->setToolTip("Click to cycle theme (Light → Dark → Hacker)");
    m_themeBtn->setCursor(Qt::PointingHandCursor);
    m_themeBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    actionsCol->addWidget(m_version);
    actionsCol->addWidget(m_themeBtn);

    topRow->addLayout(brandCol, 1);
    topRow->addLayout(actionsCol, 0);

    m_status = new QLabel("● STATUS: OFFLINE", m_panel);

    panelLayout->addLayout(topRow);
    panelLayout->addWidget(m_status);
    outer->addWidget(m_panel);

    applyTheme(AppConfig::instance().uiThemeMode());
}

QPushButton* StatusBanner::themeButton() const {
    return m_themeBtn;
}

void StatusBanner::setStatusText(const QString& text) {
    QString display = text;
    if (!display.startsWith("●")) {
        display = "● " + display;
    }
    m_status->setText(display);
}

void StatusBanner::setStatusColor(const QString& colorHex) {
    m_status->setStyleSheet(QStringLiteral("QLabel { color: %1; font-weight: 700; }").arg(colorHex));
}

void StatusBanner::applyTheme(AppThemeMode mode) {
    AppTheme::applyPanelFrame(m_panel, mode);
    const auto c = AppTheme::statusColor(mode, "scan");
    const auto muted = AppTheme::statusColor(mode, "offline");
    m_version->setStyleSheet(QStringLiteral(
        "QLabel { background-color: palette(alternate-base); color: %1; "
        "border: 1px solid palette(mid); border-radius: 6px; padding: 5px 12px; "
        "font-size: 11px; font-weight: 600; }")
                                 .arg(muted));
    Q_UNUSED(c);
}

}  // namespace components
