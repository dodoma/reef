#ifndef __MERR_MACRO_H__
#define __MERR_MACRO_H__

/*
 * merr_macro, macros for error handle
 */
__BEGIN_DECLS

#define MERR_NOT_NULLA(pa) \
    if (!(pa)) return merr_raise(MERR_ASSERT, "paramter null");
#define MERR_NOT_NULLB(pa, pb) \
    if (!(pa) || !(pb)) return merr_raise(MERR_ASSERT, "paramter null");
#define MERR_NOT_NULLC(pa, pb, pc) \
    if (!(pa) || !(pb) || !(pc)) return merr_raise(MERR_ASSERT, "paramter null");

__END_DECLS
#endif
