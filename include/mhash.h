#ifndef __MHASH_H__
#define __MHASH_H__

/*
 * mhash, hash table data type
 */
__BEGIN_DECLS

MERR* mhash_init(MHASH **table, MHASH_HASH_FUNC hash_func,
                 MHASH_COMP_FUNC comp_func, MHASH_DESTROY_FUNC destroy_func);
void  mhash_destroy(MHASH **table);
MERR* mhash_insert(MHASH *table, void *key, void *value);
void* mhash_lookup(MHASH *table, void *key);
bool  mhash_has_key(MHASH *table, void *key);
bool  mhash_remove(MHASH *table, void *key);
void* mhash_next(MHASH *table, void **key);

uint32_t mhash_length(MHASH *table);

uint32_t mhash_str_hash(const void *a);
int      mhash_str_comp(const void *a, const void *b);
void     mhash_str_free(void *a);

uint32_t mhash_int_hash(const void *a);
int      mhash_int_comp(const void *a, const void *b);

__END_DECLS
#endif
