#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QDate>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent) {
    setWindowTitle("Финансовый трекер");
    resize(750, 550);
    setupUI();
}

void MainWindow::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // === Группа: Новый расход ===
    QGroupBox *inputGroup = new QGroupBox("Новый расход");
    QHBoxLayout *inputLayout = new QHBoxLayout();

    categoryCombo = new QComboBox();
    categoryCombo->addItems({
        "Еда", "Транспорт", "Жильё",
        "Развлечения", "Здоровье", "Одежда", "Другое"
    });
    categoryCombo->setMinimumWidth(120);

    descriptionEdit = new QLineEdit();
    descriptionEdit->setPlaceholderText("Описание...");
    descriptionEdit->setMaxLength(100);

    amountSpin = new QDoubleSpinBox();
    amountSpin->setRange(0.01, 999999.99);
    amountSpin->setPrefix("₽ ");
    amountSpin->setDecimals(2);
    amountSpin->setValue(0.01);

    addBtn = new QPushButton("Добавить");
    addBtn->setStyleSheet("background-color: #27ae60; color: white; font-weight: bold; padding: 6px 16px;");

    inputLayout->addWidget(new QLabel("Категория:"));
    inputLayout->addWidget(categoryCombo);
    inputLayout->addWidget(new QLabel("Описание:"));
    inputLayout->addWidget(descriptionEdit, 1);
    inputLayout->addWidget(new QLabel("Сумма:"));
    inputLayout->addWidget(amountSpin);
    inputLayout->addWidget(addBtn);

    inputGroup->setLayout(inputLayout);
    mainLayout->addWidget(inputGroup);

    // === Таблица ===
    table = new QTableWidget();
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels(
        {"Дата", "Категория", "Описание", "Сумма"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);  // Описание тянется
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);

    mainLayout->addWidget(table, 1);  // 1 = stretch factor


    // === Фильтр + кнопка удаления ===
    QHBoxLayout *controlRow = new QHBoxLayout();

    controlRow->addWidget(new QLabel("Фильтр:"));
    filterCombo = new QComboBox();
    filterCombo->addItem("Все");
    filterCombo->addItems({
        "Еда", "Транспорт", "Жильё",
        "Развлечения", "Здоровье", "Одежда", "Другое"
    });
    controlRow->addWidget(filterCombo);

    controlRow->addStretch();  // пружина

    deleteBtn = new QPushButton("Удалить выбранное");
    deleteBtn->setStyleSheet("background-color: #e74c3c; color: white; padding: 6px 12px;");
        controlRow->addWidget(deleteBtn);

        mainLayout->addLayout(controlRow);

        // === Итоги ===
        QGroupBox *summaryGroup = new QGroupBox("Итоги");
        QHBoxLayout *summaryLayout = new QHBoxLayout();

        totalLabel = new QLabel("Потрачено: 0.00 ₽");
        budgetLabel = new QLabel(
            "Бюджет: " + QString::number(budget, 'f', 2) + " ₽");
        remainLabel = new QLabel("Остаток: 0.00 ₽");

        totalLabel->setStyleSheet("font-size: 14px;");
        budgetLabel->setStyleSheet("font-size: 14px;");
        remainLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

        summaryLayout->addWidget(totalLabel);
        summaryLayout->addWidget(budgetLabel);
        summaryLayout->addWidget(remainLabel);

        summaryGroup->setLayout(summaryLayout);
        mainLayout->addWidget(summaryGroup);


    connect(addBtn, &QPushButton::clicked, this, &MainWindow::addExpense);
    // Добавление по Enter из category поля описания
    connect(descriptionEdit, &QLineEdit::returnPressed, this, &MainWindow::addExpense);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::deleteSelected);
    connect(filterCombo, &QComboBox::currentTextChanged, this, &MainWindow::filterByCategory);
}


void MainWindow::addExpense() {
    QString desc = descriptionEdit->text().trimmed();
    double amount = amountSpin->value();

    if (desc.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите описание расхода.");
        descriptionEdit->setFocus();
        return;
    }

    Expense e;
    e.category = categoryCombo->currentText();
    e.description = desc;
    e.amount = amount;
    e.date = QDate::currentDate().toString("dd.MM.yyyy");

    expenses.append(e);

    // Сброс формы
    descriptionEdit->clear();
    amountSpin->setValue(0.01);
    descriptionEdit->setFocus();

    refreshTable();
    updateSummary();
}


void MainWindow::deleteSelected() {
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::information(this,
                                 "Удаление",
                                 "Выделите строку для удаления.");
        return;
    }

    // Находим запись по дате+описанию
    QString date = table->item(row, 0)->text();
    QString desc = table->item(row, 2)->text();

    for (int i = 0; i < expenses.size(); ++i) {
        if (expenses[i].date == date
            && expenses[i].description == desc) {
            expenses.removeAt(i);
            break;
        }
    }

    refreshTable();
    updateSummary();
}


