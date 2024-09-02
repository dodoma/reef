#include "reef.h"

void test_equal()
{
    int i = 10;

    MTEST_ASSERT(i >= 10);
}

void test_string_equal()
{
    char *s = "hello reef";

    MTEST_ASSERT_STR_EQ(s, "hello reef");
}

void test_expression()
{
    MTEST_ASSERT((100 + 100) == 2 * 100);
    MTEST_ASSERT(100 / 0 == 0);
}

void suite_basic()
{
    mtest_add_test(test_equal, "test integer equal");
    mtest_add_test(test_string_equal, "test string equal");
}

void suite_expression()
{
    mtest_add_test(test_expression, "test simple expression");
}

int main()
{
    mtest_add_suite(suite_basic, "basic");
    mtest_add_suite(suite_expression, "expression");

    return mtest_run();
}
