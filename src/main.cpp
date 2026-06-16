#include <QApplication>

#include "ui/MainWindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("ZAP-DESK");
    app.setOrganizationName("ZAP-DESK");
    app.setApplicationVersion("0.8.0");

    MainWindow window;
    window.show();

    return app.exec();
}
