#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QProgressBar>
#include <QPushButton>
#include <QLCDNumber>
#include <QTimer>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // === 1. Создаем главное окно ===
    QWidget window;
    window.setWindowTitle("Пульт управления резервуаром");
    window.resize(600, 500);

    // === 2. Создаем элементы интерфейса (Виджеты) ===
    
    // Заголовок
    QLabel *titleLabel = new QLabel("РЕЗЕРВУАР A-1");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");

    // Слайдер (Клапан подачи)
    QLabel *sliderLabel = new QLabel("Клапан подачи воды:");
    QSlider *inputSlider = new QSlider(Qt::Horizontal);
    inputSlider->setRange(0, 100);
    inputSlider->setValue(0);

    // Индикатор уровня (Прогресс-бар)
    QLabel *levelLabel = new QLabel("Уровень жидкости:");
    QProgressBar *levelBar = new QProgressBar();
    levelBar->setRange(0, 100);
    levelBar->setValue(0);
    levelBar->setTextVisible(true);

    // LCD дисплей (цифровое отображение открытия клапана)
    QLCDNumber *lcdDisplay = new QLCDNumber();
    lcdDisplay->setDigitCount(3);
    lcdDisplay->setSegmentStyle(QLCDNumber::Flat);
    lcdDisplay->setMinimumHeight(50);

    // Статус (табличка "Норма/Авария")
    QLabel *statusLabel = new QLabel("Статус: НОРМА");
    statusLabel->setAlignment(Qt::AlignCenter);
    // Начальный стиль (зеленый)
    statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #27ae60; color: white; border-radius: 5px; font-weight: bold;");

    // Кнопки управления
    QPushButton *drainBtn = new QPushButton("Закрыть клапан (Слив)");
    QPushButton *resetBtn = new QPushButton("АВАРИЙНЫЙ СБРОС");
    resetBtn->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; padding: 10px;");


    // === 3. Размещаем элементы (Layouts) ===
    
    QVBoxLayout *mainLayout = new QVBoxLayout();
    
    mainLayout->addWidget(titleLabel);
    
    // Ряд для слайдера
    QHBoxLayout *sliderRow = new QHBoxLayout();
    sliderRow->addWidget(sliderLabel);
    sliderRow->addWidget(inputSlider);
    mainLayout->addLayout(sliderRow);

    // LCD дисплей под слайдером
    mainLayout->addWidget(lcdDisplay);

    // Ряд для уровня
    QHBoxLayout *levelRow = new QHBoxLayout();
    levelRow->addWidget(levelLabel);
    levelRow->addWidget(levelBar);
    mainLayout->addLayout(levelRow);

    // Статус и кнопки
    mainLayout->addSpacing(20); // отступ
    mainLayout->addWidget(statusLabel);
    mainLayout->addSpacing(10);
    
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addWidget(drainBtn);
    btnRow->addWidget(resetBtn);
    mainLayout->addLayout(btnRow);


    // === 4. Логика (Сигналы и Слоты) ===

    // Связываем слайдер с LCD экраном
    QObject::connect(inputSlider, &QSlider::valueChanged, lcdDisplay, QOverload<int>::of(&QLCDNumber::display));

    // Переменная уровня воды (хранится в памяти)
    double *currentLevel = new double(0.0);

    // Таймер для симуляции физики (срабатывает каждые 100 мс)
    QTimer *simTimer = new QTimer(&window);
    simTimer->setInterval(100);

    // Основная логика симуляции
    QObject::connect(simTimer, &QTimer::timeout, [=]() {
        // Получаем значения
        double inflow = inputSlider->value() * 0.3; // Вода поступает (зависит от слайдера)
        double leak = 1.0;                          // Вода утекает (постоянно)

        // Меняем уровень
        *currentLevel += (inflow - leak) * 0.1;

        // Ограничения (не меньше 0 и не больше 100)
        if (*currentLevel < 0) *currentLevel = 0;
        if (*currentLevel > 100) *currentLevel = 100;

        // Обновляем прогресс-бар
        int intLevel = static_cast<int>(*currentLevel);
        levelBar->setValue(intLevel);
	
	// --- Логика цветов и статусов ---
        if (*currentLevel > 90) {
            // КРИТИЧЕСКИЙ УРОВЕНЬ (Красный)
            statusLabel->setText("!!! КРИТИЧЕСКИЙ УРОВЕНЬ !!!");
            statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #e74c3c; color: white; border-radius: 5px; font-weight: bold;");
            levelBar->setStyleSheet("QProgressBar::chunk { background-color: #e74c3c; }");
        } 
        else if (*currentLevel > 80) {
            // ВНИМАНИЕ (Оранжевый)
            statusLabel->setText("ВНИМАНИЕ: Высокий уровень");
            statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #f39c12; color: white; border-radius: 5px; font-weight: bold;");
            levelBar->setStyleSheet("QProgressBar::chunk { background-color: #f39c12; }");
        } 
        else {
            // НОРМА (Зеленый)
            statusLabel->setText("Статус: НОРМА");
            statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #27ae60; color: white; border-radius: 5px; font-weight: bold;");
            levelBar->setStyleSheet("QProgressBar::chunk { background-color: #2ecc71; }");
        }
    });

    // Запуск таймера
    simTimer->start();

    // Кнопка "Слив" (просто ставит слайдер в 0, вода начинает уходить через утечку)
    QObject::connect(drainBtn, &QPushButton::clicked, [=]() {
        inputSlider->setValue(0);
    });

    // Кнопка "Аварийный сброс" (мгновенно опустошает бак)
    QObject::connect(resetBtn, &QPushButton::clicked, [=]() {
        inputSlider->setValue(0);     // Закрыть кран
        *currentLevel = 0.0;          // Обнулить переменную
        levelBar->setValue(0);        // Обнулить бар
        // Статус обновится сам на следующем тике таймера
    });

    // === 5. Запуск окна ===
    window.setLayout(mainLayout);
    window.show();

    return app.exec();
}









































