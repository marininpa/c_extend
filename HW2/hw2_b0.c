#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

typedef struct list {
    uint64_t address;
    size_t size;
    char comment[64];
    struct list *next;
} list;

/* Возвращает адрес блока с максимальным size.
   Пустой список -> 0. При нескольких максимумах годится любой. */
uint64_t findMaxBlock(list *head)
{
    if (head == NULL)
        return 0;

    uint64_t max_address = head->address;
    size_t   max_size    = head->size;

    for (list *p = head->next; p != NULL; p = p->next)
    {
        if (p->size > max_size)
        {
            max_size    = p->size;
            max_address = p->address;
        }
    }
    return max_address;
}

int main(void)
{
    int n;
    if (scanf("%d", &n) != 1)
        return 1;

    list *head = NULL, *tail = NULL;
    for (int i = 0; i < n; ++i)
    {
        list *node = malloc(sizeof(list));
        if (node == NULL)
            return 1;

        node->comment[0] = '\0';
        node->next = NULL;

        if (scanf("%" SCNu64 " %zu", &node->address, &node->size) != 2)
        {
            free(node);
            return 1;
        }

        if (head == NULL)
            head = node;
        else
            tail->next = node;
        tail = node;
    }

    printf("%" PRIu64 "\n", findMaxBlock(head));

    while (head != NULL)
    {
        list *tmp = head->next;
        free(head);
        head = tmp;
    }
    return 0;
}
