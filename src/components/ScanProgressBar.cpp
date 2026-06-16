#include "ScanProgressBar.hpp"

#include "UiKit.hpp"

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

namespace components {

ScanProgressBar::ScanProgressBar(QWidget* parent) : QWidget(parent) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto section = UiKit::createSection("Recon Progress", QString(), this);

    m_label = new QLabel("Recon idle — waiting to start", section.frame);
    m_label->setObjectName("hint");

    m_bar = new QProgressBar(section.frame);
    m_bar->setRange(0, 100);
    m_bar->setValue(0);
    m_bar->setTextVisible(true);
    m_bar->setFormat("%p%");
    m_bar->setMinimumHeight(24);

    section.layout->addWidget(m_label);
    section.layout->addWidget(m_bar);
    outer->addWidget(section.frame);
}

void ScanProgressBar::setIdle() {
    m_label->setText("Recon idle — waiting to start");
    m_bar->setValue(0);
}

void ScanProgressBar::setPhase(int step, int total, const QString& phase, const QString& message) {
    m_label->setText(QString("Step %1/%2 · %3 — %4").arg(step).arg(total).arg(phase, message));
    if (total > 0) {
        m_bar->setValue((step * 100) / total);
    }
}

void ScanProgressBar::setFailed() {
    m_label->setText("Recon failed — see system log for details");
    m_bar->setValue(0);
}

}  // namespace components