//-------------------------------------------------------------------------------------------------------------------

// // --- Подключение необходимых библиотек (заголовочных файлов) ---
// #include <QApplication>  // Подключает основной класс, управляющий всем приложением
// #include <QWidget>       // Подключает базовый класс для всех визуальных элементов (окон)
// #include <QVBoxLayout>   // Подключает класс для вертикального размещения элементов (в столбик)
// #include <QHBoxLayout>   // Подключает класс для горизонтального размещения элементов (в ряд)
// #include <QLabel>        // Подключает класс для отображения текста (надписей)
// #include <QSlider>       // Подключает класс для создания ползунка (слайдера)
// #include <QProgressBar>  // Подключает класс для полосы загрузки (прогресс-бара)
// #include <QPushButton>   // Подключает класс для создания кнопок
// #include <QLCDNumber>    // Подключает класс для экранчика с цифрами (как на калькуляторе)
// #include <QTimer>        // Подключает таймер для выполнения действий через промежутки времени

// int main(int argc, char *argv[]) { // Точка входа в программу
//     QApplication app(argc, argv);  // Создаем главный объект приложения, который следит за событиями (кликами, таймерами)

//     // === 1. Создаем главное окно ===
//     QWidget window; // Создаем объект окна (пока пустого)
//     window.setWindowTitle("Пульт управления резервуаром"); // Задаем текст в заголовке окна
//     window.resize(600, 500); // Устанавливаем размер окна: ширина 600px, высота 500px

//     // === 2. Создаем элементы интерфейса (Виджеты) ===

//     // Заголовок
//     QLabel *titleLabel = new QLabel("РЕЗЕРВУАР A-1"); // Создаем надпись с текстом
//     titleLabel->setAlignment(Qt::AlignCenter); // Выравниваем текст по центру
//     titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;"); // Настраиваем стиль (размер шрифта, жирность, отступ снизу) CSS-подобным синтаксисом

//     // Слайдер (Клапан подачи)
//     QLabel *sliderLabel = new QLabel("Клапан подачи воды:"); // Создаем подпись для слайдера
//     QSlider *inputSlider = new QSlider(Qt::Horizontal); // Создаем горизонтальный ползунок
//     inputSlider->setRange(0, 100); // Устанавливаем диапазон значений от 0 до 100
//     inputSlider->setValue(0); // Устанавливаем начальное значение в 0 (клапан закрыт)

