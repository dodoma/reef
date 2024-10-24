#include "reef.h"

static inline uint32_t _get_tid()
{
#if defined(MOS_LINUX)
    return syscall(__NR_gettid);
#elif defined(MOS_OSX) || defined(MOS_ESP)
    uint64_t id;
    pthread_threadid_np(NULL, &id);
    return id;
#elif defined(ANDROID)
    return gettid();
#else
#error "unsupport platform"
#endif
}

void* mos_malloc(size_t size)
{
    if (size > 1000 * 1048576) {
        size++;                       /* for debug */
    }

    void *p = malloc(size);
    if (!p) {
        fprintf(stderr, "malloc %zd out of memory, exit!!!\n", size);
        exit(MERR_NOMEM);
    }

    return p;
}

void* mos_calloc(size_t nmemb, size_t size)
{
    if (size * nmemb > 1000 * 1048576) {
        size++;                       /* for debug */
    }

    void *p = calloc(nmemb, size);
    if (!p) {
        fprintf(stderr, "calloc %zd %zd out of memory, exit!!!\n", nmemb, size);
        exit(MERR_NOMEM);
    }

    return p;
}

void* mos_realloc(void *ptr, size_t size)
{
    if (size > 1000 * 1048576) {
        size++;                       /* for debug */
    }

    void *p = realloc(ptr, size);
    if (!p) {
        fprintf(stderr, "realloc %zd out of memory, exit!!!\n", size);
        exit(MERR_NOMEM);
    }

    return p;
}

uint32_t mos_rand(uint32_t max)
{
    static bool inited = false;

    if (!inited) {
        inited = true;
        uint32_t tid = _get_tid();
#if defined(MOS_ESP)
        srand(time(NULL) + tid);
#else
        srand48(time(NULL) + tid);
#endif
    }

#if defined(MOS_ESP)
    return rand() * max;
#else
    return drand48() * max;
#endif
}

bool _mkdir(char *path, mode_t mode)
{
    struct stat st;

    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return true;
        else return false;      /* not directory */
    } else {
        if (mkdir(path, mode) == 0) return true;
        else return false;
    }
}

bool mos_mkdir(char *path, mode_t mode)
{
    if (!path) return false;

    char *newpath = strdup(path), *p;

    for (p = newpath + 1; *p; p++) {
        if(*p == '/') {
            *p = '\0';
            if (!_mkdir(newpath, mode)) {
                free(newpath);
                return false;
            }
            *p = '/';
        }
    }

    free(newpath);

    return _mkdir(path, mode);
}

bool mos_mkdirf(mode_t mode, char *fmt, ...)
{
    char path[PATH_MAX];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(path, sizeof(path), fmt, ap);
    va_end(ap);

    return mos_mkdir(path, mode);
}

double mos_timef()
{
    struct timeval tv;
    double r;

    if (gettimeofday(&tv, NULL) == 0) r = tv.tv_sec + (tv.tv_usec / 1000000.0);
    else r = time(NULL);

    return r;
}
