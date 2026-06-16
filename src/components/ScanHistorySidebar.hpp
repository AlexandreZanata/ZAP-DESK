#pragma once

#include <QWidget>

class QListWidget;

namespace components {

class ScanHistorySidebar : public QWidget {
    Q_OBJECT

public:
    explicit ScanHistorySidebar(QWidget* parent = nullptr);

    void addEntry(const QString& target, bool success);

private:
    QListWidget* m_list{};
};

}  // namespace components
