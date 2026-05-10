#include "game.h"
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTimer>
#include <QStyle>

// ================= ФАЗА 1: Реализация =================

RunawayButton::RunawayButton(QWidget *parent) : QPushButton("ВЫХОД", parent) {
    // Стиль: красный фон, белый текст, крупный шрифт
    setStyleSheet("background-color: red; color: white; font-weight: bold; font-size: 16px; border-radius: 5px;");
    setFixedSize(120, 50);
    setCursor(Qt::PointingHandCursor);
}

void RunawayButton::enterEvent(QEnterEvent *event) {
    QWidget *parent = parentWidget();
    if (!parent) return;

    // Генерируем новую позицию
    int maxX = parent->width() - width();
    int maxY = parent->height() - height();

    // Защита от нулевых размеров
    if (maxX <= 0) maxX = 1;
    if (maxY <= 0) maxY = 1;

    int newX, newY;
    int currentX = x();
    int currentY = y();

    // Пытаемся найти позицию не ближе 100px (максимум 10 попыток чтобы не зависнуть)
    for(int i=0; i<10; ++i) {
        newX = QRandomGenerator::global()->bounded(maxX);
        newY = QRandomGenerator::global()->bounded(maxY);

        int dist = (newX - currentX)*(newX - currentX) + (newY - currentY)*(newY - currentY);
        if (dist > 100*100) break;
    }

    move(newX, newY);
    QPushButton::enterEvent(event);
}

Phase1Widget::Phase1Widget(QWidget *parent) : QWidget(parent) {
    // Label с инструкцией
    QLabel *lblInfo = new QLabel("Нажми ВЫХОД, чтобы закрыть", this);
    lblInfo->setAlignment(Qt::AlignCenter);
    lblInfo->setStyleSheet("font-size: 18px; margin-top: 20px;");

    // Label с подсказкой (внизу)
    QLabel *lblHint = new QLabel("Подсказка: мышка — не единственный способ управления...", this);
    lblHint->setStyleSheet("color: gray; font-size: 10px;");
    lblHint->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

    // Layout для текстов (кнопка не в лейауте!)
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(lblInfo);
    layout->addStretch();
    layout->addWidget(lblHint);

    // Кнопка
    btnExit = new RunawayButton(this);
    // Начальная позиция кнопки (примерно центр)
    btnExit->move(300, 200);

    connect(btnExit, &QPushButton::clicked, this, &Phase1Widget::onBtnClicked);
}

void Phase1Widget::onBtnClicked() {
    emit phaseFinished();
}


// ================= ФАЗА 2: Реализация =================

Phase2Widget::Phase2Widget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *lblProvoke = new QLabel("Ладно, просто закрой окно", this);
    lblProvoke->setAlignment(Qt::AlignCenter);
    lblProvoke->setStyleSheet("font-size: 20px; font-weight: bold;");
    layout->addWidget(lblProvoke);

    // Скрытое поле ввода
    codeStream = new QLineEdit(this);
    codeStream->setPlaceholderText("Введите код...");
    codeStream->hide(); // Изначально скрыто
    layout->addWidget(codeStream);

    feedbackLabel = new QLabel("", this);
    feedbackLabel->setAlignment(Qt::AlignCenter);
    feedbackLabel->setStyleSheet("color: red;");
    layout->addWidget(feedbackLabel);

    layout->addStretch();

    // Еле заметная подсказка
    QLabel *lblHint = new QLabel("Подсказка: Ctrl+Q", this);
    lblHint->setStyleSheet("color: #E0E0E0;"); // Почти цвет фона
    lblHint->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    layout->addWidget(lblHint);

    connect(codeStream, &QLineEdit::returnPressed, this, &Phase2Widget::checkCode);
}

void Phase2Widget::showCheatInput() {
    codeStream->show();
    codeStream->setFocus();
}

void Phase2Widget::checkCode() {
    if (codeStream->text().trimmed() == "quit") {
        emit phaseFinished();
    } else {
        codeStream->clear();
        feedbackLabel->setText("Не-а, попробуй ещё");
    }
}