void MainWindow::updateSummary() {
double total = 0.0;
    for (const auto &e : expenses)
        total += e.amount;

    double remain = budget - total;

    totalLabel->setText("Потрачено: " + QString::number(total, 'f', 2) + " ₽");
    remainLabel->setText("Остаток: "  + QString::number(remain, 'f', 2) + " ₽");

    // Цветовая индикация
    if (remain < 0) {
        remainLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #e74c3c;");
    } else if (remain < budget * 0.2) {
        remainLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #f39c12;");
    } else {
        remainLabel->setStyleSheet("font-size: 14px; font-weight: bold;   color: #27ae60;");
    }
}


void MainWindow::filterByCategory(const QString &category) {
    Q_UNUSED(category);
    refreshTable();
}


void MainWindow::refreshTable() {
    // Запоминаем текущий фильтр
    QString filter = filterCombo ? filterCombo->currentText() : "Все";

    table->setRowCount(0);

    for (const auto &e : expenses) {
        if (filter != "Все" && e.category != filter)
            continue;

        int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0,
                       new QTableWidgetItem(e.date));
        table->setItem(row, 1,
                       new QTableWidgetItem(e.category));
        table->setItem(row, 2,
                       new QTableWidgetItem(e.description));

        auto *amountItem = new QTableWidgetItem(
            QString::number(e.amount, 'f', 2) + " ₽");
        amountItem->setTextAlignment(
            Qt::AlignRight | Qt::AlignVCenter);
        table->setItem(row, 3, amountItem);
    }

}




































//-----------------------------------------------------------------------------------------

// #include "mainwindow.h"
// #include <QVBoxLayout> // Подключаем вертикальный слой (элементы друг под другом)
// #include <QHBoxLayout> // Подключаем горизонтальный слой (элементы в ряд)
// #include <QGroupBox>   // Подключаем рамку с заголовком (группировка элементов)
// #include <QHeaderView> // Для управления заголовками таблицы (растягивание колонок)
// #include <QDate>       // Для работы с текущей датой
// #include <QMessageBox> // Для всплывающих окон с ошибками или предупреждениями


// // Конструктор главного окна
// MainWindow::MainWindow(QWidget *parent)
//     : QWidget(parent) {
//     setWindowTitle("Финансовый трекер"); // Устанавливаем текст в шапке окна
//     resize(750, 550); // Задаем начальный размер окна (ширина, высота)
//     setupUI(); // Запускаем функцию, которая нарисует все кнопочки и поля
// }

// // Функция настройки интерфейса (создание всех виждетов)
// void MainWindow::setupUI() {
//     QVBoxLayout *mainLayout = new QVBoxLayout(this); // Создаем главный вертикальный слой для всего окна

//     // === Группа: Новый расход (Верхняя часть) ===
//     QGroupBox *inputGroup = new QGroupBox("Новый расход"); // Создаем рамку с названием
//     QHBoxLayout *inputLayout = new QHBoxLayout(); // Внутри рамки элементы будут стоять горизонтально

//     categoryCombo = new QComboBox(); // Создаем выпадающий список
//     // Добавляем в список варианты категорий
//     categoryCombo->addItems({
//         "Еда", "Транспорт", "Жильё",
//         "Развлечения", "Здоровье", "Одежда", "Другое"
//     });
//     categoryCombo->setMinimumWidth(120); // Чтобы список не был слишком узким

//     descriptionEdit = new QLineEdit(); // Поле для ввода текста
//     descriptionEdit->setPlaceholderText("Описание..."); // Подсказка, исчезающая при вводе
//     descriptionEdit->setMaxLength(100); // Ограничение на длину текста

//     amountSpin = new QDoubleSpinBox(); // Поле для ввода дробных чисел (денег)
//     amountSpin->setRange(0.01, 999999.99); // Минимальное и максимальное значение
//     amountSpin->setPrefix("₽ "); // Значок валюты перед цифрами
//     amountSpin->setDecimals(2); // Два знака после запятой (копейки)
//     amountSpin->setValue(0.01); // Значение по умолчанию

//     addBtn = new QPushButton("Добавить"); // Кнопка подтверждения
//     // Красим кнопку в зеленый цвет через CSS-стили
//     addBtn->setStyleSheet(
//         "background-color: #27ae60; color: white; font-weight: bold; padding: 6px 16px;");

