#include "FindingsTable.hpp"

#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace components {

FindingsTable::FindingsTable(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_title = new QLabel(">> FINDINGS (ZAP + nuclei)");
    m_table = new QTableWidget(0, 5);
    m_table->setHorizontalHeaderLabels({"SRC", "RISK", "FINDING", "URL", "DESC"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    layout->addWidget(m_title);
    layout->addWidget(m_table, 1);
}

void FindingsTable::setFindings(const QVector<FindingRow>& rows) {
    m_table->setRowCount(rows.size());
    for (int i = 0; i < rows.size(); ++i) {
        const auto& row = rows[i];
        m_table->setItem(i, 0, new QTableWidgetItem(row.source));
        m_table->setItem(i, 1, new QTableWidgetItem(row.risk));
        m_table->setItem(i, 2, new QTableWidgetItem(row.name));
        m_table->setItem(i, 3, new QTableWidgetItem(row.url));
        m_table->setItem(i, 4, new QTableWidgetItem(row.description));
    }
}

void FindingsTable::clear() {
    m_table->setRowCount(0);
}

}  // namespace components
