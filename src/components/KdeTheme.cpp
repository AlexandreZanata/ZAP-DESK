#include "KdeTheme.hpp"

#include <QApplication>
#include <QStyleFactory>
#include <QWidget>

#include <KColorScheme>
#include <KConfig>
#include <KSharedConfig>

namespace components {
namespace {

QPalette hackerPalette() {
    const QColor window(0x05, 0x08, 0x05);
    const QColor base(0x0a, 0x10, 0x0a);
    const QColor altBase(0x0d, 0x16, 0x0d);
    const QColor text(0xc8, 0xff, 0xc8);
    const QColor button(0x12, 0x20, 0x12);
    const QColor highlight(0x39, 0xff, 0x14);
    const QColor highlightedText(0x05, 0x08, 0x05);
    const QColor mid(0x1a, 0x4d, 0x1a);

    QPalette palette;
    for (int group = 0; group < QPalette::NColorGroups; ++group) {
        const auto role = static_cast<QPalette::ColorGroup>(group);
        palette.setColor(role, QPalette::Window, window);
        palette.setColor(role, QPalette::WindowText, text);
        palette.setColor(role, QPalette::Base, base);
        palette.setColor(role, QPalette::AlternateBase, altBase);
        palette.setColor(role, QPalette::Text, text);
        palette.setColor(role, QPalette::Button, button);
        palette.setColor(role, QPalette::ButtonText, text);
        palette.setColor(role, QPalette::BrightText, QColor(0x39, 0xff, 0x14));
        palette.setColor(role, QPalette::Highlight, highlight);
        palette.setColor(role, QPalette::HighlightedText, highlightedText);
        palette.setColor(role, QPalette::Link, QColor(0x00, 0xe5, 0xc0));
        palette.setColor(role, QPalette::Mid, mid);
        palette.setColor(role, QPalette::Dark, mid.darker(120));
        palette.setColor(role, QPalette::Light, mid.lighter(120));
    }
    return palette;
}

QPalette paletteFromSchemeFile(const QString& fileName) {
    const QString path = QStringLiteral("/usr/share/color-schemes/") + fileName;
    KSharedConfig::Ptr config = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    return KColorScheme::createApplicationPalette(config);
}

}  // namespace

void KdeTheme::install(QApplication& app) {
    if (QStyle* breeze = QStyleFactory::create(QStringLiteral("breeze"))) {
        app.setStyle(breeze);
    } else {
        app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    }
}

QPalette KdeTheme::paletteFor(AppThemeMode mode) {
    switch (mode) {
        case AppThemeMode::Dark:
            return paletteFromSchemeFile(QStringLiteral("BreezeDark.colors"));
        case AppThemeMode::Hacker:
            return hackerPalette();
        case AppThemeMode::Light:
        default:
            return paletteFromSchemeFile(QStringLiteral("BreezeLight.colors"));
    }
}

void KdeTheme::apply(AppThemeMode mode) {
    const QPalette palette = paletteFor(mode);
    qApp->setPalette(palette);

    const QList<QWidget*> widgets = qApp->topLevelWidgets();
    for (QWidget* widget : widgets) {
        widget->setPalette(palette);
        widget->update();
    }
}

}  // namespace components
