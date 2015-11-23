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


#define TRACE_NOK(err)                          \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        mtc_err("%s", _moon_str.buf);           \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
    }

#define JUMP_NOK(err, label)                    \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        mtc_err("%s", _moon_str.buf);           \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
        goto label;                             \
    }


__END_DECLS
#endif
