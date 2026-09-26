#include <stdio.h>

int main(void)
{
    unsigned int N;

    if (scanf("%u", &N) != 1)
        return 1;

    unsigned int count = 0;


    while (N != 0)
    {
        N &= N - 1;
        ++count;
    }

    printf("%u\n", count);
    return 0;
}