//     // Индикатор уровня (Прогресс-бар)
//     QLabel *levelLabel = new QLabel("Уровень жидкости:"); // Создаем подпись для индикатора
//     QProgressBar *levelBar = new QProgressBar(); // Создаем саму полосу прогресса
//     levelBar->setRange(0, 100); // Диапазон от 0% до 100%
//     levelBar->setValue(0); // Начальное заполнение 0%
//     levelBar->setTextVisible(true); // Разрешаем отображать проценты текстом в центре полосы

//     // LCD дисплей (цифровое отображение открытия клапана)
//     QLCDNumber *lcdDisplay = new QLCDNumber(); // Создаем цифровое табло
//     lcdDisplay->setDigitCount(3); // Говорим, что будет максимум 3 цифры
//     lcdDisplay->setSegmentStyle(QLCDNumber::Flat); // Делаем стиль цифр плоским (черным), а не "светящимся"
//     lcdDisplay->setMinimumHeight(50); // Задаем минимальную высоту, чтобы цифры были крупными

//     // Статус (табличка "Норма/Авария")
//     QLabel *statusLabel = new QLabel("Статус: НОРМА"); // Создаем метку статуса
//     statusLabel->setAlignment(Qt::AlignCenter); // Текст по центру
//     // Задаем начальный зеленый стиль (цвет фона, цвет текста, скругление углов)
//     statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #27ae60; color: white; border-radius: 5px; font-weight: bold;");

//     // Кнопки управления
//     QPushButton *drainBtn = new QPushButton("Закрыть клапан (Слив)"); // Создаем кнопку закрытия
//     QPushButton *resetBtn = new QPushButton("АВАРИЙНЫЙ СБРОС"); // Создаем кнопку сброса
//     resetBtn->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; padding: 10px;"); // Красим кнопку сброса в красный цвет


//     // === 3. Размещаем элементы (Layouts) ===

//     QVBoxLayout *mainLayout = new QVBoxLayout(); // Создаем главный вертикальный контейнер (все элементы будут друг под другом)

//     mainLayout->addWidget(titleLabel); // Добавляем заголовок в самый верх

//     // Ряд для слайдера (чтобы подпись и слайдер были на одной строке)
//     QHBoxLayout *sliderRow = new QHBoxLayout(); // Создаем горизонтальный контейнер
//     sliderRow->addWidget(sliderLabel); // Слева добавляем текст
//     sliderRow->addWidget(inputSlider); // Справа добавляем сам слайдер
//     mainLayout->addLayout(sliderRow); // Вставляем этот горизонтальный ряд в главный вертикальный контейнер

//     // LCD дисплей под слайдером
//     mainLayout->addWidget(lcdDisplay); // Добавляем экранчик с цифрами ниже

//     // Ряд для уровня
//     QHBoxLayout *levelRow = new QHBoxLayout(); // Еще один горизонтальный контейнер
//     levelRow->addWidget(levelLabel); // Текст "Уровень..."
//     levelRow->addWidget(levelBar); // Сам прогресс-бар
//     mainLayout->addLayout(levelRow); // Добавляем ряд в главное окно

//     // Статус и кнопки
//     mainLayout->addSpacing(20); // Добавляем пустое пространство (отступ) в 20 пикселей
//     mainLayout->addWidget(statusLabel); // Добавляем цветную плашку статуса
//     mainLayout->addSpacing(10); // Еще небольшой отступ

//     QHBoxLayout *btnRow = new QHBoxLayout(); // Горизонтальный ряд для кнопок
//     btnRow->addWidget(drainBtn); // Кнопка слива слева
//     btnRow->addWidget(resetBtn); // Кнопка сброса справа
//     mainLayout->addLayout(btnRow); // Добавляем ряд кнопок в низ окна


//     // === 4. Логика (Сигналы и Слоты) ===

//     // Связываем движение слайдера с цифрами на LCD.
//     // Когда слайдер меняет значение (valueChanged), LCD показывает это число (display).
//     QObject::connect(inputSlider, &QSlider::valueChanged, lcdDisplay, QOverload<int>::of(&QLCDNumber::display));

