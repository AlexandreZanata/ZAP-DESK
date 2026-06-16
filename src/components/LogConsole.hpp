#pragma once

#include <QWidget>

class QPlainTextEdit;

namespace components {

class LogConsole : public QWidget {
    Q_OBJECT

public:
    explicit LogConsole(QWidget* parent = nullptr);

    void append(const QString& message);

private:
    QPlainTextEdit* m_log{};
};

}  // namespace components