// ================= ФАЗА 3: Реализация =================


CloneWindow::CloneWindow(bool isSpecial, QWidget *parent)
    : QWidget(nullptr, Qt::Window), m_isSpecial(isSpecial) // Qt::Window делает его независимым окном
{
    Q_UNUSED(parent);
    resize(300, 200);
    setWindowTitle("Клон");

    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *lbl = new QLabel(this);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setWordWrap(true);

    if (m_isSpecial) {
        // Особое окно
        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor(220, 255, 220)); // Слегка зеленоватый
        setAutoFillBackground(true);
        setPalette(pal);

        lbl->setText("Ты нашел меня! Я сдаюсь.");
        QPushButton *btnWin = new QPushButton("Сдаюсь, ты победил!", this);
        layout->addWidget(lbl);
        layout->addWidget(btnWin);
        connect(btnWin, &QPushButton::clicked, this, &CloneWindow::specialBtnClicked);
    } else {
        // Обычное окно
        QStringList phrases = {"Не это окно!", "Попробуй другое!", "Я просто клон", "Ха-ха, мимо!"};
        lbl->setText(phrases.at(QRandomGenerator::global()->bounded(phrases.size())));
        layout->addWidget(lbl);
    }
}

void CloneWindow::closeEvent(QCloseEvent *event) {
    emit windowClosed(this, m_isSpecial);
    // Не вызываем event->ignore(), позволяем окну закрыться,
    // но логика спавна новых будет в Phase3Widget
    QWidget::closeEvent(event);
}

Phase3Widget::Phase3Widget(QWidget *parent) : QWidget(parent) {
    QLabel *lbl = new QLabel("Ты думал, это конец?\n(Ищи окно с другим фоном)", this);
    lbl->setAlignment(Qt::AlignCenter);
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(lbl);
}

void Phase3Widget::startPhase() {
    // Генерируем случайный индекс от 0 до 4, который будет "особым"
    int specialIndex = QRandomGenerator::global()->bounded(5);

    // Создаем 5 начальных окон
    for (int i = 0; i < 5; ++i) {
        // Если текущий номер совпадает со случайным индексом — это особое окно
        spawnClone(i == specialIndex);
    }
}

void Phase3Widget::spawnClone(bool isSpecial) {
    if (clones.size() >= 20) return;

    CloneWindow *w = new CloneWindow(isSpecial);

    // ВАЖНО: Удаляем флаг Qt::Window, пробуем Qt::Dialog или Qt::Tool,
    // к ним Ubuntu относится проще. Но попробуем оставить Window для начала.
    // Если не поможет, раскомментируйте следующую строку:
    // w->setWindowFlags(Qt::Window | Qt::FramelessWindowHint); // Уберет рамку, но точно сработает

    // Расчет координат
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeom = screen->availableGeometry();
    int maxX = screenGeom.width() - 300; // 300 - ширина клона
    int maxY = screenGeom.height() - 200; // 200 - высота клона

    int x = screenGeom.x() + QRandomGenerator::global()->bounded(maxX > 0 ? maxX : 1);
    int y = screenGeom.y() + QRandomGenerator::global()->bounded(maxY > 0 ? maxY : 1);

    // Сначала показываем (Ubuntu поставит его криво)
    w->show();

    // ХАК: Ждем 50мс и переставляем окно
    QTimer::singleShot(50, w, [w, x, y]() {
        w->move(x, y);
        w->raise(); // Поднять наверх
    });

    connect(w, &CloneWindow::windowClosed, this, &Phase3Widget::onCloneClosed);
    connect(w, &CloneWindow::specialBtnClicked, this, &Phase3Widget::onVictory);

    clones.append(w);
}

void Phase3Widget::onCloneClosed(CloneWindow *sender, bool wasSpecial) {
    clones.removeAll(sender);
    sender->deleteLater(); // Удаляем объект

    // При закрытии одного, создаем 2 новых
    // Если закрыли особое, одно из новых должно стать особым
    spawnClone(wasSpecial);
    spawnClone(false);
}

