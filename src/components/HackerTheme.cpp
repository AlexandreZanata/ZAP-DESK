#include "HackerTheme.hpp"

namespace components {

QString HackerTheme::stylesheet() {
    return R"(
        * { font-family: "Courier New", "DejaVu Sans Mono", "Liberation Mono", monospace; }
        QMainWindow, QWidget { background: #0a0a0a; color: #00ff41; font-size: 13px; }
        #banner { color: #00ff41; font-size: 12px; padding: 6px; background: #050505;
                  border: 1px solid #00ff41; border-radius: 0; }
        #status { color: #ffb000; font-weight: bold; padding: 4px 0; }
        #hint { color: #00cccc; font-size: 12px; }
        QTabWidget::pane { border: 1px solid #00ff41; background: #0a0a0a; }
        QTabBar::tab { background: #111; color: #00ff41; padding: 8px 16px;
                       border: 1px solid #004400; margin-right: 2px; }
        QTabBar::tab:selected { background: #002200; color: #00ff41; border-color: #00ff41; }
        QGroupBox { border: 1px solid #004400; border-radius: 0; margin-top: 14px; padding-top: 14px;
                    color: #00cccc; font-weight: bold; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }
        QLineEdit { background: #000; color: #00ff41; border: 1px solid #00ff41;
                    border-radius: 0; padding: 8px; selection-background-color: #004400; }
        QPushButton { background: #001a00; color: #00ff41; border: 1px solid #00ff41;
                       border-radius: 0; padding: 10px 14px; font-weight: bold; }
        QPushButton:hover { background: #003300; color: #ffffff; }
        QPushButton:disabled { background: #111; color: #336633; border-color: #223322; }
        QCheckBox { color: #00cccc; spacing: 8px; }
        QCheckBox::indicator { width: 14px; height: 14px; border: 1px solid #00ff41; background: #000; }
        QCheckBox::indicator:checked { background: #00ff41; }
        QTableWidget { background: #000; color: #00ff41; gridline-color: #004400;
                       border: 1px solid #00ff41; border-radius: 0; }
        QHeaderView::section { background: #001100; color: #00ff41; padding: 6px; border: none;
                               border-bottom: 1px solid #00ff41; }
        QTextEdit, QListWidget { background: #000; color: #00ff41; border: 1px solid #004400; border-radius: 0; }
        QProgressBar { background: #000; border: 1px solid #00ff41; border-radius: 0; text-align: center; color: #00ff41; }
        QProgressBar::chunk { background: #00ff41; }
        QScrollBar:vertical { background: #0a0a0a; width: 12px; }
        QScrollBar::handle:vertical { background: #004400; min-height: 20px; }
        #crtOverlay { background: transparent; }
    )";
}

}  // namespace components
