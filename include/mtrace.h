#ifndef __MTRACE_H__
#define __MTRACE_H__

/*
 * mtrace, trace module (debug, warnning, error...)
 */
__BEGIN_DECLS

/* 默认日志级别: debug */
#define MTC_DEFAULT_LEVEL    5

/* 单个日志文件最大: 500M */
#define MTC_MAX_FILE_SIZE  524288000

/* 最多保留日志文件: 5个 */
#define MTC_MAX_FILE_NUM   5

typedef enum {
    MTC_DIE = 0,                /* 0 */
    MTC_FOO,                    /* 1 */
    MTC_ERROR,                  /* 2 */
    MTC_WARNING,                /* 3 */
    MTC_INFO,                   /* 4 */
    MTC_DEBUG,                  /* 5 */
    MTC_NOISE,                  /* 6 */
    MTC_MAX                     /* 7 */
} MTC_LEVEL;

int  mtc_init(const char *fn, MTC_LEVEL level);
void mtc_set_level(MTC_LEVEL level);
bool mtc_msg(const char *func, const char *file, long line, MTC_LEVEL level,
             const char *fmt, ...) ATTRIBUTE_PRINTF(5, 6);

__END_DECLS
#endif
