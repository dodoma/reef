#include "rheads.h"

static pthread_mutex_t m_lock = PTHREAD_MUTEX_INITIALIZER;

static char m_filename[PATH_MAX] = {0};
static int  m_cur_level = MTC_DEFAULT_LEVEL;
static char *m_levels[MTC_MAX] = {"DIE", "MESSAGE", "ERROR",
                                  "WARNING", "INFO", "DEBUG",
                                  "NOISE"};

static char m_linebuf[2096];
static FILE *m_fp = NULL;

static void _leave()
{
    memset(m_filename, 0x0, sizeof(m_filename));

    if (m_fp) fclose(m_fp);
    m_fp = NULL;
}

static inline void _shift_file()
{
    struct stat fs;
    char ofn[PATH_MAX], nfn[PATH_MAX];
    int i;

    if (stat(m_filename, &fs) == -1 || fs.st_size < MTC_MAX_FILE_SIZE) return;

    if (m_fp) fclose(m_fp);

    for (i = MTC_MAX_FILE_NUM - 1; i > 1; i--) {
        sprintf(ofn, "%s.%d", m_filename, i - 1);
        sprintf(nfn, "%s.%d", m_filename, i);
        rename(ofn, nfn);
    }

    if (MTC_MAX_FILE_NUM > 1) {
        strcpy(ofn, m_filename);
        sprintf(nfn, "%s.1", m_filename);
        rename(ofn, nfn);
    }

    m_fp = fopen(m_filename, "a+");
    setvbuf(m_fp, m_linebuf, _IOLBF, 2096);
}

int mtc_init(const char *fn, MTC_LEVEL level)
{
    if (!fn) return MERR_ASSERT;

    if (level >= 0 && level < MTC_MAX) m_cur_level = level;

    strncpy(m_filename, fn, sizeof(m_filename));

    if (m_fp) fclose(m_fp);
    m_fp = fopen(m_filename, "a+");
    if (!m_fp) return MERR_OPENFILE;
    setvbuf(m_fp, m_linebuf, _IOLBF, 2096);

    atexit(_leave);

    return 0;
}

void mtc_set_level(MTC_LEVEL level)
{
    if (level >= 0 && level < MTC_MAX) m_cur_level = level;
}

bool mtc_msg(const char *func, const char *file, long line, MTC_LEVEL level,
             const char *fmt, ...)
{
    if (level > m_cur_level || !m_fp) return false;

    va_list ap;
    struct timeval tv;
    struct tm *tm;
    char timestr[25] = {0};

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    strftime(timestr, 25, "%Y-%m-%d %H:%M:%S", tm);
    timestr[24] = '\0';

    pthread_mutex_lock(&m_lock);

    fprintf(m_fp, "[%s.%06u]", timestr, (unsigned)tv.tv_usec);

    switch (level) {
    case MTC_WARNING:
        fprintf(m_fp, "%s", MCOLOR_YELLOW);
        break;
    case MTC_ERROR:
        fprintf(m_fp, "%s", MCOLOR_RED);
        break;
    case MTC_FOO:
        fprintf(m_fp, "%s", MCOLOR_BLUE);
        break;
    case MTC_DIE:
        fprintf(m_fp, "%s", MCOLOR_PURPLE);
        break;
    default:
        break;
    }
    fprintf(m_fp, "[%s] %s", m_levels[level], MCOLOR_RESET);

    fprintf(m_fp, "[%s:%li %s] ", file, line, func);

    va_start(ap, (void*)fmt);
    vfprintf(m_fp, fmt, ap);
    va_end(ap);

    fprintf(m_fp, "\n");

    _shift_file();

    pthread_mutex_unlock(&m_lock);

    return true;
}
