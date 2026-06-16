#include <QApplication>
#include <QIcon>

#include "components/KdeTheme.hpp"
#include "ui/MainWindow.hpp"

int main(int argc, char* argv[]) {
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName("ZAP-DESK");
    app.setOrganizationName("ZAP-DESK");
    app.setApplicationVersion("0.8.0");

    components::KdeTheme::install(app);

    const QIcon appIcon(":/icons/zap.svg");
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }

    MainWindow window;
    window.show();

    return app.exec();
}
