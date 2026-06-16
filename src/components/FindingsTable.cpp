#include "FindingsTable.hpp"

#include "AppTheme.hpp"
#include "KdeTheme.hpp"
#include "UiKit.hpp"
#include "config/AppConfig.hpp"

#include <QAbstractItemView>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>

namespace components {

FindingsModel::FindingsModel(QObject* parent) : QAbstractTableModel(parent) {}

int FindingsModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : m_rows.size();
}

int FindingsModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 5;
}

QVariant FindingsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_rows.size()) {
        return {};
    }

    const auto& row = m_rows[index.row()];
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:
                return row.source;
            case 1:
                return row.risk;
            case 2:
                return row.name;
            case 3:
                return row.url;
            case 4:
                return row.description;
            default:
                return {};
        }
    }

    if (role == Qt::ForegroundRole && index.column() == 1) {
        const auto mode = AppConfig::instance().uiThemeMode();
        return QBrush(QColor(AppTheme::riskColor(mode, row.risk)));
    }

    if (role == Qt::FontRole && index.column() == 1) {
        QFont font;
        font.setBold(true);
        return font;
    }

    return {};
}

QVariant FindingsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    static const QStringList headers = {"Source", "Risk", "Finding", "URL", "Description"};
    if (section >= 0 && section < headers.size()) {
        return headers[section];
    }
    return {};
}

void FindingsModel::setFindings(const QVector<FindingRow>& rows) {
    beginResetModel();
    m_rows = rows;
    endResetModel();
}

void FindingsModel::clear() {
    beginResetModel();
    m_rows.clear();
    endResetModel();
}

FindingsTable::FindingsTable(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto section =
        UiKit::createSection("Findings", "Unified results from ZAP and nuclei scans.", this);

    m_model = new FindingsModel(this);
    m_view = new QTableView(section.frame);
    m_view->setModel(m_model);
    m_view->horizontalHeader()->setStretchLastSection(true);
    m_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_view->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_view->verticalHeader()->setVisible(false);
    m_view->setAlternatingRowColors(true);
    m_view->setShowGrid(false);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setWordWrap(false);
    m_view->setMinimumHeight(UiKit::kTableMinHeight);
    m_view->setFrameShape(QFrame::NoFrame);

    section.layout->addWidget(m_view, 1);
    layout->addWidget(section.frame, 1);

    applyTheme(AppConfig::instance().uiThemeMode());
}

void FindingsTable::setFindings(const QVector<FindingRow>& rows) {
    m_model->setFindings(rows);
    m_view->resizeRowsToContents();
}

void FindingsTable::clear() {
    m_model->clear();
}

void FindingsTable::applyTheme(AppThemeMode mode) {
    AppTheme::applyItemViewPalette(m_view, mode);
    if (m_view->horizontalHeader()) {
        m_view->horizontalHeader()->setPalette(KdeTheme::paletteFor(mode));
    }
    m_view->viewport()->update();
}

}  // namespace components
