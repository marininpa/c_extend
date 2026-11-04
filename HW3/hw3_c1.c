/*
 * Задание 2: выход за границы экрана + столкновение змеи с самой собой
 * (добавлена функция из задания 1).
 *
 * База — 14-19_snake.c. Изменено:
 *   - убрано зацикливание по левому краю (в исходнике змея «телепортировалась»
 *     с левого края на правый);
 *   - добавлена функция outsideField() — проверка выхода за пределы поля;
 *   - добавлена функция selfCollision() — столкновение головы с телом (из задания 1);
 *   - go() считает новую позицию и проверяет обе ситуации.
 *
 * Поле: x в [0, max_x-1], y в [MIN_Y, max_y-1]. Верхняя строка 0 занята
 * подсказкой, поэтому верхняя граница — MIN_Y (= 2).
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#define MIN_Y  2
enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10)};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=20, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10};

/* Причина завершения игры */
enum {OVER_NONE=0, OVER_WALL, OVER_SELF};

// Здесь храним коды управления змейкой
struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
}control_buttons;

struct control_buttons default_controls = {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT};

typedef struct snake_t
{
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail_t *tail;
    struct control_buttons controls;
} snake_t;

typedef struct tail_t
{
    int x;
    int y;
} tail_t;

void initTail(struct tail_t t[], size_t size)
{
    struct tail_t init_t={0,0};
    for(size_t i=0; i<size; i++)
    {
        t[i]=init_t;
    }
}

void initHead(struct snake_t *head, int x, int y)
{
    head->x = x;
    head->y = y;
    head->direction = RIGHT;
}

void initSnake(snake_t *head, size_t size, int x, int y)
{
    tail_t*  tail  = (tail_t*) malloc(MAX_TAIL_SIZE*sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head, x, y);
    head->tail = tail; // прикрепляем к голове хвост
    head->tsize = size+1;
    head->controls = default_controls;

    // Тело стартует в точке головы (см. hw3_c0.c — почему).
    for (size_t i = 0; i < head->tsize; ++i)
    {
        tail[i].x = x;
        tail[i].y = y;
    }
}

/*
 * Новая клетка (x,y) не должна совпадать с телом змейки.
 * Последний сегмент (индекс tsize-1) не учитываем: на этом же шаге он
 * сдвинется и освободит свою клетку.
 */
int selfCollision(const snake_t *head, int x, int y)
{
    for (size_t i = 0; i + 1 < head->tsize; ++i)
    {
        if (head->tail[i].x == x && head->tail[i].y == y)
            return 1;
    }
    return 0;
}

/*
 * Точка (x,y) вне поля? Координаты экрана: строка сверху вниз (y),
 * столбец слева вправо (x). Верхняя доступная строка — MIN_Y.
 */
int outsideField(int x, int y, int max_x, int max_y)
{
    return (x < 0) || (x >= max_x) || (y < MIN_Y) || (y >= max_y);
}

/*
 * Движение головы. Новая позиция сначала проверяется на границы,
 * при ударе игра завершается (змейка не переносится на другую сторону).
 */
void go(struct snake_t *head, int *gameover)
{
    int max_x=0, max_y=0;
    getmaxyx(stdscr, max_y, max_x); // macro - размер терминала

    int nx = head->x;
    int ny = head->y;

    switch (head->direction)
    {
        case LEFT:  --nx; break;
        case RIGHT: ++nx; break;
        case UP:    --ny; break;
        case DOWN:  ++ny; break;
        default: return;
    }

    if (outsideField(nx, ny, max_x, max_y))
    {
        *gameover = OVER_WALL;
        return;
    }
    if (selfCollision(head, nx, ny))
    {
        *gameover = OVER_SELF;
        return;
    }

    mvprintw(head->y, head->x, " "); // очищаем один символ
    head->x = nx;
    head->y = ny;
    mvprintw(head->y, head->x, "%c", '@');
    refresh();
}

void changeDirection(struct snake_t* snake, const int32_t key)
{
    if (key == snake->controls.down)
        snake->direction = DOWN;
    else if (key == snake->controls.up)
        snake->direction = UP;
    else if (key == snake->controls.right)
        snake->direction = RIGHT;
    else if (key == snake->controls.left)
        snake->direction = LEFT;
}

void goTail(struct snake_t *head)
{
    char ch = '*';
    mvprintw(head->tail[head->tsize-1].y, head->tail[head->tsize-1].x, " ");
    for(size_t i = head->tsize-1; i>0; i--)
    {
        head->tail[i] = head->tail[i-1];
        if( head->tail[i].y || head->tail[i].x)
            mvprintw(head->tail[i].y, head->tail[i].x, "%c", ch);
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

int main()
{
    snake_t* snake = (snake_t*)malloc(sizeof(snake_t));
    initSnake(snake,START_TAIL_SIZE,10,10);
    initscr();
    keypad(stdscr, TRUE); // Включаем F1, F2, стрелки и т.д.
    raw();                // Откдючаем line buffering
    noecho();            // Отключаем echo() режим при вызове getch
    curs_set(FALSE);    //Отключаем курсор
    mvprintw(0, 0,"Use arrows for control. Press 'F10' for EXIT");
    timeout(0);    //Отключаем таймаут после нажатия клавиши в цикле
    int key_pressed=0;
    int gameover = OVER_NONE;
    while( key_pressed != STOP_GAME && gameover == OVER_NONE )
    {
        key_pressed = getch(); // Считываем клавишу
        go(snake, &gameover);
        if (gameover == OVER_NONE)
            goTail(snake);
        timeout(100); // Задержка при отрисовке
        changeDirection(snake, key_pressed);
    }
    endwin(); // Завершаем режим curses mod
    if (gameover == OVER_WALL)
        printf("Game over: snake hit the wall\n");
    else if (gameover == OVER_SELF)
        printf("Game over: snake ran into itself\n");
    free(snake->tail);
    free(snake);
    return 0;
}