//     // Добавляем созданные элементы в горизонтальный слой по порядку
//     inputLayout->addWidget(new QLabel("Категория:"));
//     inputLayout->addWidget(categoryCombo);
//     inputLayout->addWidget(new QLabel("Описание:"));
//     inputLayout->addWidget(descriptionEdit, 1); // 1 означает, что это поле будет растягиваться
//     inputLayout->addWidget(new QLabel("Сумма:"));
//     inputLayout->addWidget(amountSpin);
//     inputLayout->addWidget(addBtn);

//     inputGroup->setLayout(inputLayout); // Помещаем слой внутрь рамки
//     mainLayout->addWidget(inputGroup); // Помещаем рамку в главное окно

//     // === Таблица (Центральная часть) ===
//     table = new QTableWidget(); // Создаем таблицу
//     table->setColumnCount(4); // У нас будет 4 колонки
//     // Задаем названия колонок
//     table->setHorizontalHeaderLabels(
//         {"Дата", "Категория", "Описание", "Сумма"});
//     table->horizontalHeader()->setStretchLastSection(true); // Последняя колонка тянется до края
//     table->horizontalHeader()->setSectionResizeMode(
//         2, QHeaderView::Stretch);  // 3-я колонка (Описание) тоже растягивается, занимая место
//     table->setSelectionBehavior(
//         QAbstractItemView::SelectRows); // При клике выделяется вся строка, а не одна ячейка
//     table->setEditTriggers(
//         QAbstractItemView::NoEditTriggers); // Запрещаем редактировать ячейки вручную
//     table->setAlternatingRowColors(true); // Чередование цветов строк (зебра) для удобства чтения

//     mainLayout->addWidget(table, 1);  // Добавляем таблицу. '1' заставляет её занимать всё свободное место по высоте


//     // === Фильтр + кнопка удаления (Нижняя панель управления) ===
//     QHBoxLayout *controlRow = new QHBoxLayout(); // Горизонтальный ряд

//     controlRow->addWidget(new QLabel("Фильтр:"));
//     filterCombo = new QComboBox(); // Выпадающий список для фильтрации
//     filterCombo->addItem("Все"); // Пункт "показать всё"
//     filterCombo->addItems({
//         "Еда", "Транспорт", "Жильё",
//         "Развлечения", "Здоровье", "Одежда", "Другое"
//     });
//     controlRow->addWidget(filterCombo);

//     controlRow->addStretch();  // "Пружина": отталкивает всё, что слева, от того, что справа

//     deleteBtn = new QPushButton("Удалить выбранное"); // Кнопка удаления
//     deleteBtn->setStyleSheet("background-color: #e74c3c; color: white; padding: 6px 12px;"); // Красный цвет
//     controlRow->addWidget(deleteBtn);

//     mainLayout->addLayout(controlRow); // Добавляем этот ряд кнопок под таблицу

//     // === Итоги (Самый низ) ===
//     QGroupBox *summaryGroup = new QGroupBox("Итоги");
//     QHBoxLayout *summaryLayout = new QHBoxLayout();

//     totalLabel = new QLabel("Потрачено: 0.00 ₽");
//     // QString::number превращает число в строку. 'f', 2 — формат float с 2 знаками после запятой
//     budgetLabel = new QLabel(
//         "Бюджет: " + QString::number(budget, 'f', 2) + " ₽");
//     remainLabel = new QLabel("Остаток: 0.00 ₽");

//     // Настраиваем размер шрифта
//     totalLabel->setStyleSheet("font-size: 14px;");
//     budgetLabel->setStyleSheet("font-size: 14px;");
//     remainLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

//     summaryLayout->addWidget(totalLabel);
//     summaryLayout->addWidget(budgetLabel);
//     summaryLayout->addWidget(remainLabel);

//     summaryGroup->setLayout(summaryLayout);
//     mainLayout->addWidget(summaryGroup);

//     // === Связь событий (Сигналы и Слоты) ===
//     // Если нажали кнопку "Добавить" -> вызвать функцию addExpense
//     connect(addBtn, &QPushButton::clicked, this, &MainWindow::addExpense);
//     // Если нажали Enter в поле описания -> тоже вызвать addExpense
//     connect(descriptionEdit, &QLineEdit::returnPressed, this, &MainWindow::addExpense);
//     // Нажали кнопку "Удалить" -> вызвать deleteSelected
//     connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::deleteSelected);
//     // Сменили категорию в фильтре -> вызвать filterByCategory
//     connect(filterCombo, &QComboBox::currentTextChanged, this, &MainWindow::filterByCategory);
// }

// // Функция добавления нового расхода
// void MainWindow::addExpense() {
//     QString desc = descriptionEdit->text().trimmed(); // Берем текст и удаляем пробелы по краям
//     double amount = amountSpin->value(); // Берем число из поля суммы

