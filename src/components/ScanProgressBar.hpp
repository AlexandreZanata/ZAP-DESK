#pragma once

#include <QWidget>

class QLabel;
class QProgressBar;

namespace components {

class ScanProgressBar : public QWidget {
    Q_OBJECT

public:
    explicit ScanProgressBar(QWidget* parent = nullptr);

    void setIdle();
    void setPhase(int step, int total, const QString& phase, const QString& message);
    void setFailed();

private:
    QLabel* m_label{};
    QProgressBar* m_bar{};
};

}  // namespace components
