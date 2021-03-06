#ifndef __MTRACE_MACRO_H__
#define __MTRACE_MACRO_H__

/*
 * mtrace_macro, macros for trace
 */
__BEGIN_DECLS

MERR* mtc_initf(MTC_LEVEL level, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);


/*
 * not c++, we don't use __PRETTY_FUNCTION__
 */

#define mtc_die(f,...)                                                  \
    do {                                                                \
        mtc_msg(__func__,__FILE__,__LINE__,MTC_DIE,f,##__VA_ARGS__);    \
        exit(-1);                                                       \
    } while(0)

#define mtc_foo(f,...) \
    mtc_msg(__func__,__FILE__,__LINE__,MTC_FOO,f,##__VA_ARGS__)

#define mtc_err(f,...) \
    mtc_msg(__func__,__FILE__,__LINE__,MTC_ERROR,f,##__VA_ARGS__)

#define mtc_warn(f,...) \
    mtc_msg(__func__,__FILE__,__LINE__,MTC_WARNING,f,##__VA_ARGS__)

#define mtc_dbg(f,...)  \
    mtc_msg(__func__,__FILE__,__LINE__,MTC_DEBUG,f,##__VA_ARGS__)

#define mtc_info(f,...) \
    mtc_msg(__func__,__FILE__,__LINE__,MTC_INFO,f,##__VA_ARGS__)

#define mtc_noise(f,...) \
    mtc_msg(__func__,__FILE__,__LINE__,MTC_NOISE,f,##__VA_ARGS__)


__END_DECLS
#endif
