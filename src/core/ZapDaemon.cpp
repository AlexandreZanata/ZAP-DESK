#include "ZapDaemon.hpp"

#include "config/AppConfig.hpp"

#include <QFile>
#include <QTextStream>
#include <csignal>

ZapDaemon::ZapDaemon(QObject* parent) : QObject(parent) {
    const auto& cfg = AppConfig::instance();
    m_pidFile = cfg.zapPidFile();
    m_launchScript = cfg.zapLaunchScript();

    connect(&m_process, &QProcess::started, this, [this]() {
        writePid(m_process.processId());
        emit logMessage("ZAP iniciado em modo daemon (porta 8080)");
        emit stateChanged(true);
    });

    connect(&m_process, &QProcess::finished, this, [this](int code, QProcess::ExitStatus) {
        clearPid();
        emit logMessage(QString("ZAP encerrado (código %1)").arg(code));
        emit stateChanged(false);
    });

    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            emit logMessage("Erro: não foi possível iniciar o ZAP. Verifique a instalação.");
            emit stateChanged(false);
        }
    });
}

bool ZapDaemon::isRunning() const {
    const int pid = readPid();
    if (pid <= 0) return false;
    return kill(pid, 0) == 0;
}

void ZapDaemon::start() {
    if (isRunning()) {
        emit logMessage("ZAP já está em execução.");
        emit stateChanged(true);
        return;
    }

    const int port = AppConfig::instance().zapApiPort();
    m_process.setProgram(m_launchScript);
    m_process.setArguments({"-daemon", "-port", QString::number(port), "-config", "api.disablekey=true"});
    m_process.start();
}

void ZapDaemon::stop() {
    const int pid = readPid();
    if (pid > 0) {
        kill(pid, SIGTERM);
        clearPid();
        emit logMessage("ZAP parado.");
        emit stateChanged(false);
        return;
    }

    if (m_process.state() != QProcess::NotRunning) {
        m_process.terminate();
    }
}

void ZapDaemon::writePid(int pid) {
    QFile file(m_pidFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
    QTextStream(&file) << pid;
}

int ZapDaemon::readPid() const {
    QFile file(m_pidFile);
    if (!file.open(QIODevice::ReadOnly)) return -1;
    bool ok = false;
    const int pid = QTextStream(&file).readAll().trimmed().toInt(&ok);
    return ok ? pid : -1;
}

void ZapDaemon::clearPid() {
    QFile::remove(m_pidFile);
}
