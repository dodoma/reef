#include "reef.h"
#include "_mdf.h"

MERR* mdf_set_valuef(MDF *node, const char *fmt, ...)
{
    char buf[1024], *key, *val;
    int len;
    va_list ap, tmpap;
    MERR *err;

    va_start(ap, fmt);
    va_copy(tmpap, ap);
    len = vsnprintf(buf, 1024, fmt, tmpap);
    if (len >= 1024) {
        key = mos_malloc(len);
        vsprintf(buf, fmt, ap);
    } else key = buf;
    va_end(ap);

    val = strchr(key, '=');
    if (!val) {
        if (len >= 1024) mos_free(key);
        return merr_raise(MERR_ASSERT, "No equals found: %s", key);
    }

    *val++ = '\0';

    err = mdf_set_value(node, key, val);
    if (err) {
        if (len >= 1024) mos_free(key);
        return merr_pass(err);
    }

    if (len >= 1024) mos_free(key);

    return MERR_OK;
}

MERR* mdf_copyf(MDF *dst, MDF *src, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return merr_pass(mdf_copy(dst, key, src));
}

bool mdf_path_existf(MDF *node, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_path_exist(node, key);
}
