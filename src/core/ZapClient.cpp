#include "ZapClient.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

ZapClient::ZapClient(QObject* parent) : QObject(parent) {}

void ZapClient::setBaseUrl(const QString& url) { m_baseUrl = url; }

QString ZapClient::baseUrl() const { return m_baseUrl; }

void ZapClient::get(const QString& path, std::function<void(bool, QString)> callback) {
    auto* nam = new QNetworkAccessManager(this);
    QNetworkRequest req{QUrl(m_baseUrl + path)};
    auto* reply = nam->get(req);

    connect(reply, &QNetworkReply::finished, this, [reply, nam, callback]() {
        const bool ok = reply->error() == QNetworkReply::NoError;
        const QString body = QString::fromUtf8(reply->readAll());
        callback(ok, ok ? body : reply->errorString());
        reply->deleteLater();
        nam->deleteLater();
    });
}

void ZapClient::post(const QString& path, std::function<void(bool, QString)> callback) {
    auto* nam = new QNetworkAccessManager(this);
    QNetworkRequest req{QUrl(m_baseUrl + path)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    auto* reply = nam->post(req, QByteArray{});

    connect(reply, &QNetworkReply::finished, this, [reply, nam, callback]() {
        const bool ok = reply->error() == QNetworkReply::NoError;
        const QString body = QString::fromUtf8(reply->readAll());
        callback(ok, ok ? body : reply->errorString());
        reply->deleteLater();
        nam->deleteLater();
    });
}

void ZapClient::checkConnection(std::function<void(bool, QString)> callback) {
    get("/JSON/core/view/version/", [callback](bool ok, const QString& body) {
        if (!ok) {
            callback(false, QString{});
            return;
        }
        const auto json = QJsonDocument::fromJson(body.toUtf8()).object();
        callback(true, json.value("version").toString());
    });
}

void ZapClient::accessUrl(const QString& url, std::function<void(bool, QString)> callback) {
    const QString path = "/JSON/core/action/accessUrl/?url=" + QUrl::toPercentEncoding(url);
    get(path, callback);
}

void ZapClient::startAjaxSpider(const QString& url, std::function<void(bool, QString)> callback) {
    const QString path = "/JSON/ajaxSpider/action/scan/?url=" + QUrl::toPercentEncoding(url) + "&inScope=false";
    get(path, callback);
}

void ZapClient::ajaxSpiderStatus(std::function<void(QString)> callback) {
    get("/JSON/ajaxSpider/view/status/", [callback](bool ok, const QString& body) {
        if (!ok) {
            callback("offline");
            return;
        }
        const auto json = QJsonDocument::fromJson(body.toUtf8()).object();
        callback(json.value("status").toString());
    });
}

void ZapClient::stopAjaxSpider(std::function<void(bool)> callback) {
    get("/JSON/ajaxSpider/action/stop/", [callback](bool ok, const QString&) { callback(ok); });
}

void ZapClient::startActiveScan(const QString& url, std::function<void(bool, QString)> callback) {
    const QString path = "/JSON/ascan/action/scan/?url=" + QUrl::toPercentEncoding(url) + "&recurse=true&inScopeOnly=false";
    get(path, callback);
}

void ZapClient::activeScanStatus(std::function<void(QString)> callback) {
    get("/JSON/ascan/view/status/", [callback](bool ok, const QString& body) {
        if (!ok) {
            callback("-1");
            return;
        }
        const auto json = QJsonDocument::fromJson(body.toUtf8()).object();
        callback(json.value("status").toString());
    });
}

void ZapClient::stopActiveScan(std::function<void(bool)> callback) {
    get("/JSON/ascan/action/stop/", [callback](bool ok, const QString&) { callback(ok); });
}

void ZapClient::fetchAlerts(std::function<void(QVector<ZapAlert>, QString)> callback) {
    get("/JSON/alert/view/alerts/?start=0&count=200", [callback](bool ok, const QString& body) {
        if (!ok) {
            callback({}, "Falha ao buscar alertas");
            return;
        }

        QVector<ZapAlert> alerts;
        const auto root = QJsonDocument::fromJson(body.toUtf8()).object();
        const auto arr = root.value("alerts").toArray();

        for (const auto& item : arr) {
            const auto obj = item.toObject();
            alerts.push_back({
                obj.value("risk").toString(),
                obj.value("name").toString(),
                obj.value("url").toString(),
                obj.value("description").toString(),
            });
        }

        callback(alerts, {});
    });
}
