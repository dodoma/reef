#include "reef.h"

typedef struct {
    int id;
    char *name;
} newtype_t;

int strcompare(const void *a, const void *b)
{
    char *sa, *sb;

    sa = *(char **)a;
    sb = *(char **)b;

    return strcmp(sa, sb);
}

int newtypecompare(const void *a, const void *b)
{
    newtype_t *pa, *pb;

    pa = *(newtype_t**)a;
    pb = *(newtype_t**)b;

    //return pa->id - pb->id;
    return strcmp(pa->name, pb->name);
}

void newtypefree(void *p)
{
    newtype_t *x = p;

    mos_free(x->name);
    mos_free(x);
}

/*
 *  business code
 *  =============
 */

void test_basic()
{
    MLIST *alist;
    MERR *err;
    MSTR astr;

    mstr_init(&astr);

    mlist_init(&alist, NULL);

    /*
     * 有2种方式插入数字(建议使用2.方便移植)
     * 1. 64位机器上 使用 int64_t 因为 sizeof(int64_t) >= sizeof(void*)
     */
    //for (int64_t i = 0; i < 10000; i++) {
    //    mlist_append(alist, (void*)i);
    //}

    /*
     * 2. 使用 MOS_OFFSET_2_MEM()
     */
    for (int i = 0; i < 10000; i++) {
        mlist_append(alist, MOS_OFFSET_2_MEM(i));
    }

    int m = 1;
    int *p = &m;
    //int x;   // 无论哪种方式设置， 此时使用 x 而不是 y  会 coredump， 因为64位机器上 *(&x) 会越界给 p 赋值
    int *y;
    int *q = &m;

    for (int i = 9999; i >= 0; i--) {
        //mlist_pop(alist, (void**)&x);
        //MTEST_ASSERT(x == i);

        mlist_pop(alist, (void**)&y);
        MTEST_ASSERT((int)y == i);

        MTEST_ASSERT(*p == 1);
        MTEST_ASSERT(*q == 1);
    }

    mlist_append(alist, (void*)"aaa");
    mlist_append(alist, (void*)"bbb");
    mlist_append(alist, (void*)"ccc");

    MTEST_ASSERT(mlist_length(alist) == 3);

    char *s;
    mlist_pop(alist, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "ccc");

    mlist_append(alist, s);
    mlist_delete(alist, mlist_length(alist) - 1);

    mlist_pop(alist, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "bbb");
    /* aaa */

    err = mlist_insert(alist, 1, "xxx");
    //merr_traceback(err, &astr);
    //printf("%s\n", astr.buf);
    MTEST_ASSERT(err != MERR_OK);
    merr_destroy(&err);

    err = mlist_insert(alist, 0, "xxx");
    MTEST_ASSERT(err == MERR_OK);
    /* xxx, aaa */

    MTEST_ASSERT(mlist_length(alist) == 2);

    mlist_get(alist, 1, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "aaa");

    /* xxx, yyy */
    mlist_set(alist, 1, "yyy");
    mlist_get(alist, 1, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "yyy");

    /* yyy */
    mlist_cut(alist, 0, (void**)&s);
    mlist_get(alist, 0, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "yyy");

    // printf("============ %d \n", alist->num);
    // MLIST_ITERATE(alist, s) {
    //     printf("%s\n", s);
    // }

    err = mlist_get(alist, 1, (void**)&s);
    MTEST_ASSERT(err != MERR_OK);

    mlist_set(alist, 0, "xxx");
    mlist_append(alist, "bbb");
    mlist_append(alist, "yyy");
    /* xxx, bbb, yyy */

    mlist_get(alist, 1, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "bbb");

    /* yyy, bbb, xxx */
    mlist_reverse(alist);
    mlist_get(alist, 0, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "yyy");
    mlist_get(alist, 1, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "bbb");
    mlist_get(alist, 2, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "xxx");


    /* yyy, bbb, xxx, zzz */
    /* zzz, xxx, bbb, yyy */
    mlist_append(alist, "zzz");
    mlist_reverse(alist);
    MTEST_ASSERT(mlist_length(alist) == 4);

    mlist_get(alist, 0, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "zzz");
    mlist_get(alist, 1, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "xxx");
    mlist_get(alist, 2, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "bbb");
    mlist_get(alist, 3, (void**)&s);
    MTEST_ASSERT_STR_EQ(s, "yyy");

    mlist_destroy(&alist);
}

