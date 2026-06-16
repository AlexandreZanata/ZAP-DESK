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
    void updateFooter();

    QLabel* m_summary{};
    QLabel* m_footer{};
};

}  // namespace components
