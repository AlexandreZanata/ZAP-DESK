#include "ScanHistorySidebar.hpp"

#include <QDateTime>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

namespace components {

ScanHistorySidebar::ScanHistorySidebar(QWidget* parent) : QWidget(parent) {
    setFixedWidth(220);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* title = new QLabel(">> SCAN HISTORY");
    title->setObjectName("hint");

    m_list = new QListWidget;

    layout->addWidget(title);
    layout->addWidget(m_list, 1);
}

void ScanHistorySidebar::addEntry(const QString& target, bool success) {
    const auto ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
    const QString status = success ? "OK" : "FAIL";
    m_list->insertItem(0, QString("[%1] %2 — %3").arg(ts, status, target));
}

}  // namespace components