//     // Переменная уровня воды. Используем указатель (new double), чтобы можно было менять значение внутри лямбда-функции таймера.
//     double *currentLevel = new double(0.0);

//     // Таймер для симуляции физики
//     QTimer *simTimer = new QTimer(&window); // Создаем таймер, привязанный к окну
//     simTimer->setInterval(100); // Устанавливаем интервал срабатывания: каждые 100 миллисекунд (0.1 сек)

//     // Основная логика симуляции (срабатывает каждый "тик" таймера)
//     // [=] означает, что внутрь функции мы передаем доступ ко всем внешним переменным (слайдеры, бары и т.д.)
//     QObject::connect(simTimer, &QTimer::timeout, [=]() {
//         // Получаем значения
//         double inflow = inputSlider->value() * 0.3; // Рассчитываем приток: чем больше значение слайдера, тем больше воды
//         double leak = 1.0;                          // Постоянная утечка (вода уходит всегда)

//         // Меняем уровень: (пришло - ушло) * коэффициент скорости
//         *currentLevel += (inflow - leak) * 0.1;

//         // Ограничения (физика бака): уровень не может быть меньше 0 и больше 100
//         if (*currentLevel < 0) *currentLevel = 0;
//         if (*currentLevel > 100) *currentLevel = 100;

//         // Обновляем прогресс-бар на экране
//         int intLevel = static_cast<int>(*currentLevel); // Превращаем дробное число в целое для бара
//         levelBar->setValue(intLevel); // Ставим значение

//         // --- Логика цветов и статусов (изменяем вид в зависимости от уровня) ---
//         if (*currentLevel > 90) {
//             // Если > 90% - КРИТИЧЕСКАЯ СИТУАЦИЯ (Красный цвет)
//             statusLabel->setText("!!! КРИТИЧЕСКИЙ УРОВЕНЬ !!!"); // Меняем текст
//             // Меняем стиль плашки статуса на красный
//             statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #e74c3c; color: white; border-radius: 5px; font-weight: bold;");
//             // Меняем цвет самого прогресс-бара на красный (используя CSS селектор chunk)
//             levelBar->setStyleSheet("QProgressBar::chunk { background-color: #e74c3c; }");
//         }
//         else if (*currentLevel > 80) {
//             // Если > 80% - ПРЕДУПРЕЖДЕНИЕ (Оранжевый цвет)
//             statusLabel->setText("ВНИМАНИЕ: Высокий уровень");
//             statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #f39c12; color: white; border-radius: 5px; font-weight: bold;");
//             levelBar->setStyleSheet("QProgressBar::chunk { background-color: #f39c12; }");
//         }
//         else {
//             // Иначе - НОРМА (Зеленый цвет)
//             statusLabel->setText("Статус: НОРМА");
//             statusLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #27ae60; color: white; border-radius: 5px; font-weight: bold;");
//             levelBar->setStyleSheet("QProgressBar::chunk { background-color: #2ecc71; }");
//         }
//     });

//     // Запуск таймера (симуляция начинается)
//     simTimer->start();

//     // Логика кнопки "Слив"
//     QObject::connect(drainBtn, &QPushButton::clicked, [=]() {
//         inputSlider->setValue(0); // Просто сдвигаем слайдер в 0. Вода начнет уходить из-за `leak` в таймере.
//     });

//     // Логика кнопки "Аварийный сброс"
//     QObject::connect(resetBtn, &QPushButton::clicked, [=]() {
//         inputSlider->setValue(0);     // Закрываем кран (слайдер в 0)
//         *currentLevel = 0.0;          // Мгновенно обнуляем переменную уровня воды в памяти
//         levelBar->setValue(0);        // Мгновенно обнуляем вид прогресс-бара
//         // Текст статуса обновится сам на следующем тике таймера (через 100мс)
//     });

//     // === 5. Запуск окна ===
//     window.setLayout(mainLayout); // Применяем созданную структуру (Layout) к главному окну
//     window.show(); // Делаем окно видимым на экране

//     return app.exec(); // Запускаем бесконечный цикл обработки событий приложения (пока не закроем окно)
// }



