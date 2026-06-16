#include "CrtOverlay.hpp"

#include "AppTheme.hpp"
#include "config/AppConfig.hpp"

#include <QPainter>
#include <QPaintEvent>

namespace components {

CrtOverlay::CrtOverlay(QWidget* parent) : QWidget(parent) {
    setObjectName("crtOverlay");
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, true);
}

void CrtOverlay::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    QColor line(AppTheme::crtLineColor(AppConfig::instance().uiThemeMode()));
    line.setAlpha(28);
    painter.setPen(line);
    for (int y = 0; y < height(); y += 3) {
        painter.drawLine(0, y, width(), y);
    }
}

}  // namespace components
