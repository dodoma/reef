#include "reef.h"

struct _MDLIST {
    struct _MDLIST *prev;
    struct _MDLIST *next;

    void *data;
};

MDLIST* mdlist_new(void *data)
{
    if (!data) return NULL;

    MDLIST *zeta = mos_calloc(1, sizeof(MDLIST));
    zeta->prev = zeta->next = NULL;

    zeta->data = data;

    return zeta;
}

void* mdlist_data(MDLIST *alist)
{
    if (!alist) return NULL;

    return alist->data;
}

MDLIST* mdlist_head(MDLIST *alist)
{
    if (!alist) return NULL;

    while (alist->prev) alist = alist->prev;

    return alist;
}

MDLIST* mdlist_tail(MDLIST *alist)
{
    if (!alist) return NULL;

    while (alist->next) alist = alist->next;

    return alist;
}

MDLIST* mdlist_prev(MDLIST *alist)
{
    if (!alist) return NULL;

    return alist->prev;
}

MDLIST* mdlist_next(MDLIST *alist)
{
    if (!alist) return NULL;

    return alist->next;
}

bool mdlist_alone(MDLIST *alist)
{
    if (!alist) return false;

    if (alist->next || alist->prev) return false;

    return true;
}

size_t mdlist_length(MDLIST *alist)
{
    size_t len = 0;

    MDLIST *node = mdlist_head(alist);
    while (node) {
        node = node->next;
        len++;
    }

    return len;
}

MDLIST* mdlist_find(MDLIST *alist, void *key, int __F(compareitem)(MDLIST *blist, void *key))
{
    MDLIST *node = mdlist_head(alist);

    while (node) {
        if (compareitem(node, key) == 0) return node;
        node = node->next;
    }

    return NULL;
}

MDLIST* mdlist_insert(MDLIST *alist, MDLIST *blist)
{
    if (!alist) return NULL;
    if (!blist) return alist;

    MDLIST *next = alist->next;
    MDLIST *bhead = mdlist_head(blist);
    MDLIST *btail = mdlist_tail(blist);

    /* 连接 a,b */
    alist->next = bhead;
    bhead->prev = alist;

    if (next) {
        /* 接上 a 断掉部分 */
        next->prev = btail;
        btail->next = next;
    }

    return alist;
}

MDLIST* mdlist_concat(MDLIST *alist, MDLIST *blist)
{
    MDLIST *tail = mdlist_tail(alist);

    if (!tail) return blist;
    else {
        MDLIST *head = mdlist_head(blist);
        if (head) {
            head->prev = tail;
            tail->next = head;
        }

        return alist;
    }
}

MDLIST* mdlist_eject(MDLIST *alist, void __F(freeitem)(void*))
{
    if (!alist) return NULL;

    MDLIST *head = mdlist_head(alist);

    MDLIST *prev, *current, *next;

    prev = NULL;
    current = head;
    while (current) {
        if (current == alist) {
            /* 删除该节点 */
            next = current->next;

            if (prev != NULL) {
                prev->next = next;
            }
            if (next != NULL) {
                next->prev = prev;
            }

            if (current == head) head = next;

            if (freeitem) freeitem(current->data);
            mos_free(current);

            break;
        } else {
            /* 不用删除该节点 */
            prev = current;
            current = current->next;
        }
    }

    return head;
}

void mdlist_free(MDLIST *alist, void __F(freeitem)(void*))
{
    if (!alist) return;

    MDLIST *current, *next;

    current = next = alist;

    while (current) {
        next = current->next;

        if (freeitem) freeitem(current->data);
        mos_free(current);

        current = next;
    }
}
