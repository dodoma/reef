#ifndef __MTRACE_MT_H__
#define __MTRACE_MT_H__

/*
 * mtrace multi thread
 * 多线程使用场景时，该模块为每个线程打开了一个单独的日志文件，
 * 线程内可以使用 mtc_mt_xxx 安全的输出日志
 */
__BEGIN_DECLS

FILE* mtc_mt_file();
MERR* mtc_mt_init(const char *fn, const char *modulename, MTC_LEVEL level);
void  mtc_mt_set_level(MTC_LEVEL level);
bool  mtc_mt_msg(const char *func, const char *file, long line, MTC_LEVEL level,
                 const char *fmt, ...) ATTRIBUTE_PRINTF(5, 6);

__END_DECLS
#endif
