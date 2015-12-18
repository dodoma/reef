#include "reef.h"

void* mos_malloc(size_t size)
{
    void *p = malloc(size);
    if (!p) {
        fprintf(stderr, "malloc %ld out of memory, exit!!!\n", size);
        exit(MERR_NOMEM);
    }

    return p;
}

void* mos_calloc(size_t nmemb, size_t size)
{
    void *p = calloc(nmemb, size);
    if (!p) {
        fprintf(stderr, "calloc %ld %ld out of memory, exit!!!\n", nmemb, size);
        exit(MERR_NOMEM);
    }

    return p;
}

void* mos_realloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if (!p) {
        fprintf(stderr, "realloc %ld out of memory, exit!!!\n", size);
        exit(MERR_NOMEM);
    }

    return p;
}

int mos_rand(int max)
{
    static bool inited = false;

    if (!inited) {
        srand48(time(NULL));
        inited = true;
    }

    return drand48() * max;
}

double mos_timef()
{
    struct timeval tv;
    double r;

    if (gettimeofday(&tv, NULL) == 0) r = tv.tv_sec + (tv.tv_usec / 1000000.0);
    else r = time(NULL);

    return r;
}
