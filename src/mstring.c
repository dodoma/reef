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

char* mstr_strip(char *s, char n)
{
    int x;

    if (!s) return NULL;

    x = strlen(s) - 1;
    while (x>=0 && s[x]==n) s[x--] = '\0';

    while (*s && *s==n) s++;

    return s;
}

char* mstr_strip_space(char *s)
{
    int x;

    if (!s) return NULL;

    x = strlen(s) - 1;
    while (x>=0 && isspace(s[x])) s[x--] = '\0';

    while (*s && isspace(*s)) s++;

    return s;
}

char* mstr_ndup(const char *s, size_t n)
{
    if (!s || n == 0) return NULL;

    char *r = malloc(n + 1);
    if (!r) return NULL;

    for (int i = 0; i < n; i++) {
        r[i] = s[i];
    }
    r[n] = '\0';

    return r;
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

void mstr_appendvf(MSTR *astr, const char *fmt, va_list ap)
{
    char buf[1024];
    int len;
    va_list tmpap;

    if (!astr || !fmt) return;

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

void mstr_rand_hexstring(char *s, size_t len)
{
    size_t x;

    if (!s) return;

    for (x = 0; x < len; x++) {
        int v = mos_rand(16);
        if (v < 10) s[x] = (char)('0' + v);
        else s[x] = (char)('a' + v - 10);
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
        err = mlist_append(llist, mstr_ndup(p, q - p));
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

char* mstr_tolower(char *s)
{
    if (!s) return NULL;

    unsigned char *p = (unsigned char*)s;
    for ( ; *p; p++) *p = tolower(*p);

    return s;
}

char* mstr_toupper(char *s)
{
    if (!s) return NULL;

    unsigned char *p = (unsigned char*)s;
    for ( ; *p; ++p) *p = toupper(*p);

    return s;
}

void mstr_bin2str(uint8_t *hexin, unsigned int inlen, char *charout)
{
    /* 48 '0' */
    /* 97 'a'  122 'z'  65 'A' */
#define HEX2STR(in, out)                        \
    do {                                        \
        if (((in) & 0xf) < 10) {                \
            (out) = ((in)&0xf) + 48;            \
        } else {                                \
            (out) = ((in)&0xf) - 10 + 65;       \
        }                                       \
    } while (0)

    if (hexin == NULL || charout == NULL)
        return;

    unsigned int i, j;
    memset(charout, 0x0, inlen*2+1);

    for (i = 0, j = 0; i < inlen; i++) {
        if (hexin[i] == 9 || hexin[i] == 10 ||
            (hexin[i] > 31 && hexin[i] < 127)) {
            /*
             * resolve printable charactors
             * see man ascii
             */
            charout[j] = hexin[i];
            j++;
        } else {
            HEX2STR(hexin[i]>>4, charout[j]);
            HEX2STR(hexin[i], charout[j+1]);
            j += 2;
        }
    }

    charout[j] = '\0';
}

/*
 * use < 10 judgement, or, you can use array ['0', '1', ..., 'e', 'f']
 */
void mstr_bin2hexstr(uint8_t *hexin, unsigned int inlen, char *charout)
{
    /* 48 '0' */
    /* 97 'a'  122 'z'  65 'A' */
#define HEX2STR(in, out)                        \
    do {                                        \
        if (((in) & 0xf) < 10) {                \
            (out) = ((in)&0xf) + 48;            \
        } else {                                \
            (out) = ((in)&0xf) - 10 + 65;       \
        }                                       \
    } while (0)

    if (hexin == NULL || charout == NULL)
        return;

    unsigned int i, j;
    memset(charout, 0x0, inlen*2+1);

    for (i = 0, j = 0; i < inlen; i++, j += 2) {
        HEX2STR(hexin[i]>>4, charout[j]);
        HEX2STR(hexin[i], charout[j+1]);
    }

    charout[j] = '\0';
}

void mstr_hexstr2bin(const unsigned char *charin, unsigned int inlen, uint8_t *hexout)
{
#define STR2HEX(in1, in2, out)                          \
    do {                                                \
        if (in1 < ':')                                  \
            (out) = ((in1 - 48) & 0xf) << 4;            \
        else if (in1 < 'G')                             \
            (out) = ((in1 - 65 + 10) & 0xf) << 4;       \
        else                                            \
            (out) = ((in1 - 97 + 10) & 0xf) << 4;       \
        if (in2 < ':')                                  \
            (out) = (out) | ((in2 - 48) & 0xf);         \
        else if (in2 < 'G')                             \
            (out) = (out) | ((in2 - 65 + 10) & 0xf);    \
        else                                            \
            (out) = (out) | ((in2 - 97 + 10) & 0xf);    \
    } while (0)

    unsigned int i, j;
    for (i = 0, j = 0; i < inlen; i += 2, j++) {
        STR2HEX(charin[i], charin[i+1], hexout[j]);
    }
}

int mstr_version_compare(char *a, char *b)
{
    int x, y;

    if (!a) return -1;
    if (!b) return 1;

    char *p = a, *q = b;

    while (*p && *q) {
        x = atoi(p);
        y = atoi(q);

        if (x > y) return 1;
        else if (x < y) return -1;

        p = strchr(p, '.');
        if (p) p += 1;

        q = strchr(q, '.');
        if (q) q += 1;

        if (!p || !*p) {
            if (!q || !*q) return 0;
            else return -1;
        }

        if (!q || !*q) return 1;
    }

    return 0;
}
