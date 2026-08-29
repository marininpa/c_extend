#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint32_t N;
    int K;

    scanf("%u %d", &N, &K);

    uint32_t result = (N >> K) | (N << (32 - K));

    printf("%u\n", result);

    return 0;
}