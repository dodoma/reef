#ifndef __MTRACE_ERROR_H__
#define __MTRACE_ERROR_H__

/*
 * mtrace_error, macros for trace
 */
__BEGIN_DECLS

#define mtc_die(f,...)                                                  \
    do {                                                                \
        mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,MTC_DIE,f,##__VA_ARGS__); \
        exit(-1);                                                       \
    } while(0)

#define mtc_foo(f,...) \
    mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,MTC_FOO,f,##__VA_ARGS__)

#define mtc_err(f,...) \
    mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,MTC_ERROR,f,##__VA_ARGS__)

#define mtc_warn(f,...) \
    mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,MTC_WARNING,f,##__VA_ARGS__)

#define mtc_dbg(f,...)  \
    mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,MTC_DEBUG,f,##__VA_ARGS__)

#define mtc_info(f,...) \
    mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,MTC_INFO,f,##__VA_ARGS__)

#define mtc_noise(f,...) \
    mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,MTC_NOISE,f,##__VA_ARGS__)


__END_DECLS
#endif
