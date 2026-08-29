#include <stdio.h>

int main(void)
{
    int N;
    scanf("%d", &N);

    int ch;
    /* Пропускаем пробелы/переводы строки между N и сообщением.
       Сам разделитель не входит в сообщение. */
    while ((ch = getchar()) == ' ' || ch == '\t' || ch == '\n') {}

    while (ch != '.')   /* читаем символы сообщения до точки */
    {
        if (ch >= 'a' && ch <= 'z')                  /* строчная буква */
        {
            putchar('a' + ((ch - 'a' + N) % 26));
        }
        else if (ch >= 'A' && ch <= 'Z')             /* заглавная буква */
        {
            putchar('A' + ((ch - 'A' + N) % 26));
        }
        else                                         /* пробел и прочее */
        {
            putchar(ch);
        }
        ch = getchar();
    }

    putchar('.');   /* выводим завершающую точку */
    return 0;
}