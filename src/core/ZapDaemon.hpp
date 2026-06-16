#pragma once

#include <QObject>
#include <QProcess>

class ZapDaemon : public QObject {
    Q_OBJECT

public:
    explicit ZapDaemon(QObject* parent = nullptr);

    bool isRunning() const;
    void start();
    void stop();

signals:
    void logMessage(const QString& message);
    void stateChanged(bool running);

private:
    QProcess m_process;
    QString m_pidFile;
    QString m_launchScript;

    void writePid(int pid);
    int readPid() const;
    void clearPid();
};
