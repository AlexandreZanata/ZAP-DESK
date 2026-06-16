#pragma once

#include "AppTheme.hpp"

#include <QAbstractTableModel>
#include <QVector>
#include <QWidget>

class QTableView;

namespace components {

struct FindingRow {
    QString source;
    QString risk;
    QString name;
    QString url;
    QString description;
};

class FindingsModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit FindingsModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setFindings(const QVector<FindingRow>& rows);
    void clear();

private:
    QVector<FindingRow> m_rows;
};

class FindingsTable : public QWidget {
    Q_OBJECT

public:
    explicit FindingsTable(QWidget* parent = nullptr);

    void setFindings(const QVector<FindingRow>& rows);
    void clear();
    void applyTheme(AppThemeMode mode);

private:
    FindingsModel* m_model{};
    QTableView* m_view{};
};

}  // namespace components
