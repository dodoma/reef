#ifndef __MOS_H__
#define __MOS_H__

/*
 * mos, operation system wrapper
 */
__BEGIN_DECLS

#define mos_free(p)                             \
    do {                                        \
        if ((p) != NULL) {                      \
            free(p);                            \
            (p) = NULL;                         \
        }                                       \
    } while (0)

void* mos_calloc(size_t nmemb, size_t size);
void* mos_realloc(void *ptr, size_t size);


__END_DECLS
#endif
