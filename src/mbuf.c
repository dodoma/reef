#include "reef.h"

void mbuf_init(MBUF *abuf, size_t initial_capacity)
{
    if (abuf) {
        abuf->len = abuf->max = 0;
        abuf->buf = NULL;
        mbuf_resize(abuf, initial_capacity);
    }
}

void mbuf_clear(MBUF *abuf)
{
    if (abuf) {
        mos_free(abuf->buf);
        mbuf_init(abuf, 0);
    }
}

void mbuf_resize(MBUF *abuf, size_t new_size)
{
    if (!abuf) return;

    if (new_size > abuf->max || (new_size < abuf->max && new_size >= abuf->len)) {
        char *buf = mos_realloc(abuf->buf, new_size);
        if (buf != NULL) {
            abuf->buf = buf;
            abuf->max = new_size;
        }
    }
}

void mbuf_shrink(MBUF *abuf)
{
    mbuf_resize(abuf, abuf->len);
}

void mbuf_remove(MBUF *abuf, size_t len)
{
    if (!abuf) return;

    if (len > 0 && len <= abuf->len) {
        memmove(abuf->buf, abuf->buf + len, abuf->len - len);
        abuf->len -= len;
    }
}

size_t mbuf_insert(MBUF *abuf, size_t offset, const void *data, size_t len)
{
    if (!abuf) return 0;
    if (offset > abuf->len) return 0;

    size_t from = offset;
    size_t to = offset + len;
    size_t delta = abuf->len - offset;

    if (abuf->len + len <= abuf->max) {
        if (delta > 0) memmove(abuf->buf + to, abuf->buf + from, delta);
    } else {
        size_t new_size = (size_t)((abuf->len + len) * 2);
        abuf->buf = mos_realloc(abuf->buf, new_size);
        if (delta > 0) memmove(abuf->buf + to, abuf->buf + from, delta);
        abuf->max = new_size;
    }

    if (data) memcpy(abuf->buf + offset, data, len);
    abuf->len += len;

    return offset;
}

size_t mbuf_append(MBUF *abuf, const void *data, size_t len)
{
    return mbuf_insert(abuf, abuf->len, data, len);
}
