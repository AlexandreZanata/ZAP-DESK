#include "LogConsole.hpp"

#include "UiKit.hpp"

#include <QDateTime>
#include <QPlainTextEdit>
#include <QVBoxLayout>

namespace components {

LogConsole::LogConsole(QWidget* parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumHeight(UiKit::kLogMinHeight + 72);
    setMaximumHeight(UiKit::kLogMaxHeight + 88);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto section = UiKit::createSection("System Log", "Runtime events and pipeline output.", this);
    m_log = UiKit::createLogView(section.frame);
    section.layout->addWidget(m_log);
    layout->addWidget(section.frame);
}

void LogConsole::append(const QString& message) {
    const auto ts = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_log->appendPlainText(QString("[%1] %2").arg(ts, message));
}

}  // namespace components
