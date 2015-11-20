#include "reef.h"

void test_str()
{
    MSTR astr;

    mstr_init(&astr);

    MTEST_ASSERT(astr.buf == NULL);

    mstr_append(&astr, "string");
    MTEST_ASSERT_STR_EQ(astr.buf, "string");

    mstr_appendn(&astr, " appendn", 3);
    MTEST_ASSERT_STR_EQ(astr.buf, "string ap");

    mstr_appendc(&astr, 'x');
    MTEST_ASSERT_STR_EQ(astr.buf, "string apx");

    mstr_appendf(&astr, " I am %d years old", 33);
    MTEST_ASSERT_STR_EQ(astr.buf, "string apx I am 33 years old");

    mstr_appendf(&astr, ", and you?");
    MTEST_ASSERT_STR_EQ(astr.buf, "string apx I am 33 years old, and you?");

    mstr_set(&astr, "hello again");
    MTEST_ASSERT_STR_EQ(astr.buf, "hello again");

    mstr_clear(&astr);
}

void test_long_str()
{
    MSTR astr;

    mstr_init(&astr);
    mstr_set(&astr, "hello again");

    char *longs = "very looooooooooooooooooooooooooooooooooooooooooooooooooooong";
    size_t len = strlen(longs);
    char longsa[len*1024 + 1];

    for (int i = 0; i < 1024; i++) {
        strcpy(longsa+i*len, longs);
    }
    longsa[len*1024] = '\0';

    mstr_appendf(&astr, "%s", longsa);
    MTEST_ASSERT(astr.len == len*1024 + strlen("hello again"));

    mstr_clear(&astr);
    mstr_append(&astr, longsa);
    MTEST_ASSERT(astr.len == len*1024);

    mstr_clear(&astr);
    for (int i = 0; i < 1024; i++) {
        mstr_appendf(&astr, "%4d %s", i, longs);
    }
    MTEST_ASSERT(astr.len == (5 + len) * 1024);

    //printf("%s\n", astr.buf);

    mstr_clear(&astr);
}

void test_advance()
{
    char rands[101] = {0}, *s;
    MSTR astr;
    MLIST *alist;
    MERR *err;

    mstr_rand_string(rands, 100);
    MTEST_ASSERT(strlen(rands) < 100);

    mstr_rand_string_fixlen(rands, 100);
    MTEST_ASSERT(strlen(rands) == 100);

    s = "";
    err = mstr_array_split(&alist, s, " ", 100);
    MTEST_ASSERT(err == MERR_OK);
    mlist_destroy(&alist);

    s = "hello";
    err = mstr_array_split(&alist, s, " ", 100);
    MTEST_ASSERT(err == MERR_OK);
    mlist_destroy(&alist);

    s = " he   llo ";
    err = mstr_array_split(&alist, s, " ", 100);
    MTEST_ASSERT(err == MERR_OK);
    mlist_destroy(&alist);

    s = "xxxx hexxxxxllxxxxxoxx";
    err = mstr_array_split(&alist, s, "xx", 100);
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT(mlist_length(alist) == 3);
    mlist_destroy(&alist);

    s = "hello string array     split  ";
    err = mstr_array_split(&alist, s, " ", 100);
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT(mlist_length(alist) == 4);
    mlist_destroy(&alist);
}

void suite_advance()
{
    mtest_add_test(test_advance, "advance func");
}

void suite_basic()
{
    mtest_add_test(test_str, "string");
    mtest_add_test(test_long_str, "long string");
}

int main()
{
    mtest_add_suite(suite_basic, "basic");
    mtest_add_suite(suite_advance, "advance");

    return mtest_run();
}
