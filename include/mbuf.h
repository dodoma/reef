#ifndef __MBUF_H__
#define __MBUF_H__

/*
 * mbuf, Memory Buffers stolen from Cesanta，mstring 的二进制版，用来存放一些二进制数据
 *
 * Mbufs are mutable/growing memory buffers
 * Mbuf can append data to the end of a buffer or insert data into arbitrary
 * position in the middle of a buffer. The buffer grows automatically when
 * needed.
 */

void mbuf_init(MBUF *abuf, size_t initial_capacity);
void mbuf_clear(MBUF *abuf);

/* Removes `data_size` bytes from the beginning of the buffer. */
void mbuf_remove(MBUF *abuf, size_t data_size);
void mbuf_resize(MBUF *abuf, size_t new_size);
/* Shrinks an Mbuf by resizing its `size` to `len`. */
void mbuf_shrink(MBUF *abuf);

size_t mbuf_insert(MBUF *abuf, size_t offset, const void *data, size_t data_size);
size_t mbuf_append(MBUF *abuf, const void *data, size_t data_size);

#endif