void test_strsearch()
{
    MLIST *alist;

    mlist_init(&alist, NULL);

    mlist_append(alist, (void*)"aaa");
    mlist_append(alist, (void*)"bbb");
    mlist_append(alist, (void*)"ccc");

    char *key = "fff";
    char key2[10] = "fff", *keyp2 = key2;

    char *x = "ddd";
    char *y = "eee";
    char *z = strdup(key);

    mlist_append(alist, x);
    mlist_append(alist, z);
    mlist_append(alist, y);

    /* aaa, bbb, ccc, ddd, fff, eee */

    char *s = *(char**)mlist_in(alist, &key, strcompare);
    char *s2 = *(char**)mlist_in(alist, &keyp2, strcompare);

    MTEST_ASSERT_STR_EQ(s, key);
    MTEST_ASSERT_STR_EQ(s2, keyp2);

    key = "aaa";
    MTEST_ASSERT(mlist_index(alist, &key, strcompare) == 0);
    key = "eee";
    MTEST_ASSERT(mlist_index(alist, &key, strcompare) == 5);
    key = "fff";
    MTEST_ASSERT(mlist_index(alist, &key, strcompare) == 4);
    key = "xxx";
    MTEST_ASSERT(mlist_index(alist, &key, strcompare) == -1);

    key = "fff";
    s = *(char **)mlist_search(alist, &key, strcompare);

    MTEST_ASSERT_STR_EQ(s, key);
    key = "fff";
    MTEST_ASSERT(mlist_index(alist, &key, strcompare) != 4);

    mlist_destroy(&alist);
}

void test_newtypesearch()
{
    newtype_t *x, *key;
    MLIST *alist;
    MERR *err;

    mlist_init(&alist, newtypefree);

    for (int i = 0; i < 10000; i++) {
        char str[100];

        mstr_rand_string(str, 100);

        x = mos_calloc(1, sizeof(newtype_t));
        x->id = i;
        x->name = strdup(str);

        mlist_append(alist, x);
    }

    key = mos_calloc(1, sizeof(newtype_t));
    key->id = 100;
    key->name = strdup("__unexist__");
    MTEST_ASSERT(mlist_in(alist, &key, newtypecompare) == NULL);
    newtypefree(key);

    err = mlist_get(alist, 100, (void**)&x);
    MTEST_ASSERT(err == MERR_OK);
    key = x;
    MTEST_ASSERT(mlist_in(alist, &key, newtypecompare) != NULL);

    x = mos_calloc(1, sizeof(newtype_t));
    x->id = key->id;
    x->name = strdup(key->name);
    mlist_append(alist, x);

    MTEST_ASSERT(mlist_in(alist, &key, newtypecompare) != NULL);

    x = *(newtype_t**)mlist_search(alist, &key, newtypecompare);
    MTEST_ASSERT(x != NULL);

    // printf("============ %d \n", alist->num);
    // MLIST_ITERATE(alist, x) {
    //     printf("%d %s\n", x->id, x->name);
    // }

    mlist_destroy(&alist);
}

void suite_basic()
{
    mtest_add_test(test_basic, "basic");
}

void suite_search()
{
    mtest_add_test(test_strsearch, "search");
    mtest_add_test(test_newtypesearch, "new struct type search");
}

int main()
{
    mtest_add_suite(suite_basic, "basic");
    mtest_add_suite(suite_search, "search");

    return mtest_run();
}
