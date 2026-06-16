#include "ReconRateLimiter.hpp"

namespace security {

ReconRateLimiter::ReconRateLimiter(int minIntervalSeconds)
    : m_minIntervalSeconds(minIntervalSeconds) {}

void ReconRateLimiter::setMinIntervalSeconds(int seconds) {
    m_minIntervalSeconds = qMax(0, seconds);
}

bool ReconRateLimiter::canStart(QString* reason) const {
    if (m_minIntervalSeconds <= 0 || !m_lastStart.isValid()) return true;

    const qint64 elapsed = m_lastStart.secsTo(QDateTime::currentDateTime());
    if (elapsed >= m_minIntervalSeconds) return true;

    const int remaining = static_cast<int>(m_minIntervalSeconds - elapsed);
    if (reason) {
        *reason = QString("Recon rate limit: wait %1 second(s) before next launch.").arg(remaining);
    }
    return false;
}

void ReconRateLimiter::recordStart() {
    m_lastStart = QDateTime::currentDateTime();
}

}  // namespace security
