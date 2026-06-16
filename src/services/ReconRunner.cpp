#include "ReconRunner.hpp"

#include "config/AppConfig.hpp"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

namespace {
constexpr const char* kDeskPrefix = "@@ZAP-DESK@@PHASE|";
}

ReconRunner::ReconRunner(QObject* parent) : QObject(parent) {
    connect(&m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        const QString output = QString::fromUtf8(m_process.readAllStandardOutput());
        for (const QString& line : output.split('\n', Qt::SkipEmptyParts)) {
            handleOutputLine(line.trimmed());
        }
    });

    connect(&m_process, &QProcess::readyReadStandardError, this, [this]() {
        const QString output = QString::fromUtf8(m_process.readAllStandardError()).trimmed();
        for (const QString& line : output.split('\n', Qt::SkipEmptyParts)) {
            emit logLine("[stderr] " + line);
        }
    });

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus status) {
                const bool success = status == QProcess::NormalExit && exitCode == 0;
                if (success) {
                    setState(State::Finished);
                    emit finished(true, m_lastSummaryPath);
                } else {
                    setState(State::Failed);
                    emit finished(false, {});
                }
            });

    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        setState(State::Failed);
        emit finished(false, {});
    });
}

ReconRunner::State ReconRunner::state() const {
    return m_state;
}

QString ReconRunner::lastOutputDir() const {
    return m_lastOutputDir;
}

QString ReconRunner::lastSummaryPath() const {
    return m_lastSummaryPath;
}

QString ReconRunner::currentPhase() const {
    return m_currentPhase;
}

void ReconRunner::handleOutputLine(const QString& line) {
    if (line.startsWith(kDeskPrefix)) {
        const QString payload = line.mid(int(qstrlen(kDeskPrefix)));
        const QStringList parts = payload.split('|');
        if (parts.size() >= 4) {
            const int step = parts[0].toInt();
            const int total = parts[1].toInt();
            m_currentPhase = parts[2];
            const QString message = parts.mid(3).join('|');
            emit phaseChanged(step, total, m_currentPhase, message);
            emit logLine(QString(">> [%1/%2] %3 — %4").arg(step).arg(total).arg(m_currentPhase, message));
            return;
        }
    }

    if (!line.isEmpty()) {
        emit logLine(line);
    }
}

void ReconRunner::start(const QString& target, bool fastMode, bool skipNuclei, bool useZapProxy) {
    if (m_state == State::Running) {
        emit logLine("Recon already running.");
        return;
    }

    const auto& cfg = AppConfig::instance();
    m_lastOutputDir = cfg.resultsDir();
    m_lastSummaryPath.clear();
    m_currentPhase.clear();
    setState(State::Running);

    QStringList args;
    args << "-m" << "reconner"
         << "-t" << target
         << "-o" << m_lastOutputDir
         << "--quiet";

    if (fastMode) args << "--fast";
    if (skipNuclei) args << "--skip-nuclei";
    if (useZapProxy) {
        args << "--proxy" << QString("http://127.0.0.1:%1").arg(cfg.zapApiPort());
    }

    m_process.setProgram("python3");
    m_process.setArguments(args);
    m_process.setWorkingDirectory(cfg.reconnerDir());

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONPATH", cfg.reconnerDir());
    env.insert("ZAP_DESK_MODE", "1");
    m_process.setProcessEnvironment(env);

    emit logLine(QString(">> reconner -t %1").arg(target));
    m_process.start();
}

void ReconRunner::stop() {
    if (m_process.state() != QProcess::NotRunning) {
        m_process.terminate();
        emit logLine("Recon stopped.");
    }
    setState(State::Idle);
}

void ReconRunner::setState(State state) {
    if (m_state == state) return;
    m_state = state;
    emit stateChanged(state);

    if (state == State::Finished) {
        QDir results(m_lastOutputDir);
        const auto entries = results.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
        for (const QFileInfo& entry : entries) {
            const QString candidate = entry.filePath() + "/summary.json";
            if (QFileInfo::exists(candidate)) {
                m_lastSummaryPath = candidate;
                break;
            }
        }
    }
}
