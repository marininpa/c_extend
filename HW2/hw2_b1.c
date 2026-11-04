#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

typedef struct list {
    void *address;
    size_t size;
    char comment[64];
    struct list *next;
} list;

/* Суммарный размер всех блоков списка. Пустой список -> 0. */
size_t totalMemoryUsage(list *head)
{
    size_t total = 0;

    for (list *p = head; p != NULL; p = p->next)
        total += p->size;

    return total;
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

        uint64_t addr;
        if (scanf("%" SCNu64 " %zu", &addr, &node->size) != 2)
        {
            free(node);
            return 1;
        }
        node->address = (void *)(uintptr_t)addr;

        if (head == NULL)
            head = node;
        else
            tail->next = node;
        tail = node;
    }

    printf("%zu\n", totalMemoryUsage(head));

    while (head != NULL)
    {
        list *tmp = head->next;
        free(head);
        head = tmp;
    }
    return 0;
}
