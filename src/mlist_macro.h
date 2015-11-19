#ifndef __MLIST_MACRO_H__
#define __MLIST_MACRO_H__

/*
 * mlist_macro, macros for list
 */
__BEGIN_DECLS

#define MLIST_ITERATE(alist, item)                                      \
    item = alist->items[0];                                             \
    for (int _moon_i = 0; _moon_i < alist->num; item = alist->items[++_moon_i])

__END_DECLS
#endif