void Phase3Widget::closeAllClones() {
    for (auto w : clones) {
        // Отключаем сигналы, чтобы не спавнить новых при закрытии
        disconnect(w, nullptr, this, nullptr);
        w->close();
        w->deleteLater();
    }
    clones.clear();
}

void Phase3Widget::onVictory() {
    closeAllClones();
    emit phaseFinished();
}


// ================= ГЛАВНОЕ ОКНО: Реализация =================

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    resize(600, 400);
    setWindowTitle("Окно, которое не хочет умирать");

    stackedWidget = new QStackedWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(stackedWidget);
    layout->setContentsMargins(0,0,0,0);

    // Инициализация страниц
    phase1 = new Phase1Widget(this);
    phase2 = new Phase2Widget(this);
    phase3 = new Phase3Widget(this);
    phase4Placeholder = new QLabel("Ты победил!", this);
    dynamic_cast<QLabel*>(phase4Placeholder)->setAlignment(Qt::AlignCenter);

    stackedWidget->addWidget(phase1); // Index 0
    stackedWidget->addWidget(phase2); // Index 1
    stackedWidget->addWidget(phase3); // Index 2
    stackedWidget->addWidget(phase4Placeholder); // Index 3


    // Связи переходов между фазами
    connect(phase1, &Phase1Widget::phaseFinished, this, &MainWindow::goToPhase2);
    connect(phase2, &Phase2Widget::phaseFinished, this, &MainWindow::goToPhase3);
    connect(phase3, &Phase3Widget::phaseFinished, this, &MainWindow::goToPhase4);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Логика Фазы 2
    if (stackedWidget->currentIndex() == 1) {
        event->ignore(); // Отменяем закрытие

        // Таймер нужен, чтобы разорвать связь с событием закрытия
        QTimer::singleShot(10, this, [this]() {
            if (isMaximized()) showNormal();

            QScreen *screen = QGuiApplication::primaryScreen();
            QRect screenGeom = screen->availableGeometry();

            // Вычисляем границы
            int maxX = screenGeom.width() - width();
            int maxY = screenGeom.height() - height();

            // Генерируем координаты
            int newX = screenGeom.x() + QRandomGenerator::global()->bounded(maxX > 0 ? maxX : 1);
            int newY = screenGeom.y() + QRandomGenerator::global()->bounded(maxY > 0 ? maxY : 1);

            // ВМЕСТО move() ИСПОЛЬЗУЕМ setGeometry()
            // Это более жесткая команда: "Встань сюда и прими такой размер"
            this->setGeometry(newX, newY, width(), height());

            // Если и это не сработает, вызываем принудительный update
            this->raise();
            this->activateWindow();
        });

        return;
    }

    QWidget::closeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    // Логика Фазы 2: Отлов Ctrl+Q
    if (stackedWidget->currentIndex() == 1) {
        if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Q) {
            phase2->showCheatInput();
            return;
        }
    }

    QWidget::keyPressEvent(event);
}

