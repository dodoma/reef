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
#define MERR_NOT_NULLD(pa, pb, pc, pd)                                  \
    if (!(pa) || !(pb) || !(pc) || (!pd)) return merr_raise(MERR_ASSERT, "paramter null");


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

#define TRACE_NOK_MT(err)                       \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        mtc_mt_err("%s", _moon_str.buf);        \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
    }

#define JUMP_NOK_MT(err, label)                 \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        mtc_mt_err("%s", _moon_str.buf);        \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
        goto label;                             \
    }

#define DIE_NOK(err)                            \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        fprintf(stderr, "%s\n", _moon_str.buf); \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
        exit(1);                                \
    }

#define PRINT_NOK(err)                          \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        fprintf(stderr, "%s\n", _moon_str.buf); \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
    }


__END_DECLS
#endif
