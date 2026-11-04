/*
 * HW4 (Занятие 4). Доработка игры «змейка».
 *
 * База:
 * - 14-19_snake.c.
 * - HW3/HW3_c2.c
 * Добавлено:
 *   1) управление клавишами WASD независимо от регистра. Раскладки хранятся
 *      в массиве default_controls[CONTROLS]: элемент 0 — стрелки,
 *      элемент 1 — буквы 'W','S','A','D'. Регистр снимается битом 0x20
 *      (в ASCII 'W'=87, 'w'=119, различаются ровно этим битом).
 *   2) функция checkDirection() — запрещает разворот на 180°
 *      (LEFT при движении RIGHT и т.д.).
 *
 * Сопоставление «код клавиши -> направление» вынесено в keyToDirection(),
 * чтобы правило регистра жило в одном месте и им пользовались обе функции.
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
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10};
enum {CONTROLS=2};

// Коды управления змейкой: по одному int на направление
struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
};

// Две раскладки: стрелки и буквы WASD (заглавных достаточно — регистр снимает |0x20)
struct control_buttons default_controls[CONTROLS] = {
    {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT},
    {'S',      'W',    'A',       'D'}
};

/*
 Голова змейки содержит в себе
 x,y - координаты текущей позиции
 direction - направление движения
 tsize - размер хвоста
 *tail - ссылка на хвост
 controls - массив раскладок управления
 */
typedef struct snake_t
{
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail_t *tail;
    struct control_buttons controls[CONTROLS];
} snake_t;

/*
 Хвост это массив состоящий из координат x,y
 */
typedef struct tail_t
{
    int x;
    int y;
} tail_t;

void initTail(struct tail_t t[], size_t size)
{
    struct tail_t init_t = {0, 0};
    for (size_t i = 0; i < size; i++)
    {
        t[i] = init_t;
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
    tail_t* tail = (tail_t*) malloc(MAX_TAIL_SIZE * sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head, x, y);
    head->tail = tail; // прикрепляем к голове хвост
    head->tsize = size + 1;

    // controls теперь массив — копируем каждую раскладку
    for (int i = 0; i < CONTROLS; i++)
        head->controls[i] = default_controls[i];
}

/*
 Определить, какому направлению соответствует клавиша key.
 Возвращает LEFT/UP/RIGHT/DOWN, если клавиша — команда движения,
 либо 0, если это не наша клавиша.

 Сравнение через |0x20 делает его регистронезависимым для букв
 ('W'|0x20 == 'w'|0x20) и не путается с кодами стрелок (> 255):
 KEY_UP|0x20 (291) не совпадает ни с одной буквой.
 */
int keyToDirection(const snake_t *s, int32_t key)
{
    for (int i = 0; i < CONTROLS; i++)
    {
        if ((key | 0x20) == (s->controls[i].up    | 0x20)) return UP;
        if ((key | 0x20) == (s->controls[i].down  | 0x20)) return DOWN;
        if ((key | 0x20) == (s->controls[i].left  | 0x20)) return LEFT;
        if ((key | 0x20) == (s->controls[i].right | 0x20)) return RIGHT;
    }
    return 0;
}

/*
 Движение головы с учетом текущего направления движения
 */
void go(struct snake_t *head)
{
    char ch = '@';
    int max_x = getmaxx(stdscr); // ширина терминала
    mvprintw(head->y, head->x, " "); // очищаем один символ
    switch (head->direction)
    {
        case LEFT:
            if (head->x <= 0) // Циклическое движение, чтобы не уходить за пределы экрана
                head->x = max_x;
            mvprintw(head->y, --(head->x), "%c", ch);
        break;
        case RIGHT:
            mvprintw(head->y, ++(head->x), "%c", ch);
        break;
        case UP:
            mvprintw(--(head->y), head->x, "%c", ch);
        break;
        case DOWN:
            mvprintw(++(head->y), head->x, "%c", ch);
        break;
        default:
        break;
    }
    refresh();
}

/*
 Проверка корректности выбранного направления.
 Змейка не может наступать на свой хвост, поэтому разворот на 180° запрещён:
   при движении RIGHT  нельзя LEFT,
   при движении UP     нельзя DOWN,
   при движении LEFT   нельзя RIGHT,
   при движении DOWN   нельзя UP.
 Возвращает 1, если направление допустимо, и 0, если это разворот.

 Противоположные направления отличаются на 2 (LEFT=1/RIGHT=3, UP=2/DOWN=4),
 поэтому проверка сводится к |dir - snake->direction| == 2.
 */
int checkDirection(snake_t* snake, int32_t key)
{
    int dir = keyToDirection(snake, key);

    if (dir == 0)                       // не команда движения — ничего не меняем
        return 1;
    if (abs(dir - snake->direction) == 2) // разворот на 180°
        return 0;

    return 1;
}

void changeDirection(struct snake_t* snake, const int32_t key)
{
    int dir = keyToDirection(snake, key);
    if (dir)
        snake->direction = dir;
}

/*
 Движение хвоста с учетом движения головы
 */
void goTail(struct snake_t *head)
{
    char ch = '*';
    mvprintw(head->tail[head->tsize-1].y, head->tail[head->tsize-1].x, " ");
    for (size_t i = head->tsize-1; i > 0; i--)
    {
        head->tail[i] = head->tail[i-1];
        if (head->tail[i].y || head->tail[i].x)
            mvprintw(head->tail[i].y, head->tail[i].x, "%c", ch);
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

int main()
{
    snake_t* snake = (snake_t*)malloc(sizeof(snake_t));
    initSnake(snake, START_TAIL_SIZE, 10, 10);
    initscr();
    keypad(stdscr, TRUE); // Включаем F1, F2, стрелки и т.д.
    raw();                // Откдючаем line buffering
    noecho();             // Отключаем echo() режим при вызове getch
    curs_set(FALSE);      // Отключаем курсор
    mvprintw(0, 0, "Use arrows or WASD (any case) for control. Press 'F10' for EXIT");
    timeout(0);           // Отключаем таймаут после нажатия клавиши в цикле
    int key_pressed = 0;
    while (key_pressed != STOP_GAME)
    {
        key_pressed = getch(); // Считываем клавишу
        go(snake);
        goTail(snake);
        timeout(100);          // Задержка при отрисовке
        if (checkDirection(snake, key_pressed)) // не даём развернуться на 180°
            changeDirection(snake, key_pressed);
    }
    endwin(); // Завершаем режим curses mod
    free(snake->tail);
    free(snake);
    return 0;
}
