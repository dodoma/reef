#include "reef.h"
#include "_mdf.h"

void mdf_set_typef(MDF *node, MDF_TYPE type, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_set_type(node, key, type);
}


char* mdf_get_valuef(MDF *node, char *dftvalue, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_get_value(node, key, dftvalue);
}

int mdf_get_int_valuef(MDF *node, int dftvalue, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_get_int_value(node, key, dftvalue);
}

int64_t mdf_get_int64_valuef(MDF *node, int64_t dftvalue, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_get_int64_value(node, key, dftvalue);
}

float mdf_get_float_valuef(MDF *node, float dftvalue, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_get_float_value(node, key, dftvalue);
}

bool mdf_get_bool_valuef(MDF *node, bool dftvalue, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_get_bool_value(node, key, dftvalue);
}


MERR* mdf_set_valuevf(MDF *node, char *key, const char *fmt, va_list ap)
{
    char buf[1024], *val;
    int len;
    va_list tmpap;
    MERR *err;

    va_copy(tmpap, ap);
    len = vsnprintf(buf, 1024, fmt, tmpap);
    if (len >= 1024) {
        val = mos_malloc(len);
        vsprintf(buf, fmt, ap);
    } else val = buf;

    err = mdf_set_value(node, key, val);
    if (err) {
        if (len >= 1024) mos_free(val);
        return merr_pass(err);
    }

    if (len >= 1024) mos_free(val);

    return MERR_OK;
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

MERR* mdf_set_int_valuef(MDF *node, int val, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_set_int_value(node, key, val);
}

MERR* mdf_set_int64_valuef(MDF *node, int64_t val, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_set_int64_value(node, key, val);
}

MERR* mdf_set_float_valuef(MDF *node, float val, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_set_float_value(node, key, val);
}

MERR* mdf_set_bool_valuef(MDF *node, bool val, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_set_bool_value(node, key, val);
}


int mdf_add_int_valuef(MDF *node, int val, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_add_int_value(node, key, val);
}

int64_t mdf_add_int64_valuef(MDF *node, int64_t val, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_add_int64_value(node, key, val);
}

float mdf_add_float_valuef(MDF *node, float val, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_add_float_value(node, key, val);
}

char* mdf_append_string_valuef(MDF *node, char *str, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_append_string_value(node, key, str);
}

char* mdf_preppend_string_valuef(MDF *node, char *str, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mdf_preppend_string_value(node, key, str);
}

MERR* mdf_copyf(MDF *dst, MDF *src, bool overwrite, const char *fmt, ...)
{
    char key[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return merr_pass(mdf_copy(dst, key, src, overwrite));
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
