#include "reef.h"

void test_url_unescape()
{

    char s[100] = "%E4%BA%AE%E5%93%A5%E5%93%A5";

    MTEST_ASSERT_STR_EQ("亮哥哥", mhttp_url_unescape(s, strlen(s), '%'));
}

void suite_basic()
{
    mtest_add_test(test_url_unescape, "test url unescape");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
