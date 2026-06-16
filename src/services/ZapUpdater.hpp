#pragma once

#include <QObject>
#include <QString>
#include <functional>

class QNetworkAccessManager;

struct ZapUpdateInfo {
    QString localVersion;
    QString latestVersion;
    QString releaseUrl;
    QString downloadUrl;
    bool updateAvailable{false};
};

class ZapUpdater : public QObject {
    Q_OBJECT

public:
    explicit ZapUpdater(QObject* parent = nullptr);

    void checkForUpdates(const QString& localVersion,
                         std::function<void(ZapUpdateInfo)> callback);

    void launchUpdateScript(std::function<void(bool, QString)> callback);

signals:
    void logMessage(const QString& message);

private:
    QNetworkAccessManager* m_network;
};
