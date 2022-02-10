#include "reef.h"

static void _on_string(unsigned char *buf, size_t len, const char *type, void *p)
{
    MSTR *astr = (MSTR*)p;

    //mtc_dbg("%s", (char*)buf);
    if (astr) mstr_appendn(astr, (char*)buf, len);
}

static void _on_download(unsigned char *buf, size_t len, const char *type, void *p)
{
    FILE *fp = (FILE*)p;

    fwrite(buf, 1, len, fp);
}

void mhttp_getx(const char *url)
{
    if (!url) return;

    MDF *xnode;
    mdf_init(&xnode);

    MERR *err = mhttp_get(url, NULL, xnode, NULL, NULL);
    TRACE_NOK(err);

    mdf_destroy(&xnode);
}

void mhttp_getxf(const char *fmt, ...)
{
    char url[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(url, sizeof(url), fmt, ap);
    va_end(ap);

    return mhttp_getx(url);
}

MERR* mhttp_get_json(const char *url, MDF *body)
{
    MDF *xnode;
    MSTR astr;
    MERR *err;

    MERR_NOT_NULLB(url, body);

    mdf_init(&xnode);
    mstr_init(&astr);

#define RETURN(ret)                             \
    do {                                        \
        mdf_destroy(&xnode);                    \
        mstr_clear(&astr);                      \
        return (ret);                           \
    } while (0)

    err = mhttp_get(url, NULL, xnode, _on_string, &astr);
    if (err != MERR_OK) RETURN(merr_pass(err));

    int code = mdf_get_int_value(xnode, "HEADER.code", 200);
    if (code != 200) RETURN(merr_raise(MERR_ASSERT, "response code %d", code));

    err = mdf_json_import_string(body, astr.buf);
    if (err != MERR_OK) RETURN(merr_pass(err));

    RETURN(MERR_OK);

#undef RETURN
}

MERR* mhttp_get_jsonf(MDF *body, const char *fmt, ...)
{
    char url[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(url, sizeof(url), fmt, ap);
    va_end(ap);

    return mhttp_get_json(url, body);
}

MERR* mhttp_download(const char *url, const char *filename)
{
    MDF *xnode;
    MERR *err;

    MERR_NOT_NULLB(url, filename);

    FILE *fp = fopen(filename, "w");
    if (!fp) return merr_raise(MERR_ASSERT, "open %s failure", filename);

    mdf_init(&xnode);

#define RETURN(ret)                             \
    do {                                        \
        mdf_destroy(&xnode);                    \
        fclose(fp);                             \
        return (ret);                           \
    } while (0)

    err = mhttp_get(url, NULL, xnode, _on_download, fp);
    if (err != MERR_OK) RETURN(merr_pass(err));

    int code = mdf_get_int_value(xnode, "HEADER.code", 200);
    if (code != 200) RETURN(merr_raise(MERR_ASSERT, "response code %d", code));

    RETURN(MERR_OK);

#undef RETURN
}

MERR* mhttp_downloadf(const char *url, const char *fmt, ...)
{
    char key[PATH_MAX];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return mhttp_download(url, key);
}

MERR* mhttp_post_file(const char *url, const char *key, const char *filename, MSTR *astr)
{
    MERR *err;
    MERR_NOT_NULLC(url, key, filename);

    MDF *dnode, *xnode;
    mdf_init(&dnode);
    mdf_init(&xnode);
    mdf_set_valuef(dnode, "%s=@%s", key, filename);

#define RETURN(ret)                             \
    do {                                        \
        mdf_destroy(&xnode);                    \
        mdf_destroy(&dnode);                    \
        return (ret);                           \
    } while (0)

    err = mhttp_post_with_file(url, dnode, xnode, _on_string, astr);
    if (err != MERR_OK) RETURN(merr_pass(err));

    int code = mdf_get_int_value(xnode, "HEADER.code", 200);
    if (code != 200) RETURN(merr_raise(MERR_ASSERT, "response code %d", code));

    RETURN(MERR_OK);

#undef RETURN
}
