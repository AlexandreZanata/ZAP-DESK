#pragma once

#include "AppTheme.hpp"

#include <QWidget>

class QListWidget;

namespace components {

class ScanHistorySidebar : public QWidget {
    Q_OBJECT

public:
    explicit ScanHistorySidebar(QWidget* parent = nullptr);

    void addEntry(const QString& target, bool success);
    void applyTheme(AppThemeMode mode);

private:
    QListWidget* m_list{};
};

}  // namespace components
