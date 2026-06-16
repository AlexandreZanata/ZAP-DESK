#pragma once

#include <QWidget>

class QLabel;

namespace components {

class StatusBanner : public QWidget {
    Q_OBJECT

public:
    explicit StatusBanner(const QString& version, QWidget* parent = nullptr);

    void setStatusText(const QString& text);
    void setStatusColor(const QString& colorHex);

private:
    QLabel* m_banner{};
    QLabel* m_status{};
};

}  // namespace components
