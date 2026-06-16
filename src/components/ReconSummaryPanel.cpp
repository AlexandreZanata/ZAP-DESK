#include "ReconSummaryPanel.hpp"

#include <QLabel>
#include <QVBoxLayout>

namespace components {

ReconSummaryPanel::ReconSummaryPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_title = new QLabel(">> RECON SUMMARY");
    m_title->setObjectName("hint");
    m_summary = new QLabel(">> RECON SUMMARY: —");
    m_summary->setObjectName("hint");
    m_summary->setWordWrap(true);

    layout->addWidget(m_title);
    layout->addWidget(m_summary);
}

void ReconSummaryPanel::setSummaryText(const QString& text) {
    m_summary->setText(text);
}

void ReconSummaryPanel::clear() {
    m_summary->setText(">> RECON SUMMARY: —");
}

}  // namespace components
