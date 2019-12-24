#ifndef __MTREE_H__
#define __MTREE_H__

/*
 * mtree, self balancing binary search tree(stolen from https://github.com/ccxvii/minilibs)
 */
__BEGIN_DECLS

MTREE* mtree_insert(MTREE *node, const char *key, void *value);
void*  mtree_lookup(MTREE *node, const char *key);
void   mtree_free(MTREE *node);
void   mtree_print(MTREE *node, int level);

__END_DECLS
#endif
