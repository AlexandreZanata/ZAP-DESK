#include "ScanProgressBar.hpp"

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

namespace components {

ScanProgressBar::ScanProgressBar(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_label = new QLabel(">> RECON: idle");
    m_label->setObjectName("hint");

    m_bar = new QProgressBar;
    m_bar->setRange(0, 100);
    m_bar->setValue(0);
    m_bar->setTextVisible(true);
    m_bar->setFormat("%p%");

    layout->addWidget(m_label);
    layout->addWidget(m_bar);
}

void ScanProgressBar::setIdle() {
    m_label->setText(">> RECON: idle");
    m_bar->setValue(0);
}

void ScanProgressBar::setPhase(int step, int total, const QString& phase, const QString& message) {
    m_label->setText(QString(">> RECON [%1/%2] %3 — %4").arg(step).arg(total).arg(phase, message));
    if (total > 0) {
        m_bar->setValue((step * 100) / total);
    }
}

void ScanProgressBar::setFailed() {
    m_label->setText(">> RECON FAILED");
    m_bar->setValue(0);
}

}  // namespace components
