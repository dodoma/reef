#include "reef.h"

#define NODE_NUM 100000
#define KEY_LEN 100

void test_basic()
{
    MHASH *table;
    char s[KEY_LEN + 1], *ps[NODE_NUM];
    MERR *err;

    for (int i = 0; i < NODE_NUM; i++) {
        //mstr_rand_string(s, KEY_LEN);
        snprintf(s, sizeof(s), "str %d", i);
        ps[i] = strdup(s);
    }

    err = mhash_init(&table, mhash_str_hash, mhash_str_comp, mhash_str_free);
    MTEST_ASSERT(err == MERR_OK);

    for (int i = 0; i < NODE_NUM; i++) {
        err = mhash_insert(table, ps[i], ps[i]);
        MTEST_ASSERT(err == MERR_OK);
    }

    MTEST_ASSERT(mhash_length(table) == NODE_NUM);

    for (int i = 0; i < NODE_NUM; i++) {
        MTEST_ASSERT(mhash_has_key(table, ps[i]));
    }

    for (int i = 0; i < NODE_NUM; i++) {
        MTEST_ASSERT(mhash_lookup(table, ps[i]) != NULL);
    }

    for (int i = 0; i < NODE_NUM; i++) {
        MTEST_ASSERT(mhash_remove(table, ps[i]));
    }

    MTEST_ASSERT(mhash_length(table) == 0);

    mhash_destroy(&table);
}

void test_iterate()
{
    MHASH *table;
    char s[KEY_LEN + 1], *ps[100];
    MERR *err;

    for (int i = 0; i < 100; i++) {
        //mstr_rand_string(s, KEY_LEN);
        snprintf(s, sizeof(s), "str %d", i);
        ps[i] = strdup(s);
    }

    err = mhash_init(&table, mhash_str_hash, mhash_str_comp, mhash_str_free);
    MTEST_ASSERT(err == MERR_OK);

    for (int i = 0; i < 100; i++) {
        err = mhash_insert(table, ps[i], ps[i]);
        MTEST_ASSERT(err == MERR_OK);
    }

    int x = 0;
    char *key, *val;
    MHASH_ITERATE(table, key, val) {
        //printf("key %s %s\n", key, ps[x]);
        MTEST_ASSERT_STR_EQ(key, val);
        x++;
    }

    mhash_destroy(&table);
}

void suite_basic()
{
    mtest_add_test(test_basic, "basic");
    mtest_add_test(test_iterate, "hash iterate");
}

int main()
{
    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
