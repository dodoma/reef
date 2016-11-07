#include "reef.h"

void* mhash_lookupf(MHASH *table, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mhash_lookup(table, key);
}


MERR* mhash_insertf(MHASH *table, void *value, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mhash_insert(table, key, value);
}

bool mhash_removef(MHASH *table, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mhash_remove(table, key);
}
