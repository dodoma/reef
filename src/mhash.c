#include "reef.h"

struct node {
    uint32_t hashv;
    void *key;
    void *value;
    struct node *next;
};

struct _MHASH {
    uint32_t rownum;
    uint32_t num;

    struct node **nodes;

    uint32_t __F(hash_func)(const void*);
    int __F(comp_func)(const void *, const void *);
    void __F(destroy_func)(void *node);
};

/*
 * return node pointer's address, so, we can append to / remove it if we want
 */
static inline struct node** _lookup_node(MHASH *table, void *key)
{
    uint32_t hashv, row;
    struct node **node;

    hashv = table->hash_func(key);
    row = hashv & (table->rownum - 1);

    node = &(table->nodes[row]);
    while (*node && table->comp_func(key, (*node)->key)) node = &((*node)->next);

    return node;
}

static inline void _hash_resize(MHASH *table)
{
    struct node **new_nodes, *node, *prev, *next;
    size_t oldnum, moveto, oldmask, newmask;

    if (table->num < table->rownum) return;

    oldnum = table->rownum;
    table->rownum = oldnum << 1;
    oldmask = oldnum - 1;
    newmask = table->rownum - 1;

    new_nodes = mos_realloc(table->nodes, table->rownum * sizeof(struct node*));
    table->nodes = new_nodes;
    memset(table->nodes + oldnum, 0x0, oldnum * sizeof(struct node*));

    for (int i = 0; i < oldnum; i++) {
        prev = node = table->nodes[i];
        moveto = i + oldnum;

        while (node) {
            /* next is current node's next */
            next = node->next;

            if ((node->hashv & newmask) != i) {
                /* remove */
                if (prev == node) prev = table->nodes[i] = next;
                else prev->next = next;

                /* pre-pend */
                node->next = table->nodes[moveto];
                table->nodes[moveto] = node;
            } else {
                /* prev is oldrow's fix node confirmed */
                prev = node;
            }

            node = next;
        }
    }
}

static inline uint32_t _hash_fnv1a(const char *s, size_t len)
{
/*
 * 32 bit magic FNV-1a prime
 */
#define FNV_32_PRIME ((Fnv32_t)0x01000193)

    uint32_t hval = 0x811c9dc5;

    unsigned char *bp = (unsigned char *)s;
    unsigned char *be = (unsigned char *)s + len;

    while (bp < be) {
        hval ^= (uint32_t)*bp++;

#if defined(NO_FNV_GCC_OPTIMIZATION)
        hval *= FNV_32_PRIME;
#else
        hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
#endif
    }

    return hval;
}

MERR* mhash_init(MHASH **table, MHASH_HASH_FUNC hash_func,
                 MHASH_COMP_FUNC comp_func, MHASH_DESTROY_FUNC destroy_func)
{
    MERR_NOT_NULLC(table, hash_func, comp_func);

    MHASH *tbl = mos_calloc(1, sizeof(MHASH));

    tbl->rownum = 0xFF + 1;
    tbl->num = 0;
    tbl->hash_func = hash_func;
    tbl->comp_func = comp_func;
    tbl->destroy_func = destroy_func;

    tbl->nodes = mos_calloc(tbl->rownum, sizeof(struct node*));

    *table = tbl;

    return MERR_OK;
}

void mhash_destroy(MHASH **table)
{
    struct node *node, *next;
    MHASH *tbl;

    if (!table || !*table) return;

    tbl = *table;

    for (int x = 0; x < tbl->rownum; x++) {
        node = tbl->nodes[x];
        //mtc_dbg("****** row %d %x", x, node);

        while (node) {
            next = node->next;
            //mtc_dbg("destroy %s", (char*)node->key);

            if (tbl->destroy_func) tbl->destroy_func(node);
            mos_free(node);

            node = next;
        }
    }

    mos_free(tbl->nodes);
    mos_free(tbl);

    *table = NULL;
}

MERR* mhash_insert(MHASH *table, void *key, void *value)
{
    struct node **node;

    MERR_NOT_NULLB(table, key);

    node = _lookup_node(table, key);
    if (*node != NULL) {
        (*node)->value = value;
    } else {
        struct node *lnode = mos_calloc(1, sizeof(struct node));
        lnode->hashv = table->hash_func(key);
        lnode->key = key;
        lnode->value = value;
        lnode->next = NULL;

        *node = lnode;
        table->num++;
    }

    _hash_resize(table);

    return MERR_OK;
}

void* mhash_lookup(MHASH *table, void *key)
{
    struct node **node;

    if (!table || !key) return NULL;

    node = _lookup_node(table, key);

    if (*node) return (*node)->value;
    else return NULL;
}

bool mhash_has_key(MHASH *table, void *key)
{
    struct node **node;

    if (!table || !key) return false;

    node = _lookup_node(table, key);
    if (*node) return true;
    else return false;
}

bool mhash_remove(MHASH *table, void *key)
{
    struct node **node;

    if (!table || !key) return false;

    node = _lookup_node(table, key);
    if (*node) {
        struct node *lnode = *node;
        *node = lnode->next;

        if (table->destroy_func) table->destroy_func(lnode);
        mos_free(lnode);

        table->num--;
        return true;
    }

    return false;
}

void* mhash_next(MHASH *table, void **key)
{
    struct node **node;
    uint32_t row;

    if (!table || !key) return NULL;

    /*
     * return 1st node
     */
    if (*key == NULL) {
        row = 0;
        while (row < table->rownum) {
            if (table->nodes[row]) {
                *key = table->nodes[row]->key;
                return table->nodes[row]->value;
            }
            row++;
        }
    }

    /*
     * return next follow the *key
     */
    row = table->hash_func(*key) & (table->rownum - 1);
    node = _lookup_node(table, *key);
    if (node && *node) {
        if ((*node)->next) {
            *key = (*node)->next->key;
            return (*node)->next->value;
        }

        /* next row */
        row += 1;
        while (row < table->rownum) {
            if (table->nodes[row]) {
                *key = table->nodes[row]->key;
                return table->nodes[row]->value;
            }
            row++;
        }
    }

    return NULL;
}

uint32_t mhash_length(MHASH *table)
{
    if (!table) return 0;
    return table->num;
}

uint32_t mhash_str_hash(const void *a)
{
    const char *s = (const char*)a;

    return _hash_fnv1a(s, strlen(s));
}

int mhash_str_comp(const void *a, const void *b)
{
    const char *sa = (const char*)a;
    const char *sb = (const char*)b;

    if (*sa != *sb) return *sa - *sb;
    else return strcmp((const char*)a, (const char*)b);
}

void mhash_str_free(void *a)
{
    struct node *node = (struct node*)a;

    if(node) mos_free(node->key);
}

uint32_t mhash_int_hash(const void *a)
{
    return (uint32_t)(long)(a);
}

int mhash_int_comp(const void *a, const void *b)
{
    return a - b;
}
