#include "reef.h"
#include "_md5.h"
#include "_sha1.h"
#include "_sha256.h"

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
    void __F(destroy_func)(void *key, void *value);
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
    size_t oldnum, moveto, newmask;

    if (table->num < table->rownum) return;

    oldnum = table->rownum;
    table->rownum = oldnum << 1;
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

            if (tbl->destroy_func) tbl->destroy_func(node->key, node->value);
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

        if (table->destroy_func) table->destroy_func(lnode->key, lnode->value);
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
        return NULL;
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

void mhash_str_free(void *key, void *val)
{
    mos_free(key);
    mos_free(val);
}

uint32_t mhash_int_hash(const void *a)
{
    return (uint32_t)(long)(a);
}

int mhash_int_comp(const void *a, const void *b)
{
    int ia = *(int*)a;
    int ib = *(int*)b;
    return ia - ib;
}


void mhash_md5_buf(unsigned char *in, size_t len, unsigned char out[16])
{
    if (!in) return;

    md5_ctx contex;

    memset(out, 0x0, 16);

    MD5Init(&contex);
    MD5Update(&contex, in, (unsigned long)len);
    MD5Final(out, &contex);
}

void mhash_sha1_buf(unsigned char *in, size_t len, unsigned char out[20])
{
    SHA1_CTX contex;
    unsigned int ii;

    memset(out, 0x0, 20);

    SHA1Init(&contex);
    for (ii = 0; ii < len; ii++) {
        SHA1Update(&contex, in + ii, 1);
    }
    SHA1Final(out, &contex);
}

void mhash_sha256_buf(unsigned char *in, size_t len, unsigned char out[32])
{
    return sha256_hash(out, in, len);
}

/* from [aperezdc](https://github.com/aperezdc/hmac-sha256) */
#define B 64
#define L 32
#define K 64
#define I_PAD 0x36
#define O_PAD 0x5C
void mhash_hmac_sha256(unsigned char *data, size_t data_len, unsigned char *key, size_t key_len,
                       unsigned char out[32])
{
    sha256_t ss;
    uint8_t kh[32];

    /*
     * If the key length is bigger than the buffer size B, apply the hash
     * function to it first and use the result instead.
     */
    if (key_len > B) {
        sha256_hash(kh, key, key_len);
        key_len = 32;
        key = kh;
    }

    /*
     * (1) append zeros to the end of K to create a B byte string
     *     (e.g., if K is of length 20 bytes and B=64, then K will be
     *     appended with 44 zero bytes 0x00)
     * (2) XOR (bitwise exclusive-OR) the B byte string computed in step
     *     (1) with ipad
     */
    uint8_t kx[B];
    for (size_t i = 0; i < key_len; i++) kx[i] = I_PAD ^ key[i];
    for (size_t i = key_len; i < B; i++) kx[i] = I_PAD ^ 0;

    /*
     * (3) append the stream of data 'text' to the B byte string resulting
     *     from step (2)
     * (4) apply H to the stream generated in step (3)
     */
    sha256_init(&ss);
    sha256_update(&ss, kx, B);
    sha256_update(&ss, data, data_len);
    sha256_final(&ss, out);

    /*
     * (5) XOR (bitwise exclusive-OR) the B byte string computed in
     *     step (1) with opad
     *
     * NOTE: The "kx" variable is reused.
     */
    for (size_t i = 0; i < key_len; i++) kx[i] = O_PAD ^ key[i];
    for (size_t i = key_len; i < B; i++) kx[i] = O_PAD ^ 0;

    /*
     * (6) append the H result from step (4) to the B byte string
     *     resulting from step (5)
     * (7) apply H to the stream generated in step (6) and output
     *     the result
     */
    sha256_init(&ss);
    sha256_update(&ss, kx, B);
    sha256_update(&ss, out, 32);
    sha256_final(&ss, out);
}

void mhash_md5_init(MD5CTX *ctx)
{
    MD5Init(ctx);
}

void mhash_md5_update(MD5CTX *ctx, unsigned char *buf, size_t len)
{
    MD5Update(ctx, buf, (unsigned long)len);
}

void mhash_md5_final(unsigned char checksum[16], MD5CTX *ctx)
{
    MD5Final(checksum, ctx);
}
