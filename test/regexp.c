#include "reef.h"

void test_basic()
{
    MRE *reo = mre_init();
    mre_compile(reo, "^abc$");
    MTEST_ASSERT(mre_match(reo, "abc") == true);

    mre_destroy(&reo);
}

void suite_basic()
{
    mtest_add_test(test_basic, "basic");
}

int main()
{
    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
