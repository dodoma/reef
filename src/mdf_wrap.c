#include "reef.h"
#include "_mdf.h"

char* mdf_get_valuef(MDF *node, char *dftvalue, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_get_value(node, key, dftvalue);
}

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

MERR* mdf_set_typef(MDF *node, MDF_TYPE type, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_set_type(node, key, type);
}

MDF* mdf_get_nodef(MDF *node, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_get_node(node, key);
}

MDF* mdf_get_or_create_nodef(MDF *node, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_get_or_create_node(node, key);
}

MERR* mdf_json_import_filef(MDF *node, const char *fmt, ...)
{
    char fname[PATH_MAX];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(fname, sizeof(fname), fmt, ap);
    va_end(ap);

    return mdf_json_import_file(node, fname);
}

MERR* mdf_json_export_filef(MDF *node, const char *fmt, ...)
{
    char fname[PATH_MAX];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(fname, sizeof(fname), fmt, ap);
    va_end(ap);

    return mdf_json_export_file(node, fname);
}
