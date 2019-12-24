#ifndef __MUTREE_H__
#define __MUTREE_H__

/*
 * mutree, mtree's uint64_t key version
 */
__BEGIN_DECLS

MUTREE* mutree_insert(MUTREE *node, uint64_t key, void *value);
void*   mutree_lookup(MUTREE *node, uint64_t key);
void    mutree_free(MUTREE *node);
void    mutree_print(MUTREE *node, int level);

__END_DECLS
#endif
