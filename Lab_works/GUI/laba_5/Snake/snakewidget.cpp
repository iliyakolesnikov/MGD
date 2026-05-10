#include "snakewidget.h"

// ---------------------------------------------------------------
//  Конструктор — здесь всё готово, менять не нужно
// ---------------------------------------------------------------
SnakeWidget::SnakeWidget(QWidget *parent)
    : QWidget(parent)
    , direction(Direction::Right)
    , nextDirection(Direction::Right)
    , state(GameState::Running)
    , score(0)
{
    setFocusPolicy(Qt::StrongFocus);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SnakeWidget::gameLoop);

    initGame();
}

// ---------------------------------------------------------------
//  Главный цикл — вызывается по таймеру
// ---------------------------------------------------------------
void SnakeWidget::gameLoop()
{
    if (state != GameState::Running)
        return;

    direction = nextDirection;
    moveSnake();
    update();  // вызывает paintEvent
}

// ---------------------------------------------------------------
//  paintEvent — каркас отрисовки (готов)
// ---------------------------------------------------------------
void SnakeWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Фон
    painter.fillRect(rect(), QColor(30, 30, 30));

    drawGrid(painter);
    drawFood(painter);
    drawSnake(painter);
    drawUI(painter);
}

// ---------------------------------------------------------------
//  keyPressEvent — каркас обработки клавиш (готов)
// ---------------------------------------------------------------
void SnakeWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:    case Qt::Key_W: handleDirectionChange(Direction::Up);    break;
    case Qt::Key_Down:  case Qt::Key_S: handleDirectionChange(Direction::Down);  break;
    case Qt::Key_Left:  case Qt::Key_A: handleDirectionChange(Direction::Left);  break;
    case Qt::Key_Right: case Qt::Key_D: handleDirectionChange(Direction::Right); break;
    case Qt::Key_P:
        if (state == GameState::Running) {
            state = GameState::Paused;
            timer->stop();
        } else if (state == GameState::Paused) {
            state = GameState::Running;
            timer->start(INIT_SPEED);
        }
        update();
        break;
    case Qt::Key_R:
        initGame();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}


// ===========================================================
//  ЗАГЛУШКИ — РЕАЛИЗУЙ КАЖДУЮ ФУНКЦИЮ
// ===========================================================

void SnakeWidget::initGame()
{
    // TODO:
    // 1. Очистить snake и поместить 3-4 сегмента в центр поля
    //    Пример: snake = { {15,15}, {14,15}, {13,15} };
    // 2. Установить direction = nextDirection = Direction::Right
    // 3. score = 0, state = GameState::Running
    // 4. Вызвать spawnFood()
    // 5. Запустить timer с интервалом INIT_SPEED
}

QPoint SnakeWidget::calcNewHead()
{
    // TODO:
    // Взять snake.first() и сдвинуть на 1 клетку
    // в зависимости от direction:
    //   Up    → y - 1
    //   Down  → y + 1
    //   Left  → x - 1
    //   Right → x + 1
    // Вернуть новую точку.

    return QPoint();  // заглушка
}

bool SnakeWidget::checkCollision(const QPoint &head)
{
    // TODO:
    // Вернуть true, если:
    //   — head выходит за границы [0, FIELD_SIZE)
    //   — head совпадает с любым сегментом snake
    // Иначе false.

    return false;  // заглушка
}

void SnakeWidget::moveSnake()
{
    // TODO:
    // 1. QPoint newHead = calcNewHead();
    // 2. Если checkCollision(newHead) → state = GameOver, timer->stop(), return
    // 3. Вставить newHead в начало snake (prepend)
    // 4. Если newHead == food → score++, spawnFood()
    //    Иначе → удалить последний элемент snake (removeLast)

}

void SnakeWidget::spawnFood()
{
    // TODO:
    // Генерировать случайную точку (x, y) в пределах [0, FIELD_SIZE)
    // до тех пор, пока она попадает на тело змейки.
    // Записать результат в food.
    //
    // Подсказка: QRandomGenerator::global()->bounded(FIELD_SIZE)

}

void SnakeWidget::drawGrid(QPainter &painter)
{
    // TODO:
    // Нарисовать тонкие линии сетки CELL_SIZE × CELL_SIZE.
    // Цвет: например, QColor(50, 50, 50).
    // Вертикальные линии: x от 0 до FIELD_SIZE * CELL_SIZE с шагом CELL_SIZE
    // Горизонтальные — аналогично.

}

void SnakeWidget::drawSnake(QPainter &painter)
{
    // TODO:
    // Пройти по snake. Для каждого сегмента:
    //   painter.fillRect(seg.x() * CELL_SIZE, seg.y() * CELL_SIZE,
    //                    CELL_SIZE, CELL_SIZE, color);
    // Голову (snake.first()) можно покрасить в другой цвет.

}

void SnakeWidget::drawFood(QPainter &painter)
{
    // TODO:
    // Нарисовать еду (красный квадрат или кружок) по координатам food.
    // Например:
    //   painter.setBrush(Qt::red);
    //   painter.drawEllipse(food.x()*CELL_SIZE, food.y()*CELL_SIZE,
    //                       CELL_SIZE, CELL_SIZE);

}

void SnakeWidget::drawUI(QPainter &painter)
{
    // TODO:
    // 1. Вывести "Score: <score>" в углу экрана
    //    painter.setPen(Qt::white);
    //    painter.drawText(10, 20, QString("Score: %1").arg(score));
    //
    // 2. Если state == GameOver — крупный текст "GAME OVER" по центру
    //    + подсказку "Press R to restart"
    //
    // 3. Если state == Paused — текст "PAUSED" по центру

}

void SnakeWidget::handleDirectionChange(Direction newDir)
{
    // TODO:
    // Запретить разворот на 180°:
    //   если direction == Up,    нельзя newDir == Down
    //   если direction == Down,  нельзя newDir == Up
    //   если direction == Left,  нельзя newDir == Right
    //   если direction == Right, нельзя newDir == Left
    // Если разворота нет → nextDirection = newDir;

}

