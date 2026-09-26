#include <stdio.h>

int main(void)
{
    unsigned int N;
    int K;

    if (scanf("%u %d", &N, &K) != 2)
        return 1;

    
    unsigned int mask = (1u << K) - 1;

    unsigned int best = 0;

    
    for (int p = 0; p <= 32 - K; ++p)
    {
        unsigned int win = (N >> p) & mask;
        if (win > best)
            best = win;
    }

    printf("%u\n", best);
    return 0;
}