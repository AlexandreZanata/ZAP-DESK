#pragma once

#include <QString>

namespace security {

class AuditLogger {
public:
    explicit AuditLogger(const QString& logPath);

    void log(const QString& action, const QString& detail = {});

    QString logPath() const;

private:
    QString m_logPath;
};

}  // namespace security
