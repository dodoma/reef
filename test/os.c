#include "rheads.h"

void test_memory()
{
    char *p, *s = "hello os memory";
    size_t len;

    len = strlen(s);

    p = mos_calloc(1, 1024);
    MTEST_ASSERT(p != NULL);
    MTEST_ASSERT(p[0] == '\0');
    MTEST_ASSERT(p[len] == '\0');

    strcpy(p, s);
    MTEST_ASSERT_STR_EQ(p, s);
    MTEST_ASSERT(p[len] == '\0');

    p = mos_realloc(p, 2048);
    MTEST_ASSERT(p != NULL);
    MTEST_ASSERT_STR_EQ(p, s);
    MTEST_ASSERT(p[len] == '\0');

    mos_free(p);
    MTEST_ASSERT(p == NULL);

    //mos_calloc(1, 102400000000);
}

void suite_basic()
{
    mtest_add_test(test_memory, "memory operation");
}

int main()
{
    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
