#ifndef SNAKEWIDGET_H
#define SNAKEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QPainter>
#include <QList>
#include <QPoint>
#include <QRandomGenerator>

// Направление движения змейки
enum class Direction { Up, Down, Left, Right };

// Состояние игры
enum class GameState { Running, Paused, GameOver };

class SnakeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SnakeWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();

private:
    // ===== Константы =====
    static const int CELL_SIZE  = 20;   // размер одной клетки в пикселях
    static const int FIELD_SIZE = 30;   // поле 30x30 клеток
    static const int INIT_SPEED = 150;  // начальный интервал таймера (мс)

    // ===== Состояние игры =====
    QList<QPoint> snake;        // тело змейки (голова — snake.first())
    QPoint        food;         // позиция еды
    Direction     direction;    // текущее направление
    Direction     nextDirection;// буфер следующего направления (защита от разворота)
    GameState     state;
    int           score;
    QTimer       *timer;

    // ==========================================================
    //  ФУНКЦИИ-ЗАГЛУШКИ — нужно дописать реализацию
    // ==========================================================

    /// Инициализирует / сбрасывает всё состояние игры:
    /// змейку в центр, направление вправо, счёт = 0, генерирует еду.
    void initGame();

    /// Перемещает змейку на одну клетку в текущем направлении.
    /// Возвращает новую позицию головы (НЕ модифицирует snake).
    QPoint calcNewHead();

    /// Проверяет, столкнулась ли голова со стеной или с собственным телом.
    /// @param head — новая позиция головы
    /// @return true, если произошло столкновение
    bool checkCollision(const QPoint &head);

    /// Двигает змейку: добавляет новую голову, удаляет хвост
    /// (если не съели еду). Обновляет счёт при поедании.
    void moveSnake();

    /// Генерирует новую позицию еды на свободной клетке
    /// (не на теле змейки).
    void spawnFood();

    /// Отрисовывает сетку поля.
    void drawGrid(QPainter &painter);

    /// Отрисовывает змейку (голову можно выделить другим цветом).
    void drawSnake(QPainter &painter);

    /// Отрисовывает еду.
    void drawFood(QPainter &painter);

    /// Отрисовывает текст счёта (и Game Over, если нужно).
    void drawUI(QPainter &painter);

    /// Обрабатывает нажатие клавиши-направления.
    /// Должна запрещать разворот на 180°.
    void handleDirectionChange(Direction newDir);
};

#endif // SNAKEWIDGET_H
