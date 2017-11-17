#include "reef.h"

struct _MLIST {
    int num;
    int max;
    bool sorted;
    void __F(free)(void *node);
    void **items;
};

static inline void _check_length(MLIST *alist, int size)
{
    if (size > alist->max) {
        void **items;
        int newsize;

        newsize = alist->max * 2;
        if (size > newsize) newsize = size + alist->max;

        items = mos_realloc(alist->items, newsize * sizeof(void*));
        alist->items = items;
        alist->max = newsize;
    }
}

MERR* mlist_init(MLIST **alist, void __F(freeitem)(void*))
{
    int size;
    MLIST *rl;

    MERR_NOT_NULLA(alist);

    size = 256;

    rl = mos_calloc(1, sizeof(MLIST));

    rl->items = mos_calloc(size, sizeof(void*));
    rl->num = 0;
    rl->max = size;
    rl->sorted = true;

    rl->free = freeitem;

    *alist = rl;

    return MERR_OK;
}

MERR* mlist_append(MLIST *alist, void *data)
{
    MERR_NOT_NULLA(alist);

    _check_length(alist, alist->num + 1);

    alist->items[alist->num] = data;
    alist->num++;

    alist->sorted = false;

    return MERR_OK;
}

MERR* mlist_pop(MLIST *alist, void **data)
{
    MERR_NOT_NULLB(alist, data);

    if (alist->num <= 0) return merr_raise(MERR_EMPTY, "alist empty");

    *data = alist->items[alist->num - 1];
    alist->num--;

    return MERR_OK;
}

MERR* mlist_insert(MLIST *alist, int x, void *data)
{
    void **start;

    MERR_NOT_NULLB(alist, data);

    if (x < 0 || x >= alist->num)
        return merr_raise(MERR_ASSERT, "%d out of range [0 ~ %d)", x, alist->num);

    _check_length(alist, alist->num + 1);

    start = &(alist->items[x]);

    memmove(start + 1, start, sizeof(void*) * (alist->num - x));
    alist->items[x] = data;
    alist->num++;

    alist->sorted = false;

    return MERR_OK;
}

MERR* mlist_delete(MLIST *alist, int x)
{
    void **start;

    MERR_NOT_NULLA(alist);

    if (x < 0 || x >= alist->num)
        return merr_raise(MERR_ASSERT, "%d out of range [0 ~ %d)", x, alist->num);

    if (alist->free) alist->free(alist->items[x]);
    start = &(alist->items[x]);
    memmove(start, start + 1, sizeof(void*) * (alist->num - x - 1));

    alist->num--;

    return MERR_OK;
}


MERR* mlist_get(MLIST *alist, int x, void **data)
{
    MERR_NOT_NULLB(alist, data);

    if (x < 0 || x >= alist->num)
        return merr_raise(MERR_ASSERT, "%d out of range [0 ~ %d)", x, alist->num);

    *data = alist->items[x];

    return MERR_OK;
}

MERR* mlist_set(MLIST *alist, int x, void *data)
{
    MERR_NOT_NULLB(alist, data);

    if (x < 0 || x >= alist->num)
        return merr_raise(MERR_ASSERT, "%d out of range [0 ~ %d)", x, alist->num);

    alist->items[x] = data;

    alist->sorted = false;

    return MERR_OK;
}

MERR* mlist_cut(MLIST *alist, int x, void **data)
{
    MERR_NOT_NULLB(alist, data);

    if (x < 0 || x >= alist->num)
        return merr_raise(MERR_ASSERT, "%d out of range [0 ~ %d)", x, alist->num);

    *data = alist->items[x];

    return merr_pass(mlist_delete(alist, x));
}

MERR* mlist_reverse(MLIST *alist)
{
    for (int i = 0; i < alist->num / 2; i++) {
        void *item = alist->items[i];

        alist->items[i] = alist->items[alist->num-1-i];
        alist->items[alist->num-1-i] = item;
    }

    if (alist) alist->sorted = false;

    return MERR_OK;
}

int mlist_length(MLIST *alist)
{

    if (!alist) return 0;

    return alist->num;
}


void* mlist_in(MLIST *alist, const void *key,
               int __F(compare)(const void*, const void*))
{
    MERR_NOT_NULLC(alist, key, compare);

    for (int i = 0; i < alist->num; i++) {
        if (!compare(key, &alist->items[i])) return &alist->items[i];
    }

    return NULL;
}

int mlist_index(MLIST *alist, const void *key,
                  int __F(compare)(const void*, const void*))
{
    if (!alist || !key || !compare) return -1;

    for (int i = 0; i < alist->num; i++) {
        if (!compare(key, &alist->items[i])) return i;
    }

    return -1;
}

void mlist_clear(MLIST *alist)
{
    if (!alist) return;

    if (alist->free) {
        for (int i = 0; i < alist->num; i++) {
            alist->free(alist->items[i]);
        }
    }

    alist->num = 0;
    alist->sorted = true;
}

void mlist_destroy(MLIST **alist)
{
    if (!alist || !*alist) return;

    MLIST *o = *alist;

    if (o->free) {
        for (int i = 0; i < o->num; i++) {
            o->free(o->items[i]);
        }
    }

    mos_free(o->items);
    mos_free(o);

    *alist = NULL;
}

void mlist_free(void *alist)
{
    if (!alist) return;

    MLIST *o = alist;

    if (o->free) {
        for (int i = 0; i < o->num; i++) {
            o->free(o->items[i]);
        }
    }

    mos_free(o->items);
    mos_free(o);
}


void mlist_sort(MLIST *alist, int __F(compare)(const void *, const void*))
{
    if (!alist || !compare) return;

    qsort(alist->items, alist->num, sizeof(void*), compare);
    alist->sorted = true;
}

void* mlist_search(MLIST *alist, const void *key,
                   int __F(compare)(const void*, const void*))
{
    if (!alist || !key || !compare) return NULL;

    if (!alist->sorted) {
        qsort(alist->items, alist->num, sizeof(void*), compare);
        alist->sorted = true;
    }

    return bsearch(key, alist->items, alist->num, sizeof(void*), compare);
}
