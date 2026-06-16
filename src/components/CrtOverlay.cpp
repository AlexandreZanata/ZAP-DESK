#include "CrtOverlay.hpp"

#include <QPainter>
#include <QPaintEvent>

namespace components {

CrtOverlay::CrtOverlay(QWidget* parent) : QWidget(parent) {
    setObjectName("crtOverlay");
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
}

void CrtOverlay::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setPen(QColor(0, 255, 65, 18));
    for (int y = 0; y < height(); y += 3) {
        painter.drawLine(0, y, width(), y);
    }
}

}  // namespace components
