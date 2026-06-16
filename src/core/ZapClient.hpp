#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <functional>

struct ZapAlert {
    QString risk;
    QString name;
    QString url;
    QString description;
};

class ZapClient : public QObject {
    Q_OBJECT

public:
    explicit ZapClient(QObject* parent = nullptr);

    void setBaseUrl(const QString& url);
    QString baseUrl() const;

    void setApiKey(const QString& key);
    QString apiKey() const;
    QString authenticatedPath(const QString& path) const;

    void checkConnection(std::function<void(bool, QString)> callback);
    void accessUrl(const QString& url, std::function<void(bool, QString)> callback);
    void startAjaxSpider(const QString& url, std::function<void(bool, QString)> callback);
    void ajaxSpiderStatus(std::function<void(QString)> callback);
    void stopAjaxSpider(std::function<void(bool)> callback);
    void startActiveScan(const QString& url, std::function<void(bool, QString)> callback);
    void activeScanStatus(std::function<void(QString)> callback);
    void stopActiveScan(std::function<void(bool)> callback);
    void fetchAlerts(std::function<void(QVector<ZapAlert>, QString)> callback);

private:
    QString m_baseUrl{"http://127.0.0.1:8080"};
    QString m_apiKey;

    void get(const QString& path, std::function<void(bool, QString)> callback);
    void post(const QString& path, std::function<void(bool, QString)> callback);
};
