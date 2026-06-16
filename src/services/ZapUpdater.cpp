#include "ZapUpdater.hpp"

#include "config/AppConfig.hpp"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QRegularExpression>

ZapUpdater::ZapUpdater(QObject* parent)
    : QObject(parent), m_network(new QNetworkAccessManager(this)) {}

void ZapUpdater::checkForUpdates(const QString& localVersion,
                                 std::function<void(ZapUpdateInfo)> callback) {
    QNetworkRequest request(QUrl("https://api.github.com/repos/zaproxy/zap/releases/latest"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "ZAP-DESK/0.2.0");

    QNetworkReply* reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, localVersion, callback]() {
        ZapUpdateInfo info;
        info.localVersion = localVersion;

        if (reply->error() != QNetworkReply::NoError) {
            info.updateAvailable = false;
            callback(info);
            reply->deleteLater();
            return;
        }

        const QJsonObject release = QJsonDocument::fromJson(reply->readAll()).object();
        info.latestVersion = release["tag_name"].toString().removePrefix("v");
        info.releaseUrl = release["html_url"].toString();

        const QJsonArray assets = release["assets"].toArray();
        for (const QJsonValue& asset : assets) {
            const QJsonObject obj = asset.toObject();
            const QString name = obj["name"].toString();
            if (name.contains("Linux") && name.endsWith(".tar.gz")) {
                info.downloadUrl = obj["browser_download_url"].toString();
                break;
            }
        }

        const QRegularExpression versionRe(R"((\d+)\.(\d+)\.(\d+))");
        const auto parse = [&](const QString& v) {
            const auto match = versionRe.match(v);
            if (!match.hasMatch()) return std::tuple<int, int, int>{0, 0, 0};
            return std::make_tuple(match.captured(1).toInt(), match.captured(2).toInt(),
                                 match.captured(3).toInt());
        };

        const auto [lMaj, lMin, lPat] = parse(localVersion);
        const auto [rMaj, rMin, rPat] = parse(info.latestVersion);
        info.updateAvailable = std::tie(rMaj, rMin, rPat) > std::tie(lMaj, lMin, lPat);

        callback(info);
        reply->deleteLater();
    });
}

void ZapUpdater::launchUpdateScript(std::function<void(bool, QString)> callback) {
    const QString script = QDir(AppConfig::instance().projectRoot()).filePath("scripts/update-zap.sh");
    auto* process = new QProcess(this);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [process, callback](int code, QProcess::ExitStatus status) {
                const bool ok = status == QProcess::NormalExit && code == 0;
                callback(ok, ok ? QString() : QString("Update failed (exit code %1)").arg(code));
                process->deleteLater();
            });

    process->setProgram(script);
    process->start();
}
