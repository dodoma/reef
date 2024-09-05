#include "reef.h"

#if defined(MOS_LINUX) || defined(MOS_OSX)

/* 默认日志级别: debug */
#define MTC_DEFAULT_LEVEL    5

/* 单个日志文件最大: 500M */
#define MTC_MAX_FILE_SIZE  524288000

/* 最多保留日志文件: 5个 */
#define MTC_MAX_FILE_NUM   5

static pthread_mutex_t m_lock = PTHREAD_MUTEX_INITIALIZER;

static char *m_levels[MTC_MAX] = {"DIE", "MESSAGE", "ERROR",
                                  "WARNING", "INFO", "DEBUG",
                                  "NOISE"};

struct _entry {
    uint32_t tid;
    int level;
    char modulename[256];
    char filename[PATH_MAX-32];
    char linebuf[2096];
    FILE *fp;
};

static struct _entry **m_logs = NULL;
static int m_num = 0;

static inline uint32_t _get_tid()
{
#if defined(MOS_LINUX)
    return syscall(__NR_gettid);
#elif defined(MOS_OSX) || defined(MOS_ESP)
    uint64_t id;
    pthread_threadid_np(NULL, &id);
    return id;
#else
#error "unsupport platform"
#endif
}

static int _entry_compare(const void *a, const void *b)
{
    struct _entry *sa, *sb;

    sa = *(struct _entry**)a;
    sb = *(struct _entry**)b;

    return sa->tid - sb->tid;
}

static void _entry_sort()
{
    if (!m_logs) return;

    qsort(m_logs, m_num, sizeof(void*), _entry_compare);
}

static struct _entry* _entry_search(uint32_t tid)
{
    if (!m_logs) return NULL;

    struct _entry le = {.tid = tid, .fp = NULL};
    struct _entry *le_addr = &le;

    struct _entry **addr = bsearch(&le_addr, m_logs, m_num, sizeof(void*), _entry_compare);

    if (addr) return *addr;
    else return NULL;
}

static void _leave()
{
    if (!m_logs) return;

    for (int i = 0; i < m_num; i++) {
        struct _entry *e = m_logs[i];

        memset(e->filename, 0x0, sizeof(e->filename));
        memset(e->modulename, 0x0, sizeof(e->modulename));
        if (e->fp) fclose(e->fp);
        e->fp = NULL;
        mos_free(e);
    }

    mos_free(m_logs);
    m_logs = NULL;
}

static inline void _shift_file(struct _entry *e)
{
    struct stat fs;
    char ofn[PATH_MAX], nfn[PATH_MAX];
    int i;

    if (e->filename[0] == '-' && e->filename[1] == '\0') return;
    if (stat(e->filename, &fs) == -1 || fs.st_size < MTC_MAX_FILE_SIZE) return;

    if (e->fp) fclose(e->fp);

    for (i = MTC_MAX_FILE_NUM - 1; i > 1; i--) {
        sprintf(ofn, "%s.%d", e->filename, i - 1);
        sprintf(nfn, "%s.%d", e->filename, i);
        rename(ofn, nfn);
    }

    if (MTC_MAX_FILE_NUM > 1) {
        strcpy(ofn, e->filename);
        sprintf(nfn, "%s.1", e->filename);
        rename(ofn, nfn);
    }

    e->fp = fopen(e->filename, "a+");
    setvbuf(e->fp, e->linebuf, _IOLBF, 2096);
}

