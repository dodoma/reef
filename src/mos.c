#include "reef.h"

void* mos_calloc(size_t nmemb, size_t size)
{
    void *p = calloc(nmemb, size);
    if (!p) {
        fprintf(stderr, "calloc out of memory, exit!!!\n");
        exit(MERR_NOMEM);
    }

    return p;
}

void* mos_realloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if (!p) {
        fprintf(stderr, "realloc out of memory, exit!!!\n");
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
