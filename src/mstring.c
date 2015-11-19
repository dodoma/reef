#include "rheads.h"

static void _check_length(MSTR *str, size_t len)
{
    if (str->buf == NULL) {
        if (len * 10 > 256) str->max = len * 10;
        else str->max = 256;

        str->buf = mos_calloc(1, sizeof(char) * str->max);
    } else if (str->len + len >= str->max) {
        char *newbuf;
        size_t newmax;

        newmax = str->max;
        do {
            newmax *= 2;
        } while (str->len + len >= newmax);

        newbuf = mos_realloc(str->buf, sizeof(char) * newmax);

        str->buf = newbuf;
        str->max = newmax;
    }
}

void mstr_init(MSTR *str)
{
    if (!str) return;

    str->buf = NULL;
    str->len = 0;
    str->max = 0;
}

void mstr_append(MSTR *str, const char *buf)
{
    if (!str || !buf) return;

    size_t len = strlen(buf);

    _check_length(str, len);

    strcpy(str->buf + str->len, buf);
    str->len += len;
}

void mstr_appendc(MSTR *str, char c)
{
    if (!str) return;

    _check_length(str, 1);

    str->buf[str->len] = c;
    str->buf[str->len + 1] = '\0';
    str->len += 1;
}

void mstr_appendn(MSTR *str, const char *buf, size_t len)
{
    if (!str || !buf || len == 0) return;

    _check_length(str, len);

    memcpy(str->buf + str->len, buf, len);
    str->len += len;
    str->buf[str->len] = '\0';
}

void mstr_appendf(MSTR *str, const char *fmt, ...)
{
    char buf[1024];
    int len;
    va_list ap, tmpap;

    if (!str || !fmt) return;

    va_start(ap, fmt);
    va_copy(tmpap, ap);
    len = vsnprintf(buf, 1024, fmt, tmpap);

    if (len >= 0 && len < 1024) {
        mstr_appendn(str, buf, len);
    } else {
        /* from vsnprintf() manual:
         * a return value of size or more means that the output was truncated.
         */
        _check_length(str, len + 1);
        vsprintf(str->buf + str->len, fmt, ap);
        str->len += len;
        str->buf[str->len] = '\0';
    }

    va_end(ap);
}

void mstr_set(MSTR *str, const char *buf)
{
    if (!str) return;

    str->len = 0;
    mstr_append(str, buf);
}

void mstr_clear(MSTR *str)
{
    if (!str) return;

    mos_free(str->buf);
    mstr_init(str);
}

void mstr_rand_string(char *s, size_t maxlen)
{
    size_t len, x;

    if (!s) return;

    len = (size_t)mos_rand(maxlen);
    for (x = 0; x < len; x++) {
        s[x] = (char)('a' + mos_rand(26));
    }
    s[x] = '\0';
}

void mstr_rand_string_fixlen(char *s, size_t len)
{
    size_t x;

    if (!s) return;

    for (x = 0; x < len; x++) {
        s[x] = (char)('a' + mos_rand(26));
    }
    s[x] = '\0';
}

MERR* mstr_array_split(MLIST **alist, char *str, const char *sep, int max)
{
    size_t lenstr, lensep;
    char *p, *q;
    MLIST *llist;
    MERR *err;

    MERR_NOT_NULLC(alist, str, sep);

    if (sep[0] == '\0') return merr_raise(MERR_ASSERT, "seperator empty");

    lenstr = strlen(str);
    lensep = strlen(sep);

    err = mlist_init(alist, free);
    if (err) return merr_pass(err);

    llist = *alist;

    p = q = str;
    q = lensep > 1 ? strstr(p, sep) : strchr(p, sep[0]);
    while (p && q && mlist_length(llist) < max) {
        err = mlist_append(llist, strndup(p, q - p));
        JUMP_NOK(err, error);

        p = q + lenstr;
        if (p) q = lensep > 1 ? strstr(p, sep) : strchr(p, sep[0]);
    }

    if (*p) {
        err = mlist_append(llist, strdup(p));
        JUMP_NOK(err, error);
    }

    return MERR_OK;

error:
    mlist_destroy(alist);
    return merr_pass(err);
}
