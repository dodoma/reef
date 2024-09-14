#ifndef __MERR_MACRO_H__
#define __MERR_MACRO_H__

/*
 * merr_macro, macros for error handle
 */
__BEGIN_DECLS

#define MERR_NOT_NULLA(pa)                                      \
    if (!(pa)) return merr_raise(MERR_ASSERT, "paramter null");
#define MERR_NOT_NULLB(pa, pb)                                          \
    if (!(pa) || !(pb)) return merr_raise(MERR_ASSERT, "paramter null");
#define MERR_NOT_NULLC(pa, pb, pc)                                      \
    if (!(pa) || !(pb) || !(pc)) return merr_raise(MERR_ASSERT, "paramter null");
#define MERR_NOT_NULLD(pa, pb, pc, pd)                                  \
    if (!(pa) || !(pb) || !(pc) || (!pd)) return merr_raise(MERR_ASSERT, "paramter null");

#ifdef ANDROID
#define MERR_LOG printf
#define MERR_LOG_MM printf
#define MERR_LOG_MT printf
#else
#define MERR_LOG mtc_err
#define MERR_LOG_MM mtc_mm_err
#define MERR_LOG_MT mtc_mt_err
#endif


#define TRACE_NOK(err)                          \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        MERR_LOG("%s", _moon_str.buf);          \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
    }

#define JUMP_NOK(err, label)                    \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        MERR_LOG("%s", _moon_str.buf);          \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
        goto label;                             \
    }

#define TRACE_NOK_MT(err)                       \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        MERR_LOG_MT("%s", _moon_str.buf);          \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
    }

#define TRACE_NOK_MM(err, module)               \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        MERR_LOG_MM("%s", _moon_str.buf);       \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
    }

#define JUMP_NOK_MT(err, label)                 \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        MERR_LOG_MT("%s", _moon_str.buf);          \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
        goto label;                             \
    }

#define JUMP_NOK_MM(err, label, module)         \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        MERR_LOG_MM("%s", _moon_str.buf);          \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
        goto label;                             \
    }

#define RETURN_NOK(err)                         \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        MERR_LOG_MT("%s", _moon_str.buf);          \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
        return;                                 \
    }

#define RETURN_NOK_MM(err, module)              \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        MERR_LOG_MM("%s", _moon_str.buf);          \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
        return;                                 \
    }

#define RETURN_V_NOK(err, v)                    \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        MERR_LOG_MT("%s", _moon_str.buf);          \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
        return (v);                             \
    }

#define RETURN_V_NOK_MM(err, module, v)         \
    if (err != MERR_OK) {                       \
        MSTR _moon_str;                         \
        mstr_init(&_moon_str);                  \
        merr_traceback(err, &_moon_str);        \
        MERR_LOG_MM("%s", _moon_str.buf);          \
        mstr_clear(&_moon_str);                 \
        merr_destroy(&err);                     \
        return (v);                             \
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
