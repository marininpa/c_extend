#include <stdio.h>

int main(void)
{
    int N;
    scanf("%d", &N);

    int max = 0;
    int count = 0;

    for (int i = 0; i < N; i++)
    {
        int x;
        scanf("%d", &x);

        if (i == 0)          /* первое число — текущий максимум */
        {
            max = x;
            count = 1;
        }
        else if (x > max)    /* нашли новый максимум — счётчик сбрасываем */
        {
            max = x;
            count = 1;
        }
        else if (x == max)   /* ещё одно вхождение текущего максимума */
        {
            count++;
        }
    }

    printf("%d\n", count);
    return 0;
}