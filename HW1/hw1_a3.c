#include <stdio.h>

int main(void)
{
    unsigned int N;
    int K;

    scanf("%u %d", &N, &K);

    unsigned int mask = (1u << K) - 1u;   /* K младших бит = 1, остальные 0 */
    unsigned int result = N & mask;       /* оставляем только эти биты */

    printf("%u\n", result);

    return 0;
}