#ifndef __MTRACE_MT_MACRO_H__
#define __MTRACE_MT_MACRO_H__

/*
 * mtrace_macro, macros for multi thread trace
 */
__BEGIN_DECLS

MERR* mtc_mt_initf(const char *modulename, MTC_LEVEL level, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);

/*
 * not c++, we don't use __PRETTY_FUNCTION__
 */

#define mtc_mt_die(f,...)                                               \
    do {                                                                \
        mtc_mt_msg(__func__,__FILE__,__LINE__,MTC_DIE,f,##__VA_ARGS__); \
        exit(-1);                                                       \
    } while(0)

#define mtc_mt_foo(f,...) \
    mtc_mt_msg(__func__,__FILE__,__LINE__,MTC_FOO,f,##__VA_ARGS__)

#define mtc_mt_err(f,...) \
    mtc_mt_msg(__func__,__FILE__,__LINE__,MTC_ERROR,f,##__VA_ARGS__)

#define mtc_mt_warn(f,...) \
    mtc_mt_msg(__func__,__FILE__,__LINE__,MTC_WARNING,f,##__VA_ARGS__)

#define mtc_mt_dbg(f,...)  \
    mtc_mt_msg(__func__,__FILE__,__LINE__,MTC_DEBUG,f,##__VA_ARGS__)

#define mtc_mt_info(f,...) \
    mtc_mt_msg(__func__,__FILE__,__LINE__,MTC_INFO,f,##__VA_ARGS__)

#define mtc_mt_noise(f,...) \
    mtc_mt_msg(__func__,__FILE__,__LINE__,MTC_NOISE,f,##__VA_ARGS__)


__END_DECLS
#endif
