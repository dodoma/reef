#include "reef.h"

static void _check_length(MSTR *astr, size_t len)
{
    if (astr->buf == NULL) {
        if (len * 10 > 256) astr->max = len * 10;
        else astr->max = 256;

        astr->buf = mos_calloc(1, sizeof(char) * astr->max);
    } else if (astr->len + len >= astr->max) {
        char *newbuf;
        size_t newmax;

        newmax = astr->max;
        do {
            newmax *= 2;
        } while (astr->len + len >= newmax);

        newbuf = mos_realloc(astr->buf, sizeof(char) * newmax);

        astr->buf = newbuf;
        astr->max = newmax;
    }
}

void mstr_init(MSTR *astr)
{
    if (!astr) return;

    astr->buf = NULL;
    astr->len = 0;
    astr->max = 0;
}

void mstr_append(MSTR *astr, const char *buf)
{
    if (!astr || !buf) return;

    size_t len = strlen(buf);

    _check_length(astr, len);

    strcpy(astr->buf + astr->len, buf);
    astr->len += len;
}

void mstr_appendc(MSTR *astr, char c)
{
    if (!astr) return;

    _check_length(astr, 1);

    astr->buf[astr->len] = c;
    astr->buf[astr->len + 1] = '\0';
    astr->len += 1;
}

void mstr_appendn(MSTR *astr, const char *buf, size_t len)
{
    if (!astr || !buf || len == 0) return;

    _check_length(astr, len);

    memcpy(astr->buf + astr->len, buf, len);
    astr->len += len;
    astr->buf[astr->len] = '\0';
}

void mstr_appendf(MSTR *astr, const char *fmt, ...)
{
    char buf[1024];
    int len;
    va_list ap, tmpap;

    if (!astr || !fmt) return;

    va_start(ap, fmt);
    va_copy(tmpap, ap);
    len = vsnprintf(buf, 1024, fmt, tmpap);

    if (len >= 0 && len < 1024) {
        mstr_appendn(astr, buf, len);
    } else {
        /* from vsnprintf() manual:
         * a return value of size or more means that the output was truncated.
         */
        _check_length(astr, len + 1);
        vsprintf(astr->buf + astr->len, fmt, ap);
        astr->len += len;
        astr->buf[astr->len] = '\0';
    }

    va_end(ap);
}

void mstr_set(MSTR *astr, const char *buf)
{
    if (!astr) return;

    astr->len = 0;
    mstr_append(astr, buf);
}

void mstr_clear(MSTR *astr)
{
    if (!astr) return;

    mos_free(astr->buf);
    mstr_init(astr);
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

MERR* mstr_array_split(MLIST **alist, const char *sin, const char *sep, int max)
{
    size_t lensep;
    char *p, *q;
    MLIST *llist;
    MERR *err;

    MERR_NOT_NULLC(alist, sin, sep);

    if (sep[0] == '\0') return merr_raise(MERR_ASSERT, "seperator empty");

    lensep = strlen(sep);

    err = mlist_init(alist, free);
    if (err) return merr_pass(err);

    llist = *alist;

    p = q = (char*)sin;

    q = lensep > 1 ? strstr(p, sep) : strchr(p, sep[0]);
    /* 处理冗余标识符 */
    while (p && p == q) {
        p += lensep;
        if (p) q = lensep > 1 ? strstr(p, sep) : strchr(p, sep[0]);
    }

    while (p && q && mlist_length(llist) < max) {
        err = mlist_append(llist, strndup(p, q - p));
        JUMP_NOK(err, error);

        p = q + lensep;
        if (p) {
            q = lensep > 1 ? strstr(p, sep) : strchr(p, sep[0]);
            while (p && p == q) {
                p += lensep;
                if (p) q = lensep > 1 ? strstr(p, sep) : strchr(p, sep[0]);
            }
        }
    }

    if (p && *p) {
        err = mlist_append(llist, strdup(p));
        JUMP_NOK(err, error);
    }

    return MERR_OK;

error:
    mlist_destroy(alist);
    return merr_pass(err);
}
