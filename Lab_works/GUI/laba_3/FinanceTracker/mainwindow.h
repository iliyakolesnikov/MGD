#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QVector>

struct Expense {
    QString category;
    QString description;
    double amount;
    QString date;  // для простоты храним как строку
};

class MainWindow : public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void addExpense();
    void deleteSelected();
    void updateSummary();
    void filterByCategory(const QString &category);

private:
    void setupUI();
    void refreshTable();

    // Данные
    QVector<Expense> expenses;
    double budget = 50000.0;

    // Виджеты ввода
    QComboBox *categoryCombo = nullptr;
    QLineEdit *descriptionEdit = nullptr;
    QDoubleSpinBox *amountSpin = nullptr;
    QPushButton *addBtn = nullptr;
    QPushButton *deleteBtn = nullptr;

    // Таблиц
    QTableWidget *table = nullptr;

    // Фильтр
    QComboBox *filterCombo = nullptr;

    // Итоги
    QLabel *totalLabel = nullptr;
    QLabel *budgetLabel = nullptr;
    QLabel *remainLabel = nullptr;
};

#endif

