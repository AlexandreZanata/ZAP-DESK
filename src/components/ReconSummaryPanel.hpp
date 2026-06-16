#pragma once

#include <QWidget>

class QLabel;

namespace components {

class ReconSummaryPanel : public QWidget {
    Q_OBJECT

public:
    explicit ReconSummaryPanel(QWidget* parent = nullptr);

    void setSummaryText(const QString& text);
    void clear();

private:
    QLabel* m_title{};
    QLabel* m_summary{};
};

}  // namespace components
