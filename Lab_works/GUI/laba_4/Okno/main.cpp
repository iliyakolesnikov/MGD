#include <QApplication>
#include "game.h"
#include <stdlib.h> // <--- Добавить

int main(int argc, char *argv[]) {
    // ЗАСТАВЛЯЕМ UBUNTU ИСПОЛЬЗОВАТЬ X11
    // Вставляем это ДО создания QApplication
    setenv("QT_QPA_PLATFORM", "xcb", 1);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
