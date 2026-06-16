#include "ReconSummary.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

ReconSummaryStats ReconSummary::parse(const QString& summaryPath) {
    ReconSummaryStats stats;

    QFile file(summaryPath);
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

QString ReconSummary::format(const ReconSummaryStats& stats) {
    if (!stats.valid) return ">> RECON SUMMARY: no data";
    return QString(">> RECON SUMMARY: %1 subdomains | %2 live hosts | %3 open ports | %4 paths | %5 vulns")
        .arg(stats.subdomains)
        .arg(stats.liveHosts)
        .arg(stats.openPorts)
        .arg(stats.paths)
        .arg(stats.vulnerabilities);
}
