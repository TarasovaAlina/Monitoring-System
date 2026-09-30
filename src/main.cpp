#include "gui/main_window.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    gui::MainWindow window;
    window.show();

    return app.exec();
}