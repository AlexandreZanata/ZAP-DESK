#include "JsonReconSummaryReader.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

namespace infrastructure {

domain::ReconSummaryData JsonReconSummaryReader::load(const std::string& summaryPath) const {
    domain::ReconSummaryData stats;

    QFile file(QString::fromStdString(summaryPath));
    if (!file.open(QIODevice::ReadOnly)) return stats;

    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    if (root.isEmpty()) return stats;

    const QJsonObject scanStats = root.value("statistics").toObject();
    stats.subdomains = scanStats.value("total_subdomains").toInt(
        root.value("subdomains").toObject().value("total").toInt());
    stats.liveHosts = scanStats.value("total_live_hosts").toInt(
        root.value("live_hosts").toObject().value("total").toInt());
    stats.openPorts = scanStats.value("total_open_ports").toInt();
    stats.paths = scanStats.value("total_paths_found").toInt(
        root.value("gobuster_results").toArray().size());
    stats.vulnerabilities = scanStats.value("total_vulnerabilities").toInt(
        root.value("nuclei_results").toArray().size());
    stats.errors = root.value("errors").toArray().size();
    stats.valid = true;
    return stats;
}

std::vector<std::string> JsonReconSummaryReader::loadUrls(const std::string& summaryPath) const {
    QFile file(QString::fromStdString(summaryPath));
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
        const QString url = entry.toObject()["url"].toString().trimmed();
        if (!url.isEmpty()) unique.insert(url);
    }

    std::vector<std::string> urls;
    urls.reserve(unique.size());
    for (const QString& url : unique) {
        urls.push_back(url.toStdString());
    }
    return urls;
}

std::vector<NucleiFinding> JsonReconSummaryReader::loadNucleiFindings(
    const std::string& summaryPath) const {
    std::vector<NucleiFinding> findings;

    QFile file(QString::fromStdString(summaryPath));
    if (!file.open(QIODevice::ReadOnly)) return findings;

    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    const QJsonArray nuclei = root.value("nuclei_results").toArray();
    findings.reserve(nuclei.size());

    for (const QJsonValue& value : nuclei) {
        const QJsonObject item = value.toObject();
        NucleiFinding finding;
        finding.templateId = item.value("template").toString().toStdString();
        finding.severity = item.value("severity").toString("unknown").toStdString();
        finding.url = item.value("url").toString().toStdString();
        finding.description = item.value("matcher-name").toString().toStdString();
        if (finding.description.empty()) {
            finding.description = finding.templateId;
        }
        if (!finding.url.empty()) {
            findings.push_back(std::move(finding));
        }
    }

    return findings;
}

}  // namespace infrastructure
