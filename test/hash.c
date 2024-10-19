#include "reef.h"

#define NODE_NUM 100000
#define KEY_LEN 100

void test_basic()
{
    MHASH *table;
    char s[KEY_LEN + 1], *ps[NODE_NUM], *data;
    MERR *err;

    for (int i = 0; i < NODE_NUM; i++) {
        //mstr_rand_string(s, KEY_LEN);
        snprintf(s, sizeof(s), "str %d", i);
        ps[i] = strdup(s);
    }

    err = mhash_init(&table, mhash_str_hash, mhash_str_comp, NULL);
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
        data = mhash_lookup(table, ps[i]);
        MTEST_ASSERT(data == ps[i]);
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
        ps[i] = strdup(s);      /* TODO memory leak */
    }

    err = mhash_init(&table, mhash_str_hash, mhash_str_comp, mhash_str_free);
    MTEST_ASSERT(err == MERR_OK);

    for (int i = 0; i < 100; i++) {
        err = mhash_insert(table, ps[i], strdup(ps[i]));
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

void test_iterate_break()
{
    MHASH *table;
    char s[KEY_LEN + 1], *ps[100];
    MERR *err;

    for (int i = 0; i < 100; i++) {
        //mstr_rand_string(s, KEY_LEN);
        snprintf(s, sizeof(s), "str %d", i);
        ps[i] = strdup(s);      /* TODO memory leak */
    }

    err = mhash_init(&table, mhash_str_hash, mhash_str_comp, mhash_str_free);
    MTEST_ASSERT(err == MERR_OK);

    for (int i = 0; i < 100; i++) {
        err = mhash_insert(table, ps[i], strdup(ps[i]));
        MTEST_ASSERT(err == MERR_OK);
    }

    char *key, *val;
    key = NULL;
    /* 删除后退出循环 */
    MHASH_ITERATE(table, key, val) {
        if (!strcmp(key, "str 13")) {
            mhash_remove(table, key);
            break;
        }
    }

    /* 删除后继续循环 */
    char *pkey = NULL;
    MHASH_ITERATE(table, key, val) {
        if (!strcmp(key, "str 29") || !strcmp(key, "str 42")) {
            mhash_remove(table, key);
            key = pkey;
        }

        pkey = key;
    }

    MHASH_ITERATE(table, key, val) {
        //printf("key %s val %s\n", key, val);
        MTEST_ASSERT_STR_NEQ(key, "str 13");
        MTEST_ASSERT_STR_NEQ(key, "str 29");
        MTEST_ASSERT_STR_NEQ(key, "str 42");
    }

    mhash_destroy(&table);
}

void test_hash()
{
    unsigned char outa[16], outb[20];
    char sa[33], sb[41];

    mhash_md5_buf((unsigned char*)"reef", 4, outa);
    mhash_sha1_buf((unsigned char*)"reef", 4, outb);

    mstr_bin2hexstr(outa, 16, sa);
    mstr_bin2hexstr(outb, 20, sb);

    //printf("%s \n %s", sa, sb);

    MTEST_ASSERT_STR_EQ(sa, "94981B447947C1E6AF5D8BE1E262DD7E");
    MTEST_ASSERT_STR_EQ(sb, "CFB5E25BE6C3BD8074B0B1C6EFEDC131B107F903");
}

void test_file()
{
    char sa[33], sb[41];

    mhash_md5_file_s("./base64.c", sa);
    mhash_sha1_file_s("./base64.c", sb);

    printf("%s \n %s", sa, sb);

    //MTEST_ASSERT_STR_EQ(sa, "94981B447947C1E6AF5D8BE1E262DD7E");
    //MTEST_ASSERT_STR_EQ(sb, "CFB5E25BE6C3BD8074B0B1C6EFEDC131B107F903");
}

void suite_basic()
{
    mtest_add_test(test_basic, "basic");
    mtest_add_test(test_iterate, "hash iterate");
    mtest_add_test(test_iterate_break, "hash iterate with break node");
    mtest_add_test(test_hash, "other hash function");
    mtest_add_test(test_file, "file function");
}

int main()
{
    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
