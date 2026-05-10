#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Главное окно
    QWidget window;
    window.setWindowTitle("Обо мне");
    window.resize(300, 200);

    // Текст
    QLabel *label = new QLabel("Меня зовут Илья.\nЯ студент технического вуза.");
    label->setAlignment(Qt::AlignCenter);

    // Кнопка
    QPushButton *button = new QPushButton("Нажми меня");

    // Компоновка
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(button);

    window.setLayout(layout);

    // Изменение текста при нажатии
    QObject::connect(button, &QPushButton::clicked, [=]() {
        label->setText("Я изучаю C++ и Qt.\nПриятно познакомиться!");
    });

    window.show();

    return app.exec();
}












