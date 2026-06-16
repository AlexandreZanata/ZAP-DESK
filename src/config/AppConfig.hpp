#pragma once

#include <QString>

class AppConfig {
public:
    static AppConfig& instance();

    QString projectRoot() const;
    QString reconnerDir() const;
    QString reconnerModule() const;
    QString resultsDir() const;
    QString zapLaunchScript() const;
    QString zapHome() const;
    QString zapPidFile() const;
    QString zapApiUrl() const;
    int zapApiPort() const;

    void reload();

private:
    AppConfig();
    QString detectProjectRoot() const;
    QString m_projectRoot;
};
