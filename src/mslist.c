#include "reef.h"

struct _MSLIST {
    void *data;

    struct _MSLIST *next;
};

MSLIST* mslist_add(MSLIST *alist, void *data)
{
    MSLIST *node = mos_calloc(1, sizeof(MSLIST));
    node->data = data;
    node->next = alist;

    return node;
}

MSLIST* mslist_join(MSLIST *alist, MSLIST *blist)
{
    MSLIST *node = alist;
    while (node) {
        if (node->next == NULL) {
            node->next = blist;
            break;
        }

        node = node->next;
    }

    return alist;
}

uint32_t mslist_length(MSLIST *alist)
{
    uint32_t count = 0;

    MSLIST *node = alist;
    while (node) {
        count++;
        node = node->next;
    }

    return count;
}

MSLIST* mslist_search(MSLIST *alist, void *key, int (*compare)(void *data, void *key))
{
    if (!alist || !key || !compare) return NULL;

    MSLIST *node = alist;
    while (node) {
        if (compare(node->data, key) == 0) return node;

        node = node->next;
    }

    return NULL;
}

MSLIST* mslist_remove(MSLIST *alist, void *key,
                      int (*compare)(void *data, void *key),
                      void (*freeitem)(void *data))
{
    MSLIST *node = alist, *prev = NULL, *next = NULL;

    while (node) {
        next = node->next;

        if ((compare)(node->data, key) == 0) {
            if (prev) prev->next = next;
            else alist = next;

            if (freeitem) freeitem(node->data);
            free(node);
        } else prev = node;

        node = next;
    }

    return alist;
}

void mslist_iterate(MSLIST *alist, bool (*action)(void *data))
{
    MSLIST *node = alist;

    while (node) {
        if (!action(node->data)) break;

        node = node->next;
    }
}

void mslist_free(MSLIST *alist, void (*freeitem)(void *data))
{

    MSLIST *node = alist, *next = NULL;

    while (node) {
        next = node->next;

        if (freeitem) freeitem(node->data);
        free(node);

        node = next;
    }
}
