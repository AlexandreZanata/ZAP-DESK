#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

class ReconRunner : public QObject {
    Q_OBJECT

public:
    enum class State { Idle, Running, Finished, Failed };

    explicit ReconRunner(QObject* parent = nullptr);

    State state() const;
    QString lastOutputDir() const;
    QString lastSummaryPath() const;

    void start(const QString& target, bool fastMode = false, bool skipNuclei = false,
               bool useZapProxy = true);

public slots:
    void stop();

signals:
    void logLine(const QString& line);
    void stateChanged(ReconRunner::State state);
    void finished(bool success, const QString& summaryPath);

private:
    QProcess m_process;
    State m_state{State::Idle};
    QString m_lastOutputDir;
    QString m_lastSummaryPath;

    void setState(State state);
};
