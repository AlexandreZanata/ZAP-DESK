#pragma once

#include "AppTheme.hpp"

#include <QWidget>

class QFrame;
class QLabel;
class QPushButton;

namespace components {

class StatusBanner : public QWidget {
    Q_OBJECT

public:
    explicit StatusBanner(const QString& version, QWidget* parent = nullptr);

    QPushButton* themeButton() const;
    void setStatusText(const QString& text);
    void setStatusColor(const QString& colorHex);
    void applyTheme(AppThemeMode mode);

private:
    QFrame* m_panel{};
    QLabel* m_title{};
    QLabel* m_subtitle{};
    QLabel* m_version{};
    QLabel* m_status{};
    QPushButton* m_themeBtn{};
};

}  // namespace components
