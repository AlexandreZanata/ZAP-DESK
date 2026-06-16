#include "LogConsole.hpp"

#include <QDateTime>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

namespace components {

LogConsole::LogConsole(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_title = new QLabel(">> SYSTEM LOG");
    m_log = new QTextEdit;
    m_log->setReadOnly(true);
    m_log->setMaximumHeight(180);

    layout->addWidget(m_title);
    layout->addWidget(m_log);
}

void LogConsole::append(const QString& message) {
    const auto ts = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_log->append(QString("[%1] %2").arg(ts, message));
}

}  // namespace components
