#pragma once

#include "components/AppTheme.hpp"

#include <QString>

class AppConfig {
public:
    static AppConfig& instance();

    QString projectRoot() const;
    QString dataDir() const;
    QString reconnerDir() const;
    QString reconnerModule() const;
    QString resultsDir() const;
    QString zapLaunchScript() const;
    QString zapHome() const;
    QString zapConfigDir() const;
    QString zapPidFile() const;
    QString zapApiUrl() const;
    int zapApiPort() const;
    bool crtOverlayEnabled() const;
    QString uiTheme() const;
    components::AppThemeMode uiThemeMode() const;
    void setUiTheme(const QString& theme);
    QString appIconPath() const;
    QString zapApiKey() const;
    bool zapDevMode() const;
    int reconRateLimitSeconds() const;
    QString auditLogPath() const;

    void reload();

private:
    AppConfig();
    QString detectProjectRoot() const;
    QString m_projectRoot;
};
