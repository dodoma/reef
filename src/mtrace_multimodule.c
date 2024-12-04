#include "reef.h"

#define MTC_MAX_MODULE    100
#define MTC_DEFAULT_LEVEL 5
#define MTC_MAX_FILE_SIZE 524288000
#define MTC_MAX_FILE_NUM  5

struct _entry {
    char module[256];
    char filename[PATH_MAX-32];
    char linebuf[2096];
    FILE *fp;
    int level;
};

static struct _entry *m_logs[MTC_MAX_MODULE];
static int m_num = 0;
static pthread_mutex_t m_lock = PTHREAD_MUTEX_INITIALIZER;

static char *m_levels[MTC_MAX] = {
    "DIE", "MESSAGE", "ERROR", "WARNING", "INFO", "DEBUG", "NOISE"
};

static int _entry_compare(const void *a, const void *b)
{
    struct _entry *sa, *sb;

    sa = *(struct _entry**)a;
    sb = *(struct _entry**)b;

    return strcmp(sa->module, sb->module);
}

static void _entry_sort()
{
    if (m_num == 0) return;

    qsort(m_logs, m_num, sizeof(void*), _entry_compare);
}

static struct _entry* _entry_search(const char *module)
{
    if (m_num == 0 || !module) return NULL;

    struct _entry le;
    struct _entry *le_addr = &le;
    strncpy(le.module, module, sizeof(le.module));
    le.module[sizeof(le.module)-1] = 0;

    struct _entry **addr = bsearch(&le_addr, m_logs, m_num, sizeof(void*), _entry_compare);

    if (addr) return *addr;
    else return NULL;
}

static void _leave()
{
    if (m_num == 0) return;

    for (int i = 0; i < m_num; i++) {
        struct _entry *e = m_logs[i];

        memset(e->module, 0x0, sizeof(e->module));
        memset(e->filename, 0x0, sizeof(e->filename));
        if (e->fp) fclose(e->fp);
        e->fp = NULL;
        mos_free(e);
    }

    m_num = 0;
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

MERR* mtc_mm_init(const char *fn, const char *module, MTC_LEVEL level)
{
    if (!fn || !module || level > MTC_MAX)  return merr_raise(MERR_ASSERT, "parameter null");

    if (m_num == 0) atexit(_leave);

    if (m_num >= MTC_MAX_MODULE - 1) return merr_raise(MERR_ASSERT, "too many module");

    if (_entry_search(module) != NULL) return MERR_OK;

    pthread_mutex_lock(&m_lock);

    m_logs[m_num] = mos_calloc(1, sizeof(struct _entry));
    struct _entry *e = m_logs[m_num];
    m_num++;

    e->level = level;
    strncpy(e->module, module, sizeof(e->module));
    e->module[sizeof(e->module)-1] = 0;
    strncpy(e->filename, fn, sizeof(e->filename));
    e->filename[sizeof(e->filename)-1] = 0;
    if (!strcmp(e->filename, "-")) e->fp = stdout;
    else e->fp = fopen(e->filename, "a+");
    if (!e->fp) {
        pthread_mutex_unlock(&m_lock);
        return merr_raise(MERR_OPENFILE, "open trace file %s failure", e->filename);
    }
    setvbuf(e->fp, e->linebuf, _IOLBF, 2096);

    _entry_sort();

    pthread_mutex_unlock(&m_lock);

    return MERR_OK;
}

MERR* mtc_mm_initf(const char *module, MTC_LEVEL level, const char *fmt, ...)
{
    char fname[PATH_MAX];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(fname, sizeof(fname), fmt, ap);
    va_end(ap);

    return mtc_mm_init(fname, module, level);
}

void mtc_mm_set_level(const char *module, MTC_LEVEL level)
{
    if (level <= MTC_MAX) {
        struct _entry *e = _entry_search(module);
        if (e) e->level = level;
    }
}

bool mtc_mm_msg(const char *func, const char *file, long line, MTC_LEVEL level,
                const char *module, const char *fmt, ...)
{
    struct _entry *e = _entry_search(module);
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
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    strftime(timestr, 25, "%Y-%m-%d %H:%M:%S", &tm);
    timestr[24] = '\0';
#endif

    fprintf(e->fp, "[%s.%06u]", timestr, (unsigned)tv.tv_usec);

    //if (e->filename[0] == '-' && e->filename[1] == '\0') {
    fprintf(e->fp, "[%s]", e->module);
    //}

    if (e->fp == stdout) {
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

FILE* mtc_mm_file(const char *module)
{
    struct _entry *e = _entry_search(module);
    if (e) return e->fp;
    else return NULL;
}
