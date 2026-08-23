#include <QApplication>
#include <QIcon>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("NexShortcutManager");
    app.setApplicationDisplayName("Nex Shortcut Manager (NSM)");
    app.setOrganizationName("Nex");
    app.setWindowIcon(QIcon::fromTheme("preferences-desktop", QIcon::fromTheme("system-run")));

    MainWindow window;
    window.show();

    return app.exec();
}
