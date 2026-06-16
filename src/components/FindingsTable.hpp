#pragma once

#include <QVector>
#include <QWidget>

class QLabel;
class QTableWidget;

namespace components {

struct FindingRow {
    QString source;
    QString risk;
    QString name;
    QString url;
    QString description;
};

class FindingsTable : public QWidget {
    Q_OBJECT

public:
    explicit FindingsTable(QWidget* parent = nullptr);

    void setFindings(const QVector<FindingRow>& rows);
    void clear();

private:
    QLabel* m_title{};
    QTableWidget* m_table{};
};

}  // namespace components
