#ifndef __MLIST_MACRO_H__
#define __MLIST_MACRO_H__

/*
 * mlist_macro, macros for list
 */
__BEGIN_DECLS

/*
 * QJYQ(奇技淫巧) 如果在循环列表的过程中想删掉该元素，只需在循环最后加上 _moon_i--;
 */
#define MLIST_ITERATE(alist, item)                                      \
    for (int _moon_i = 0;                                               \
         _moon_i < mlist_length(alist) &&                               \
             (mlist_get(alist, _moon_i, (void**)&(item)) == MERR_OK);   \
         _moon_i++)

#define MLIST_ITERATEB(alist, item)                                     \
    for (int _moon_j = 0;                                               \
         _moon_j < mlist_length(alist) &&                               \
             (mlist_get(alist, _moon_j, (void**)&(item)) == MERR_OK);   \
         _moon_j++)

__END_DECLS
#endif
