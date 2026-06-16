#pragma once

#include <QString>

class QFrame;
class QWidget;

namespace components {

enum class AppThemeMode { Light, Dark, Hacker };

class AppTheme {
public:
    static AppThemeMode fromString(const QString& value);
    static QString toString(AppThemeMode mode);
    static QString displayName(AppThemeMode mode);

    static QString statusColor(AppThemeMode mode, const QString& role);
    static QString riskColor(AppThemeMode mode, const QString& risk);
    static QString crtLineColor(AppThemeMode mode);
    static bool supportsCrtOverlay(AppThemeMode mode);

    static void applyWindowBackground(QWidget* widget, AppThemeMode mode);
    static void applyPanelFrame(QFrame* frame, AppThemeMode mode);
    static void applySidebar(QWidget* widget, AppThemeMode mode);
    static void applyItemViewPalette(QWidget* view, AppThemeMode mode);
    static void refreshSurfaces(QWidget* root, AppThemeMode mode);
};

}  // namespace components
