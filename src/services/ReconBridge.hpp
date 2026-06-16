#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <functional>

class ZapClient;

class ReconBridge : public QObject {
    Q_OBJECT

public:
    explicit ReconBridge(ZapClient* client, QObject* parent = nullptr);

    QStringList loadUrlsFromSummary(const QString& summaryPath) const;
    void feedZap(const QString& summaryPath,
                 std::function<void(int seeded, QString error)> callback);

private:
    ZapClient* m_client;
};
