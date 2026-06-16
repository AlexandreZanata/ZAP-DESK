#include "ReconBridge.hpp"

#include "core/ZapClient.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

QStringList ReconBridge::loadUrlsFromSummary(const QString& summaryPath) const {
    QFile file(summaryPath);
    if (!file.open(QIODevice::ReadOnly)) return {};

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return {};

    QSet<QString> unique;
    const QJsonObject root = doc.object();

    const auto addFromArray = [&](const QJsonObject& parent, const QString& key) {
        if (!parent.contains(key) || !parent[key].isObject()) return;
        const QJsonArray list = parent[key].toObject()["list"].toArray();
        for (const QJsonValue& value : list) {
            const QString url = value.toString().trimmed();
            if (!url.isEmpty()) unique.insert(url);
        }
    };

    addFromArray(root, "live_hosts");
    addFromArray(root, "subdomains");

    const QJsonArray gobuster = root["gobuster_results"].toArray();
    for (const QJsonValue& entry : gobuster) {
        const QJsonObject obj = entry.toObject();
        const QString url = obj["url"].toString().trimmed();
        if (!url.isEmpty()) unique.insert(url);
    }

    return unique.values();
}

ReconBridge::ReconBridge(ZapClient* client, QObject* parent)
    : QObject(parent), m_client(client) {}

void ReconBridge::feedZap(const QString& summaryPath,
                          std::function<void(int seeded, QString error)> callback) {
    const QStringList urls = loadUrlsFromSummary(summaryPath);
    if (urls.isEmpty()) {
        callback(0, "No URLs found in summary.json");
        return;
    }

    auto* state = new struct {
        int seeded = 0;
        int pending = 0;
        QString error;
    };

    state->pending = urls.size();

    for (const QString& url : urls) {
        m_client->accessUrl(url, [state, callback](bool ok, const QString& err) {
            if (ok) {
                ++state->seeded;
            } else if (state->error.isEmpty()) {
                state->error = err;
            }

            --state->pending;
            if (state->pending == 0) {
                callback(state->seeded, state->error);
                delete state;
            }
        });
    }
}
