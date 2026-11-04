/*
 * Задание 1: змея врезается сама в себя.
 *
 * База — 14-19_snake.c. Добавлено:
 *   - функция selfCollision() — проверка, попадает ли новая клетка головы
 *     в тело змейки;
 *   - в go() новая позиция сначала вычисляется, проверяется, и только потом
 *     применяется; при попадании в тело игра завершается;
 *   - хвост в initSnake() инициализируется координатами головы (в исходнике
 *     хвост был забит {0,0}, что давало ложные совпадения в точке (0,0)).
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

// Здесь храним коды управления змейкой
struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
}control_buttons;

struct control_buttons default_controls = {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT};

/*
 Голова змейки содержит в себе
 x,y - координаты текущей позиции
 direction - направление движения
 tsize - размер хвоста
 *tail -  ссылка на хвост
 */

typedef struct snake_t
{
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail_t *tail;
    struct control_buttons controls;
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

    // Тело стартует в точке головы, а не в {0,0}.
    // Иначе проверка столкновения ловила бы «фантомные» сегменты в (0,0).
    for (size_t i = 0; i < head->tsize; ++i)
    {
        tail[i].x = x;
        tail[i].y = y;
    }
}

/*
 * Новая клетка (x,y) не должна совпадать с телом змейки.
 * Последний сегмент (индекс tsize-1) не учитываем: на этом же шаге он
 * сдвинется на место предыдущего и освободит свою клетку, поэтому заход
 * в него — не столкновение (иначе хвост «кусал» бы голову на каждом шаге).
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
 * Движение головы с учётом текущего направления.
 * Сначала считаем новую позицию, проверяем столкновение, и только затем
 * двигаем голову. gameover выставляется, если змея врезалась в себя.
 */
void go(struct snake_t *head, int *gameover)
{
    int max_x = getmaxx(stdscr); // ширина терминала

    int nx = head->x;
    int ny = head->y;

    switch (head->direction)
    {
        case LEFT:
            --nx;
            if (nx < 0) // выход за левый край — по-прежнему зацикливаем
                nx = max_x - 1;
        break;
        case RIGHT: ++nx; break;
        case UP:    --ny; break;
        case DOWN:  ++ny; break;
        default: return;
    }

    if (selfCollision(head, nx, ny))
    {
        *gameover = 1;
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
    int gameover = 0;
    while( key_pressed != STOP_GAME && !gameover )
    {
        key_pressed = getch(); // Считываем клавишу
        go(snake, &gameover);
        if (!gameover)
            goTail(snake);
        timeout(100); // Задержка при отрисовке
        changeDirection(snake, key_pressed);
    }
    endwin(); // Завершаем режим curses mod
    if (gameover)
        printf("Game over: snake ran into itself\n");
    free(snake->tail);
    free(snake);
    return 0;
}
