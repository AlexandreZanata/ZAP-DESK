#include "AppTheme.hpp"

#include "KdeTheme.hpp"

#include <QApplication>
#include <QAbstractScrollArea>
#include <QFrame>
#include <QPalette>
#include <QWidget>

#include <KColorScheme>
#include <KConfig>
#include <KSharedConfig>

namespace components {
namespace {

struct ThemeColors {
    QString bg;
    QString surface;
    QString surfaceAlt;
    QString border;
    QString text;
    QString textMuted;
    QString title;
    QString primary;
    QString primaryOn;
    QString danger;
    QString dangerHover;
    QString accent;
    QString success;
    QString warning;
    QString offline;
    QString scan;
    QString riskHigh;
    QString riskMedium;
    QString riskLow;
    QString riskInfo;
    QString crtLine;
};

ThemeColors hackerColors() {
    return {"#050805", "#0a100a", "#0d160d", "#1a4d1a", "#c8ffc8", "#5a9a5a", "#39ff14",
            "#39ff14", "#050805", "#ff4444", "#1a0808", "#00e5c0", "#39ff14", "#ffb000",
            "#ff4444", "#00e5c0", "#ff4444", "#ffb000", "#00e5c0", "#5a9a5a", "#39ff1418"};
}

KSharedConfig::Ptr schemeConfig(AppThemeMode mode) {
    const QString file = mode == AppThemeMode::Dark ? QStringLiteral("BreezeDark.colors")
                                                    : QStringLiteral("BreezeLight.colors");
    return KSharedConfig::openConfig(QStringLiteral("/usr/share/color-schemes/") + file,
                                     KConfig::SimpleConfig);
}

ThemeColors colorsFromKdeScheme(AppThemeMode mode) {
    const auto config = schemeConfig(mode);
    KColorScheme windowScheme(QPalette::Active, KColorScheme::Window, config);
    KColorScheme viewScheme(QPalette::Active, KColorScheme::View, config);
    KColorScheme buttonScheme(QPalette::Active, KColorScheme::Button, config);
    KColorScheme selectionScheme(QPalette::Active, KColorScheme::Selection, config);

    const QColor bg = windowScheme.background(KColorScheme::NormalBackground).color();
    const QColor surface = viewScheme.background(KColorScheme::NormalBackground).color();
    const QColor surfaceAlt = viewScheme.background(KColorScheme::AlternateBackground).color();
    const QColor border = KColorScheme::shade(surface, KColorScheme::DarkShade);
    const QColor text = viewScheme.foreground(KColorScheme::NormalText).color();
    const QColor textMuted = viewScheme.foreground(KColorScheme::InactiveText).color();
    const QColor title = windowScheme.foreground(KColorScheme::NormalText).color();
    const QColor primary = selectionScheme.background(KColorScheme::NormalBackground).color();
    const QColor primaryOn = selectionScheme.foreground(KColorScheme::NormalText).color();
    const QColor danger = viewScheme.foreground(KColorScheme::NegativeText).color();
    const QColor success = viewScheme.foreground(KColorScheme::PositiveText).color();
    const QColor accent = buttonScheme.foreground(KColorScheme::LinkText).color();

    return {bg.name(),
            surface.name(),
            surfaceAlt.name(),
            border.name(),
            text.name(),
            textMuted.name(),
            title.name(),
            primary.name(),
            primaryOn.name(),
            danger.name(),
            danger.lighter(125).name(),
            accent.name(),
            success.name(),
            QColor(0xf5, 0x9e, 0x0b).name(),
            danger.name(),
            primary.name(),
            danger.name(),
            QColor(0xf5, 0x9e, 0x0b).name(),
            QColor(0x60, 0xa5, 0xfa).name(),
            textMuted.name(),
            primary.name() + "55"};
}

ThemeColors colorsFor(AppThemeMode mode) {
    if (mode == AppThemeMode::Hacker) {
        return hackerColors();
    }
    return colorsFromKdeScheme(mode);
}

QString panelStyle(const ThemeColors& c, const QString& objectName) {
    return QStringLiteral(
               "QFrame#%1 { background-color: %2; border: 1px solid %3; border-radius: 8px; }")
        .arg(objectName, c.surface, c.border);
}

QString sidebarStyle(const ThemeColors& c) {
    return QStringLiteral("QWidget#sidebar { background-color: %1; border: 1px solid %2; border-radius: 8px; }")
        .arg(c.surface, c.border);
}

}  // namespace

AppThemeMode AppTheme::fromString(const QString& value) {
    const QString v = value.trimmed().toLower();
    if (v == "light") return AppThemeMode::Light;
    if (v == "dark") return AppThemeMode::Dark;
    return AppThemeMode::Hacker;
}

QString AppTheme::toString(AppThemeMode mode) {
    switch (mode) {
        case AppThemeMode::Light:
            return "light";
        case AppThemeMode::Dark:
            return "dark";
        case AppThemeMode::Hacker:
        default:
            return "hacker";
    }
}

QString AppTheme::displayName(AppThemeMode mode) {
    switch (mode) {
        case AppThemeMode::Light:
            return "Light";
        case AppThemeMode::Dark:
            return "Dark";
        case AppThemeMode::Hacker:
        default:
            return "Hacker (CRT)";
    }
}

bool AppTheme::supportsCrtOverlay(AppThemeMode mode) {
    return mode == AppThemeMode::Hacker;
}

QString AppTheme::crtLineColor(AppThemeMode mode) {
    return colorsFor(mode).crtLine;
}

QString AppTheme::statusColor(AppThemeMode mode, const QString& role) {
    const auto c = colorsFor(mode);
    if (role == "online") return c.success;
    if (role == "offline") return c.offline;
    if (role == "warning") return c.warning;
    if (role == "scan") return c.scan;
    return c.textMuted;
}

QString AppTheme::riskColor(AppThemeMode mode, const QString& risk) {
    const QString r = risk.trimmed().toLower();
    const auto c = colorsFor(mode);
    if (r.contains("high") || r == "critical") return c.riskHigh;
    if (r.contains("medium")) return c.riskMedium;
    if (r.contains("low")) return c.riskLow;
    if (r.contains("info")) return c.riskInfo;
    return c.text;
}

void AppTheme::applyWindowBackground(QWidget* widget, AppThemeMode mode) {
    Q_UNUSED(mode);
    if (!widget) return;
    widget->setAutoFillBackground(true);
    widget->setPalette(qApp->palette());
}

void AppTheme::applyPanelFrame(QFrame* frame, AppThemeMode mode) {
    if (!frame) return;
    const auto c = colorsFor(mode);
    frame->setAttribute(Qt::WA_StyledBackground, true);
    frame->setStyleSheet(panelStyle(c, frame->objectName()));
}

void AppTheme::applySidebar(QWidget* widget, AppThemeMode mode) {
    if (!widget) return;
    widget->setAttribute(Qt::WA_StyledBackground, true);
    widget->setStyleSheet(sidebarStyle(colorsFor(mode)));
}

void AppTheme::applyItemViewPalette(QWidget* view, AppThemeMode mode) {
    if (!view) return;
    const QPalette palette = KdeTheme::paletteFor(mode);
    view->setPalette(palette);
    view->setAutoFillBackground(true);
    if (auto* scrollArea = qobject_cast<QAbstractScrollArea*>(view)) {
        if (auto* viewport = scrollArea->viewport()) {
            viewport->setPalette(palette);
            viewport->setAutoFillBackground(true);
        }
    }
}

void AppTheme::refreshSurfaces(QWidget* root, AppThemeMode mode) {
    if (!root) return;

    if (root->objectName() == "centralRoot") {
        applyWindowBackground(root, mode);
    }

    for (auto* frame : root->findChildren<QFrame*>()) {
        const QString name = frame->objectName();
        if (name == "panelCard" || name == "headerPanel") {
            applyPanelFrame(frame, mode);
        }
    }

    if (auto* sidebar = root->findChild<QWidget*>("sidebar")) {
        applySidebar(sidebar, mode);
    }
}

}  // namespace components
