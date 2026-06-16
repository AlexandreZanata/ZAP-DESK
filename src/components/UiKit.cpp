#include "UiKit.hpp"

#include "config/AppConfig.hpp"

#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace components {

QFrame* UiKit::createPanel(QWidget* parent) {
    auto* panel = new QFrame(parent);
    stylePanel(panel, AppConfig::instance().uiThemeMode());
    return panel;
}

void UiKit::stylePanel(QFrame* panel, AppThemeMode mode) {
    panel->setObjectName("panelCard");
    AppTheme::applyPanelFrame(panel, mode);
}

QLabel* UiKit::createSectionTitle(const QString& title, QWidget* parent) {
    auto* label = new QLabel(title.toUpper(), parent);
    QFont font = label->font();
    font.setPointSize(font.pointSize() - 1);
    font.setWeight(QFont::DemiBold);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    label->setFont(font);
    return label;
}

QLabel* UiKit::createSectionHint(const QString& hint, QWidget* parent) {
    auto* label = new QLabel(hint, parent);
    label->setWordWrap(true);
    QFont font = label->font();
    font.setPointSize(font.pointSize() - 1);
    label->setFont(font);
    return label;
}

SectionPanel UiKit::createSection(const QString& title, const QString& hint, QWidget* parent) {
    SectionPanel section;
    section.frame = createPanel(parent);
    section.layout = new QVBoxLayout(section.frame);
    section.layout->setContentsMargins(kCardPadding, kCardPadTop, kCardPadding, kCardPadding);
    section.layout->setSpacing(kCardSpacing);
    section.layout->addWidget(createSectionTitle(title, section.frame));

    if (!hint.isEmpty()) {
        section.layout->addWidget(createSectionHint(hint, section.frame));
    }
    return section;
}

QScrollArea* UiKit::createTabScrollPage(QWidget*& page, QWidget* parent) {
    auto* scroll = new QScrollArea(parent);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scroll->setMinimumHeight(0);

    page = new QWidget;
    page->setObjectName("pageContent");
    page->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    page->setMinimumHeight(0);
    scroll->setWidget(page);
    return scroll;
}

QVBoxLayout* UiKit::createPageLayout(QWidget* page) {
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(kPagePadding, kPagePadding, kPagePadding, kPagePadding);
    layout->setSpacing(kSectionGap);
    layout->setAlignment(Qt::AlignTop);
    return layout;
}

QGridLayout* UiKit::createActionGrid() {
    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(kGridSpacing);
    grid->setVerticalSpacing(kGridSpacing);
    grid->setContentsMargins(0, 4, 0, 4);
    return grid;
}

void UiKit::tuneActionButton(QPushButton* button) {
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    button->setMinimumHeight(kButtonMinHeight);
}

void UiKit::stylePrimaryButton(QPushButton* button) {
    tuneActionButton(button);
    button->setDefault(true);
}

void UiKit::styleDangerButton(QPushButton* button) {
    tuneActionButton(button);
}

QPlainTextEdit* UiKit::createLogView(QWidget* parent) {
    auto* log = new QPlainTextEdit(parent);
    log->setReadOnly(true);
    QFont mono = log->font();
    mono.setFamily(QStringLiteral("JetBrains Mono, Cascadia Code, Consolas, monospace"));
    mono.setPointSize(mono.pointSize() - 1);
    log->setFont(mono);
    log->setMinimumHeight(kLogMinHeight);
    log->setMaximumHeight(kLogMaxHeight);
    return log;
}

}  // namespace components
