#pragma once

#include <QString>

#include "AppTheme.hpp"

class QFrame;
class QGridLayout;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class QWidget;

namespace components {

struct SectionPanel {
    QFrame* frame{};
    QVBoxLayout* layout{};
};

class UiKit {
public:
    static constexpr int kOuterMargin = 24;
    static constexpr int kColumnGap = 20;
    static constexpr int kSectionGap = 18;
    static constexpr int kPagePadding = 20;
    static constexpr int kCardPadding = 20;
    static constexpr int kCardPadTop = 16;
    static constexpr int kCardSpacing = 14;
    static constexpr int kGridSpacing = 14;
    static constexpr int kButtonMinHeight = 40;
    static constexpr int kSidebarWidth = 280;
    static constexpr int kLogMinHeight = 100;
    static constexpr int kLogMaxHeight = 140;
    static constexpr int kTableMinHeight = 140;
    static constexpr int kSummaryMinHeight = 72;

    static QScrollArea* createTabScrollPage(QWidget*& page, QWidget* parent = nullptr);
    static QFrame* createPanel(QWidget* parent = nullptr);
    static void stylePanel(QFrame* panel, AppThemeMode mode);
    static QLabel* createSectionTitle(const QString& title, QWidget* parent = nullptr);
    static QLabel* createSectionHint(const QString& hint, QWidget* parent = nullptr);
    static SectionPanel createSection(const QString& title, const QString& hint = {},
                                      QWidget* parent = nullptr);
    static QVBoxLayout* createPageLayout(QWidget* page);
    static QGridLayout* createActionGrid();
    static void tuneActionButton(QPushButton* button);
    static void stylePrimaryButton(QPushButton* button);
    static void styleDangerButton(QPushButton* button);
    static QPlainTextEdit* createLogView(QWidget* parent = nullptr);
};

}  // namespace components
