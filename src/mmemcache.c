#include "reef.h"

#ifdef USE_MEMCACHE
#include <libmemcached/memcached.h>

MERR* mmc_init(const char *ip, int port, memcached_st **m)
{
    MERR *err;

    MERR_NOT_NULLB(ip, m);

    memcached_st *r = memcached_create(NULL);
    if (!r) return merr_raise(MERR_ASSERT, "memcached_create");

    //mtc_dbg("add memcached server %s %d", ip, port);

    memcached_return_t rc = memcached_server_add(r, ip, port);
    if (rc != MEMCACHED_SUCCESS) {
        memcached_free(r);
        return merr_raise(MERR_ASSERT, "%s %d %s", ip, port, memcached_strerror(r, rc));
    }

    err = mmc_store(r, MMC_OP_SET, "__reef__test", "xxx", 0, 100, 0);
    if (err) return merr_pass(err);

    char *tests = mmc_get(r, "__reef__test", NULL, NULL);
    if (!tests || strcmp(tests, "xxx")) {
        return merr_raise(MERR_ASSERT, "can't get from %s %d", ip, port);
    } else mos_free(tests);

    *m = r;

    return MERR_OK;
}

char* mmc_get(memcached_st *m, const char *key, size_t *vlen, uint32_t *flags)
{
    memcached_return_t rc;
    size_t dontcarea;
    uint32_t dontcareb;

    if (!m || !key) return NULL;

    if (!vlen) vlen = &dontcarea;
    if (!flags) flags = &dontcareb;

    char *value = memcached_get(m, key, strlen(key), vlen, flags, &rc);
    if (!value || *vlen < 1) {
        return NULL;
    }
    *(value+*vlen) = '\0';

    return value;
}

MERR* mmc_store(memcached_st *m, MMC_OP op, const char *key, char *value, size_t vlen, time_t exp, uint32_t flags)
{
    memcached_return_t rc;

    MERR_NOT_NULLC(m, key, value);

    if (vlen == 0) vlen = strlen(value);

    switch (op) {
    case MMC_OP_SET:
        rc = memcached_set(m, key, strlen(key), value, vlen, exp, flags);
        break;
    case MMC_OP_ADD:
        rc = memcached_add(m, key, strlen(key), value, vlen, exp, flags);
        break;
    case MMC_OP_REPLACE:
        rc = memcached_replace(m, key, strlen(key), value, vlen, exp, flags);
        break;
    case MMC_OP_APPEND:
        rc = memcached_append(m, key, strlen(key), value, vlen, exp, flags);
        break;
    case MMC_OP_PREPEND:
        rc = memcached_prepend(m, key, strlen(key), value, vlen, exp, flags);
        break;
    case MMC_OP_CAS:
        rc = memcached_cas(m, key, strlen(key), value, vlen, exp, flags, 0);
        break;
    default:
        return merr_raise(MERR_ASSERT, "op %d not support", op);
    }

    if (rc != MEMCACHED_SUCCESS) {
        return merr_raise(MERR_ASSERT, "store '%s=%s' %s", key, value, memcached_strerror(m, rc));
    }

    return MERR_OK;
}

MERR* mmc_store_int(memcached_st *m, const char *key, int value, time_t exp, uint32_t flags)
{
    char svalue[64];
    sprintf(svalue, "%d", value);

    return mmc_store(m, MMC_OP_SET, key, svalue, 0, exp, flags);
}

MERR* mmc_delete(memcached_st *m, const char *key)
{
    memcached_return_t rc;

    MERR_NOT_NULLB(m, key);

    rc = memcached_delete(m, key, strlen(key), 0);
    if (rc != MEMCACHED_SUCCESS) {
        return merr_raise(MERR_ASSERT, "delete %s %s", key, memcached_strerror(m, rc));
    }

    return MERR_OK;
}

#endif
