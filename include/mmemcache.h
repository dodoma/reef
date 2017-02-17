#ifndef __MMEMCACHE_H__
#define __MMEMCACHE_H__

/*
 * mmemcache, mem cache client wrapper(使用 http://libmemcached.org/libMemcached.html)
 */
__BEGIN_DECLS

#ifdef USE_MEMCACHE

#include <libmemcached/memcached.h>

MERR* mmc_init(const char *ip, int port, memcached_st **m);

/*
 * vlen: a size_t pointer which will be filled with size of of the object
 * flags: a uint32_t pointer to contain whatever  flags  you  stored  with  the value
 * return: Any object returned by mmc_get() must be released by the caller application
 */
char* mmc_get(memcached_st *m, const char *key, size_t *vlen, uint32_t *flags);

MERR* mmc_store(memcached_st *m, MMC_OP op, const char *key, char *value, size_t vlen, time_t exp, uint32_t flags);
MERR* mmc_store_int(memcached_st *m, const char *key, int value, time_t exp, uint32_t flags);

MERR* mmc_delete(memcached_st *m, const char *key);

#endif

__END_DECLS
#endif
