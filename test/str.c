#include "rheads.h"

void test_str()
{
    MSTR str;

    mstr_init(&str);

    MTEST_ASSERT(str.buf == NULL);

    mstr_append(&str, "string");
    MTEST_ASSERT_STR_EQ(str.buf, "string");

    mstr_appendn(&str, " appendn", 3);
    MTEST_ASSERT_STR_EQ(str.buf, "string ap");

    mstr_appendc(&str, 'x');
    MTEST_ASSERT_STR_EQ(str.buf, "string apx");

    mstr_appendf(&str, " I am %d years old", 33);
    MTEST_ASSERT_STR_EQ(str.buf, "string apx I am 33 years old");

    mstr_appendf(&str, ", and you?");
    MTEST_ASSERT_STR_EQ(str.buf, "string apx I am 33 years old, and you?");

    mstr_set(&str, "hello again");
    MTEST_ASSERT_STR_EQ(str.buf, "hello again");

    mstr_clear(&str);
}

void test_long_str()
{
    MSTR str;

    mstr_init(&str);
    mstr_set(&str, "hello again");

    char *longs = "very looooooooooooooooooooooooooooooooooooooooooooooooooooong";
    size_t len = strlen(longs);
    char longsa[len*1024 + 1];

    for (int i = 0; i < 1024; i++) {
        strcpy(longsa+i*len, longs);
    }
    longsa[len*1024] = '\0';

    mstr_appendf(&str, "%s", longsa);
    MTEST_ASSERT(str.len == len*1024 + strlen("hello again"));

    mstr_clear(&str);
    mstr_append(&str, longsa);
    MTEST_ASSERT(str.len == len*1024);

    mstr_clear(&str);
    for (int i = 0; i < 1024; i++) {
        mstr_appendf(&str, "%4d %s", i, longs);
    }
    MTEST_ASSERT(str.len == (5 + len) * 1024);

    //printf("%s\n", str.buf);

    mstr_clear(&str);
}

void suite_basic()
{
    mtest_add_test(test_str, "string");
    mtest_add_test(test_long_str, "long string");
}

int main()
{
    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
