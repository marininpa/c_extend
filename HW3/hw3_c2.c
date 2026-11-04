/*
 * Задание 3 (*): переписать timeout через функцию clock() и добавить
 * профилирование кода. Плюс функции из заданий 1 и 2:
 * столкновение с собой и выход за границы экрана.
 *
 * База — 14-19_snake.c. Изменено:
 *   - getch() вызывается в неблокирующем режиме (nodelay), а темп игры задаёт
 *     САМ цикл: кадр отрисовывается, когда от прошлого кадра прошло >= 100 мс
 *     процессорного времени по clock(). timeout(100) из ncurses убран;
 *   - в каждом кадре замеряется время go() и goTail(); по выходу из игры
 *     печатается профиль (число кадров и суммарное/среднее время);
 *   - selfCollision() — столкновение головы с телом (из задания 1);
 *   - outsideField() — выход за границы поля (из задания 2).
 *
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
 * Точка (x,y) вне поля? Верхняя доступная строка — MIN_Y
 * (строка 0 занята подсказкой).
 */
int outsideField(int x, int y, int max_x, int max_y)
{
    return (x < 0) || (x >= max_x) || (y < MIN_Y) || (y >= max_y);
}

/*
 * Движение головы с учётом направления. Новая позиция проверяется на границы
 * и на столкновение с телом; при нарушении игра завершается
 * (over = OVER_WALL / OVER_SELF).
 */
void go(struct snake_t *head, int *over)
{
    int max_x = getmaxx(stdscr); // ширина терминала
    int max_y = getmaxy(stdscr); // высота терминала

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
        *over = OVER_WALL;
        return;
    }
    if (selfCollision(head, nx, ny))
    {
        *over = OVER_SELF;
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

/* Профиль: сколько кадров отрисовано и сколько времени ушло на go/goTail */
typedef struct profile_t
{
    unsigned long frames;
    double time_go;
    double time_tail;
} profile_t;

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

    // Не блокируемся на getch: темп задаёт сам цикл по clock()
    nodelay(stdscr, TRUE);

    const clock_t frame_ticks = 100 * CLOCKS_PER_SEC / 1000; // 100 мс в тиках clock
    clock_t last_frame = clock();

    profile_t prof = {0, 0.0, 0.0};

    int key_pressed=0;
    int over = OVER_NONE;
    while( key_pressed != STOP_GAME && over == OVER_NONE )
    {
        key_pressed = getch(); // Считываем клавишу (не блокирует)
        changeDirection(snake, key_pressed);

        clock_t now = clock();
        if (now - last_frame >= frame_ticks) // наступил момент нового кадра
        {
            last_frame = now;

            clock_t t0 = clock();
            go(snake, &over);
            clock_t t1 = clock();
            if (over == OVER_NONE)
                goTail(snake);
            clock_t t2 = clock();

            prof.time_go   += (double)(t1 - t0) / CLOCKS_PER_SEC;
            prof.time_tail += (double)(t2 - t1) / CLOCKS_PER_SEC;
            prof.frames++;
        }
    }
    endwin(); // Завершаем режим curses mod

    if (over == OVER_WALL)
        printf("Game over: snake hit the wall\n");
    else if (over == OVER_SELF)
        printf("Game over: snake ran into itself\n");

    double total = prof.time_go + prof.time_tail;
    printf("Profiling: frames=%lu  go=%.6f s  goTail=%.6f s  total=%.6f s  avg=%.6f s/frame\n",
           prof.frames, prof.time_go, prof.time_tail, total,
           prof.frames ? total / (double)prof.frames : 0.0);

    free(snake->tail);
    free(snake);
    return 0;
}
