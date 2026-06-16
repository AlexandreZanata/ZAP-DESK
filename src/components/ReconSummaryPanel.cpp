#include "ReconSummaryPanel.hpp"

#include "UiKit.hpp"
#include "config/AppConfig.hpp"

#include <QLabel>
#include <QVBoxLayout>

namespace components {

ReconSummaryPanel::ReconSummaryPanel(QWidget* parent) : QWidget(parent) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto section = UiKit::createSection(
        "Recon Summary", "Pipeline output and artifact paths.", this);
    m_summary = new QLabel("No recon results yet.", section.frame);
    m_summary->setObjectName("hint");
    m_summary->setWordWrap(true);
    m_summary->setMinimumHeight(UiKit::kSummaryMinHeight);
    section.layout->addWidget(m_summary, 1);

    m_footer = new QLabel(section.frame);
    m_footer->setObjectName("hint");
    m_footer->setWordWrap(true);
    updateFooter();
    section.layout->addWidget(m_footer);

    outer->addWidget(section.frame);
}

void ReconSummaryPanel::updateFooter() {
    m_footer->setText(QStringLiteral("Pipeline: subfinder → httpx → nmap → whatweb → gobuster → nuclei\n"
                                      "Results: %1")
                          .arg(AppConfig::instance().resultsDir()));
}

void ReconSummaryPanel::setSummaryText(const QString& text) {
    m_summary->setText(text);
}

void ReconSummaryPanel::clear() {
    m_summary->setText("No recon results yet.");
}

}  // namespace components
