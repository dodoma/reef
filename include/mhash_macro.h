#ifndef __MHASH_MACRO_H__
#define __MHASH_MACRO_H__

/*
 * mhash_macro, macros for hash
 */
__BEGIN_DECLS

/*
 * 遍历 hash table
 *
 * 注意：
 *  1. 通过判断 返回 hash 节点的 value 指针是否为空来结束循环，
 *     故，此宏对插入节点的 value 有空的情况不适用
 */
#define MHASH_ITERATE(table, key, value)            \
    key = NULL;                                     \
    for (value = mhash_next(table, (void**)&key);   \
         value != NULL;                             \
         value = mhash_next(table, (void**)&key))   \


__END_DECLS
#endif
