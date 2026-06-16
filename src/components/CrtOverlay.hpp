#pragma once

#include <QWidget>

class QPaintEvent;

namespace components {

class CrtOverlay : public QWidget {
    Q_OBJECT

public:
    explicit CrtOverlay(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

}  // namespace components
