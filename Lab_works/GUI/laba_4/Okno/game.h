#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>
#include <QList>
#include <QEvent>
#include <QEnterEvent>

// --- ФАЗА 1: Классы ---

class RunawayButton : public QPushButton {
    Q_OBJECT
public:
    explicit RunawayButton(QWidget *parent = nullptr);
protected:
    void enterEvent(QEnterEvent *event) override; // Отслеживаем наведение мыши
};

class Phase1Widget : public QWidget {
    Q_OBJECT
public:
    explicit Phase1Widget(QWidget *parent = nullptr);
signals:
    void phaseFinished(); // Сигнал перехода к фазе 2
private slots:
    void onBtnClicked();
private:
    RunawayButton *btnExit;
};

// --- ФАЗА 2: Классы ---

class Phase2Widget : public QWidget {
    Q_OBJECT
public:
    explicit Phase2Widget(QWidget *parent = nullptr);
    void showCheatInput(); // Показать скрытое поле ввода
signals:
    void phaseFinished(); // Сигнал перехода к фазе 3
private slots:
    void checkCode();
private:
    QLineEdit *codeStream;
    QLabel *feedbackLabel;
};

// --- ФАЗА 3: Классы ---

class CloneWindow : public QWidget {
    Q_OBJECT
public:
    explicit CloneWindow(bool isSpecial, QWidget *parent = nullptr);
signals:
    void windowClosed(CloneWindow *sender, bool wasSpecial);
    void specialBtnClicked(); // Победа
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    bool m_isSpecial;
};

class Phase3Widget : public QWidget {
    Q_OBJECT
public:
    explicit Phase3Widget(QWidget *parent = nullptr);
    void startPhase(); // Запуск спавна окон
    void closeAllClones(); // Очистка
signals:
    void phaseFinished(); // Переход к фазе 4
private slots:
    void onCloneClosed(CloneWindow *sender, bool wasSpecial);
    void onVictory();
private:
    QList<CloneWindow*> clones;
    void spawnClone(bool isSpecial);
};

// --- ГЛАВНОЕ ОКНО ---

class MainWindow : public QWidget { // Используем QWidget как контейнер верхнего уровня
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
private slots:
    void goToPhase2();
    void goToPhase3();
    void goToPhase4();
private:
    QStackedWidget *stackedWidget;
    Phase1Widget *phase1;
    Phase2Widget *phase2;
    Phase3Widget *phase3;
    QWidget *phase4Placeholder; // Заглушка для фазы 4
};

#endif // GAME_H
