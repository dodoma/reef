#include "reef.h"

void test_basic()
{
    MERR *err;

    MRE *reo = mre_init();
    err = mre_compile(reo, "ab|cc|newman|foo|bar|jia|yi|bing");
    TRACE_NOK(err);

    mtc_dbg("codes");
    mre_dump(reo);

    MTEST_ASSERT(mre_match(reo, "a9eicnewmansdfo") == true);

    mre_destroy(&reo);
}

void suite_basic()
{
    mtest_add_test(test_basic, "basic");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
