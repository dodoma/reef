#ifndef __MTRACE_MM_MACRO_H__
#define __MTRACE_MM_MACRO_H__

/*
 * mtrace_macro, macros for multi module trace
 */
__BEGIN_DECLS

MERR* mtc_mm_initf(const char *module, MTC_LEVEL level, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);

/*
 * not c++, we don't use __PRETTY_FUNCTION__
 */

#define mtc_mm_die(m,f,...)                                             \
    do {                                                                \
        mtc_mm_msg(__func__,__FILE__,__LINE__,MTC_DIE,m,f,##__VA_ARGS__); \
        exit(-1);                                                       \
    } while(0)

#define mtc_mm_foo(m,f,...)                                             \
    mtc_mm_msg(__func__,__FILE__,__LINE__,MTC_FOO,m,f,##__VA_ARGS__)

#define mtc_mm_err(m,f,...)                                             \
    mtc_mm_msg(__func__,__FILE__,__LINE__,MTC_ERROR,m,f,##__VA_ARGS__)

#define mtc_mm_warn(m,f,...)                                            \
    mtc_mm_msg(__func__,__FILE__,__LINE__,MTC_WARNING,m,f,##__VA_ARGS__)

#define mtc_mm_dbg(m,f,...)                                             \
    mtc_mm_msg(__func__,__FILE__,__LINE__,MTC_DEBUG,m,f,##__VA_ARGS__)

#define mtc_mm_info(m,f,...)                                            \
    mtc_mm_msg(__func__,__FILE__,__LINE__,MTC_INFO,m,f,##__VA_ARGS__)

#define mtc_mm_noise(m,f,...)                                           \
    mtc_mm_msg(__func__,__FILE__,__LINE__,MTC_NOISE,m,f,##__VA_ARGS__)


__END_DECLS
#endif
