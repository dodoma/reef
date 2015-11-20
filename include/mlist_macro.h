#ifndef __MLIST_MACRO_H__
#define __MLIST_MACRO_H__

/*
 * mlist_macro, macros for list
 */
__BEGIN_DECLS

#define MLIST_ITERATE(alist, item)                      \
    mlist_get(alist, 0, (void**)&(item));               \
    for (int _moon_i = 0;                               \
         _moon_i < mlist_length(alist);                 \
         mlist_get(alist, ++_moon_i, (void**)&(item)))

__END_DECLS
#endif
