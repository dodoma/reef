#include "reef.h"

static void _on_json(unsigned char *buf, size_t len, const char *type, void *p)
{
    MSTR *astr = (MSTR*)p;

    mstr_appendn(astr, (char*)buf, len);
}

static void _on_download(unsigned char *buf, size_t len, const char *type, void *p)
{
    FILE *fp = (FILE*)p;

    fwrite(buf, 1, len, fp);
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

    err = mhttp_get(url, NULL, xnode, _on_json, &astr);
    if (err != MERR_OK) RETURN(merr_pass(err));

    int code = mdf_get_int_value(xnode, "HEADER.code", 200);
    if (code != 200) RETURN(merr_raise(MERR_ASSERT, "response code %d", code));

    err = mdf_json_import_string(body, astr.buf);
    if (err != MERR_OK) RETURN(merr_pass(err));

    RETURN(MERR_OK);

#undef RETURN
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
