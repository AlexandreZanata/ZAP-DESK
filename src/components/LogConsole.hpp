#pragma once

#include <QWidget>

class QLabel;
class QTextEdit;

namespace components {

class LogConsole : public QWidget {
    Q_OBJECT

public:
    explicit LogConsole(QWidget* parent = nullptr);

    void append(const QString& message);

private:
    QLabel* m_title{};
    QTextEdit* m_log{};
};

}  // namespace components