MERR* mtc_mt_init(const char *fn, const char *modulename, MTC_LEVEL level)
{
    if (!fn || !modulename || level > MTC_MAX) return merr_raise(MERR_ASSERT, "paramter null");

    if (!m_logs) atexit(_leave);

    uint32_t tid = _get_tid();

    if (_entry_search(tid) != NULL) return MERR_OK;

    pthread_mutex_lock(&m_lock);

    static int etime = 0;
    if (etime * 32 <= m_num) {
        size_t olen = etime * 32 * sizeof(struct _entry*);
        etime++;
        size_t nlen = etime * 32 * sizeof(struct _entry*);

        m_logs = mos_realloc(m_logs, nlen);
        memset(m_logs + olen, 0x0, nlen - olen);
    }

    m_logs[m_num] = mos_calloc(1, sizeof(struct _entry));
    struct _entry *e = m_logs[m_num];

    e->tid = tid;
    e->level = level;
    strncpy(e->filename, fn, sizeof(e->filename));
    strncpy(e->modulename, modulename, sizeof(e->modulename));
    if (!strcmp(e->filename, "-")) e->fp = stdout;
    else e->fp = fopen(e->filename, "a+");
    if (!e->fp) {
        pthread_mutex_unlock(&m_lock);
        return merr_raise(MERR_OPENFILE, "open trace file %s failure", e->filename);
    }
    setvbuf(e->fp, e->linebuf, _IOLBF, 2096);

    m_num++;

    _entry_sort();

    pthread_mutex_unlock(&m_lock);

    return MERR_OK;
}

MERR* mtc_mt_initf(const char *modulename, MTC_LEVEL level, const char *fmt, ...)
{
    char fname[PATH_MAX];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(fname, sizeof(fname), fmt, ap);
    va_end(ap);

    return mtc_mt_init(fname, modulename, level);
}

void mtc_mt_set_level(MTC_LEVEL level)
{
    if (level <= MTC_MAX) {
        uint32_t tid = _get_tid();
        struct _entry *e = _entry_search(tid);
        if (e) e->level = level;
    }
}

bool mtc_mt_msg(const char *func, const char *file, long line, MTC_LEVEL level,
                const char *fmt, ...)
{
    bool force = false;
    uint32_t tid = _get_tid();
    struct _entry *e = _entry_search(tid);

    if (!e && m_num > 0) {
        e = m_logs[0];
        force = true;
    }

    if (!e || e->level < level) return false;

    /*
     * 很多时候，只需要单纯输出 ' ', '\t', '\n' 之类，原样输出
     */
    if (*fmt && *(fmt+1) == '\0') {
        fprintf(e->fp, "%c", *fmt);
        return true;
    }

    va_list ap;
    struct timeval tv;
    char timestr[25] = {0};

    gettimeofday(&tv, NULL);

#if defined(TRACE_SHORT)
    snprintf(timestr, 24, "%04ld", tv.tv_sec % 10000);
    timestr[4] = '\0';
#else
    struct tm *tm;
    tm = localtime(&tv.tv_sec);
    strftime(timestr, 25, "%Y-%m-%d %H:%M:%S", tm);
    timestr[24] = '\0';
#endif

    fprintf(e->fp, "[%s.%06u]", timestr, (unsigned)tv.tv_usec);

    if (e->fp == stdout) {
        fprintf(e->fp, "[%s]", force ? "DEFAULT" : e->modulename);

        switch (level) {
        case MTC_WARNING:
            fprintf(e->fp, "%s", MCOLOR_YELLOW);
            break;
        case MTC_ERROR:
            fprintf(e->fp, "%s", MCOLOR_RED);
            break;
        case MTC_FOO:
            fprintf(e->fp, "%s", MCOLOR_BLUE);
            break;
        case MTC_DIE:
            fprintf(e->fp, "%s", MCOLOR_PURPLE);
            break;
        default:
            break;
        }
    }

    fprintf(e->fp, "[%s]", m_levels[level]);

    if (e->fp == stdout && level <= MTC_WARNING) fprintf(e->fp, MCOLOR_RESET);

#if defined(TRACE_SHORT)
    fprintf(e->fp, "[%s:%li] ", file, line, func);
#else
    fprintf(e->fp, "[%s:%li %s] ", file, line, func);
#endif

    va_start(ap, fmt);
    vfprintf(e->fp, fmt, ap);
    va_end(ap);

    fprintf(e->fp, "\n");

    _shift_file(e);

    return true;
}

FILE* mtc_mt_file()
{
    uint32_t tid = _get_tid();
    struct _entry *e = _entry_search(tid);

    if (e) return e->fp;
    else return 0;
}


#endif  /* MOS_LINUX || MOS_OSX */
