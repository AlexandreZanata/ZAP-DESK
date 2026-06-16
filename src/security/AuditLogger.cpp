#include "AuditLogger.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace security {

AuditLogger::AuditLogger(const QString& logPath) : m_logPath(logPath) {
    QDir().mkpath(QFileInfo(logPath).absolutePath());
}

void AuditLogger::log(const QString& action, const QString& detail) {
    QFile file(m_logPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) return;

    const auto ts = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    QTextStream stream(&file);
    if (detail.isEmpty()) {
        stream << ts << " | " << action << '\n';
    } else {
        stream << ts << " | " << action << " | " << detail << '\n';
    }
}

QString AuditLogger::logPath() const {
    return m_logPath;
}

}  // namespace security
