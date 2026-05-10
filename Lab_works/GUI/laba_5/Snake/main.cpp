#include <QApplication>
#include "snakewidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SnakeWidget window;
    window.setWindowTitle("Snake Game");
    window.setFixedSize(600, 600);
    window.show();

    return app.exec();
}
