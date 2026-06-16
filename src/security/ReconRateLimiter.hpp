#pragma once

#include <QDateTime>
#include <QString>

namespace security {

class ReconRateLimiter {
public:
    explicit ReconRateLimiter(int minIntervalSeconds = 30);

    void setMinIntervalSeconds(int seconds);

    bool canStart(QString* reason = nullptr) const;
    void recordStart();

private:
    int m_minIntervalSeconds;
    QDateTime m_lastStart;
};

}  // namespace security
