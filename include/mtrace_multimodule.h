#ifndef __MTRACE_MM_H__
#define __MTRACE_MM_H__

/*
 * mtrace multi module
 * 多模块使用场景时，为每个模块打开了一个单独的日志文件，
 * 模块内可以使用 mtc_mm_xxx 安全的输出日志
 */
__BEGIN_DECLS

MERR* mtc_mm_init(const char *fn, const char *module, MTC_LEVEL level);
void  mtc_mm_set_level(const char *module, MTC_LEVEL level);
bool  mtc_mm_msg(const char *func, const char *file, long line, MTC_LEVEL level,
                 const char *module, const char *fmt, ...) ATTRIBUTE_PRINTF(6, 7);
FILE* mtc_mm_file(const char *module);

__END_DECLS
#endif