//     // Проверка: если описание пустое, показываем ошибку
//     if (desc.isEmpty()) {
//         QMessageBox::warning(this,
//                              "Ошибка", "Введите описание расхода.");
//         descriptionEdit->setFocus(); // Возвращаем курсор в поле ввода
//         return; // Прерываем функцию
//     }

//     // Создаем объект расхода (структура Expense должна быть описана в .h файле)
//     Expense e;
//     e.category = categoryCombo->currentText(); // Берем выбранную категорию
//     e.description = desc;
//     e.amount = amount;
//     e.date = QDate::currentDate().toString("dd.MM.yyyy"); // Текущая дата в формате день.месяц.год

//     expenses.append(e); // Добавляем расход в общий список (QList или QVector)

//     // Сброс формы (очистка полей после добавления)
//     descriptionEdit->clear();
//     amountSpin->setValue(0.01);
//     descriptionEdit->setFocus();

//     refreshTable(); // Перерисовываем таблицу
//     updateSummary(); // Пересчитываем итоги
// }

// // Функция удаления расхода
// void MainWindow::deleteSelected() {
//     int row = table->currentRow(); // Получаем номер выделенной строки
//     if (row < 0) { // Если ничего не выделено, возвращается -1
//         QMessageBox::information(this,
//                                  "Удаление",
//                                  "Выделите строку для удаления.");
//         return;
//     }

//     // Получаем данные из ячеек, чтобы найти, что именно удалять из списка
//     QString date = table->item(row, 0)->text(); // Дата из 0-й колонки
//     QString desc = table->item(row, 2)->text(); // Описание из 2-й колонки

//     // Ищем этот расход в нашем списке данных
//     for (int i = 0; i < expenses.size(); ++i) {
//         if (expenses[i].date == date && expenses[i].description == desc) {
//             expenses.removeAt(i); // Удаляем из списка
//             break; // Выходим из цикла, раз нашли
//         }
//     }

//     refreshTable(); // Обновляем таблицу
//     updateSummary(); // Обновляем цифры итогов
// }

// // Функция пересчета итогов
// void MainWindow::updateSummary() {
//     double total = 0.0;
//     // Пробегаем по всем расходам и суммируем
//     for (const auto &e : expenses)
//         total += e.amount;

//     double remain = budget - total; // Вычисляем остаток

//     // Обновляем текст на метках
//     totalLabel->setText("Потрачено: " + QString::number(total, 'f', 2) + " ₽");
//     remainLabel->setText("Остаток: "  + QString::number(remain, 'f', 2) + " ₽");

//     // Цветовая индикация в зависимости от остатка
//     if (remain < 0) {
//         // Если ушли в минус — красный жирный текст
//         remainLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #e74c3c;");
//     } else if (remain < budget * 0.2) {
//         // Если осталось меньше 20% — оранжевый
//         remainLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #f39c12;");
//     } else {
//         // Если денег много — зеленый
//         remainLabel->setStyleSheet("font-size: 14px; font-weight: bold;   color: #27ae60;");
//     }
// }

// // Функция фильтрации (вызывается при смене комбобокса фильтра)
// void MainWindow::filterByCategory(const QString &category) {
//     Q_UNUSED(category); // Говорим компилятору не ругаться, что переменная не используется напрямую здесь
//     refreshTable(); // Просто перерисовываем таблицу (логика фильтра внутри refreshTable)
// }

// // Функция перерисовки таблицы
// void MainWindow::refreshTable() {
//     // Запоминаем, что выбрано в фильтре сейчас (или "Все", если фильтра еще нет)
//     QString filter = filterCombo ? filterCombo->currentText() : "Все";

//     table->setRowCount(0); // Полностью очищаем таблицу (удаляем все строки)

//     // Проходим по списку расходов
//     for (const auto &e : expenses) {
//         // Если фильтр не "Все" И категория расхода не совпадает с фильтром -> пропускаем
//         if (filter != "Все" && e.category != filter)
//             continue;

//         int row = table->rowCount(); // Узнаем текущее количество строк
//         table->insertRow(row); // Добавляем новую пустую строку

//         // Заполняем ячейки данными
//         table->setItem(row, 0,
//                        new QTableWidgetItem(e.date));
//         table->setItem(row, 1,
//                        new QTableWidgetItem(e.category));
//         table->setItem(row, 2,
//                        new QTableWidgetItem(e.description));

//         // Для суммы создаем элемент отдельно, чтобы выровнять его
//         auto *amountItem = new QTableWidgetItem(
//             QString::number(e.amount, 'f', 2) + " ₽");
//         amountItem->setTextAlignment(
//             Qt::AlignRight | Qt::AlignVCenter); // Выравнивание по правому краю и центру по вертикали
//         table->setItem(row, 3, amountItem);
//     }
// }