void MainWindow::goToPhase2() {
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::goToPhase3() {
    stackedWidget->setCurrentIndex(2);
    phase3->startPhase(); // Начинаем спавн окон
    hide(); // По логике фазы 3, главное окно может исчезнуть или остаться фоном.
        // В задании: "Окно пишет текст и открывает 5 копий".
        // Если главное окно останется видимым с текстом - это ок.
        // Если нужно скрыть главное, раскомментируйте hide();
    // Но так как фаза 4 требует возврата к главному окну, лучше его не закрывать совсем, а оставить с текстом.
}

void MainWindow::goToPhase4() {
    show(); // Если было скрыто
    stackedWidget->setCurrentIndex(3);
    }











//------------------------------------------------------------------------------------------------------------

















// #include "game.h" // Подключаем заголовочный файл с объявлениями наших классов
// #include <QVBoxLayout> // Подключаем класс для вертикального размещения виджетов
// #include <QRandomGenerator> // Подключаем генератор случайных чисел
// #include <QApplication> // Подключаем основной класс приложения (нужен для получения экрана)
// #include <QScreen> // Подключаем класс для получения информации об экране (разрешение и т.д.)
// #include <QMouseEvent> // Подключаем события мыши
// #include <QMessageBox> // Подключаем класс всплывающих сообщений (здесь не используется, можно убрать)
// #include <QTimer> // Подключаем таймер (критически важен для хаков под Ubuntu)
// #include <QStyle> // Подключаем стили (для стандартных иконок и прочего)

//     // ================= ФАЗА 1: Реализация ("Убегающая кнопка") =================

//     // Конструктор кнопки, которая убегает от мыши
//     RunawayButton::RunawayButton(QWidget *parent) : QPushButton("ВЫХОД", parent) {
//         // Устанавливаем CSS-стили: красный фон, белый жирный текст, скругленные углы
//         setStyleSheet("background-color: red; color: white; font-weight: bold; font-size: 16px; border-radius: 5px;");
//         // Жестко задаем размер кнопки, чтобы она не растягивалась
//         setFixedSize(120, 50);
//         // Меняем курсор на "руку" при наведении, чтобы кнопка выглядела кликабельной
//         setCursor(Qt::PointingHandCursor);
//     }

//     // Переопределяем событие входа мыши в область кнопки
//     void RunawayButton::enterEvent(QEnterEvent *event) {
//         // Получаем указатель на родительское окно (Phase1Widget), чтобы знать границы
//         QWidget *parent = parentWidget();
//         // Если родителя нет, выходим, иначе будет ошибка (краш)
//         if (!parent) return;

//         // Вычисляем максимально возможную координату X (ширина окна минус ширина кнопки)
//         int maxX = parent->width() - width();
//         // Вычисляем максимально возможную координату Y (высота окна минус высота кнопки)
//         int maxY = parent->height() - height();

//         // Защита: если окно слишком маленькое, ставим 1, чтобы программа не вылетела при делении на 0
//         if (maxX <= 0) maxX = 1;
//         if (maxY <= 0) maxY = 1;

//         // Переменные для новых координат
//         int newX, newY;
//         // Текущие координаты кнопки
//         int currentX = x();
//         int currentY = y();

//         // Цикл попыток найти хорошую позицию (максимум 10 раз), чтобы кнопка не прыгнула слишком близко
//         for(int i=0; i<10; ++i) {
//             // Генерируем случайный X в пределах окна
//             newX = QRandomGenerator::global()->bounded(maxX);
//             // Генерируем случайный Y в пределах окна
//             newY = QRandomGenerator::global()->bounded(maxY);

//             // Считаем квадрат расстояния между старой и новой точ��ой (теорема Пифагора без корня)
//             int dist = (newX - currentX)*(newX - currentX) + (newY - currentY)*(newY - currentY);
//             // Если расстояние больше 100 пикселей (100*100), нас это устраивает, прерываем цикл
//             if (dist > 100*100) break;
//         }

//         // Физически перемещаем кнопку на новые координаты
//         move(newX, newY);
//         // Вызываем базовую реализацию события (хороший тон в Qt)
//         QPushButton::enterEvent(event);
//     }

//     // Конструктор виджета первой фазы
//     Phase1Widget::Phase1Widget(QWidget *parent) : QWidget(parent) {
//         // Создаем текстовую метку с инструкцией
//         QLabel *lblInfo = new QLabel("Нажми ВЫХОД, чтобы закрыть", this);
//         // Выравниваем текст по центру
//         lblInfo->setAlignment(Qt::AlignCenter);
//         // Задаем размер шрифта и отступ сверху
//         lblInfo->setStyleSheet("font-size: 18px; margin-top: 20px;");

//         // Создаем метку с подсказкой
//         QLabel *lblHint = new QLabel("Подсказка: мышка — не единственный способ управления...", this);
//         // Делаем шрифт мелким и серым
//         lblHint->setStyleSheet("color: gray; font-size: 10px;");
//         // Выравниваем по низу и по центру
//         lblHint->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

//         // Создаем вертикальный менеджер компоновки (layout)
//         QVBoxLayout *layout = new QVBoxLayout(this);
//         // Добавляем верхний текст в лайаут
//         layout->addWidget(lblInfo);
//         // Добавляем "пружину" (stretch), которая займет все свободное место и прижмет подсказку вниз
//         layout->addStretch();
//         // Добавляем нижнюю подсказку
//         layout->addWidget(lblHint);

//         // Создаем убегающую кнопку. ВНИМАНИЕ: мы НЕ добавляем её в layout, чтобы двигать свободно!
//         btnExit = new RunawayButton(this);
//         // Вручную ставим кнопку примерно в центр (300, 200)
//         btnExit->move(300, 200);

//         // Подключаем сигнал нажатия кнопки к слоту завершения фазы
//         connect(btnExit, &QPushButton::clicked, this, &Phase1Widget::onBtnClicked);
//     }

//     // Слот, который срабатывает, если кнопку все-таки нажали (через Tab + Enter)
//     void Phase1Widget::onBtnClicked() {
//         // Отправляем сигнал главному окну, что фаза 1 пройдена
//         emit phaseFinished();
//     }


//     // ================= ФАЗА 2: Реализация ("Убегающее окно") =================

//     // Конструктор виджета второй фазы
//     Phase2Widget::Phase2Widget(QWidget *parent) : QWidget(parent) {
//         // Создаем вертикальный лайаут
//         QVBoxLayout *layout = new QVBoxLayout(this);

//         // Создаем провоцирующий текст
//         QLabel *lblProvoke = new QLabel("Ладно, просто закрой окно", this);
//         // Выравниваем по центру
//         lblProvoke->setAlignment(Qt::AlignCenter);
//         // Делаем текст жирным и большим
//         lblProvoke->setStyleSheet("font-size: 20px; font-weight: bold;");
//         // Добавляем в лайаут
//         layout->addWidget(lblProvoke);

//         // Создаем поле ввода для чит-кода
//         codeStream = new QLineEdit(this);
//         // Текст-подсказка внутри поля (исчезает при вводе)
//         codeStream->setPlaceholderText("Введите код...");
//         // Скрываем поле, чтобы игрок его не видел сразу
//         codeStream->hide();
//         // Добавляем скрытое поле в лайаут
//         layout->addWidget(codeStream);

//         // Создаем метку для вывода ошибок (красный текст)
//         feedbackLabel = new QLabel("", this);
//         feedbackLabel->setAlignment(Qt::AlignCenter);
//         feedbackLabel->setStyleSheet("color: red;");
//         layout->addWidget(feedbackLabel);

//         // Добавляем пружину
//         layout->addStretch();

//         // Создаем еле заметную подсказку внизу справа
//         QLabel *lblHint = new QLabel("Подсказка: Ctrl+Q", this);
//         // Цвет текста почти совпадает с фоном (#E0E0E0 - светло-серый)
//         lblHint->setStyleSheet("color: #E0E0E0;");
//         // Выравнивание вправо-вниз
//         lblHint->setAlignment(Qt::AlignRight | Qt::AlignBottom);
//         layout->addWidget(lblHint);

//         // Если в поле ввода нажали Enter, вызываем функцию проверки кода
//         connect(codeStream, &QLineEdit::returnPressed, this, &Phase2Widget::checkCode);
//     }

//     // Функция показа скрытого поля (вызывается из MainWindow по Ctrl+Q)
//     void Phase2Widget::showCheatInput() {
//         // Делаем поле видимым
//         codeStream->show();
//         // Переводим фокус клавиатуры в поле, чтобы можно было сразу печатать
//         codeStream->setFocus();
//     }

//     // Проверка введенного кода
//     void Phase2Widget::checkCode() {
//         // Получаем текст, убираем пробелы (trimmed) и проверяем, равен ли он "quit"
//         if (codeStream->text().trimmed() == "quit") {
//             // Если верно, сигнализируем о завершении фазы
//             emit phaseFinished();
//         } else {
//             // Если неверно, очищаем поле
//             codeStream->clear();
//             // Пишем обидное сообщение
//             feedbackLabel->setText("Не-а, попробуй ещё");
//         }
//     }


//     // ================= ФАЗА 3: Реализация ("Размножение") =================

//     // Конструктор окна-клона
//     CloneWindow::CloneWindow(bool isSpecial, QWidget *parent)
//         : QWidget(nullptr, Qt::Window), m_isSpecial(isSpecial) // parent=nullptr (чтобы окно было отдельным), флаг Window
//     {
//         // Помечаем parent как неиспользуемый, чтобы компилятор не ругался (мы передали nullptr выше)
//         Q_UNUSED(parent);
//         // Задаем размер окна
//         resize(300, 200);
//         // Заголовок окна
//         setWindowTitle("Клон");

//         // Вертикальный лайаут
//         QVBoxLayout *layout = new QVBoxLayout(this);
//         // Метка для текста
//         QLabel *lbl = new QLabel(this);
//         lbl->setAlignment(Qt::AlignCenter);
//         // Разрешаем перенос слов
//         lbl->setWordWrap(true);

//         // Если это "особое" окно (выигрышное)
//         if (m_isSpecial) {
//             // Получаем текущую палитру
//             QPalette pal = palette();
//             // Меняем цвет фона на светло-зеленый
//             pal.setColor(QPalette::Window, QColor(220, 255, 220));
//             // Разрешаем виджету закрашивать фон
//             setAutoFillBackground(true);
//             // Применяем палитру
//             setPalette(pal);

//             // Пишем текст победы
//             lbl->setText("Ты нашел меня! Я сдаюсь.");
//             // Создаем кнопку победы
//             QPushButton *btnWin = new QPushButton("Сдаюсь, ты победил!", this);
//             layout->addWidget(lbl);
//             layout->addWidget(btnWin);
//             // При нажатии кнопки вызываем специальный слот
//             connect(btnWin, &QPushButton::clicked, this, &CloneWindow::specialBtnClicked);
//         } else {
//             // Если окно обычное
//             // Список дразнилок
//             QStringList phrases = {"Не это окно!", "Попробуй другое!", "Я просто клон", "Ха-ха, мимо!"};
//             // Выбираем случайную фразу из списка
//             lbl->setText(phrases.at(QRandomGenerator::global()->bounded(phrases.size())));
//             layout->addWidget(lbl);
//         }
//     }

//     // Событие закрытия клона (нажатие на крестик)
//     void CloneWindow::closeEvent(QCloseEvent *event) {
//         // Сообщаем Phase3Widget, что окно закрылось (и передаем информацию, было ли оно особым)
//         emit windowClosed(this, m_isSpecial);
//         // Разрешаем стандартное закрытие (окно исчезнет, но память удалим позже)
//         QWidget::closeEvent(event);
//     }

//     // Конструктор контроллера 3-й фазы
//     Phase3Widget::Phase3Widget(QWidget *parent) : QWidget(parent) {
//         // Просто текст в главном окне
//         QLabel *lbl = new QLabel("Ты думал, это конец?\n(Ищи окно с другим фоном)", this);
//         lbl->setAlignment(Qt::AlignCenter);
//         QVBoxLayout *lay = new QVBoxLayout(this);
//         lay->addWidget(lbl);
//     }

//     // Запуск фазы 3
//     void Phase3Widget::startPhase() {
//         // Выбираем случайный номер от 0 до 4, который будет выигрышным
//         int specialIndex = QRandomGenerator::global()->bounded(5);

//         // Цикл создания 5 окон
//         for (int i = 0; i < 5; ++i) {
//             // Если i совпадает с specialIndex, передаем true (особое окно)
//             spawnClone(i == specialIndex);
//         }
//     }

//     // Функция создания нового окна
//     void Phase3Widget::spawnClone(bool isSpecial) {
//         // Ограничение: если окон больше 20, новые не создаем (чтобы комп не завис)
//         if (clones.size() >= 20) return;

//         // Создаем новый экземпляр окна в куче (heap)
//         CloneWindow *w = new CloneWindow(isSpecial);

//         // Получаем главный экран
//         QScreen *screen = QGuiApplication::primaryScreen();
//         // Получаем доступную геометрию (без панелей задач)
//         QRect screenGeom = screen->availableGeometry();
//         // Считаем границы для рандома (ширина экрана - ширина окна)
//         int maxX = screenGeom.width() - 300;
//         int maxY = screenGeom.height() - 200;

//         // Генерируем случайные координаты, добавляя смещение экрана (x, y) для мультимониторов
//         int x = screenGeom.x() + QRandomGenerator::global()->bounded(maxX > 0 ? maxX : 1);
//         int y = screenGeom.y() + QRandomGenerator::global()->bounded(maxY > 0 ? maxY : 1);

//         // Сначала показываем окно (Ubuntu может поставить его куда захочет)
//         w->show();

//         // === ХАК ДЛЯ UBUNTU ===
//         // Запускаем таймер на 50 мс. Код внутри лямбды выполнится ПОЗЖЕ.
//         // Это нужно, чтобы перебить настройки оконного менеджера GNOME.
//         QTimer::singleShot(50, w, [w, x, y]() {
//             w->move(x, y); // Перемещаем окно в наши случайные координаты
//             w->raise(); // Поднимаем окно на самый верх (чтобы видеть его)
//         });

//         // Подключаем сигналы: закрытие клона -> обработчик
//         connect(w, &CloneWindow::windowClosed, this, &Phase3Widget::onCloneClosed);
//         // Кнопка победы -> обработчик победы
//         connect(w, &CloneWindow::specialBtnClicked, this, &Phase3Widget::onVictory);

//         // Добавляем окно в список активных клонов
//         clones.append(w);
//     }

//     // Обработка закрытия клона
//     void Phase3Widget::onCloneClosed(CloneWindow *sender, bool wasSpecial) {
//         // Удаляем указатель из списка
//         clones.removeAll(sender);
//         // Планируем удаление объекта из памяти (безопасное удаление)
//         sender->deleteLater();

//         // Логика Гидры: отрубил одну голову — выросло две
//         // Если закрыли особое окно (крестиком), оно должно переродиться
//         spawnClone(wasSpecial);
//         // И еще одно обычное в наказание
//         spawnClone(false);
//     }

//     // Удаление всех клонов (при победе)
//     void Phase3Widget::closeAllClones() {
//         // Проходим по всем клонам в списке
//         for (auto w : clones) {
//             // Отключаем сигналы, чтобы закрытие не вызывало создание новых окон
//             disconnect(w, nullptr, this, nullptr);
//             // Закрываем окно
//             w->close();
//             // Удаляем из памяти
//             w->deleteLater();
//         }
//         // Очищаем список
//         clones.clear();
//     }

//     // Победа в 3 фазе
//     void Phase3Widget::onVictory() {
//         // Убираем все окна
//         closeAllClones();
//         // Сигнализируем о переходе к 4 фазе
//         emit phaseFinished();
//     }


//     // ================= ГЛАВНОЕ ОКНО: Реализация =================

//     // Конструктор главного окна
//     MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
//         // Устанавливаем размер 600x400
//         resize(600, 400);
//         // Заголовок окна
//         setWindowTitle("Окно, которое не хочет умирать");

//         // Создаем виджет-стек (страницы, которые меняются друг за другом)
//         stackedWidget = new QStackedWidget(this);
//         // Лайаут для главного окна
//         QVBoxLayout *layout = new QVBoxLayout(this);
//         layout->addWidget(stackedWidget);
//         // Убираем отступы по краям
//         layout->setContentsMargins(0,0,0,0);

//         // Создаем экземпляры страниц для всех фаз
//         phase1 = new Phase1Widget(this);
//         phase2 = new Phase2Widget(this);
//         phase3 = new Phase3Widget(this);
//         // Заглушка для 4 фазы (просто текст)
//         phase4Placeholder = new QLabel("Ты победил!", this);
//         // Приводим тип к QLabel*, чтобы вызвать метод выравнивания
//         dynamic_cast<QLabel*>(phase4Placeholder)->setAlignment(Qt::AlignCenter);

//         // Добавляем страницы в стек (по порядку индексов: 0, 1, 2, 3)
//         stackedWidget->addWidget(phase1);
//         stackedWidget->addWidget(phase2);
//         stackedWidget->addWidget(phase3);
//         stackedWidget->addWidget(phase4Placeholder);

//         // Связываем сигналы завершения фаз с методами переключения страниц
//         connect(phase1, &Phase1Widget::phaseFinished, this, &MainWindow::goToPhase2);
//         connect(phase2, &Phase2Widget::phaseFinished, this, &MainWindow::goToPhase3);
//         connect(phase3, &Phase3Widget::phaseFinished, this, &MainWindow::goToPhase4);
//     }

//     // Событие закрытия главного окна
//     void MainWindow::closeEvent(QCloseEvent *event) {
//         // Проверяем, активна ли сейчас Фаза 2 (индекс 1)
//         if (stackedWidget->currentIndex() == 1) {
//             // Игнорируем событие -> ЗАПРЕЩАЕМ закрытие окна
//             event->ignore();

//             // === ХАК ДЛЯ UBUNTU ===
//             // Запускаем таймер с минимальной задержкой (10 мс).
//             // Это разрывает поток выполнения: сначала завершится closeEvent (отказ),
//             // а потом выполнится этот код перемещения. Без этого Ubuntu блокирует move().
//             QTimer::singleShot(10, this, [this]() {
//                 // Если окно развернуто на весь экран, возвращаем в обычный режим
//                 if (isMaximized()) showNormal();

//                 // Получаем размеры экрана
//                 QScreen *screen = QGuiApplication::primaryScreen();
//                 QRect screenGeom = screen->availableGeometry();

//                 // Считаем доступные границы
//                 int maxX = screenGeom.width() - width();
//                 int maxY = screenGeom.height() - height();

//                 // Генерируем координаты
//                 int newX = screenGeom.x() + QRandomGenerator::global()->bounded(maxX > 0 ? maxX : 1);
//                 int newY = screenGeom.y() + QRandomGenerator::global()->bounded(maxY > 0 ? maxY : 1);

//                 // ВМЕСТО move() ИСПОЛЬЗУЕМ setGeometry()
//                 // Это более "агрессивная" команда: она задает и позицию, и размер одновременно.
//                 // Ubuntu реже игнорирует её, чем просто move().
//                 this->setGeometry(newX, newY, width(), height());

//                 // Поднимаем окно на передний план (на всякий случай)
//                 this->raise();
//                 this->activateWindow();
//             });

//             // Выходим из функции, так как мы обработали фазу 2
//             return;
//         }

//         // Если фаза не 2, разрешаем закрытие (стандартное поведение)
//         QWidget::closeEvent(event);
//     }

//     // Событие нажатия клавиш
//     void MainWindow::keyPressEvent(QKeyEvent *event) {
//         // Если сейчас Фаза 2
//         if (stackedWidget->currentIndex() == 1) {
//             // Проверяем нажатие Ctrl (modifiers) и клавиши Q
//             if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Q) {
//                 // Показываем скрытое поле ввода
//                 phase2->showCheatInput();
//                 return;
//             }
//         }
//         // Вызываем стандартный обработчик для остальных клавиш
//         QWidget::keyPressEvent(event);
//     }

//     // Переход к Фазе 2
//     void MainWindow::goToPhase2() {
//         stackedWidget->setCurrentIndex(1);
//     }

//     // Переход к Фазе 3
//     void MainWindow::goToPhase3() {
//         stackedWidget->setCurrentIndex(2);
//         phase3->startPhase(); // Запускаем спавн окон-клонов
//         hide(); // Скрываем главное окно (чтобы остались только клоны)
//     }

//     // Переход к Фазе 4 (победа)
//     void MainWindow::goToPhase4() {
//         show(); // Показываем главное окно обратно (так как мы его скрыли в goToPhase3)
//         stackedWidget->setCurrentIndex(3); // Показываем текст победы
//     }







