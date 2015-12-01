#ifndef __MLIST_MACRO_H__
#define __MLIST_MACRO_H__

/*
 * mlist_macro, macros for list
 */
__BEGIN_DECLS

#define MLIST_ITERATE(alist, item)                                      \
    for (int _moon_i = 0;                                               \
         _moon_i < mlist_length(alist) &&                               \
             (mlist_get(alist, _moon_i, (void**)&(item)) == MERR_OK);   \
         _moon_i++)

__END_DECLS
#endif
