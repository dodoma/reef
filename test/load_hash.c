#include "reef.h"

#define NODE_NUM 1000000

int main()
{
    MHASH *table;
    MERR *err;
    char str[101], *pstr[NODE_NUM];

    mtc_init("load.log", MTC_DEBUG);

    for (int i = 0; i < NODE_NUM; i++) {
        //mstr_rand_string(str, 100);
        //mstr_rand_string_fixlen(str, 100);
        snprintf(str, sizeof(str), "%d", i);

        pstr[i] = strdup(str);
    }

    err = mhash_init(&table, mhash_str_hash, mhash_str_comp, mhash_str_free);
    TRACE_NOK(err);

    mtimer_start();
    for (int i = 0; i < NODE_NUM; i++) {
        mhash_insert(table, pstr[i], NULL);
    }
    mtimer_stop("hash insert");

    mtimer_start();
    for (int i = 0; i < NODE_NUM; i++) {
        mhash_lookup(table, pstr[i]);
    }
    mtimer_stop("hash lookup");

    mhash_destroy(&table);

    return 0;
}
